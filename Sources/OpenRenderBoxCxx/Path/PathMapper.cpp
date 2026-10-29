//
//  PathMapper.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include "PathMapper.hpp"
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

struct Mapper {
    CGMutablePathRef path = CGPathCreateMutable();

    ~Mapper() { CGPathRelease(path); }

    bool apply(ORBPathElement element, const CGFloat *values, const void *info) {
        auto type = static_cast<unsigned>(element);
        auto *transform = static_cast<const CGAffineTransform *>(info);
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
        mapped = std::make_shared<Storage>(sizeof(Storage));
        CGPathApply(mapper.path, mapped.get(), +[](void *context, const CGPathElement *element) {
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
    return CGPathRetain(mapper.path);
}

} /* Path */
} /* ORB */

#endif
