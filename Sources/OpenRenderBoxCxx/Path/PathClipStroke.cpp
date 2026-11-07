//
//  PathClipStroke.cpp
//  OpenRenderBox

#include "PathClipStroke.hpp"

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include "ClipStrokeKeyframes.hpp"
#include "PathGeometry.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace ORB::Path {
namespace {

using Geometry::Cubic;
using Geometry::curve_measure;
using Geometry::distance;
using Geometry::mix;
using ClipStroke::Keyframe;

CGPoint add(CGPoint a, CGPoint b) { return {a.x + b.x, a.y + b.y}; }
CGPoint subtract(CGPoint a, CGPoint b) { return {a.x - b.x, a.y - b.y}; }
CGPoint multiply(CGPoint a, double b) { return {a.x * b, a.y * b}; }
CGPoint scale_add(CGPoint a, CGPoint b, double c) { return {std::fma(b.x, c, a.x), std::fma(b.y, c, a.y)}; }
double dot(CGPoint a, CGPoint b) { return std::fma(a.x, b.x, a.y * b.y); }
CGPoint unit(CGPoint a) { return multiply(a, 1 / std::sqrt(a.x * a.x + a.y * a.y)); }
bool equal(CGPoint a, CGPoint b) { return a.x == b.x && a.y == b.y; }
bool finite(CGPoint p) { return std::isfinite(p.x) && std::isfinite(p.y); }
CGPoint apply_transform(CGPoint p, const CGAffineTransform *transform) {
    if (!transform) return p;
    const auto &t = *transform;
    return {std::fma(t.c, p.y, std::fma(t.a, p.x, t.tx)),
            std::fma(t.d, p.y, std::fma(t.b, p.x, t.ty))};
}

struct Segment {
    unsigned degree = 1;
    CGPoint points[4] = {};
    double length = 0;

    CGPoint end() const { return points[degree]; }

    Cubic cubic() const {
        if (degree == 3) return {{points[0], points[1], points[2], points[3]}};
        return {{points[0], mix(points[0], points[1], 2.0 / 3),
                 mix(points[2], points[1], 2.0 / 3), points[2]}};
    }

    double measure() const {
        if (degree == 1) return distance(points[0], points[1]);
        if (degree == 2) return Geometry::quadratic_measure({{points[0], points[1], points[2]}}, 0, false);
        return curve_measure(cubic(), 0, false);
    }

    double index(double value) const {
        if (degree == 1) return std::fmin(std::fmax(value / measure(), 0), 1);
        if (degree == 2) return Geometry::quadratic_measure({{points[0], points[1], points[2]}}, value, true);
        return curve_measure(cubic(), value, true);
    }

    void split(double t, Segment &left, Segment &right) const {
        left.degree = right.degree = degree;
        CGPoint work[4];
        std::copy_n(points, degree + 1, work);
        left.points[0] = work[0];
        right.points[degree] = work[degree];
        for (unsigned level = 1; level <= degree; ++level) {
            for (unsigned i = 0; i <= degree - level; ++i) work[i] = mix(work[i], work[i + 1], t);
            left.points[level] = work[0];
            right.points[degree - level] = work[degree - level];
        }
        left.length = left.measure();
        right.length = right.measure();
    }

    CGPoint direction(bool atEnd) const {
        if (atEnd) {
            for (unsigned i = degree; i > 0; --i) {
                CGPoint result = subtract(points[degree], points[i - 1]);
                if (dot(result, result) > 1e-8) return result;
            }
        } else {
            for (unsigned i = 1; i <= degree; ++i) {
                CGPoint result = subtract(points[i], points[0]);
                if (dot(result, result) > 1e-8) return result;
            }
        }
        return subtract(points[degree], points[0]);
    }

