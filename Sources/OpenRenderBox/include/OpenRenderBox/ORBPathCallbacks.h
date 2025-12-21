//
//  ORBPathCallbacks.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBPath.h>
#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

/// Callbacks structure for path operations
/// This allows different path storage types (CGPath, custom storage, etc.) to provide their own implementations
typedef struct ORBPathCallbacks {
    void * _Nullable reserved;              // 0x00: Reserved for future use

    void (* _Nullable retain)(const ORBPath *path);           // 0x08
    void (* _Nullable release)(const ORBPath *path);          // 0x10
    bool (* _Nullable apply)(const ORBPath *path, void * _Nullable info, ORBPathApplyCallback _Nullable callback); // 0x18
    bool (* _Nullable isEqual)(const ORBPath *path, const ORBPath *otherPath); // 0x20
    bool (* _Nullable isEmpty)(const ORBPath *path);          // 0x28
    bool (* _Nullable isSingleRect)(const ORBPath *path);     // 0x30
    uint32_t (* _Nullable bezierOrder)(const ORBPath *path);  // 0x38
    CGRect (* _Nullable boundingBox)(const ORBPath *path);    // 0x40
    #if ORB_TARGET_OS_DARWIN
    CGPathRef _Nullable (* _Nullable cgPath)(const ORBPath *path); // 0x48
    #else
    void * _Nullable (* _Nullable cgPath)(const ORBPath *path); // 0x48
    #endif
    void * _Nullable reserved2;             // 0x50
} ORBPathCallbacks ORB_SWIFT_NAME(ORBPath.Callbacks);

/// Global empty path callbacks (all null)
ORB_EXPORT
const ORBPathCallbacks ORBPathEmptyCallbacks;

#if ORB_TARGET_OS_DARWIN
/// Global callbacks for CGPath-backed paths
ORB_EXPORT
const ORBPathCallbacks ORBPathCGPathCallbacks;
#endif

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END
