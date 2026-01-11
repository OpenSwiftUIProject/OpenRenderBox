//
//  ORBColorMode.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBColorMode.h>

// Working color space lookup table
// Index: mode value (0-15), Value: ORBColorSpace
// 1 = SRGB, 2 = LinearSRGB
static const ORBColorSpace kColorModeWorkingColorSpaceTable[16] = {
    ORBColorSpaceSRGB,       // 0
    ORBColorSpaceLinearSRGB, // 1
    ORBColorSpaceLinearSRGB, // 2
    ORBColorSpaceSRGB,       // 3
    ORBColorSpaceSRGB,       // 4
    ORBColorSpaceSRGB,       // 5
    ORBColorSpaceLinearSRGB, // 6
    ORBColorSpaceLinearSRGB, // 7
    ORBColorSpaceLinearSRGB, // 8
    ORBColorSpaceSRGB,       // 9
    ORBColorSpaceLinearSRGB, // 10
    ORBColorSpaceSRGB,       // 11
    ORBColorSpaceSRGB,       // 12
    ORBColorSpaceLinearSRGB, // 13
    ORBColorSpaceSRGB,       // 14
    ORBColorSpaceLinearSRGB, // 15
};

ORBColorSpace ORBColorModeWorkingColorSpace(ORBColorMode mode) {
    if (mode < 0 || mode > 15) {
        return ORBColorSpaceDefault;
    }
    return kColorModeWorkingColorSpaceTable[mode];
}

bool ORBColorModeHasExtendedRange(ORBColorMode mode) {
    // Extended range modes: 2, 11, 12, 13
    // 0x3804 = 0011 1000 0000 0100 binary
    if (mode < 0 || mode > 15) {
        return false;
    }
    uint32_t mask = 0x3804;
    return (mask >> mode) & 1;
}