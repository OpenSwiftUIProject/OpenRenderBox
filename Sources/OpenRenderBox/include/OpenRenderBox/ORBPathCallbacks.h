//
//  ORBPathCallbacks.h
//  OpenRenderBox
//
//  Audited for 6.5.1
//  Status: Complete

#pragma once

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBPath.h>
#include <CoreFoundation/CoreFoundation.h>
#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

/// Callbacks structure for path operations
/// This allows different path storage types (CGPath, custom storage, etc.) to provide their own implementations
typedef struct ORB_SWIFT_NAME(ORBPath.Callbacks) ORBPathCallbacks {
    void (* _Nullable unknown1)(const void * object); // 0x00
    const void * _Nonnull (* _Nullable retain)(const void *object); // 0x08
    void (* _Nullable release)(const void *object); // 0x10
    bool (* _Nullable apply)(const void *object, void * info, ORBPathApplyCallback _Nullable callback); // 0x18
    bool (* _Nullable isEqual)(const void *object, const void *otherObject); // 0x20
    bool (* _Nullable isEmpty)(const void *object); // 0x28
    bool (* _Nullable isSingleElement)(const void *object); // 0x30
    uint32_t (* _Nullable bezierOrder)(const void *object); // 0x38
    CGRect (* _Nullable boundingRect)(const void *object); // 0x40
    #if ORB_TARGET_OS_DARWIN
    CGPathRef _Nullable (* _Nullable cgPath)(const void *object); // 0x48
    #else
    void * _Nullable (* _Nullable cgPath)(const void *object); // 0x48
    #endif
    void (* _Nullable unknown2)(const void *); // 0x50
} ORBPathCallbacks;

#if ORB_TARGET_OS_DARWIN
/// Global callbacks for CGPath-backed paths
ORB_EXPORT
const ORBPathCallbacks ORBPathCGPathCallbacks ORB_SWIFT_NAME(ORBPathCallbacks.cgPath);
#endif

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END
