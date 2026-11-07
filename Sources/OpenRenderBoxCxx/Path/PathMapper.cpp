//
//  PathMapper.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include "PathMapper.hpp"
#include "PathGeometry.hpp"
#include "RoundedRect.hpp"
#include "Storage.hpp"
#include <Util/assert.hpp>
#include <algorithm>
#include <cmath>
#include <memory>
#include <map>
#include <mutex>
#include <vector>

namespace ORB {
namespace Path {
namespace {

using namespace Geometry;

void add_relative_arc(CGMutablePathRef path, const CGAffineTransform *transform,
                      CGPoint center, CGFloat radius, CGFloat start, CGFloat delta) {
    CGFloat sine = std::sin(start), cosine = std::cos(start);
    CGPoint point{std::fma(radius, cosine, center.x), std::fma(radius, sine, center.y)};
    if (CGPathIsEmpty(path)) {
        CGPathMoveToPoint(path, transform, point.x, point.y);
    } else {
        CGPathAddLineToPoint(path, transform, point.x, point.y);
    }
    if (!std::isfinite(delta)) { return; }

    CGFloat sign = delta < 0 ? -1 : 1;
    CGFloat quarter = sign * M_PI_2;
    CGFloat tangent = sign * 0.5522847498307932 * radius;
    unsigned remaining = 1023;
    auto curve = [&](CGFloat nextSine, CGFloat nextCosine, CGFloat factor) {
        CGPoint end{std::fma(radius, nextCosine, center.x), std::fma(radius, nextSine, center.y)};
        CGPathAddCurveToPoint(path, transform,
            std::fma(-sine, factor, point.x), std::fma(cosine, factor, point.y),
            std::fma(nextSine, factor, end.x), std::fma(-nextCosine, factor, end.y), end.x, end.y);
        point = end;
        sine = nextSine;
        cosine = nextCosine;
    };
    while (delta > M_PI_2 || delta < -M_PI_2) {
        if (remaining-- == 0) { return; }
        curve(cosine * sign, -sine * sign, tangent);
        start += quarter;
        delta -= quarter;
    }
    // Compact storage rounds angles to Float. Ignore the residual after exact quarter turns.
    if (std::abs(delta) <= static_cast<CGFloat>(1e-5f)) { return; }
    CGFloat half = delta * 0.5;
    CGFloat factor = std::fma(-std::cos(half), 4, 4) * radius / (3 * std::sin(half));
    curve(std::sin(start + delta), std::cos(start + delta), factor);
}

void add_arc(CGMutablePathRef path, const CGAffineTransform *transform, CGPoint center,
             CGFloat radius, CGFloat start, CGFloat end, bool clockwise) {
    constexpr CGFloat turn = 2 * M_PI;
    if (CGPathIsEmpty(path) && start == 0 &&
        (clockwise ? end >= turn && end - turn < 1e-5
                   : std::abs(end - turn) < static_cast<CGFloat>(1e-5f))) {
        constexpr CGFloat k = 0.5522847498;
        CGFloat r = std::abs(radius), y = clockwise ? -r : r;
        CGAffineTransform ellipse = CGAffineTransformMake(r, 0, 0, y, center.x, center.y);
        if (transform) { ellipse = CGAffineTransformConcat(ellipse, *transform); }
        CGPathMoveToPoint(path, &ellipse, 1, 0);
        CGPathAddCurveToPoint(path, &ellipse, 1, k, k, 1, 0, 1);
        CGPathAddCurveToPoint(path, &ellipse, -k, 1, -1, k, -1, 0);
        CGPathAddCurveToPoint(path, &ellipse, -1, -k, -k, -1, 0, -1);
        CGPathAddCurveToPoint(path, &ellipse, k, -1, 1, -k, 1, 0);
        CGPathCloseSubpath(path);
        return;
    }
    if (clockwise && start < end) {
        start = std::fma(std::ceil((end - start) / turn), turn, start);
    } else if (!clockwise && end < start) {
        end = std::fma(std::ceil((start - end) / turn), turn, end);
    }
    add_relative_arc(path, transform, center, radius, start, end - start);
}

struct Segment {
    CGPathElementType type;
    Cubic curve;
    CGFloat length;
};

struct TrimInput {
    std::vector<Segment> segments;
    CGPoint current = CGPointZero;
    CGPoint start = CGPointZero;
    CGFloat length = 0;

