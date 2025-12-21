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

/// Defines the shape of a rounded rectangle's corners.
typedef ORB_ENUM(int32_t, ORBRoundedCornerStyle) {
    /// Quarter-circle rounded rect corners.
    ORBRoundedCornerStyleCircular = 0,
    /// Continuous curvature rounded rect corners.
    ORBRoundedCornerStyleContinuous = 1,
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

/// Callback function pointer types for ORBPathCallbacks
typedef void (* _Nullable ORBPathRetainCallback)(ORBPathRef path);
typedef void (* _Nullable ORBPathReleaseCallback)(ORBPathRef path);
typedef bool (* _Nullable ORBPathApplyFunction)(ORBPathRef path, void * _Nullable info, ORBPathApplyCallback _Nullable callback);
typedef bool (* _Nullable ORBPathIsEqualCallback)(ORBPathRef path, ORBPathRef otherPath);
typedef bool (* _Nullable ORBPathIsEmptyCallback)(ORBPathRef path);
typedef bool (* _Nullable ORBPathIsSingleRectCallback)(ORBPathRef path);
typedef uint32_t (* _Nullable ORBPathBezierOrderCallback)(ORBPathRef path);
#if ORB_TARGET_OS_DARWIN
typedef CGRect (* _Nullable ORBPathBoundingBoxCallback)(ORBPathRef path);
typedef CGPathRef _Nullable (* _Nullable ORBPathGetCGPathCallback)(ORBPathRef path);
#else
typedef void (* _Nullable ORBPathBoundingBoxCallback)(ORBPathRef path);
typedef void * _Nullable (* _Nullable ORBPathGetCGPathCallback)(ORBPathRef path);
#endif

/// Callbacks structure for path operations
/// This allows different path storage types (CGPath, custom storage, etc.) to provide their own implementations
typedef struct ORBPathCallbacks {
    void * _Nullable reserved;              // 0x00: Reserved for future use
    ORBPathRetainCallback retain;           // 0x08: Retain callback
    ORBPathReleaseCallback release;         // 0x10: Release callback
    ORBPathApplyFunction apply;             // 0x18: Enumerate path elements
    ORBPathIsEqualCallback isEqual;         // 0x20: Compare two paths
    ORBPathIsEmptyCallback isEmpty;         // 0x28: Check if path is empty
    ORBPathIsSingleRectCallback isSingleRect; // 0x30: Check if path is a single rectangle
    ORBPathBezierOrderCallback bezierOrder; // 0x38: Get bezier order (1=linear, 2=quad, 3=cubic)
    ORBPathBoundingBoxCallback boundingBox; // 0x40: Get bounding box
    ORBPathGetCGPathCallback cgPath;        // 0x48: Get CGPath representation
    void * _Nullable reserved2;             // 0x50: Reserved for future use
} ORBPathCallbacks;

typedef struct ORBPath {
    ORBPathStorageRef storage;
    ORBPathCallbacksRef callbacks;
} ORBPath;

/// Global empty path callbacks (all null)
ORB_EXPORT
const ORBPathCallbacks ORBPathEmptyCallbacks;

#if ORB_TARGET_OS_DARWIN
/// Global callbacks for CGPath-backed paths
ORB_EXPORT
const ORBPathCallbacks ORBPathCGPathCallbacks;
#endif

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
ORBPath ORBPathMakeWithCGPath(CGPathRef cgPath) ORB_SWIFT_NAME(ORBPath.init(cgPath:));

ORB_EXPORT
ORBPath ORBPathMakeRect(CGRect rect, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(rect:transform:));

ORB_EXPORT
ORBPath ORBPathMakeEllipse(CGRect rect, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(ellipseIn:transform:));

ORB_EXPORT
ORBPath ORBPathMakeRoundedRect(CGRect rect, CGFloat cornerWidth, CGFloat cornerHeight, ORBRoundedCornerStyle style, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(roundedRect:cornerWidth:cornerHeight:style:transform:));

ORB_EXPORT
ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat bottomLeftRadius, CGFloat bottomRightRadius, CGFloat topRightRadius, ORBRoundedCornerStyle style, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(roundedRect:topLeftRadius:bottomLeftRadius:bottomRightRadius:topRightRadius:style:transform:));

ORB_EXPORT
CGPathRef ORBPathCopyCGPath(ORBPath path) ORB_SWIFT_NAME(getter:ORBPath.cgPath(self:));
#endif

ORB_EXTERN_C_END

ORB_IMPLICIT_BRIDGING_DISABLED

ORB_ASSUME_NONNULL_END

