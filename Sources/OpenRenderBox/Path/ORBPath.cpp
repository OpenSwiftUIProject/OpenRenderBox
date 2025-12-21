//
//  ORBPath.cpp
//  OpenRenderBox
//
//  Created by Kyle on 2025/3/25.
//

#include <OpenRenderBox/ORBPath.h>

// Empty path callbacks (all null)
const ORBPathCallbacks ORBPathEmptyCallbacks = {
    nullptr, // reserved
    nullptr, // retain
    nullptr, // release
    nullptr, // apply
    nullptr, // isEqual
    nullptr, // isEmpty
    nullptr, // isSingleRect
    nullptr, // bezierOrder
    nullptr, // boundingBox
    nullptr, // cgPath
};

// Empty path (storage = null)
const ORBPath ORBPathEmpty = {
    nullptr,
    &ORBPathEmptyCallbacks,
};

// Null path (storage = 0x1)
const ORBPath ORBPathNull = {
    reinterpret_cast<ORBPathStorage *>(0x1),
    &ORBPathEmptyCallbacks,
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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

// FIXME: Not implemented correctly
const ORBPathCallbacks ORBPathCGPathCallbacks = {
    nullptr,
    // retain
    +[](ORBPathRef path) -> void {
        CFRetain(path->storage);
    },
    // release
    +[](ORBPathRef path) -> void {
        CFRelease(path->storage);
    },
    // apply
    +[](ORBPathRef path, void *info, ORBPathApplyCallback callback) -> bool {
        CGPathRef cgPath = reinterpret_cast<CGPathRef>(path->storage);
        __block bool shouldStop = false;
        CGPathApplyWithBlock(cgPath, ^(const CGPathElement *element) {
            if (shouldStop) return;
            ORBPathElement orbElement;
            double pointBuffer[6];
            switch (element->type) {
                case kCGPathElementMoveToPoint:
                    orbElement.type = ORBPathElementMoveToPoint;
                    pointBuffer[0] = element->points[0].x;
                    pointBuffer[1] = element->points[0].y;
                    orbElement.points = pointBuffer;
                    break;
                case kCGPathElementAddLineToPoint:
                    orbElement.type = ORBPathElementAddLineToPoint;
                    pointBuffer[0] = element->points[0].x;
                    pointBuffer[1] = element->points[0].y;
                    orbElement.points = pointBuffer;
                    break;
                case kCGPathElementAddQuadCurveToPoint:
                    orbElement.type = ORBPathElementAddQuadCurveToPoint;
                    pointBuffer[0] = element->points[0].x;
                    pointBuffer[1] = element->points[0].y;
                    pointBuffer[2] = element->points[1].x;
                    pointBuffer[3] = element->points[1].y;
                    orbElement.points = pointBuffer;
                    break;
                case kCGPathElementAddCurveToPoint:
                    orbElement.type = ORBPathElementAddCurveToPoint;
                    pointBuffer[0] = element->points[0].x;
                    pointBuffer[1] = element->points[0].y;
                    pointBuffer[2] = element->points[1].x;
                    pointBuffer[3] = element->points[1].y;
                    pointBuffer[4] = element->points[2].x;
                    pointBuffer[5] = element->points[2].y;
                    orbElement.points = pointBuffer;
                    break;
                case kCGPathElementCloseSubpath:
                    orbElement.type = ORBPathElementCloseSubpath;
                    orbElement.points = nullptr;
                    break;
            }
            if (callback != nullptr) {
                shouldStop = callback(info, orbElement, nullptr);
            }
        });
        return !shouldStop;
    },
    // isEqual
    +[](ORBPathRef path, ORBPathRef otherPath) -> bool {
        return CGPathEqualToPath(reinterpret_cast<CGPathRef>(path->storage), reinterpret_cast<CGPathRef>(otherPath->storage));
    },
    // isEmpty
    +[](ORBPathRef path) -> bool {
        return CGPathIsEmpty(reinterpret_cast<CGPathRef>(path->storage));
    },
    // isSingleRect - CGPath doesn't have this, always return false
    +[](ORBPathRef path) -> bool {
        return false;
    },
    // bezierOrder
    +[](ORBPathRef path) -> uint32_t {
        CGPathRef cgPath = reinterpret_cast<CGPathRef>(path->storage);
        __block uint32_t order = 1;
        CGPathApply(cgPath, &order, [](void *info, const CGPathElement *element) {
            uint32_t *orderPtr = static_cast<uint32_t*>(info);
            if (element->type == kCGPathElementAddCurveToPoint) {
                *orderPtr = 3;
            } else if (element->type == kCGPathElementAddQuadCurveToPoint && *orderPtr < 3) {
                *orderPtr = 2;
            }
        });
        return order;
    },
    // boundingBox
    +[](ORBPathRef path) -> CGRect {
        return CGPathGetPathBoundingBox(reinterpret_cast<CGPathRef>(path->storage));
    },
    // cgPath - return the CGPath itself
    +[](ORBPathRef path) -> CGPathRef {
        return reinterpret_cast<CGPathRef>(path->storage);
    },
};

// MARK: - Path Creation

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
    CGPathRef cgPath = CGPathCreateWithRect(rect, transform);
    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

ORBPath ORBPathMakeEllipse(CGRect rect, const CGAffineTransform *transform) {
    CGPathRef cgPath = CGPathCreateWithEllipseInRect(rect, transform);
    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

ORBPath ORBPathMakeRoundedRect(CGRect rect, CGFloat cornerWidth, CGFloat cornerHeight, const CGAffineTransform *transform) {
    CGPathRef cgPath = CGPathCreateWithRoundedRect(rect, cornerWidth, cornerHeight, transform);
    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat bottomLeftRadius, CGFloat bottomRightRadius, CGFloat topRightRadius, const CGAffineTransform *transform) {
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

#endif // ORB_TARGET_OS_DARWIN
