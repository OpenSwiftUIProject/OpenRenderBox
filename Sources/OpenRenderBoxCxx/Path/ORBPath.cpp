//
//  ORBPath.cpp
//  OpenRenderBox
//
//  Created by Kyle on 2025/3/25.
//

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBPath.h>
#include <OpenRenderBox/ORBPathCallbacks.h>
#include "Storage.hpp"
#include "NestedCallbacks.hpp"
#include <algorithm>
#include <cstring>
#include <Util/assert.hpp>

// Empty path callbacks (all null) - C++ internal linkage
static const ORBPathCallbacks empty_path_callbacks = {
    {},
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

// Empty path (storage = null)
const ORBPath ORBPathEmpty = {
    nullptr,
    &empty_path_callbacks,
};

// Null path (storage = 0x1)
const ORBPath ORBPathNull = {
    reinterpret_cast<ORBPathStorage *>(0x1),
    &empty_path_callbacks,
};

ORBPath ORBPathRetain(ORBPath path) {
    auto retain = path.callbacks->retain;
    if (retain != nullptr) {
        path.storage = reinterpret_cast<ORBPathStorage *>(
            const_cast<void *>(retain(path.storage))
        );
    }
    return path;
}

void ORBPathRelease(ORBPath path) {
    auto release = path.callbacks->release;
    if (release != nullptr) {
        release(path.storage);
    }
}

#if ORB_TARGET_OS_DARWIN

ORBPath ORBPathCopyTransformedPath(ORBPath path, const CGAffineTransform *transform) {
    if (ORBPathIsNull(path)) {
        return ORBPathNull;
    }
    CGPathRef source = ORBPathCopyCGPath(path);
    CGPathRef transformed = CGPathCreateCopyByTransformingPath(source, transform);
    ORBPath result = ORBPathMakeWithCGPath(transformed);
    CGPathRelease(transformed);
    CGPathRelease(source);
    return result;
}

// MARK: - Path Creation

namespace {
ORBPath make_rect(CGRect rect, const CGAffineTransform *transform, ORBPathElement element) {
    if (CGRectIsNull(rect)) {
        return ORBPathNull;
    }
    if (transform == nullptr || CGAffineTransformIsIdentity(*transform)) {
        // TODO
        return ORBPathNull;
    } else {
        // TODO
        return ORBPathNull;
    }
}
} /* anonymous namespace */

// TODO: TO be implemented natively

ORBPath ORBPathMakeWithCGPath(CGPathRef cgPath) {
    if (cgPath == nullptr) {
        return ORBPathNull;
    }
    CFRetain(cgPath);
    return ORBPath {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
}

ORBPath ORBPathMakeRect(CGRect rect, const CGAffineTransform *transform) {
    return make_rect(rect, transform, ORBPathElementRect);
}

ORBPath ORBPathMakeEllipse(CGRect rect, const CGAffineTransform *transform) {
    CGPathRef cgPath = CGPathCreateWithEllipseInRect(rect, transform);
    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

ORBPath ORBPathMakeRoundedRect(CGRect rect, CGFloat cornerWidth, CGFloat cornerHeight, ORBPathRoundedCornerStyle style, const CGAffineTransform *transform) {
    // TODO: Handle ORBRoundedCornerStyleContinuous with custom bezier curves
    CGPathRef cgPath = CGPathCreateWithRoundedRect(rect, cornerWidth, cornerHeight, transform);
    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat topRightRadius, CGFloat bottomRightRadius, CGFloat bottomLeftRadius, ORBPathRoundedCornerStyle style, const CGAffineTransform *transform) {
    // TODO: Handle ORBRoundedCornerStyleContinuous with custom bezier curves
    CGMutablePathRef cgPath = CGPathCreateMutable();

    CGFloat minX = CGRectGetMinX(rect);
    CGFloat minY = CGRectGetMinY(rect);
    CGFloat maxX = CGRectGetMaxX(rect);
    CGFloat maxY = CGRectGetMaxY(rect);

    // Start at top-left corner (after the rounded corner)
    CGPathMoveToPoint(cgPath, transform, minX + topLeftRadius, minY);

    // Top edge and top-right corner
    CGPathAddLineToPoint(cgPath, transform, maxX - topRightRadius, minY);
    CGPathAddArc(cgPath, transform, maxX - topRightRadius, minY + topRightRadius, topRightRadius, -M_PI_2, 0, false);

    // Right edge and bottom-right corner
    CGPathAddLineToPoint(cgPath, transform, maxX, maxY - bottomRightRadius);
    CGPathAddArc(cgPath, transform, maxX - bottomRightRadius, maxY - bottomRightRadius, bottomRightRadius, 0, M_PI_2, false);

    // Bottom edge and bottom-left corner
    CGPathAddLineToPoint(cgPath, transform, minX + bottomLeftRadius, maxY);
    CGPathAddArc(cgPath, transform, minX + bottomLeftRadius, maxY - bottomLeftRadius, bottomLeftRadius, M_PI_2, M_PI, false);

    // Left edge and top-left corner
    CGPathAddLineToPoint(cgPath, transform, minX, minY + topLeftRadius);
    CGPathAddArc(cgPath, transform, minX + topLeftRadius, minY + topLeftRadius, topLeftRadius, M_PI, M_PI + M_PI_2, false);

    CGPathCloseSubpath(cgPath);

    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(cgPath),
        &ORBPathCGPathCallbacks,
    };
    return path;
}
#endif /* ORB_TARGET_OS_DARWIN */

bool ORBPathIsEmpty(ORBPath path) {
    if (path.callbacks == &empty_path_callbacks) {
        return true;
    }
    if (path.callbacks->flags.isExtended) {
        auto extended = reinterpret_cast<const ORBPathCallbacksExtended*>(path.callbacks);
        auto isEmptyCallback = extended->isEmpty;
        if (isEmptyCallback) {
            return isEmptyCallback(path.storage, extended);
        } else {
            bool isEmpty = true;
            auto applyCallback = extended->apply;
            if (applyCallback) {
                applyCallback(path.storage, &isEmpty, +[](void * info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo) -> bool {
                    *((bool *)info) = false;
                    return false;
                }, extended);
            }
            return isEmpty;
        }
    } else {
        auto isEmptyCallback = path.callbacks->isEmpty;
        if (isEmptyCallback) {
            return isEmptyCallback(path.storage);
        } else {
            bool isEmpty = true;
            auto applyCallback = path.callbacks->apply;
            if (applyCallback) {
                applyCallback(path.storage, &isEmpty, +[](void * info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo) -> bool {
                    *((bool *)info) = false;
                    return false;
                });
            }
            return isEmpty;
        }
    }
}

bool ORBPathIsSingleElement(ORBPath path) {
    if (path.callbacks == &empty_path_callbacks) {
        return false;
    }
    if (path.callbacks->flags.isExtended) {
        auto extended = reinterpret_cast<const ORBPathCallbacksExtended *>(path.callbacks);
        if (extended->isSingleElement != nullptr) {
            return extended->isSingleElement(path.storage, extended);
        }
    } else if (path.callbacks->isSingleElement != nullptr) {
        return path.callbacks->isSingleElement(path.storage);
    }

    ORB::Path::NestedCallbacks::SingleInfo count;
    ORBPathApplyElements(path, &count, ORB::Path::NestedCallbacks::single_element_callback);
    return count.count == 1;
}

size_t ORBPathGetBezierOrder(ORBPath path) {
    if (path.callbacks->flags.isExtended) {
        auto *extended = reinterpret_cast<const ORBPathCallbacksExtended *>(path.callbacks);
        return extended->bezierOrder ? extended->bezierOrder(path.storage, extended) : 3;
    }
    return path.callbacks->bezierOrder ? path.callbacks->bezierOrder(path.storage) : 3;
}

bool ORBPathApplyElements(ORBPath path, void *info, ORBPathApplyCallback callback) {
    if (callback == nullptr) {
        return true;
    }
    if (path.callbacks->flags.isExtended) {
        auto extended = reinterpret_cast<const ORBPathCallbacksExtended*>(path.callbacks);
        auto applyCallback = extended->apply;
        if (applyCallback) {
            return applyCallback(path.storage, info, callback, extended);
        }
        return true;
    } else {
        auto applyCallback = path.callbacks->apply;
        if (applyCallback) {
            return applyCallback(path.storage, info, callback);
        }
        return true;
    }
}

bool ORBPathEqualToPath(ORBPath lhs, ORBPath rhs) {
    if (lhs.callbacks == rhs.callbacks) {
        if (lhs.storage == rhs.storage) { return true; }
        if (lhs.callbacks->flags.isExtended) {
            auto *extended = reinterpret_cast<const ORBPathCallbacksExtended *>(lhs.callbacks);
            if (extended->isEqual) { return extended->isEqual(lhs.storage, rhs.storage, extended); }
        } else if (lhs.callbacks->isEqual) {
            return lhs.callbacks->isEqual(lhs.storage, rhs.storage);
        }
    } else {
        if (lhs.callbacks == &empty_path_callbacks) {
            return lhs.storage == ORBPathNull.storage ? false : ORBPathIsEmpty(rhs);
        }
        if (rhs.callbacks == &empty_path_callbacks) {
            return rhs.storage == ORBPathNull.storage ? false : ORBPathIsEmpty(lhs);
        }
    }
    using Storage = ORB::Path::Storage;
    // Only the left stream is normalized by Storage, matching the callback ABI.
    Storage storage(sizeof(Storage));
    ORBPathApplyElements(lhs, &storage, ORB::Path::append_element_callback);
    struct Comparison {
        const Storage &storage;
        Storage::Iterator iterator;
    } comparison{storage, {}};
    bool equal = ORBPathApplyElements(rhs, &comparison, +[](void *context, ORBPathElement type, const CGFloat *points, const void *info) -> bool {
        auto &comparison = *static_cast<Comparison *>(context);
        Storage::Element element;
        if (!comparison.iterator.next(comparison.storage, element) || type != element.type) { return false; }
        for (size_t index = 0; index < Storage::argument_count(type); ++index) {
            if (element.points[index] != points[index]) { return false; }
        }
        if (!info && !element.hasInfo) { return true; }
        switch (type) {
        case ORBPathElementCloseSubpath:
        case ORBPathElementEnd:
        case ORBPathElementTrimmed:
        case ORBPathElementDilateGlyph:
        case ORBPathElementOffset:
            return true;
        case ORBPathElementStroked:
        case ORBPathElementClipStroke: {
            Storage::StrokeInfo defaults{0, type == ORBPathElementClipStroke ? 1 : 0, 0, nullptr};
            auto &a = element.hasInfo ? element.metadata.stroke : defaults;
            auto &b = info ? *static_cast<const Storage::StrokeInfo *>(info) : defaults;
            if (a.cap != b.cap || a.join != b.join || a.count != b.count) { return false; }
            if (type == ORBPathElementStroked) {
                auto *av = static_cast<const CGFloat *>(a.values), *bv = static_cast<const CGFloat *>(b.values);
                for (size_t index = 0; index < a.count; ++index) { if (av[index] != bv[index]) { return false; } }
            } else {
                auto *av = static_cast<const Storage::StrokeRecord *>(a.values), *bv = static_cast<const Storage::StrokeRecord *>(b.values);
                for (size_t index = 0; index < a.count; ++index) {
                    if (av[index].type != bv[index].type) { return false; }
                    float ap[3], bp[3];
                    std::memcpy(ap, av[index].values, sizeof(ap));
                    std::memcpy(bp, bv[index].values, sizeof(bp));
                    if (!std::equal(ap, ap + 3, bp)) { return false; }
                }
            }
            return true;
        }
        case ORBPathElementBoolean: {
            const uint8_t defaults[2] = {};
            auto *a = element.hasInfo ? element.metadata.boolean : defaults;
            auto *b = info ? static_cast<const uint8_t *>(info) : defaults;
            return a[0] == b[0] && a[1] == b[1];
        }
        default: {
            CGAffineTransform a = CGAffineTransformIdentity;
            if (element.hasInfo) { std::memcpy(&a, element.metadata.transform, sizeof(a)); }
            auto &b = info ? *static_cast<const CGAffineTransform *>(info) : CGAffineTransformIdentity;
            return CGAffineTransformEqualToTransform(a, b);
        }
        }
    });
    return equal && comparison.iterator.offset >= storage.actual_size();
}

#if ORB_TARGET_OS_DARWIN
CGPathRef ORBPathCopyCGPath(ORBPath path) {
    // TODO: Return a retained copy of the CGPath
    return nullptr;
}

bool ORBPathContainsPoint(ORBPath path, CGPoint point, bool eoFill) {
    return ORBPathContainsPoints(path, 1, &point, eoFill, nullptr);
}

uint64_t ORBPathContainsPoints(ORBPath path, uint64_t count, const CGPoint *points, bool eoFill, const CGAffineTransform *transform) {
    // TODO: Implement point containment testing with winding rule
    return false;
}
#endif /* ORB_TARGET_OS_DARWIN */

#endif /* OPENRENDERBOX_CF_CGTYPES */