    void append(const CGPathElement &element) {
        Cubic curve{{current, {}, {}, {}}};
        CGFloat segmentLength = 0;
        switch (element.type) {
            case kCGPathElementMoveToPoint:
                current = start = element.points[0];
                curve.p[0] = current;
                break;
            case kCGPathElementAddLineToPoint:
                curve.p[3] = element.points[0];
                segmentLength = distance(current, curve.p[3]);
                current = curve.p[3];
                break;
            case kCGPathElementAddQuadCurveToPoint:
                curve.p[1] = {(current.x + 2 * element.points[0].x) / 3, (current.y + 2 * element.points[0].y) / 3};
                curve.p[2] = {(element.points[1].x + 2 * element.points[0].x) / 3, (element.points[1].y + 2 * element.points[0].y) / 3};
                curve.p[3] = element.points[1];
                segmentLength = curve_measure(curve, 0, false);
                current = curve.p[3];
                break;
            case kCGPathElementAddCurveToPoint:
                std::copy_n(element.points, 3, curve.p + 1);
                segmentLength = curve_measure(curve, 0, false);
                current = curve.p[3];
                break;
            case kCGPathElementCloseSubpath:
                curve.p[3] = start;
                segmentLength = distance(current, start);
                current = start;
                break;
        }
        length += segmentLength;
        segments.push_back({element.type, curve, segmentLength});
    }
};

CGPathRef copy_trimmed_path(CGPathRef path, float from, float to) {
    if (from <= 0 && to >= 1) {
        return CGPathCreateCopy(path);
    }
    CGMutablePathRef result = CGPathCreateMutable();
    if (!(from < to)) {
        return result;
    }
    TrimInput input;
    CGPathApply(path, &input, +[](void *context, const CGPathElement *element) {
        static_cast<TrimInput *>(context)->append(*element);
    });
    CGFloat fromDistance = input.length * from, toDistance = input.length * to;
    float consumed = 0;
    bool needsMove = true, includesStart = false;
    for (const auto &segment : input.segments) {
        if (segment.type == kCGPathElementMoveToPoint) {
            needsMove = true;
            includesStart = false;
            continue;
        }
        CGFloat begin = consumed, end = begin + segment.length;
        consumed = static_cast<float>(end);
        if (segment.length == 0) {
            if (segment.type == kCGPathElementCloseSubpath && includesStart && begin <= toDistance) {
                CGPathCloseSubpath(result);
            }
            continue;
        }
        CGFloat first = fromDistance >= begin ? std::fmax((fromDistance - begin) / segment.length, 0) : 0;
        CGFloat last = toDistance < end ? std::fmin((toDistance - begin) / segment.length, 1) : 1;
        if (!(first < last)) {
            continue;
        }
        if (needsMove) {
            includesStart = first == 0;
        }
        if (segment.type == kCGPathElementAddLineToPoint || segment.type == kCGPathElementCloseSubpath) {
            CGPoint start = mix(segment.curve.p[0], segment.curve.p[3], first);
            CGPoint finish = mix(segment.curve.p[0], segment.curve.p[3], last);
            if (needsMove) {
                CGPathMoveToPoint(result, nullptr, start.x, start.y);
                needsMove = false;
            }
            CGPathAddLineToPoint(result, nullptr, finish.x, finish.y);
            if (segment.type == kCGPathElementCloseSubpath && last == 1 && includesStart) {
                CGPathCloseSubpath(result);
            }
        } else {
            Cubic curve = segment.curve;
            if (first > 0 || last < 1) {
                if (first > 0.001) {
                    first = curve_measure(curve, first * segment.length, true);
                }
                if (last < 0.999) {
                    last = curve_measure(curve, last * segment.length, true);
                }
                Cubic left, right;
                if (first > 0.001) {
                    curve.split(first, left, right);
                    curve = right;
                    last = (last - first) / (1 - first);
                }
                if (last < 0.999) {
                    curve.split(last, left, right);
                    curve = left;
                }
            }
            if (needsMove) {
                CGPathMoveToPoint(result, nullptr, curve.p[0].x, curve.p[0].y);
                needsMove = false;
            }
            CGPathAddCurveToPoint(result, nullptr, curve.p[1].x, curve.p[1].y, curve.p[2].x, curve.p[2].y, curve.p[3].x, curve.p[3].y);
        }
    }
    return result;
}


struct Dilator {
    CGMutablePathRef path;
    CGPoint amount;
    CGPoint current = CGPointZero;
    CGPoint start = CGPointZero;
    bool needsMove = true;

    CGPoint begin(CGPoint end) {
        CGFloat x = end.y < current.y ? -amount.x : end.y > current.y ? amount.x : 0;
        CGFloat y = end.x < current.x ? amount.y : end.x > current.x ? -amount.y : 0;
        CGPoint offset{x, amount.y + y};
        if (needsMove) {
            CGPathMoveToPoint(path, nullptr, current.x + offset.x, current.y + offset.y);
            needsMove = false;
        } else {
            CGPathAddLineToPoint(path, nullptr, current.x + offset.x, current.y + offset.y);
        }
        return offset;
    }

    void line(CGPoint end) {
        CGPoint offset = begin(end);
        CGPathAddLineToPoint(path, nullptr, end.x + offset.x, end.y + offset.y);
        current = end;
    }

    void close() {
        if (!needsMove) {
            if (!CGPointEqualToPoint(current, start)) {
                line(start);
            }
            CGPathCloseSubpath(path);
            needsMove = true;
        }
    }

    void move(CGPoint point) {
        close();
        current = start = point;
    }