    void replay(CGMutablePathRef path, const CGAffineTransform *transform = nullptr) const {
        CGPoint p[4];
        for (unsigned i = 1; i <= degree; ++i) p[i] = apply_transform(points[i], transform);
        if (degree == 1) CGPathAddLineToPoint(path, nullptr, p[1].x, p[1].y);
        else if (degree == 2) CGPathAddQuadCurveToPoint(path, nullptr, p[1].x, p[1].y, p[2].x, p[2].y);
        else CGPathAddCurveToPoint(path, nullptr, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);
    }
};

struct Subpath {
    CGPoint start = CGPointZero;
    std::vector<Segment> segments;
    double length = 0;

    CGPoint end() const { return segments.empty() ? start : segments.back().end(); }

    void append_line(CGPoint point) {
        Segment segment{1, {end(), point}};
        segment.length = segment.measure();
        length += segment.length;
        segments.push_back(segment);
    }

    double append(Segment segment, double suppliedLength = INFINITY) {
        bool nonzero = false;
        for (unsigned i = 0; i < segment.degree; ++i) {
            CGPoint edge = subtract(segment.points[i + 1], segment.points[i]);
            if (dot(edge, edge) > 1e-8) nonzero = true;
        }
        if (!nonzero) return length;
        if (segments.empty()) start = segment.points[0];
        else if (!equal(end(), segment.points[0])) append_line(segment.points[0]);
        segment.length = std::isfinite(suppliedLength) ? suppliedLength : segment.measure();
        if (!std::isfinite(segment.length)) return length;
        double begin = length;
        length += segment.length;
        segments.push_back(segment);
        return begin;
    }

    void close() {
        if (!segments.empty() && !equal(end(), start)) append_line(start);
    }

    void replay(CGMutablePathRef path, bool close, const CGAffineTransform *transform = nullptr) const {
        if (segments.empty()) return;
        CGPoint point = apply_transform(start, transform);
        if (close) CGPathMoveToPoint(path, nullptr, point.x, point.y);
        else CGPathAddLineToPoint(path, nullptr, point.x, point.y);
        for (const auto &segment : segments) segment.replay(path, transform);
        if (close) CGPathCloseSubpath(path);
    }

    void append_range(const Subpath &source, double from, double to, ClipStroke::DistanceMap *map = nullptr) {
        double offset = 0;
        for (const auto &segment : source.segments) {
            double end = offset + segment.length;
            auto appendPiece = [&](double begin, double finish) {
                begin = std::fmin(std::fmax(begin, offset), end);
                finish = std::fmin(std::fmax(finish, offset), end);
                if (finish - begin < 0.0001) return;
                double t0 = begin - offset < 0.0001 ? 0 : segment.index(begin - offset);
                double t1 = finish - offset > segment.length - 0.0001 ? 1 : segment.index(finish - offset);
                double destination;
                double sourceLength = finish - begin;
                if (t0 <= 0.0001 && t1 >= 0.9999) {
                    destination = append(segment, segment.length);
                    sourceLength = segment.length;
                } else {
                    Segment piece = segment, left, right;
                    bool splitEnd = t1 < 0.9999;
                    if (t0 > 0.0001) {
                        piece.split(t0, left, right);
                        piece = right;
                        if (splitEnd) t1 = (t1 - t0) / (1 - t0);
                    }
                    if (splitEnd) {
                        piece.split(t1, left, right);
                        piece = left;
                    }
                    destination = append(piece);
                }
                if (map) map->append(begin, destination, sourceLength);
            };
            if (to <= from) {
                appendPiece(offset, to);
                appendPiece(from, end);
            } else appendPiece(from, to);
            offset = end;
        }
    }

    void append_logical_range(const Subpath &source, double from, double to) {
        if (from > to) {
            append_range(source, from, source.length);
            append_range(source, 0, to);
        } else append_range(source, from, to);
    }
};

struct Clipper {
    CGMutablePathRef output;
    const ClipStrokeInfo &info;
    float progress;
    const Subpath &subpath;
    const Subpath *customCap;
    double rate;
    ClipStroke::Cursor cursor;
    double consumed = 0;
    float value;
    bool clipped;
    bool emitted = false;
    CGPoint lastPoint = CGPointZero;
    CGPoint lastDirection = CGPointZero;
    double lastSpan = 0;
    CGPoint firstPoint = {INFINITY, INFINITY};
    CGPoint firstDirection = CGPointZero;
    double firstSpan = 0;
    // A contour can emit its first segment before cap() records its start.
    float firstValue;

