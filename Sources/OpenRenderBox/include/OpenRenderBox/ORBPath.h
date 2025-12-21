//
//  ORBPath.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>
#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

/// Path element type for path enumeration
typedef ORB_ENUM(int32_t, ORBPathElement) {
    ORBPathElementMoveToPoint = 0,
    ORBPathElementAddLineToPoint = 1,
    ORBPathElementAddQuadCurveToPoint = 2,
    ORBPathElementAddCurveToPoint = 3,
    ORBPathElementCloseSubpath = 4,

    ORBPathElementFixedRoundedRectCircular = 8,
    ORBPathElementFixedRoundedRectContinuous = 9,
};

/// Defines the shape of a rounded rectangle's corners.
typedef ORB_ENUM(int32_t, ORBPathRoundedCornerStyle) {
    /// Quarter-circle rounded rect corners.
    ORBPathRoundedCornerStyleCircular = 0,
    /// Continuous curvature rounded rect corners.
    ORBPathRoundedCornerStyleContinuous = 1,
} ORB_SWIFT_NAME(ORBPath.RoundedCornerStyle);

/// Callback type for path element enumeration
/// Returns true to stop enumeration, false to continue
typedef bool (*ORBPathApplyCallback)(void * info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo);

typedef struct ORBPathCallbacks ORBPathCallbacks;

typedef struct ORBPathStorage * ORBPathStorageRef ORB_SWIFT_STRUCT ORB_SWIFT_NAME(ORBPath.Storage);

typedef struct ORBPath {
    ORBPathStorageRef storage;
    const ORBPathCallbacks * callbacks;
} ORBPath;

/// Global empty path (storage = null, callbacks = &ORBPathEmptyCallbacks)
ORB_EXPORT
const ORBPath ORBPathEmpty ORB_SWIFT_NAME(ORBPath.empty);

/// Global null path (storage = 0x1, callbacks = &ORBPathEmptyCallbacks)
ORB_EXPORT
const ORBPath ORBPathNull ORB_SWIFT_NAME(ORBPath.null);

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathRetain(ORBPath path) ORB_SWIFT_NAME(ORBPath.retain(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathRelease(ORBPath path) ORB_SWIFT_NAME(ORBPath.release(self:));

#if ORB_TARGET_OS_DARWIN

// MARK: - Path Creation

ORB_EXPORT
ORBPath ORBPathMakeWithCGPath(CGPathRef cgPath) ORB_SWIFT_NAME(ORBPath.init(cgPath:));

ORB_EXPORT
ORBPath ORBPathMakeRect(CGRect rect, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(rect:transform:));

ORB_EXPORT
ORBPath ORBPathMakeEllipse(CGRect rect, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(ellipseIn:transform:));

ORB_EXPORT
ORBPath ORBPathMakeRoundedRect(CGRect rect, CGFloat cornerWidth, CGFloat cornerHeight, ORBPathRoundedCornerStyle style, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(roundedRect:cornerWidth:cornerHeight:style:transform:));

ORB_EXPORT
ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat bottomLeftRadius, CGFloat bottomRightRadius, CGFloat topRightRadius, ORBPathRoundedCornerStyle style, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(roundedRect:topLeftRadius:bottomLeftRadius:bottomRightRadius:topRightRadius:style:transform:));

// MARK: - CGPath Interoperability

ORB_EXPORT
CGPathRef ORBPathCopyCGPath(ORBPath path) ORB_SWIFT_NAME(getter:ORBPath.cgPath(self:));

// MARK: - Point Containment

ORB_EXPORT
bool ORBPathContainsPoint(ORBPath path, CGPoint point, bool eoFill) ORB_SWIFT_NAME(ORBPath.contains(self:point:eoFill:));

ORB_EXPORT
bool ORBPathContainsPoints(ORBPath path, uint64_t count, const CGPoint *points, bool eoFill, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.containsPoints(self:count:points:eoFill:transform:));
#endif

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END
