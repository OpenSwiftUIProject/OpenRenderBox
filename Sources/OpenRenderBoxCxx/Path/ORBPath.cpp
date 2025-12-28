//
//  ORBPath.cpp
//  OpenRenderBox
//
//  Created by Kyle on 2025/3/25.
//

#if OPENRENDERBOX_CF_CGTYPE

#include <OpenRenderBox/ORBPath.h>
#include <OpenRenderBox/ORBPathCallbacks.h>
#include <OpenRenderBoxCxx/Path/Storage.hpp>
#include <OpenRenderBoxCxx/Util/assert.hpp>

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

void ORBPathRetain(ORBPath path) {
    auto retain = path.callbacks->retain;
    if (retain != nullptr) {
        retain(&path);
    }
}

void ORBPathRelease(ORBPath path) {
    auto release = path.callbacks->release;
    if (release != nullptr) {
        release(&path);
    }
}

#if ORB_TARGET_OS_DARWIN

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

ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat bottomLeftRadius, CGFloat bottomRightRadius, CGFloat topRightRadius, ORBPathRoundedCornerStyle style, const CGAffineTransform *transform) {
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
        if (lhs.storage == rhs.storage) {
            return true;
        }
        auto callbacks = lhs.callbacks;
        if (callbacks->flags.isExtended) {
            auto extended = reinterpret_cast<const ORBPathCallbacksExtended*>(callbacks);
            auto isEqualCallback = extended->isEqual;
            if (isEqualCallback) {
                return isEqualCallback(lhs.storage, rhs.storage, extended);
            }
        } else {
            auto isEqualCallback = callbacks->isEqual;
            if (isEqualCallback) {
                return isEqualCallback(lhs.storage, rhs.storage);
            }
        }
        // auto storage = ORB::Path::Storage(1088);
        // TODO
        return false;
    } else {
        if (lhs.callbacks == &empty_path_callbacks) {
            if (lhs.storage == ORBPathNull.storage) {
                return rhs.callbacks == &empty_path_callbacks && rhs.storage == ORBPathNull.storage;
            } else {
                return ORBPathIsEmpty(rhs);
            }
        } else if (rhs.callbacks == &empty_path_callbacks) {
            if (rhs.storage == ORBPathNull.storage) {
                return false;
            } else {
                return ORBPathIsEmpty(lhs);
            }
        }
    }
}

#if ORB_TARGET_OS_DARWIN
CGPathRef ORBPathCopyCGPath(ORBPath path) {
    // TODO: Return a retained copy of the CGPath
    return nullptr;
}

bool ORBPathContainsPoint(ORBPath path, CGPoint point, bool eoFill) {
    return ORBPathContainsPoints(path, 1, &point, eoFill, nullptr);
}

bool ORBPathContainsPoints(ORBPath path, uint64_t count, const CGPoint *points, bool eoFill, const CGAffineTransform *transform) {
    // TODO: Implement point containment testing with winding rule
    return false;
}
#endif /* ORB_TARGET_OS_DARWIN */

#endif /* OPENRENDERBOX_CF_CGTYPE */