    void quad(CGPoint control, CGPoint end, unsigned axis = 0) {
        if (axis < 2) {
            CGFloat first = axis == 0 ? current.x : current.y;
            CGFloat middle = axis == 0 ? control.x : control.y;
            CGFloat last = axis == 0 ? end.x : end.y;
            // Preserve the source consumer's split rule, including its denominator.
            CGFloat t = (first - middle) / (first - last);
            if (first != last && t >= 0.0001 && t <= 0.9999) {
                CGPoint a = mix(current, control, t), b = mix(control, end, t);
                quad(a, mix(a, b, t), axis + 1);
                control = b;
            }
            quad(control, end, axis + 1);
            return;
        }
        CGPoint offset = begin(end);
        CGPathAddQuadCurveToPoint(path, nullptr, control.x + offset.x, control.y + offset.y, end.x + offset.x, end.y + offset.y);
        current = end;
    }

    void cubic(Cubic curve, unsigned axis = 0) {
        if (axis < 2) {
            auto coordinate = [axis](CGPoint p) { return axis == 0 ? p.x : p.y; };
            CGFloat p0 = coordinate(curve.p[0]), p1 = coordinate(curve.p[1]);
            CGFloat p2 = coordinate(curve.p[2]), p3 = coordinate(curve.p[3]);
            CGFloat a = std::fma(p1 - p2, 3, p3 - p0);
            CGFloat b = 2 * (p2 - (p1 + p1) + p0), c = p1 - p0;
            CGFloat discriminant = std::fma(b, b, a * (-4 * c));
            if (discriminant >= 0) {
                CGFloat sum = b + std::copysign(std::sqrt(discriminant), b);
                CGFloat q = -0.5 * sum;
                CGFloat first = a != 0 ? q / a : -1;
                CGFloat second = q != 0 ? (-2 * c) / sum : -1;
                if (second < first) {
                    std::swap(first, second);
                }
                if (first < 0.0001) {
                    first = second;
                    second = 2;
                }
                if (first >= 0.0001 && first <= 0.9999) {
                    Cubic left, right;
                    curve.split(first, left, right);
                    cubic(left, axis + 1);
                    curve = right;
                    if (1 - second >= 0.0001) {
                        curve.split((second - first) / (1 - first), left, right);
                        cubic(left, axis + 1);
                        curve = right;
                    }
                }
            }
            cubic(curve, axis + 1);
            return;
        }
        CGPoint offset = begin(curve.p[3]);
        CGPathAddCurveToPoint(path, nullptr, curve.p[1].x + offset.x, curve.p[1].y + offset.y,
                             curve.p[2].x + offset.x, curve.p[2].y + offset.y,
                             curve.p[3].x + offset.x, curve.p[3].y + offset.y);
        current = curve.p[3];
    }
};

struct DilationInput {
    struct Part {
        CGPathElementType type;
        CGPoint points[4];
    };
    struct Subpath {
        std::vector<Part> parts;
    };
    std::vector<Subpath> subpaths;
    CGPoint current = CGPointZero;
    CGPoint start = CGPointZero;
    CGFloat orientation = 0;

    void close_orientation() {
        orientation += current.x * start.y - current.y * start.x;
        current = start;
    }

    void append(const CGPathElement &element) {
        if (element.type == kCGPathElementMoveToPoint) {
            close_orientation();
            current = start = element.points[0];
            subpaths.emplace_back();
            return;
        }
        if (element.type == kCGPathElementCloseSubpath) {
            close_orientation();
            subpaths.emplace_back();
            return;
        }
        if (subpaths.empty()) {
            subpaths.emplace_back();
        }
        unsigned count = element.type == kCGPathElementAddCurveToPoint ? 3 : element.type == kCGPathElementAddQuadCurveToPoint ? 2 : 1;
        Part part{element.type, {current}};
        std::copy_n(element.points, count, part.points + 1);
        for (unsigned i = 0; i < count; ++i) {
            orientation += current.x * element.points[i].y - current.y * element.points[i].x;
            current = element.points[i];
        }
        subpaths.back().parts.push_back(part);
    }
};

CGPathRef copy_dilated_path(CGPathRef path, CGPoint amount) {
    DilationInput input;
    CGPathApply(path, &input, +[](void *context, const CGPathElement *element) {
        static_cast<DilationInput *>(context)->append(*element);
    });
    input.close_orientation();
    CGMutablePathRef result = CGPathCreateMutable();
    Dilator dilator{result, amount};
    bool reverse = input.orientation < 0;
    for (auto &subpath : input.subpaths) {
        if (subpath.parts.empty()) {
            continue;
        }
        if (reverse) {
            std::reverse(subpath.parts.begin(), subpath.parts.end());
            for (auto &part : subpath.parts) {
                unsigned count = part.type == kCGPathElementAddCurveToPoint ? 3 : part.type == kCGPathElementAddQuadCurveToPoint ? 2 : 1;
                std::reverse(part.points, part.points + count + 1);
            }
        }
        dilator.move(subpath.parts[0].points[0]);
        for (const auto &part : subpath.parts) {
            switch (part.type) {
                case kCGPathElementAddLineToPoint: dilator.line(part.points[1]); break;
                case kCGPathElementAddQuadCurveToPoint: dilator.quad(part.points[1], part.points[2]); break;
                case kCGPathElementAddCurveToPoint: dilator.cubic({{part.points[0], part.points[1], part.points[2], part.points[3]}}); break;
                default: break;
            }
        }
        dilator.close();
    }
    return result;
}

CGPoint add(CGPoint a, CGPoint b) { return {a.x + b.x, a.y + b.y}; }
CGPoint subtract(CGPoint a, CGPoint b) { return {a.x - b.x, a.y - b.y}; }
CGPoint scale(CGPoint a, CGFloat b) { return {a.x * b, a.y * b}; }
CGFloat dot(CGPoint a, CGPoint b) { return std::fma(a.y, b.y, a.x * b.x); }
CGFloat cross(CGPoint a, CGPoint b) { return std::fma(a.x, b.y, -a.y * b.x); }
CGPoint normalize(CGPoint a) {
    if (a.x == 0 && a.y == 0) { return a; }
    CGFloat length = std::sqrt(dot(a, a));
    return {a.x / length, a.y / length};
}
CGPoint multiply_add(CGPoint a, CGFloat b, CGPoint c) {
    return {std::fma(a.x, b, c.x), std::fma(a.y, b, c.y)};
}

struct OffsetCubic {
    Cubic curve;
    CGPoint linear, quadratic, cubic;

