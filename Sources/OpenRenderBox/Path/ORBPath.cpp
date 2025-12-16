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
        retain(path);
    }
}

void ORBPathRelease(ORBPath path) {
    ORBPathReleaseCallback release = path.callbacks->release;
    if (release != nullptr) {
        release(path);
    }
}

#if ORB_TARGET_OS_DARWIN

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

namespace {

void cgpath_retain(ORBPath path) {
    CFRetain(path.storage);
}

void cgpath_release(ORBPath path) {
    CFRelease(path.storage);
}

bool cgpath_apply(const void *storage, void *info, ORBPathApplyCallback callback) {
    CGPathRef cgPath = static_cast<CGPathRef>(storage);

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
}

bool cgpath_isEqual(const void *storage, const void *otherStorage) {
    return CGPathEqualToPath(static_cast<CGPathRef>(storage), static_cast<CGPathRef>(otherStorage));
}

bool cgpath_isEmpty(const void *storage) {
    return CGPathIsEmpty(static_cast<CGPathRef>(storage));
}

bool cgpath_isSingleRect(const void *storage) {
    // CGPath doesn't have a direct isSingleRect check, return false
    return false;
}

uint32_t cgpath_bezierOrder(const void *storage) {
    CGPathRef cgPath = static_cast<CGPathRef>(storage);

    // Enumerate path elements to determine bezier order
    // 1 = linear, 2 = quadratic, 3 = cubic
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
}

CGRect cgpath_boundingBox(const void *storage) {
    return CGPathGetPathBoundingBox(static_cast<CGPathRef>(storage));
}

CGPathRef cgpath_cgPath(const void *storage) {
    // Return the CGPath itself
    return static_cast<CGPathRef>(storage);
}

} // anonymous namespace

const ORBPathCallbacks ORBPathCGPathCallbacks = {
    nullptr,            // reserved
    cgpath_retain,      // retain
    cgpath_release,     // release
    cgpath_apply,       // apply
    cgpath_isEqual,     // isEqual
    cgpath_isEmpty,     // isEmpty
    cgpath_isSingleRect, // isSingleRect
    cgpath_bezierOrder, // bezierOrder
    cgpath_boundingBox, // boundingBox
    cgpath_cgPath,      // cgPath
};

#endif // ORB_TARGET_OS_DARWIN
