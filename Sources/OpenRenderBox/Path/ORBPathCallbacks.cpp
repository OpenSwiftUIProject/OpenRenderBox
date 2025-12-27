//
//  ORBPathCallbacks.cpp
//  OpenRenderBox
//
//  Audited for 6.5.1
//  Status: Complete

#include <OpenRenderBox/ORBPathCallbacks.h>
#include <OpenRenderBoxCxx/Util/assert.hpp>

#if ORB_TARGET_OS_DARWIN

CG_EXTERN size_t CGPathGetNumberOfPoints(CGPathRef cg_nullable path);
CG_EXTERN size_t CGPathGetNumberOfElements(CGPathRef cg_nullable path);
typedef void (^CGPathApplyBlock2)(const CGPathElement * element, bool *stop);
CG_EXTERN void CGPathApplyWithBlock2(CGPathRef path, CGPathApplyBlock2 CF_NOESCAPE block);

namespace {
    uint32_t cgpath_bezier_order(CGPathRef cgPath) {
        size_t pointsCount = CGPathGetNumberOfPoints(cgPath);
        size_t elementsCount = CGPathGetNumberOfElements(cgPath);
        if (pointsCount > elementsCount) { return 3; }
        if (pointsCount != 3 || elementsCount != 3) { return 1; }
        bool hasCurve = false;
        CGPathApply(cgPath, &hasCurve, [](void *info, const CGPathElement *element) {
            if (element->type == kCGPathElementAddQuadCurveToPoint
                || element->type == kCGPathElementAddCurveToPoint) {
                *static_cast<bool*>(info) = true;
            }
        });
        return hasCurve ? 3 : 1;
    }
}

const ORBPathCallbacks ORBPathCGPathCallbacks = {
    .flags = {},
    .retain = CFRetain,
    .release = CFRelease,
    .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) -> bool {
        CGPathRef cgPath = reinterpret_cast<CGPathRef>(object);
        __block bool shouldStop = false;
        CGPathApplyWithBlock2(cgPath, ^(const CGPathElement *element, bool *stop) {
            bool result = callback(info,
                                   (ORBPathElement)element->type,
                                   (CGFloat *)element->points,
                                   nullptr);
            if (!result) {
                *stop = true;
                shouldStop = true;
            }
        });
        return !shouldStop;
    },
    .isEqual = +[](const void *object, const void *otherObject) -> bool {
        return CGPathEqualToPath(static_cast<CGPathRef>(object), static_cast<CGPathRef>(otherObject));
    },
    .isEmpty = +[](const void *object) -> bool {
        return CGPathIsEmpty(static_cast<CGPathRef>(object));
    },
    .isSingleElement = +[](const void *object) -> bool {
        return false;
    },
    .bezierOrder = +[](const void *object) -> uint32_t {
        return cgpath_bezier_order(static_cast<CGPathRef>(object));
    },
    .boundingRect = +[](const void *object) -> CGRect {
        return CGPathGetPathBoundingBox(static_cast<CGPathRef>(object));
    },
    .cgPath = +[](const void *object) -> CGPathRef {
        return static_cast<CGPathRef>(object);
    },
    .next = nullptr,
};

#endif /* ORB_TARGET_OS_DARWIN */
