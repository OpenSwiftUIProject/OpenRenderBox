//
//  ORBPathCallbacks.h
//  OpenRenderBox
//
//  Audited for 6.5.1
//  Status: Complete

#pragma once

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBPath.h>
#include <stddef.h>
#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

/// Flags for path callbacks
typedef struct ORB_SWIFT_NAME(ORBPath.Callbacks.Flags) ORBPathCallbacksFlags {
    uint8_t unknown0;
    uint8_t unknown1;
    bool isExtended;
    uint8_t padding[5];
} ORBPathCallbacksFlags;

/// Callbacks structure for path operations
/// This allows different path storage types (CGPath, custom storage, etc.) to provide their own implementations
typedef struct ORB_SWIFT_NAME(ORBPath.Callbacks) ORBPathCallbacks {
    ORBPathCallbacksFlags flags;
    const void * _Nonnull (* _Nullable retain)(const void *object);
    void (* _Nullable release)(const void *object);
    bool (* _Nullable apply)(const void *object, void * info, ORBPathApplyCallback _Nullable callback);
    bool (* _Nullable isEqual)(const void *object, const void *otherObject);
    bool (* _Nullable isEmpty)(const void *object);
    bool (* _Nullable isSingleElement)(const void *object);
    uint32_t (* _Nullable bezierOrder)(const void *object);
    CGRect (* _Nullable boundingRect)(const void *object);
    #if ORB_TARGET_OS_DARWIN
    CGPathRef _Nullable (* _Nullable cgPath)(const void *object);
    #else
    void * _Nullable (* _Nullable cgPath)(const void *object);
    #endif
    const struct ORBPathCallbacks * _Nullable (* _Nullable next)(const void *object);
} ORBPathCallbacks;

// static_assert without message requires C23 or C++17
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
static_assert(sizeof(struct ORBPathCallbacks) == 0x58);
static_assert(offsetof(struct ORBPathCallbacks, flags) == 0x00);
static_assert(offsetof(struct ORBPathCallbacks, retain) == 0x08);
static_assert(offsetof(struct ORBPathCallbacks, release) == 0x10);
static_assert(offsetof(struct ORBPathCallbacks, apply) == 0x18);
static_assert(offsetof(struct ORBPathCallbacks, isEqual) == 0x20);
static_assert(offsetof(struct ORBPathCallbacks, isEmpty) == 0x28);
static_assert(offsetof(struct ORBPathCallbacks, isSingleElement) == 0x30);
static_assert(offsetof(struct ORBPathCallbacks, bezierOrder) == 0x38);
static_assert(offsetof(struct ORBPathCallbacks, boundingRect) == 0x40);
static_assert(offsetof(struct ORBPathCallbacks, cgPath) == 0x48);
static_assert(offsetof(struct ORBPathCallbacks, next) == 0x50);
#endif

/// Extended callbacks structure with additional extended callbacks argument
typedef struct ORB_SWIFT_NAME(ORBPath.CallbacksExtended) ORBPathCallbacksExtended {
    ORBPathCallbacksFlags flags;
    const void * _Nonnull (* _Nullable retain)(const void *object);
    void (* _Nullable release)(const void *object);
    bool (* _Nullable apply)(const void *object, void * info, ORBPathApplyCallback _Nullable callback, const struct ORBPathCallbacksExtended *extended);
    bool (* _Nullable isEqual)(const void *object, const void *otherObject, const struct ORBPathCallbacksExtended *extended);
    bool (* _Nullable isEmpty)(const void *object, const struct ORBPathCallbacksExtended *extended);
    bool (* _Nullable isSingleElement)(const void *object, const struct ORBPathCallbacksExtended *extended);
    uint32_t (* _Nullable bezierOrder)(const void *object, const struct ORBPathCallbacksExtended *extended);
    CGRect (* _Nullable boundingRect)(const void *object, const struct ORBPathCallbacksExtended *extended);
    #if ORB_TARGET_OS_DARWIN
    CGPathRef _Nullable (* _Nullable cgPath)(const void *object, const struct ORBPathCallbacksExtended *extended);
    #else
    void * _Nullable (* _Nullable cgPath)(const void *object, const struct ORBPathCallbacksExtended *extended);
    #endif
    const struct ORBPathCallbacksExtended * _Nullable (* _Nullable next)(const void *object, const struct ORBPathCallbacksExtended *extended);
} ORBPathCallbacksExtended;

// static_assert without message requires C23 or C++17
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
static_assert(sizeof(struct ORBPathCallbacksExtended) == 0x58);
static_assert(offsetof(struct ORBPathCallbacksExtended, flags) == 0x00);
static_assert(offsetof(struct ORBPathCallbacksExtended, retain) == 0x08);
static_assert(offsetof(struct ORBPathCallbacksExtended, release) == 0x10);
static_assert(offsetof(struct ORBPathCallbacksExtended, apply) == 0x18);
static_assert(offsetof(struct ORBPathCallbacksExtended, isEqual) == 0x20);
static_assert(offsetof(struct ORBPathCallbacksExtended, isEmpty) == 0x28);
static_assert(offsetof(struct ORBPathCallbacksExtended, isSingleElement) == 0x30);
static_assert(offsetof(struct ORBPathCallbacksExtended, bezierOrder) == 0x38);
static_assert(offsetof(struct ORBPathCallbacksExtended, boundingRect) == 0x40);
static_assert(offsetof(struct ORBPathCallbacksExtended, cgPath) == 0x48);
static_assert(offsetof(struct ORBPathCallbacksExtended, next) == 0x50);
#endif

#if ORB_TARGET_OS_DARWIN
/// Global callbacks for CGPath-backed paths
ORB_EXPORT
const ORBPathCallbacks ORBPathCGPathCallbacks ORB_SWIFT_NAME(ORBPathCallbacks.cgPath);
#endif

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

#endif /* OPENRENDERBOX_CF_CGTYPES */