    Clipper(CGMutablePathRef output, const ClipStrokeInfo &info, float progress,
            const Subpath &subpath, const Subpath *cap,
            const std::vector<ClipStroke::SingleKeyframe> &keys, double rate)
        : output(output), info(info), progress(progress), subpath(subpath), customCap(cap), rate(rate),
          cursor(keys, subpath.length), value(cursor.initialValue()),
          clipped(ClipStroke::valueClipped(info.flags, progress, value)), firstValue(value) {}

    double span() const {
        if (cursor.next.tag & 1) return INFINITY;
        double amount = std::abs(cursor.next.distance - cursor.previous.distance);
        return std::fmin(amount, subpath.length - amount);
    }

    void emit(const Segment &segment) {
        bool nonzero = false;
        for (unsigned i = 0; i < segment.degree; ++i) {
            CGPoint edge = subtract(segment.points[i + 1], segment.points[i]);
            if (dot(edge, edge) > 1e-8) nonzero = true;
        }
        if (!nonzero) return;
        if (!emitted) {
            firstPoint = segment.points[0];
            firstDirection = multiply(segment.direction(false), -1);
            CGPathMoveToPoint(output, nullptr, firstPoint.x, firstPoint.y);
            emitted = true;
        }
        segment.replay(output);
        lastPoint = segment.end();
        lastDirection = segment.direction(true);
        lastSpan = span();
    }

    void line(CGPoint point) { CGPathAddLineToPoint(output, nullptr, point.x, point.y); }
    void cubic(CGPoint a, CGPoint b, CGPoint c) {
        CGPathAddCurveToPoint(output, nullptr, a.x, a.y, b.x, b.y, c.x, c.y);
    }

    void cap(CGPoint point, CGPoint direction, double nextSpan) {
        if (!emitted) {
            firstPoint = point;
            firstDirection = direction;
            firstSpan = nextSpan;
            firstValue = value;
            return;
        }
        if (info.flags & 8) {
            line(point);
            lastPoint = point;
            lastDirection = multiply(direction, -1);
            lastSpan = nextSpan;
            return;
        }
        CGPoint previousDirection = unit(lastDirection);
        CGPoint nextDirection = unit(direction);
        CGPoint axis = lastSpan > nextSpan ? previousDirection : nextDirection;
        double projection = dot(axis, subtract(point, lastPoint));
        CGPoint from = projection > 0 ? scale_add(lastPoint, axis, projection) : lastPoint;
        CGPoint to = projection > 0 ? point : scale_add(point, axis, -projection);
        if (!(info.flags & 2)) {
            float overshoot = 0;
            if (!(info.flags & 1) && value >= 1) overshoot = progress - value;
            else if ((info.flags & 1) && value <= 0) overshoot = value - progress;
            if (overshoot > 0) {
                CGPoint shift = add(previousDirection, nextDirection);
                double amount = rate * double(overshoot) / distance(CGPointZero, shift);
                from = scale_add(from, shift, amount);
                to = scale_add(to, shift, amount);
            }
        }
        if (!equal(from, lastPoint)) line(from);
        if (customCap && !customCap->segments.empty()) {
            CGPoint source = subtract(customCap->end(), customCap->start);
            CGPoint target = subtract(to, from);
            double sourceLength = std::sqrt(source.x * source.x + source.y * source.y);
            double targetLength = std::sqrt(target.x * target.x + target.y * target.y);
            source = {source.x / sourceLength, source.y / sourceLength};
            target = {target.x / targetLength, target.y / targetLength};
            double scale = targetLength / sourceLength;
            double a = (source.x * target.x + source.y * target.y) * scale;
            double b = (-source.y * target.x + source.x * target.y) * scale;
            CGAffineTransform transform = CGAffineTransformMake(a, b, -b, a, from.x, from.y);
            transform = CGAffineTransformTranslate(transform, -customCap->start.x, -customCap->start.y);
            customCap->replay(output, false, &transform);
        } else {
            CGPoint gap = subtract(to, from);
            CGPoint normal = {-axis.y, axis.x};
            if (dot(CGPoint{-gap.y, gap.x}, axis) < 0) normal = multiply(normal, -1);
            double radius = distance(from, to) * 0.5;
            CGPoint lateral = multiply(normal, radius);
            CGPoint forward = multiply(axis, radius);
            CGPoint center = scale_add(from, normal, -radius);
            switch (info.cap) {
                case 1: {
                    constexpr double k = 0.5522847498307936;
                    CGPoint middle = add(center, forward);
                    cubic(scale_add(from, forward, k), scale_add(middle, lateral, k), middle);
                    CGPoint finish = subtract(center, lateral);
                    cubic(scale_add(middle, lateral, -k), scale_add(finish, forward, k), finish);
                    break;
                }
                case 2:
                    line(add(from, forward));
                    line(add(subtract(center, lateral), forward));
                    line(subtract(center, lateral));
                    break;
                case 3: case 5:
                    line(add(center, forward));
                    line(subtract(center, lateral));
                    break;
                case 4: case 6:
                    line(add(from, forward));
                    line(center);
                    line(add(subtract(center, lateral), forward));
                    line(subtract(center, lateral));
                    break;
                default:
                    line(to);
            }
        }
        if (!equal(to, point)) line(point);
        lastPoint = point;
        lastDirection = multiply(direction, -1);
        lastSpan = nextSpan;
    }

