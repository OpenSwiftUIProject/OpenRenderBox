//
//  ORBColorMode.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBColorMode.h>

ORBColorSpace ORBColorModeWorkingColorSpace(ORBColorMode mode) {
    switch (mode) {
        case ORBColorMode0:
        case ORBColorMode3:
        case ORBColorMode4:
        case ORBColorMode5:
        case ORBColorMode9:
        case ORBColorMode11:
        case ORBColorMode12:
        case ORBColorMode14:
            return ORBColorSpaceSRGB;
        case ORBColorMode1:
        case ORBColorMode2:
        case ORBColorMode6:
        case ORBColorMode7:
        case ORBColorMode8:
        case ORBColorMode10:
        case ORBColorMode13:
        case ORBColorMode15:
            return ORBColorSpaceLinearSRGB;
        default:
            return ORBColorSpaceDefault;
    }
}

bool ORBColorModeHasExtendedRange(ORBColorMode mode) {
    switch (mode) {
        case ORBColorMode2:
        case ORBColorMode11:
        case ORBColorMode12:
        case ORBColorMode13:
            return true;
        default:
            return false;
    }
}