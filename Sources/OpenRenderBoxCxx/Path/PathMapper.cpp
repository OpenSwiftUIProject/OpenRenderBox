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

struct Frame {
    unsigned type;
    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform = CGAffineTransformIdentity;
    CGFloat values[3] = {};
    int32_t cap = 0;
    int32_t join = 0;
    std::vector<CGFloat> dash;

    explicit Frame(unsigned type) : type(type) {}
    ~Frame() {
        CGPathRelease(path);
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
        } info{this, index - 1};
        CGPathApply(path, &info, +[](void *context, const CGPathElement *element) {
            auto &info = *static_cast<AppendInfo *>(context);
            append_element(info.mapper->frames[info.index]->path, *element);
        });
        CGPathRelease(path);
        return true;
    }

    void end_path(size_t index) {
        Frame &frame = *frames[index];
        switch (frame.type) {
            case 18:
                flush(index);
                break;
            case 19:
                flush(index);
                CGPathRelease(frame.path);
                frame.path = CGPathCreateMutable();
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
        flush(frames.size() - 1);
        frames.pop_back();
        if (type == 19) {
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
            case 17: case 18: case 19: {
                auto frame = std::make_unique<Frame>(type);
                if (type == 17 && info) {
                    frame->transform = *transform;
                } else if (type == 18) {
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
                }
                frames.push_back(std::move(frame));
                break;
            }
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
