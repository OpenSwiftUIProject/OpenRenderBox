//
//  ORBColorSpace.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef CF_ENUM(uint32_t, ORBColorSpace) {
    ORBColorSpaceInvalid = 0,
    ORBColorSpaceSRGB = 1,
    ORBColorSpaceExtendedLinear = 2,
};

typedef CF_ENUM(uint32_t, ORBColorMode) {
    ORBColorMode0 = 0,
    ORBColorMode1 = 1,
    ORBColorMode2 = 2,
    ORBColorMode3 = 3,
    ORBColorMode4 = 4,
    ORBColorMode5 = 5,
    ORBColorMode6 = 6,
    ORBColorMode7 = 7,
    ORBColorMode8 = 8,
    ORBColorMode9 = 9,
    ORBColorMode10 = 10,
    ORBColorMode11 = 11,
    ORBColorMode12 = 12,
    ORBColorMode13 = 13,
    ORBColorMode14 = 14,
    ORBColorMode15 = 15,
};

ORB_EXPORT
ORBColorSpace ORBColorModeWorkingColorSpace(ORBColorMode mode) ORB_NOEXCEPT;

ORB_EXPORT
bool ORBColorModeHasExtendedRange(ORBColorMode mode) ORB_NOEXCEPT;

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

