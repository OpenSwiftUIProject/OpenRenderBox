//
//  ORBPathCallbacks.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBPathCallbacks.h>
#include <OpenRenderBoxCxx/Util/assert.hpp>

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
    nullptr, // reserved2
};

#if ORB_TARGET_OS_DARWIN
// FIXME: Not implemented correctly
const ORBPathCallbacks ORBPathCGPathCallbacks = {
    nullptr, // reserved
    // retain
    +[](const ORBPath *path) -> void {
        CFRetain(path->storage);
    },
    // release
    +[](const ORBPath *path) -> void {
        CFRelease(path->storage);
    },
    // apply
    +[](const ORBPath *path, void *info, ORBPathApplyCallback callback) -> bool {
        CGPathRef cgPath = reinterpret_cast<CGPathRef>(path->storage);
        __block bool shouldStop = false;
        CGPathApplyWithBlock(cgPath, ^(const CGPathElement *element) {
            if (shouldStop) return;
            ORBPathElement orbElement;
            double pointBuffer[6];
            double *points = nullptr;
            switch (element->type) {
                case kCGPathElementMoveToPoint:
                    orbElement = ORBPathElementMoveToPoint;
                    pointBuffer[0] = element->points[0].x;
                    pointBuffer[1] = element->points[0].y;
                    points = pointBuffer;
                    break;
                case kCGPathElementAddLineToPoint:
                    orbElement = ORBPathElementAddLineToPoint;
                    pointBuffer[0] = element->points[0].x;
                    pointBuffer[1] = element->points[0].y;
                    points = pointBuffer;
                    break;
                case kCGPathElementAddQuadCurveToPoint:
                    orbElement = ORBPathElementAddQuadCurveToPoint;
                    pointBuffer[0] = element->points[0].x;
                    pointBuffer[1] = element->points[0].y;
                    pointBuffer[2] = element->points[1].x;
                    pointBuffer[3] = element->points[1].y;
                    points = pointBuffer;
                    break;
                case kCGPathElementAddCurveToPoint:
                    orbElement = ORBPathElementAddCurveToPoint;
                    pointBuffer[0] = element->points[0].x;
                    pointBuffer[1] = element->points[0].y;
                    pointBuffer[2] = element->points[1].x;
                    pointBuffer[3] = element->points[1].y;
                    pointBuffer[4] = element->points[2].x;
                    pointBuffer[5] = element->points[2].y;
                    points = pointBuffer;
                    break;
                case kCGPathElementCloseSubpath:
                    orbElement = ORBPathElementCloseSubpath;
                    points = nullptr;
                    break;
            }
            if (callback != nullptr) {
                shouldStop = callback(info, orbElement, points, nullptr);
            }
        });
        return !shouldStop;
    },
    // isEqual
    +[](const ORBPath *path, const ORBPath *otherPath) -> bool {
        return CGPathEqualToPath(reinterpret_cast<CGPathRef>(path->storage), reinterpret_cast<CGPathRef>(otherPath->storage));
    },
    // isEmpty
    +[](const ORBPath *path) -> bool {
        return CGPathIsEmpty(reinterpret_cast<CGPathRef>(path->storage));
    },
    // isSingleRect - CGPath doesn't have this, always return false
    +[](const ORBPath *path) -> bool {
        return false;
    },
    // bezierOrder
    +[](const ORBPath *path) -> uint32_t {
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
    +[](const ORBPath *path) -> CGRect {
        return CGPathGetPathBoundingBox(reinterpret_cast<CGPathRef>(path->storage));
    },
    // cgPath - return the CGPath itself
    +[](const ORBPath *path) -> CGPathRef {
        return reinterpret_cast<CGPathRef>(path->storage);
    },
    nullptr, // reserved2
};

#endif /* ORB_TARGET_OS_DARWIN */