    void clip(const Segment &segment, double length, float nextValue) {
        bool nextClipped = ClipStroke::valueClipped(info.flags, progress, nextValue);
        if (clipped == nextClipped) {
            if (!clipped) emit(segment);
        } else {
            double fraction = 0;
            if (value != nextValue) fraction = double((std::fmin(std::fmax(progress, 0.0f), 1.0f) - value) / (nextValue - value));
            double t = segment.index(fraction * length);
            if (t > 0.9999) {
                if (!clipped) emit(segment);
            } else {
                Segment left, right;
                segment.split(t, left, right);
                if (clipped) {
                    cap(right.points[0], multiply(right.direction(false), -1), span());
                    emit(right);
                } else if (t > 0.0001) emit(left);
                clipped = nextClipped;
            }
        }
        value = nextValue;
    }

    bool undershoot() {
        float minimum = std::numeric_limits<float>::infinity();
        double from = 0, to = 0;
        for (const auto &key : cursor.keys) {
            if (key.value < minimum) {
                minimum = key.value;
                from = key.distance;
                to = INFINITY;
            } else if (key.value == minimum) {
                to = key.distance;
            }
        }
        for (const auto &key : cursor.keys) {
            if (std::isfinite(key.value) && key.value != minimum &&
                key.distance >= from && key.distance < to) {
                std::swap(from, to);
                break;
            }
        }

        Subpath selected;
        selected.append_logical_range(subpath, from, to);
        if (selected.segments.empty()) return false;

        CGPoint firstTangent = CGPointZero;
        CGPoint lastTangent = CGPointZero;
        for (const auto &segment : selected.segments) {
            bool first = equal(firstTangent, CGPointZero);
            CGPoint tangent;
            if (segment.degree == 2) {
                CGPoint edge = subtract(segment.points[1], segment.points[0]);
                CGPoint linear = add(edge, edge);
                CGPoint quadratic = subtract(segment.points[2], add(segment.points[1], edge));
                tangent = scale_add(linear, quadratic, first ? 0 : 2);
            } else if (segment.degree == 3) {
                unsigned endpoint = first ? 0 : 3;
                unsigned adjacent = first ? 1 : 2;
                auto difference = [&](unsigned point) {
                    return first ? subtract(segment.points[point], segment.points[endpoint])
                                 : subtract(segment.points[endpoint], segment.points[point]);
                };
                tangent = difference(adjacent);
                if (tangent.x * tangent.x + tangent.y * tangent.y <= 1e-8) {
                    tangent = difference(first ? 2 : 1);
                    if (tangent.x * tangent.x + tangent.y * tangent.y <= 1e-8)
                        tangent = difference(first ? 3 : 0);
                }
            } else tangent = subtract(segment.points[1], segment.points[0]);
            if (first) firstTangent = tangent;
            else lastTangent = tangent;
        }
        // A single segment only sets the first tangent in the source consumer.
        if (equal(firstTangent, CGPointZero) || equal(lastTangent, CGPointZero)) return false;
        float amount = minimum - progress;
        if (info.flags & 1) amount = -amount;
        if (!(amount > 0)) return false;

        CGPoint axis = scale_add(multiply(unit(lastTangent), -1), firstTangent,
                                1 / distance(CGPointZero, firstTangent));
        double offset = (rate * double(amount)) / distance(CGPointZero, axis);
        CGPoint translation = multiply(axis, offset);
        CGPoint shiftedStart = scale_add(selected.start, axis, offset);
        CGPoint shiftedEnd = add(selected.end(), translation);
        CGPathMoveToPoint(output, nullptr, shiftedStart.x, shiftedStart.y);
        emitted = true;
        CGAffineTransform transform = CGAffineTransformMakeTranslation(translation.x, translation.y);
        selected.replay(output, false, &transform);
        lastPoint = shiftedEnd;
        lastDirection = lastTangent;
        lastSpan = 0;
        value = minimum;
        cap(shiftedStart, multiply(firstTangent, -1), 0);
        CGPathCloseSubpath(output);
        emitted = false;
        return true;
    }

