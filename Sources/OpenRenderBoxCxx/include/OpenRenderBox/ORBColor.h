//
//  ORBColor.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>
#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CGColor.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef float ORBColorComponent;

typedef struct ORBColor {
    ORBColorComponent red;
    ORBColorComponent green;
    ORBColorComponent blue;
    ORBColorComponent alpha;
} ORBColor;

ORB_EXPORT const ORBColor ORBColorClear ORB_SWIFT_NAME(ORBColor.clear);
ORB_EXPORT const ORBColor ORBColorBlack ORB_SWIFT_NAME(ORBColor.black);
ORB_EXPORT const ORBColor ORBColorWhite ORB_SWIFT_NAME(ORBColor.white);
ORB_EXPORT const ORBColor ORBColorNull ORB_SWIFT_NAME(ORBColor.null);
ORB_EXPORT const ORBColorComponent ORBColorInvalidComponent ORB_SWIFT_NAME(ORBColor.invalidComponent);

ORB_EXPORT ORBColor ORBColorMake(float red, float green, float blue, float alpha) ORB_SWIFT_NAME(ORBColor.init(red:green:blue:alpha:));
ORB_EXPORT ORBColor ORBColorMakeLinear(float red, float green, float blue, float alpha) ORB_SWIFT_NAME(ORBColor.init(linearRed:green:blue:alpha:));
ORB_EXPORT ORBColor ORBColorToLinear(ORBColor color) ORB_SWIFT_NAME(getter:ORBColor.linear(self:));
ORB_EXPORT ORBColor ORBColorFromLinear(ORBColor color) ORB_SWIFT_NAME(ORBColor.fromLinear(self:));
ORB_EXPORT bool ORBColorEqualToColor(ORBColor lhs, ORBColor rhs) ORB_SWIFT_NAME(ORBColor.isEqual(self:to:));

#if ORB_TARGET_OS_DARWIN

ORB_EXPORT ORBColor ORBColorFromComponents(CGColorSpaceRef colorSpace, const CGFloat *components, bool premultiplied) ORB_SWIFT_NAME(ORBColor.init(colorSpace:components:premultiplied:));
ORB_EXPORT ORBColor ORBColorFromComponents2(CGColorSpaceRef colorSpace, const CGFloat *components, size_t componentCount) ORB_SWIFT_NAME(ORBColor.init(colorSpace:components:componentCount:));
ORB_EXPORT ORBColor ORBColorFromCGColor(CGColorRef color, bool premultiplied) ORB_SWIFT_NAME(ORBColor.init(_:premultiplied:));
ORB_EXPORT ORBColor ORBColorFromCGColor2(CGColorRef color, size_t componentCount) ORB_SWIFT_NAME(ORBColor.init(_:componentCount:));
ORB_EXPORT CGColorRef ORBColorCopyCGColor(ORBColor color) ORB_SWIFT_NAME(getter:ORBColor.cgColor(self:));

#endif /* ORB_TARGET_OS_DARWIN */

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

