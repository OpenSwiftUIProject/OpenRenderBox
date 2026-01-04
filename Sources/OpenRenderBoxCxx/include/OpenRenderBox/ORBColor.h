//
//  ORBColor.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef struct ORBColor {
    float red;
    float green;
    float blue;
    float alpha;
} ORBColor;

ORB_EXPORT
ORBColor ORBColorMake(float red, float green, float blue, float alpha) ORB_NOEXCEPT
    ORB_SWIFT_NAME(ORBColor.init(red:green:blue:alpha:));

ORB_EXPORT
ORBColor ORBColorMakeLinear(float red, float green, float blue, float alpha) ORB_NOEXCEPT
    ORB_SWIFT_NAME(ORBColor.init(linearRed:green:blue:alpha:));

ORB_EXPORT
ORBColor ORBColorToLinear(ORBColor color) ORB_NOEXCEPT
    ORB_SWIFT_NAME(getter:ORBColor.linear(self:));

ORB_EXPORT
ORBColor ORBColorFromLinear(ORBColor color) ORB_NOEXCEPT
    ORB_SWIFT_NAME(ORBColor.fromLinear(self:));

ORB_EXPORT
bool ORBColorEqualToColor(ORBColor lhs, ORBColor rhs) ORB_NOEXCEPT
    ORB_SWIFT_NAME(ORBColor.isEqual(self:to:));

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

