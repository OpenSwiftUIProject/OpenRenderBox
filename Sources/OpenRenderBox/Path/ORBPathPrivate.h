//
//  ORBPathPrivate.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBPath.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

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
} ORBPathCallbacks;

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