    explicit OffsetCubic(Cubic curve) : curve(curve) {
        CGPoint a = subtract(curve.p[1], curve.p[0]);
        CGPoint b = subtract(curve.p[2], curve.p[1]);
        CGPoint c = subtract(curve.p[3], curve.p[2]);
        linear = scale(a, 3);
        quadratic = scale(subtract(b, a), 3);
        cubic = add(subtract(c, scale(b, 2)), a);
    }

    CGPoint evaluate(CGFloat t) const {
        return multiply_add(multiply_add(multiply_add(cubic, t, quadratic), t, linear), t, curve.p[0]);
    }

    CGPoint derivative(CGFloat t) const {
        if (t == 0 || t == 1) {
            for (unsigned i = 1; i < 4; ++i) {
                CGPoint tangent = t == 0 ? subtract(curve.p[i], curve.p[0]) : subtract(curve.p[3], curve.p[3 - i]);
                if (dot(tangent, tangent) > 1e-8 || i == 3) { return tangent; }
            }
        }
        return multiply_add(multiply_add(cubic, 3 * t, scale(quadratic, 2)), t, linear);
    }

    bool near(CGPoint point, CGFloat t) const {
        auto squared_distance = [&](CGFloat parameter) {
            // Subtract first to match the polynomial evaluator's rounding.
            CGPoint v = multiply_add(multiply_add(multiply_add(cubic, parameter, quadratic), parameter, linear), parameter, subtract(curve.p[0], point));
            return dot(v, v);
        };
        constexpr CGFloat toleranceSquared = 0.1 * 0.1;
        CGFloat first = squared_distance(0), last = squared_distance(1), middle = squared_distance(t);
        if (first < toleranceSquared || last < toleranceSquared || middle < toleranceSquared) { return true; }
        if (!(middle < first && middle < last)) { return false; }
        constexpr CGFloat ratio = 0.6180339887498949;
        CGFloat left = 0, right = 1;
        CGFloat x = t > 0.5 ? t * ratio : t;
        CGFloat y = t > 0.5 ? t : std::fma(1 - ratio, 1 - t, t);
        CGFloat fx = x == t ? middle : squared_distance(x);
        CGFloat fy = y == t ? middle : squared_distance(y);
        if (fx < toleranceSquared || fy < toleranceSquared) { return true; }
        while (right - left > 3e-8 * (x + y)) {
            if (fy < fx) {
                left = x;
                x = std::fma(ratio, y - right, right);
                fx = squared_distance(x);
                if (fx <= toleranceSquared) { return true; }
            } else {
                right = y;
                y = std::fma(ratio, x - left, left);
                fy = squared_distance(y);
                if (fy <= toleranceSquared) { return true; }
            }
            std::swap(x, y);
            std::swap(fx, fy);
        }
        return false;
    }
};

struct Offsetter {
    CGMutablePathRef path;
    CGFloat amount;
    CGPoint start = CGPointZero;
    CGPoint current = CGPointZero;
    CGPoint firstDirection = CGPointZero;
    CGPoint lastDirection = CGPointZero;
    unsigned state = 0;

    CGPoint normal(CGPoint tangent) const {
        CGPoint unit = normalize(tangent);
        return {-unit.y * amount, unit.x * amount};
    }

    void emit_line(CGPoint p) { CGPathAddLineToPoint(path, nullptr, p.x, p.y); }
    void emit_cubic(Cubic c) {
        CGPathAddCurveToPoint(path, nullptr, c.p[1].x, c.p[1].y, c.p[2].x, c.p[2].y, c.p[3].x, c.p[3].y);
    }

