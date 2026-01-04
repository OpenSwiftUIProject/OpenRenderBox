//
//  ORBColorSpace.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef ORB_ENUM(uint32_t, ORBColorSpace) {
    ORBColorSpaceDefault = 0,
    ORBColorSpaceSRGB = 1,
    ORBColorSpaceLinearSRGB = 2,
    ORBColorSpaceDisplayP3 = 3,
    ORBColorSpaceLinearDisplayP3 = 4,
} ORB_SWIFT_NAME(ORBColor.Space);

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

