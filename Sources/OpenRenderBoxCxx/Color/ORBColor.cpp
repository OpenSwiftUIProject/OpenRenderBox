//
//  ORBColor.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBColor.h>
#include <math.h>

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

ORBColor ORBColorMake(float red, float green, float blue, float alpha) ORB_NOEXCEPT {
    return (ORBColor){ red, green, blue, alpha };
}

ORBColor ORBColorMakeLinear(float red, float green, float blue, float alpha) ORB_NOEXCEPT {
    return (ORBColor){
        sRGBToLinear(red),
        sRGBToLinear(green),
        sRGBToLinear(blue),
        alpha
    };
}

ORBColor ORBColorToLinear(ORBColor color) ORB_NOEXCEPT {
    return (ORBColor){
        sRGBToLinear(color.red),
        sRGBToLinear(color.green),
        sRGBToLinear(color.blue),
        color.alpha
    };
}

ORBColor ORBColorFromLinear(ORBColor color) ORB_NOEXCEPT {
    return (ORBColor){
        linearToSRGB(color.red),
        linearToSRGB(color.green),
        linearToSRGB(color.blue),
        color.alpha
    };
}

bool ORBColorEqualToColor(ORBColor lhs, ORBColor rhs) ORB_NOEXCEPT {
    return lhs.red == rhs.red &&
           lhs.green == rhs.green &&
           lhs.blue == rhs.blue &&
           lhs.alpha == rhs.alpha;
}
