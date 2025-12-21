//
//  ORBPath.cpp
//  OpenRenderBox
//
//  Created by Kyle on 2025/3/25.
//

#include <OpenRenderBox/ORBPathStorage.h>

void ORBPathRetain(ORBPath path) {
    ORBPathRetainCallback retain = path.callbacks->retain;
    if (retain != nullptr) {
        retain(&path);
    }
}

void ORBPathRelease(ORBPath path) {
    ORBPathReleaseCallback release = path.callbacks->release;
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

#endif // ORB_TARGET_OS_DARWIN
