//
//  ORBTransition.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef ORB_ENUM(uint32_t, ORBTransitionMethod) {
    ORBTransitionMethodEmpty = 0,
    ORBTransitionMethodDiff = 1,
    ORBTransitionMethodForwards = 2,
    ORBTransitionMethodBackwards = 3,
    ORBTransitionMethodPrefix = 4,
    ORBTransitionMethodSuffix = 5,
    ORBTransitionMethodBinary = 6,
    ORBTransitionMethodNone = 7,
    ORBTransitionMethodPrefixAndSuffix = 8,
};

typedef ORB_OPTIONS(uint32_t, ORBTransitionEvents) {
    ORBTransitionEventsAdd = 1 << 0,
    ORBTransitionEventsRemove = 1 << 1,
    ORBTransitionEventsAddRemove = ORBTransitionEventsAdd | ORBTransitionEventsRemove,
};

typedef ORB_OPTIONS(uint32_t, ORBTransitionEffectFlags) {
    ORBTransitionEffectFlagsRemoveInverts = 1 << 0,
};

typedef ORB_ENUM(uint32_t, ORBTransitionEffectType) {
    ORBTransitionEffectTypeNone = 0,
    ORBTransitionEffectTypeAlpha = 1,
    ORBTransitionEffectTypeScaleCenter = 2,
    ORBTransitionEffectTypeTranslation = 3,
    ORBTransitionEffectTypeBlur = 4,
    ORBTransitionEffectTypeMatchMove = 5,
    ORBTransitionEffectTypeScaleOrigin = 6,
    ORBTransitionEffectTypeAnchorLeft = 7,
    ORBTransitionEffectTypeAnchorRight = 8,
    ORBTransitionEffectTypeAnchorUp = 9,
    ORBTransitionEffectTypeAnchorDown = 10,
    ORBTransitionEffectTypeSequenceLeft = 11,
    ORBTransitionEffectTypeSequenceRight = 12,
    ORBTransitionEffectTypeSequenceUp = 13,
    ORBTransitionEffectTypeSequenceDown = 14,
    ORBTransitionEffectTypeRelativeTranslation = 15,
    ORBTransitionEffectTypeRelativeBlur = 16,
    ORBTransitionEffectTypeAppearDisappear = 17,
    ORBTransitionEffectTypeReplace = 18,
    ORBTransitionEffectTypeSequenceForwards = 19,
    ORBTransitionEffectTypeSequenceBackwards = 20,
};

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END
