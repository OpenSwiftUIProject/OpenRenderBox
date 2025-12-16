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

typedef struct ORB_BRIDGED_TYPE(id) ORBPathStorage * ORBPathStorageRef ORB_SWIFT_NAME(ORBPath.Storage);

struct ORBPathStorage;
struct ORBPath;

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

/// Callback function pointer types for ORBPathCallbacks
typedef void (* _Nullable ORBPathRetainCallback)(struct ORBPath path);
typedef void (* _Nullable ORBPathReleaseCallback)(struct ORBPath path);
typedef bool (* _Nullable ORBPathApplyFunction)(ORBPathStorageRef storage, void * _Nullable info, ORBPathApplyCallback _Nullable callback);
typedef bool (* _Nullable ORBPathIsEqualCallback)(ORBPathStorageRef storage, ORBPathStorageRef otherStorage);
typedef bool (* _Nullable ORBPathIsEmptyCallback)(ORBPathStorageRef storage);
typedef bool (* _Nullable ORBPathIsSingleRectCallback)(ORBPathStorageRef storage);
typedef uint32_t (* _Nullable ORBPathBezierOrderCallback)(ORBPathStorageRef storage);
#if ORB_TARGET_OS_DARWIN
typedef CGRect (* _Nullable ORBPathBoundingBoxCallback)(ORBPathStorageRef storage);
typedef CGPathRef _Nullable (* _Nullable ORBPathGetCGPathCallback)(ORBPathStorageRef storage);
#else
typedef void (* _Nullable ORBPathBoundingBoxCallback)(ORBPathStorageRef storage);
typedef void * _Nullable (* _Nullable ORBPathGetCGPathCallback)(ORBPathStorageRef storage);
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
} ORBPathCallbacks;

typedef struct ORBPath {
    ORBPathStorageRef storage;
    const ORBPathCallbacks * _Nullable callbacks;
} ORBPath;

#if ORB_TARGET_OS_DARWIN
/// Global callbacks for CGPath-backed paths
ORB_EXPORT
const ORBPathCallbacks ORBPathCGPathCallbacks;
#endif

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathRetain(ORBPath path) ORB_SWIFT_NAME(ORBPath.retain(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathRelease(ORBPath path) ORB_SWIFT_NAME(ORBPath.release(self:));

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

