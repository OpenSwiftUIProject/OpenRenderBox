//
//  ORBColorMode.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBColorSpace.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef int32_t ORBColorMode ORB_SWIFT_STRUCT ORB_SWIFT_NAME(ORBColor.Mode);

ORB_EXPORT ORBColorSpace ORBColorModeWorkingColorSpace(ORBColorMode mode) ORB_SWIFT_NAME(getter:ORBColorMode.workingColorSpace(self:));
ORB_EXPORT bool ORBColorModeHasExtendedRange(ORBColorMode mode) ORB_SWIFT_NAME(getter:ORBColorMode.hasExtendedRange(self:));

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

