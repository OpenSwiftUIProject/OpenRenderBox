//
//  ORBColor.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBColor.h>
#include <math.h>

// Global color constants
const ORBColor ORBColorClear = { 0.0f, 0.0f, 0.0f, 0.0f };
const ORBColor ORBColorBlack = { 0.0f, 0.0f, 0.0f, 1.0f };
const ORBColor ORBColorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
const ORBColor ORBColorNull = { ORBColorInvalidComponent, ORBColorInvalidComponent, ORBColorInvalidComponent, ORBColorInvalidComponent };
const float ORBColorInvalidComponent = -32768.0f;

// sRGB to linear constants
static const float kSRGBToLinearThreshold = 0.04045f;
static const float kSRGBToLinearScale = 12.92f;
static const float kSRGBToLinearGammaOffset = 0.055f;
static const float kSRGBToLinearGammaScale = 1.055f;
static const float kSRGBToLinearGamma = 2.4f;

// Linear to sRGB constants
static const float kLinearToSRGBThreshold = 0.0031308f;
static const float kLinearToSRGBScale = 12.92f;
static const float kLinearToSRGBGammaScale = 1.055f;
static const float kLinearToSRGBGammaOffset = 0.055f;
static const float kLinearToSRGBGamma = 1.0f / 2.4f;

static inline float sRGBToLinear(float value) {
    float absValue = value > 0.0f ? value : -value;
    float result;
    if (absValue <= kSRGBToLinearThreshold) {
        result = absValue / kSRGBToLinearScale;
    } else {
        result = powf((absValue + kSRGBToLinearGammaOffset) / kSRGBToLinearGammaScale, kSRGBToLinearGamma);
    }
    return value < 0.0f ? -result : result;
}

static inline float linearToSRGB(float value) {
    float absValue = value > 0.0f ? value : -value;
    float result;
    if (absValue <= kLinearToSRGBThreshold) {
        result = absValue * kLinearToSRGBScale;
    } else {
        result = kLinearToSRGBGammaScale * powf(absValue, kLinearToSRGBGamma) - kLinearToSRGBGammaOffset;
    }
    return value < 0.0f ? -result : result;
}

ORBColor ORBColorMake(float red, float green, float blue, float alpha) {
    return (ORBColor){ red, green, blue, alpha };
}

ORBColor ORBColorMakeLinear(float red, float green, float blue, float alpha) {
    return (ORBColor){
        sRGBToLinear(red),
        sRGBToLinear(green),
        sRGBToLinear(blue),
        alpha
    };
}

ORBColor ORBColorToLinear(ORBColor color) {
    return (ORBColor){
        sRGBToLinear(color.red),
        sRGBToLinear(color.green),
        sRGBToLinear(color.blue),
        color.alpha
    };
}

ORBColor ORBColorFromLinear(ORBColor color) {
    return (ORBColor){
        linearToSRGB(color.red),
        linearToSRGB(color.green),
        linearToSRGB(color.blue),
        color.alpha
    };
}

bool ORBColorEqualToColor(ORBColor lhs, ORBColor rhs) {
    return lhs.red == rhs.red &&
           lhs.green == rhs.green &&
           lhs.blue == rhs.blue &&
           lhs.alpha == rhs.alpha;
}

#if ORB_TARGET_OS_DARWIN

#include <CoreGraphics/CGColorSpace.h>

ORBColor ORBColorFromComponents(CGColorSpaceRef colorSpace, const CGFloat *components, bool premultiplied) {
    size_t componentCount = premultiplied ? 2 : 1;
    return ORBColorFromComponents2(colorSpace, components, componentCount);
}

ORBColor ORBColorFromComponents2(CGColorSpaceRef colorSpace, const CGFloat *components, size_t componentCount) {
    // TODO: Implement proper color space conversion
    (void)colorSpace;
    (void)componentCount;
    return (ORBColor){
        static_cast<float>(components[0]),
        static_cast<float>(components[1]),
        static_cast<float>(components[2]),
        static_cast<float>(components[3])
    };
}

ORBColor ORBColorFromCGColor(CGColorRef color, bool premultiplied) {
    CGColorSpaceRef colorSpace = CGColorGetColorSpace(color);
    const CGFloat *components = CGColorGetComponents(color);
    size_t componentCount = premultiplied ? 2 : 1;
    return ORBColorFromComponents2(colorSpace, components, componentCount);
}

ORBColor ORBColorFromCGColor2(CGColorRef color, size_t componentCount) {
    CGColorSpaceRef colorSpace = CGColorGetColorSpace(color);
    const CGFloat *components = CGColorGetComponents(color);
    return ORBColorFromComponents2(colorSpace, components, componentCount);
}

CGColorRef ORBColorCopyCGColor(ORBColor color) {
    CGFloat components[4] = {
        static_cast<CGFloat>(color.red),
        static_cast<CGFloat>(color.green),
        static_cast<CGFloat>(color.blue),
        static_cast<CGFloat>(color.alpha)
    };
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    CGColorRef cgColor = CGColorCreate(colorSpace, components);
    CGColorSpaceRelease(colorSpace);
    return cgColor;
}

#endif /* ORB_TARGET_OS_DARWIN */
