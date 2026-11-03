//
//  ORBBlendMode.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef enum ORBBlendMode : int32_t {
    ORBBlendModeNormal = 0,
    ORBBlendModeMultiply = 1,
    ORBBlendModeScreen = 2,
    ORBBlendModeOverlay = 3,
    ORBBlendModeDarken = 4,
    ORBBlendModeLighten = 5,
    ORBBlendModeColorDodge = 6,
    ORBBlendModeColorBurn = 7,
    ORBBlendModeSoftLight = 8,
    ORBBlendModeHardLight = 9,
    ORBBlendModeDifference = 10,
    ORBBlendModeExclusion = 11,
    ORBBlendModeHue = 12,
    ORBBlendModeSaturation = 13,
    ORBBlendModeColor = 14,
    ORBBlendModeLuminosity = 15,
    ORBBlendModeClear = 16,
    ORBBlendModeCopy = 17,
    ORBBlendModeSourceIn = 18,
    ORBBlendModeSourceOut = 19,
    ORBBlendModeSourceAtop = 20,
    ORBBlendModeDestinationOver = 21,
    ORBBlendModeDestinationIn = 22,
    ORBBlendModeDestinationOut = 23,
    ORBBlendModeDestinationAtop = 24,
    ORBBlendModeXOR = 25,
    ORBBlendModePlusDarker = 26,
    ORBBlendModePlusLighter = 27,
    ORBBlendModeLinearDodge = 1000,
    ORBBlendModeLinearBurn = 1001,
    ORBBlendModeLinearLight = 1002,
    ORBBlendModePinLight = 1003,
    ORBBlendModeSubtract = 1004,
    ORBBlendModeDivide = 1005,
    ORBBlendModeMaximum = 1006,
    ORBBlendModeAdd = 1007,
    ORBBlendModeSubtractSource = 1008,
    ORBBlendModeSubtractDestination = 1009,
    ORBBlendModeDarkenSourceOver = 1010,
    ORBBlendModeLightenSourceOver = 1011,
    ORBBlendModeMinimum = 1012,
    ORBBlendModeMinimumInverse = 1013,
    ORBBlendModePassThrough = 2000,
} ORBBlendMode;

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END