    void run() {
        bool split = cursor.next.distance > cursor.previous.distance;
        if (split && (info.flags & 6) == 4 &&
            ((info.flags & 1) ? progress > 1 : progress < 0) && undershoot()) return;
        for (const auto &input : subpath.segments) {
            if (!split) {
                emit(input);
                continue;
            }
            Segment remaining = input;
            double begin = consumed, end = begin + input.length, length = input.length;
            while (cursor.next.distance < end) {
                double t = remaining.index(cursor.next.distance - begin);
                if (t > 0.0001) {
                    Segment left, right;
                    remaining.split(t, left, right);
                    clip(left, left.length, cursor.next.value);
                    length -= left.length;
                    remaining = right;
                    begin = cursor.next.distance;
                }
                double previous = cursor.next.distance;
                cursor.advance(previous);
                if (!(cursor.next.distance > previous)) break;
            }
            clip(remaining, length, cursor.valueAt(end));
            consumed = end;
        }
        if (emitted) {
            if (clipped || ClipStroke::valueClipped(info.flags, progress, firstValue)) {
                clipped = true;
                if (finite(firstPoint)) {
                    value = firstValue;
                    cap(firstPoint, firstDirection, firstSpan);
                }
            }
            CGPathCloseSubpath(output);
        }
    }
};

struct ClipInput {
    CGMutablePathRef output;
    const ClipStrokeInfo &info;
    float progress;
    Subpath subpath;
    size_t subpathIndex = 0;
    size_t keyIndex = 0;
    CGPoint current = CGPointZero;

    void run(const Subpath &path, const Subpath *cap, const std::vector<Keyframe> &inputKeys) {
        std::vector<ClipStroke::SingleKeyframe> keys;
        double rate = 0;
        if (ClipStroke::makeKeyframes(inputKeys.data(), inputKeys.size(), path.length, progress, keys, rate)) {
            Clipper(output, info, progress, path, cap, keys, rate).run();
        } else path.replay(output, true);
    }