    void join(CGPoint point, CGPoint direction) {
        if (dot(lastDirection, direction) > 0.99) { return; }
        CGPoint oldNormal{-lastDirection.y * amount, lastDirection.x * amount};
        CGPoint newNormal{-direction.y * amount, direction.x * amount};
        CGPoint delta = subtract(oldNormal, newNormal);
        CGPoint end = add(point, newNormal);
        if (dot(delta, delta) < 0.05) { emit_line(end); return; }
        CGPoint sum = add(lastDirection, direction);
        CGFloat length = std::sqrt(dot(sum, sum));
        CGFloat factor = -4.0 / 3.0;
        if (length >= 0.001) {
            CGFloat dx = lastDirection.x - direction.x, dy = lastDirection.y - direction.y;
            if (std::abs(dy) > std::abs(dx)) {
                factor = ((8.0 / 3.0) / length - 4.0 / 3.0) * sum.x / dy;
            } else {
                factor = (4.0 / 3.0 - (8.0 / 3.0) / length) * sum.y / dx;
            }
        }
        CGPoint control1 = multiply_add(lastDirection, amount * factor, add(point, oldNormal));
        CGPoint control2 = multiply_add(direction, -amount * factor, end);
        emit_cubic({{CGPointZero, control1, control2, end}});
    }

    void begin(CGPoint point, CGPoint tangent) {
        CGPoint direction = normalize(tangent);
        if (state == 4) {
            join(point, direction);
        } else {
            state = 4;
            firstDirection = direction;
            CGPoint p = add(point, normal(tangent));
            CGPathMoveToPoint(path, nullptr, p.x, p.y);
        }
        lastDirection = direction;
    }

    bool line_segment(CGPoint a, CGPoint b) {
        CGPoint tangent = subtract(b, a);
        if (!(dot(tangent, tangent) > 1e-8)) {
            if (state == 2) { state = 3; }
            return false;
        }
        begin(a, tangent);
        emit_line(add(b, normal(tangent)));
        return true;
    }

    void fit(Cubic curve, int depth) {
        OffsetCubic source(curve);
        CGPoint firstTangent = source.derivative(0), lastTangent = source.derivative(1);
        CGPoint first = add(curve.p[0], normal(firstTangent));
        CGPoint last = add(curve.p[3], normal(lastTangent));
        CGPoint chord = subtract(curve.p[3], curve.p[0]);
        if (dot(chord, chord) <= 1e-8) { emit_line(last); return; }
        CGPoint middleTangent = source.derivative(0.5);
        CGPoint middle = add(source.evaluate(0.5), normal(middleTangent));
        CGPoint adjusted = multiply_add(subtract(scale(middle, 2), add(first, last)), 1.0 / 6.0, middle);
        auto intersection = [&](CGPoint point, CGPoint tangent) {
            CGFloat determinant = cross(tangent, middleTangent);
            if (std::abs(determinant) < 0.0001) { return adjusted; }
            CGFloat t = cross(subtract(adjusted, point), middleTangent) / determinant;
            return multiply_add(tangent, t, point);
        };
        Cubic result{{first, intersection(first, firstTangent), intersection(last, lastTangent), last}};
        if (--depth >= 0) {
            OffsetCubic candidate(result);
            for (CGFloat t : {0.5, 0.3, 0.7, 0.1, 0.9}) {
                CGPoint expected = t == 0.5 ? middle : add(source.evaluate(t), normal(source.derivative(t)));
                bool finite = std::isfinite(expected.x) && std::isfinite(expected.y);
                for (auto p : result.p) { finite = finite && std::isfinite(p.x) && std::isfinite(p.y); }
                if (finite && !candidate.near(expected, t)) {
                    Cubic left, right;
                    curve.split(0.5, left, right);
                    fit(left, depth);
                    fit(right, depth);
                    return;
                }
            }
        }
        emit_cubic(result);
    }

    void offset_curve(Cubic curve) {
        CGPoint a = subtract(curve.p[1], curve.p[0]), b = subtract(curve.p[2], curve.p[1]);
        CGPoint c = subtract(curve.p[3], curve.p[2]);
        if (dot(a, b) >= 0 && dot(b, c) >= 0) { fit(curve, 4); return; }
        if (dot(a, a) <= 1e-8 && dot(b, b) <= 1e-8 && dot(c, c) <= 1e-8) {
            emit_line(add(curve.p[3], normal(OffsetCubic(curve).derivative(1))));
            return;
        }
        Cubic left, right;
        curve.split(0.5, left, right);
        auto equal = [](const Cubic &a, const Cubic &b) {
            for (unsigned i = 0; i < 4; ++i) {
                if (!(a.p[i].x == b.p[i].x || (std::isnan(a.p[i].x) && std::isnan(b.p[i].x))) ||
                    !(a.p[i].y == b.p[i].y || (std::isnan(a.p[i].y) && std::isnan(b.p[i].y)))) { return false; }
            }
            return true;
        };
        if (!equal(curve, left)) { offset_curve(left); }
        if (!equal(curve, right)) { offset_curve(right); }
    }

