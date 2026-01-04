//
//  ORBColorSpace.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBColorSpace.h>

//static const ORBColorSpace kColorModeWorkingColorSpaceTable[16] = {
//    ORBColorSpaceSRGB,           // 0
//    ORBColorSpaceExtendedLinear, // 1
//    ORBColorSpaceExtendedLinear, // 2
//    ORBColorSpaceSRGB,           // 3
//    ORBColorSpaceSRGB,           // 4
//    ORBColorSpaceSRGB,           // 5
//    ORBColorSpaceExtendedLinear, // 6
//    ORBColorSpaceExtendedLinear, // 7
//    ORBColorSpaceExtendedLinear, // 8
//    ORBColorSpaceSRGB,           // 9
//    ORBColorSpaceExtendedLinear, // 10
//    ORBColorSpaceSRGB,           // 11
//    ORBColorSpaceSRGB,           // 12
//    ORBColorSpaceExtendedLinear, // 13
//    ORBColorSpaceSRGB,           // 14
//    ORBColorSpaceExtendedLinear, // 15
//};
//
//ORBColorSpace ORBColorModeWorkingColorSpace(ORBColorMode mode) {
//    if (mode > 15) {
//        return ORBColorSpaceInvalid;
//    }
//    return kColorModeWorkingColorSpaceTable[mode];
//}
//
//bool ORBColorModeHasExtendedRange(ORBColorMode mode) {
//    // 0x3804 = 0011 1000 0000 0100 binary
//    // Extended range modes: 2, 11, 12, 13
//    if (mode >= 14) {
//        return false;
//    }
//    uint32_t mask = 0x3804;
//    return (mask >> mode) & 1;
//}
//
