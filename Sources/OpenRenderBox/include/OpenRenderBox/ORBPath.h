//
//  ORBPath.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

ORB_IMPLICIT_BRIDGING_ENABLED

ORB_EXTERN_C_BEGIN

typedef struct ORB_BRIDGED_TYPE(id) ORBPath * ORBMutablePathRef;
typedef const struct ORB_BRIDGED_TYPE(id) ORBPath * ORBPathRef;

typedef struct ORB_BRIDGED_TYPE(id) ORBPathStorage * ORBPathStorageRef ORB_SWIFT_NAME(ORBPath.Storage);
typedef const struct ORB_BRIDGED_TYPE(id) ORBPathCallbacks * ORBPathCallbacksRef ORB_SWIFT_NAME(ORBPath.Callbacks);

struct ORBPath;
struct ORBPathStorage;
struct ORBPathCallbacks;

/// Path element type for path enumeration
typedef ORB_ENUM(int32_t, ORBPathElementType) {
    ORBPathElementMoveToPoint = 0,
    ORBPathElementAddLineToPoint = 1,
    ORBPathElementAddQuadCurveToPoint = 2,
    ORBPathElementAddCurveToPoint = 3,
    ORBPathElementCloseSubpath = 4,
};

/// An element of a path returned by path enumeration
struct ORBPathElement {
    ORBPathElementType type;
    const double * _Nullable points;
};
typedef struct ORBPathElement ORBPathElement;

/// Callback type for path element enumeration
/// Returns true to stop enumeration, false to continue
typedef bool (*ORBPathApplyCallback)(void * _Nullable info, ORBPathElement element, const void * _Nullable userInfo);

typedef struct ORBPath {
    ORBPathStorageRef _Nullable storage;
    ORBPathCallbacksRef _Nullable callbacks;
} ORBPath;

/// Global empty path (storage = null, callbacks = &ORBPathEmptyCallbacks)
ORB_EXPORT
const ORBPath ORBPathEmpty;

/// Global null path (storage = 0x1, callbacks = &ORBPathEmptyCallbacks)
ORB_EXPORT
const ORBPath ORBPathNull;

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathRetain(ORBPath path) ORB_SWIFT_NAME(ORBPath.retain(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathRelease(ORBPath path) ORB_SWIFT_NAME(ORBPath.release(self:));

// MARK: - Path Creation

#if ORB_TARGET_OS_DARWIN
ORB_EXPORT
ORBPath ORBPathMakeWithCGPath(CGPathRef _Nullable cgPath);

ORB_EXPORT
ORBPath ORBPathMakeRect(CGRect rect, const CGAffineTransform * _Nullable transform);

ORB_EXPORT
ORBPath ORBPathMakeEllipse(CGRect rect, const CGAffineTransform * _Nullable transform);

ORB_EXPORT
ORBPath ORBPathMakeRoundedRect(CGRect rect, CGFloat cornerWidth, CGFloat cornerHeight, const CGAffineTransform * _Nullable transform);

ORB_EXPORT
ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat bottomLeftRadius, CGFloat bottomRightRadius, CGFloat topRightRadius, const CGAffineTransform * _Nullable transform);
#endif

ORB_EXTERN_C_END

ORB_IMPLICIT_BRIDGING_DISABLED

ORB_ASSUME_NONNULL_END