    bool cubic_segment(Cubic curve) {
        CGPoint a = subtract(curve.p[1], curve.p[0]), b = subtract(curve.p[2], curve.p[1]);
        CGPoint c = subtract(curve.p[3], curve.p[2]);
        if (dot(a, a) <= 1e-8 && dot(b, b) <= 1e-8 && dot(c, c) <= 1e-8) {
            if (state == 2) { state = 3; }
            return false;
        }
        OffsetCubic source(curve);
        begin(curve.p[0], source.derivative(0));
        lastDirection = normalize(source.derivative(1));
        if (cross(a, b) * cross(b, c) <= 0) {
            CGFloat roots[2] = {-1, -1};
            for (unsigned axis = 0; axis < 2; ++axis) {
                CGFloat quadratic = axis == 0 ? source.quadratic.x : source.quadratic.y;
                CGFloat cubic = axis == 0 ? source.cubic.x : source.cubic.y;
                CGFloat t = -quadratic / (3 * cubic);
                if (cubic != 0 && t > 0 && t < 1) { roots[axis] = t; }
            }
            if (roots[0] == -1) { std::swap(roots[0], roots[1]); }
            if (roots[1] != -1) {
                if (std::abs(roots[0] - roots[1]) < 1e-6) { roots[1] = -1; }
                else if (roots[1] < roots[0]) { std::swap(roots[0], roots[1]); }
            }
            if (roots[0] != -1) {
                Cubic left, right;
                curve.split(roots[0], left, right);
                offset_curve(left);
                curve = right;
                if (roots[1] != -1) {
                    curve.split((roots[1] - roots[0]) / (1 - roots[0]), left, right);
                    offset_curve(left);
                    curve = right;
                }
            }
        }
        offset_curve(curve);
        return true;
    }

    void apply(const CGPathElement &element) {
        switch (element.type) {
            case kCGPathElementMoveToPoint:
                start = current = element.points[0]; state = 2; break;
            case kCGPathElementAddLineToPoint:
                if (state && line_segment(current, element.points[0])) { current = element.points[0]; }
                break;
            case kCGPathElementAddQuadCurveToPoint: {
                CGPoint control = element.points[0], end = element.points[1];
                Cubic curve{{current, multiply_add(subtract(current, control), 1.0 / 3.0, control),
                             multiply_add(subtract(end, control), 1.0 / 3.0, control), end}};
                if (state && cubic_segment(curve)) { current = end; }
                break;
            }
            case kCGPathElementAddCurveToPoint:
                if (state && cubic_segment({{current, element.points[0], element.points[1], element.points[2]}})) { current = element.points[2]; }
                break;
            case kCGPathElementCloseSubpath:
                if (state >= 2) {
                    if (state == 4) {
                        line_segment(current, start);
                        join(start, firstDirection);
                        CGPathCloseSubpath(path);
                    }
                    state = 1; current = start;
                }
                break;
        }
    }
};

CGPathRef copy_offset_path(CGPathRef path, CGFloat amount) {
    CGMutablePathRef result = CGPathCreateMutable();
    Offsetter offsetter{result, amount};
    CGPathApply(path, &offsetter, +[](void *context, const CGPathElement *element) {
        static_cast<Offsetter *>(context)->apply(*element);
    });
    return result;
}

struct Frame {
    unsigned type;
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathRef accumulated = nullptr;
    CGAffineTransform transform = CGAffineTransformIdentity;
    CGFloat values[3] = {};
    int32_t cap = 0;
    int32_t join = 0;
    uint8_t operation = 0;
    bool evenOdd = false;
    size_t emittedElements = 0;
    std::vector<CGFloat> dash;

    explicit Frame(unsigned type) : type(type) {}
    ~Frame() {
        CGPathRelease(path);
        CGPathRelease(accumulated);
    }

    void end_boolean() {
        bool hasPath = !CGPathIsEmpty(path);
        if (!accumulated) {
            if (operation != 0 && hasPath) {
                accumulated = CGPathCreateCopy(path);
                CGPathRelease(path);
                path = CGPathCreateMutable();
            }
            return;
        }
        CGPathRef next = nullptr;
        switch (operation) {
            case 1: next = hasPath ? CGPathCreateCopyByIntersectingPath(accumulated, path, evenOdd) : nullptr; break;
            case 2: next = hasPath ? CGPathCreateCopyByUnioningPath(accumulated, path, evenOdd) : CGPathRetain(accumulated); break;
            case 3: next = hasPath ? CGPathCreateCopyBySubtractingPath(accumulated, path, evenOdd) : CGPathRetain(accumulated); break;
            case 4: next = hasPath ? CGPathCreateCopyBySymmetricDifferenceOfPath(accumulated, path, evenOdd) : CGPathRetain(accumulated); break;
            case 5: next = hasPath ? CGPathCreateCopyOfLineByIntersectingPath(accumulated, path, evenOdd) : nullptr; break;
            case 6: next = hasPath ? CGPathCreateCopyOfLineBySubtractingPath(accumulated, path, evenOdd) : nullptr; break;
            default: return;
        }
        CGPathRelease(accumulated);
        accumulated = next;
    }