    void process(std::vector<Keyframe> keys) {
        Subpath customCap;
        size_t selected = keys.size();
        float selectedValue = INFINITY;
        float sign = info.flags & 1 ? 1 : -1;
        for (size_t i = 0; i < keys.size(); ++i) {
            float value = keys[i].value * sign;
            if ((keys[i].tag & 2) && value < selectedValue) {
                selected = i;
                selectedValue = value;
            }
        }
        const Subpath *cap = nullptr;
        if (selected < keys.size() && std::isfinite(selectedValue)) {
            float first = std::fmin(keys[selected].start, keys[selected].end);
            float last = std::fmax(keys[selected].start, keys[selected].end);
            bool reverse = false;
            for (size_t i = 0; i < keys.size(); ++i) {
                if (i == selected) continue;
                for (float position : {keys[i].start, keys[i].end}) {
                    if (first <= position && position < last) reverse = true;
                }
            }
            if (reverse) std::swap(first, last);
            customCap.append_logical_range(subpath,
                ClipStroke::positionDistance(first, subpath.length),
                ClipStroke::positionDistance(last, subpath.length));
            cap = &customCap;
        }
        for (size_t i = 0; i < keys.size(); ++i) {
            ClipStroke::TaggedSplit split;
            if (!ClipStroke::selectTaggedSplit(keys, i, info.flags, progress, split)) continue;
            Subpath first, second;
            ClipStroke::DistanceMap firstMap, secondMap;
            first.append_range(subpath,
                ClipStroke::positionDistance(split.firstStart, subpath.length),
                ClipStroke::positionDistance(split.firstEnd, subpath.length), &firstMap);
            first.close();
            auto firstKeys = ClipStroke::remapKeyframes(keys, i, subpath.length, first.length, firstMap);
            run(first, cap, firstKeys);
            second.append_range(subpath,
                ClipStroke::positionDistance(split.secondStart, subpath.length),
                ClipStroke::positionDistance(split.secondEnd, subpath.length), &secondMap);
            second.close();
            keys = ClipStroke::remapKeyframes(keys, i, subpath.length, second.length, secondMap);
            subpath = std::move(second);
            i = 0;
        }
        run(subpath, cap, keys);
    }

    void close() {
        if (!subpath.segments.empty()) {
            subpath.close();
            while (keyIndex < info.count && std::floor(info.keys[keyIndex].start) < float(subpathIndex)) ++keyIndex;
            size_t first = keyIndex;
            while (keyIndex < info.count && std::floor(info.keys[keyIndex].start) <= float(subpathIndex)) ++keyIndex;
            std::vector<Keyframe> keys;
            if (first < keyIndex) keys.assign(info.keys + first, info.keys + keyIndex);
            process(std::move(keys));
            subpath = {};
        }
        ++subpathIndex;
    }

    void append(const CGPathElement &element) {
        if (element.type == kCGPathElementMoveToPoint) {
            if (!subpath.segments.empty()) close();
            current = subpath.start = element.points[0];
        } else if (element.type == kCGPathElementCloseSubpath) {
            current = subpath.start;
            close();
        } else {
            unsigned degree = unsigned(element.type);
            Segment segment;
            segment.degree = degree;
            segment.points[0] = current;
            std::copy_n(element.points, degree, segment.points + 1);
            current = segment.end();
            subpath.append(segment);
        }
    }
};

} // namespace

CGPathRef copy_clip_stroke_path(CGPathRef path, float progress, const ClipStrokeInfo &info) {
    progress = ClipStroke::normalizeProgress(info.flags, progress);
    if (progress == float(1 & ~info.flags)) return CGPathCreateCopy(path);
    CGMutablePathRef result = CGPathCreateMutable();
    ClipInput input{result, info, progress, {}};
    CGPathApply(path, &input, +[](void *context, const CGPathElement *element) {
        static_cast<ClipInput *>(context)->append(*element);
    });
    if (!input.subpath.segments.empty()) input.close();
    return result;
}

} // namespace ORB::Path

#endif
