//
//  ORBSymbolAnimation.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

ORB_EXTERN_C_BEGIN

typedef ORB_ENUM(uint32_t, ORBSymbolAnimation) {
    ORBSymbolAnimationVariableValue = 0,
    ORBSymbolAnimationVariableColor = 1,
    ORBSymbolAnimationPulse = 2,
    ORBSymbolAnimationBounce = 3,
    ORBSymbolAnimationScale = 4,
    ORBSymbolAnimationAppear = 5,
    ORBSymbolAnimationReplace = 6,
    ORBSymbolAnimationInterpolate = 7,
    ORBSymbolAnimationPush = 8,
    ORBSymbolAnimationDisappear = 9,
    ORBSymbolAnimationWiggle = 10,
    ORBSymbolAnimationRotate = 11,
    ORBSymbolAnimationBreathe = 12,
};

typedef ORB_ENUM(int32_t, ORBSymbolScaleLevel) {
    ORBSymbolScaleLevelDown = -1,
    ORBSymbolScaleLevelNormal = 0,
    ORBSymbolScaleLevelUp = 1,
};

typedef ORB_OPTIONS(uint32_t, ORBSymbolAnimationBreatheFlags) {
    ORBSymbolAnimationBreatheFlagsOnlyScale = 1 << 0,
} ORB_SWIFT_NAME(ORBSymbolAnimation.BreatheOptions);

typedef ORB_OPTIONS(uint32_t, ORBSymbolAnimationBounceFlags) {
    ORBSymbolAnimationBounceFlagsUp = 1 << 0,
    ORBSymbolAnimationBounceFlagsDown = 1 << 1,
    ORBSymbolAnimationBounceFlagsContinuous = 1 << 4,
} ORB_SWIFT_NAME(ORBSymbolAnimation.BounceOptions);

typedef ORB_OPTIONS(uint32_t, ORBSymbolAnimationWiggleFlags) {
    ORBSymbolAnimationWiggleFlagsLocalized = 0x1,
    ORBSymbolAnimationWiggleFlagsRotational = 0x3,
    ORBSymbolAnimationWiggleFlagsForward = 0x10,
    ORBSymbolAnimationWiggleFlagsBackward = 0x20,
    ORBSymbolAnimationWiggleFlagsDirectionMask = 0xF0,
    ORBSymbolAnimationWiggleFlagsContinuous = 0x100,
} ORB_SWIFT_NAME(ORBSymbolAnimation.WiggleOptions);

typedef ORB_OPTIONS(uint32_t, ORBSymbolAnimationRotateFlags) {
    ORBSymbolAnimationRotateFlagsClockwise = 1 << 0,
    ORBSymbolAnimationRotateFlagsCounterClockwise = 1 << 1,
    ORBSymbolAnimationRotateFlagsContinuous = 1 << 4,
} ORB_SWIFT_NAME(ORBSymbolAnimation.RotateOptions);

typedef ORB_OPTIONS(uint32_t, ORBSymbolAnimationVariableColorFlags) {
    ORBSymbolAnimationVariableColorFlagsIterative = 1 << 0,
    ORBSymbolAnimationVariableColorFlagsHideInactiveLayers = 1 << 1,
    ORBSymbolAnimationVariableColorFlagsReversing = 1 << 3,
} ORB_SWIFT_NAME(ORBSymbolAnimation.VariableColorOptions);

typedef ORB_OPTIONS(uint32_t, ORBSymbolAnimationAppearDisappearFlags) {
    ORBSymbolAnimationAppearDisappearFlagsDown = 0x2,
    ORBSymbolAnimationAppearDisappearFlagsUp = 0x3,
    ORBSymbolAnimationAppearDisappearFlagsWholeSymbol = 0x10,
} ORB_SWIFT_NAME(ORBSymbolAnimation.AppearDisappearOptions);

typedef ORB_OPTIONS(uint32_t, ORBSymbolAnimationReplaceFlags) {
    ORBSymbolAnimationReplaceFlagsDown = 0x2,
    ORBSymbolAnimationReplaceFlagsUp = 0x3,
    ORBSymbolAnimationReplaceFlagsOffUp = 0x4,
    ORBSymbolAnimationReplaceFlagsWholeSymbol = 0x10,
    ORBSymbolAnimationReplaceFlagsNoSlashes = 0x20,
    ORBSymbolAnimationReplaceFlagsNoBadges = 0x40,
} ORB_SWIFT_NAME(ORBSymbolAnimation.ReplaceOptions);

ORB_EXTERN_C_END