    CGPathRef finish() {
        switch (type) {
            case 17:
                return CGPathCreateCopyByTransformingPath(path, &transform);
            case 18:
                return copy_trimmed_path(path, static_cast<float>(values[0]), static_cast<float>(values[1]));
            case 19: {
                CGPathRef input = dash.empty() ? CGPathRetain(path)
                    : CGPathCreateCopyByDashingPath(path, nullptr, static_cast<float>(values[2]), dash.data(), dash.size());
                CGPathRef result = CGPathCreateCopyByStrokingPath(input, nullptr, static_cast<float>(values[0]), static_cast<CGLineCap>(cap), static_cast<CGLineJoin>(join), static_cast<float>(values[1]));
                CGPathRelease(input);
                return result;
            }
            case 20:
                end_boolean();
                return operation == 0 ? CGPathCreateCopyByNormalizing(path, evenOdd) : CGPathRetain(accumulated);
            case 22:
                return copy_dilated_path(path, CGPointMake(values[0], values[1]));
            case 23:
                return copy_offset_path(path, values[0]);
            default:
                return CGPathRetain(path);
        }
    }
};

void append_element(CGMutablePathRef path, const CGPathElement &element) {
    const CGPoint *p = element.points;
    switch (element.type) {
        case kCGPathElementMoveToPoint: CGPathMoveToPoint(path, nullptr, p[0].x, p[0].y); break;
        case kCGPathElementAddLineToPoint: CGPathAddLineToPoint(path, nullptr, p[0].x, p[0].y); break;
        case kCGPathElementAddQuadCurveToPoint: CGPathAddQuadCurveToPoint(path, nullptr, p[0].x, p[0].y, p[1].x, p[1].y); break;
        case kCGPathElementAddCurveToPoint: CGPathAddCurveToPoint(path, nullptr, p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y); break;
        case kCGPathElementCloseSubpath: CGPathCloseSubpath(path); break;
    }
}

struct Mapper {
    std::vector<std::unique_ptr<Frame>> frames;
    Mapper() { frames.push_back(std::make_unique<Frame>(0)); }

    Frame &current() { return *frames.back(); }

    bool flush(size_t index) {
        Frame &frame = *frames[index];
        CGPathRef path = frame.finish();
        if (!path) { return false; }
        struct AppendInfo {
            Mapper *mapper;
            size_t index;
            size_t skip;
            size_t count = 0;
        } info{this, index - 1, frame.type == 23 ? frame.emittedElements : 0};
        CGPathApply(path, &info, +[](void *context, const CGPathElement *element) {
            auto &info = *static_cast<AppendInfo *>(context);
            if (info.count++ < info.skip) { return; }
            append_element(info.mapper->frames[info.index]->path, *element);
        });
        frame.emittedElements = info.count;
        CGPathRelease(path);
        return true;
    }

    void end_path(size_t index) {
        Frame &frame = *frames[index];
        switch (frame.type) {
            case 20:
                frame.end_boolean();
                break;
            case 18: case 22:
                flush(index);
                break;
            case 19:
                flush(index);
                CGPathRelease(frame.path);
                frame.path = CGPathCreateMutable();
                end_path(index - 1);
                break;
            case 23:
                // Offset endpath forwards the boundary without resetting segment state.
                flush(index);
                end_path(index - 1);
                break;
            default:
                break;
        }
    }

    void pop() {
        if (frames.size() == 1) {
            return;
        }
        unsigned type = current().type;
        bool hasResult = flush(frames.size() - 1);
        frames.pop_back();
        if (type == 19 || type == 23 || (type == 20 && hasResult)) {
            end_path(frames.size() - 1);
        }
    }

    bool apply(ORBPathElement element, const CGFloat *values, const void *info) {
        auto type = static_cast<unsigned>(element);
        auto *transform = static_cast<const CGAffineTransform *>(info);
        size_t index = frames.size() - 1;
        CGMutablePathRef path = current().path;
        switch (type) {
            case 0:
                CGPathMoveToPoint(path, transform, values[0], values[1]);
                break;
            case 1:
                CGPathAddLineToPoint(path, transform, values[0], values[1]);
                break;
            case 2:
                CGPathAddQuadCurveToPoint(path, transform, values[0], values[1], values[2], values[3]);
                break;
            case 3:
                CGPathAddCurveToPoint(path, transform, values[0], values[1], values[2], values[3], values[4], values[5]);
                break;
            case 4:
                CGPathCloseSubpath(path);
                break;
            case 5:
                CGPathAddRect(path, transform, CGRectMake(values[0], values[1], values[2], values[3]));
                break;
            case 6: {
                CGRect rect = CGRectStandardize(CGRectMake(values[0], values[1], values[2], values[3]));
                CGFloat left = CGRectGetMinX(rect), right = CGRectGetMaxX(rect);
                CGFloat top = CGRectGetMinY(rect), bottom = CGRectGetMaxY(rect);
                CGPathMoveToPoint(path, transform, right, CGRectGetMidY(rect));
                CGPathAddLineToPoint(path, transform, right, bottom);
                CGPathAddLineToPoint(path, transform, left, bottom);
                CGPathAddLineToPoint(path, transform, left, top);
                CGPathAddLineToPoint(path, transform, right, top);
                CGPathCloseSubpath(path);
                break;
            }
            case 7:
                CGPathAddEllipseInRect(path, transform, CGRectMake(values[0], values[1], values[2], values[3]));
                break;
            case 8: case 9: case 10: case 11:
                add_rounded_rect(path, element, values, transform);
                break;
            case 12:
                add_relative_arc(path, transform, {values[0], values[1]}, values[2], values[3], values[4]);
                break;
            case 13: case 14:
                add_arc(path, transform, {values[0], values[1]}, values[2], values[3], values[4], type == 13);
                break;
            case 15:
                CGPathAddArcToPoint(path, transform, values[0], values[1], values[2], values[3], values[4]);
                break;
            case 16:
                pop();
                break;
            case 17: case 18: case 19: case 20: case 22: case 23: {
                auto frame = std::make_unique<Frame>(type);
                if (type == 17 && info) {
                    frame->transform = *transform;
                } else if (type == 23) {
                    frame->values[0] = values[0];
                } else if (type == 18 || type == 22) {
                    frame->values[0] = values[0];
                    frame->values[1] = values[1];
                } else if (type == 19) {
                    std::copy_n(values, 3, frame->values);
                    if (info) {
                        const auto &stroke = *static_cast<const Storage::StrokeInfo *>(info);
                        frame->cap = stroke.cap;
                        frame->join = stroke.join;
                        if (stroke.count) {
                            auto *dash = static_cast<const CGFloat *>(stroke.values);
                            frame->dash.assign(dash, dash + stroke.count);
                        }
                    }
                } else if (type == 20 && info) {
                    auto *flags = static_cast<const uint8_t *>(info);
                    frame->operation = flags[0];
                    frame->evenOdd = (flags[1] & 1) != 0;
                }
                frames.push_back(std::move(frame));
                break;
            }
            case 21:
                end_path(frames.size() - 1);
                break;
            default:
                precondition_failure("unsupported path mapping element: %u", type);
        }
        return true;
    }
};

} // namespace

bool apply_mapping_element(void *context, ORBPathElement element, const CGFloat *values, const void *info) {
    return static_cast<Mapper *>(context)->apply(element, values, info);
}

namespace {
struct MapCache {
    std::mutex mutex;
    std::map<std::pair<uint32_t, size_t>, std::shared_ptr<Storage>> entries;
};

MapCache &map_cache() {
    static auto *cache = new MapCache;
    return *cache;
}
} // namespace

void invalidate_mapped_storage(uint32_t identifier) {
    auto &cache = map_cache();
    std::lock_guard<std::mutex> guard(cache.mutex);
    auto first = cache.entries.lower_bound({identifier, 0});
    while (first != cache.entries.end() && first->first.first == identifier) {
        first = cache.entries.erase(first);
    }
}

bool apply_mapped_storage(const Storage &storage, void *context, size_t depth) {
    auto &cache = map_cache();
    auto key = std::make_pair(storage.identifier(), depth);
    std::shared_ptr<Storage> mapped;
    {
        std::lock_guard<std::mutex> guard(cache.mutex);
        auto entry = cache.entries.find(key);
        if (entry != cache.entries.end()) { mapped = entry->second; }
    }
    if (!mapped) {
        Mapper mapper;
        bool complete = storage.apply_elements_at_depth(&mapper,
            +[](void *context, ORBPathElement element, const CGFloat *values, const void *info) {
                return static_cast<Mapper *>(context)->apply(element, values, info);
            }, depth);
        if (!complete) { return false; }
        while (mapper.frames.size() > 1) { mapper.pop(); }
        mapped = std::make_shared<Storage>(sizeof(Storage));
        CGPathApply(mapper.current().path, mapped.get(), +[](void *context, const CGPathElement *element) {
            static_cast<Storage *>(context)->append_element(static_cast<ORBPathElement>(element->type),
                reinterpret_cast<const CGFloat *>(element->points), nullptr);
        });
        std::lock_guard<std::mutex> guard(cache.mutex);
        mapped = cache.entries.emplace(key, mapped).first->second;
    }
    return mapped->apply_elements(context, apply_mapping_element);
}

CGPathRef copy_cg_path(ORBPath path) {
    Mapper mapper;
    bool complete = ORBPathApplyElements(path, &mapper, apply_mapping_element);
    if (!complete) {
        precondition_failure("path enumeration stopped during conversion");
    }
    while (mapper.frames.size() > 1) {
        mapper.pop();
    }
    return CGPathRetain(mapper.current().path);
}

} /* Path */
} /* ORB */

#endif
