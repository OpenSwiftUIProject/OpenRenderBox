//
//  ORBPath.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES
#include <stddef.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

/// Path element type for path enumeration
typedef ORB_ENUM(int32_t, ORBPathElement) {
    ORBPathElementMoveToPoint = 0,
    ORBPathElementAddLineToPoint = 1,
    ORBPathElementAddQuadCurveToPoint = 2,
    ORBPathElementAddCurveToPoint = 3,
    ORBPathElementCloseSubpath = 4,
    ORBPathElementRect = 5,
    ORBPathElementRoundedRect = 6,
    ORBPathElementEllipse = 7,
    ORBPathElementFixedRoundedRectCircular = 8,
    ORBPathElementFixedRoundedRectContinuous = 9,
    ORBPathElementUnevenRoundedRectCircular = 10,
    ORBPathElementUnevenRoundedRectContinuous = 11,
    ORBPathElementRelativeArc = 12,
    ORBPathElementArc = 13,
    ORBPathElementArcCounterclockwise = 14,
    ORBPathElementArcTo = 15,
    ORBPathElementEnd = 16,
    ORBPathElementTransformed = 17,
    ORBPathElementTrimmed = 18,
    ORBPathElementStroked = 19,
    ORBPathElementBoolean = 20,
    ORBPathElementBooleanSeparator = 21,
    ORBPathElementDilateGlyph = 22,
    ORBPathElementOffset = 23,
    ORBPathElementClipStroke = 24,

    ORBPathElementInvalid = 25,
} ORB_SWIFT_NAME(ORBPath.Element);

ORB_EXPORT
const uint8_t __ORBPathElementArgumentCount[25];

/// Defines the shape of a rounded rectangle's corners.
typedef ORB_ENUM(int32_t, ORBPathRoundedCornerStyle) {
    /// Quarter-circle rounded rect corners.
    ORBPathRoundedCornerStyleCircular = 0,
    /// Continuous curvature rounded rect corners.
    ORBPathRoundedCornerStyleContinuous = 1,
} ORB_SWIFT_NAME(ORBPath.RoundedCornerStyle);

/// Callback type for path element enumeration
/// Returns true to continue enumeration, false to stop
typedef bool (*ORBPathApplyCallback)(void * info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo);

typedef struct ORBPathCallbacks ORBPathCallbacks ORB_SWIFT_NAME(ORBPath.ApplyCallback);

typedef struct ORBPathStorage * ORBPathStorageRef ORB_SWIFT_STRUCT ORB_SWIFT_NAME(ORBPath.Storage);

typedef struct ORBPath {
    ORBPathStorageRef storage;
    const ORBPathCallbacks * callbacks;
} ORBPath;

/// A primitive shape recognized by ``ORBPathApplyShape``.
typedef ORB_ENUM(int32_t, ORBPathShapeType) {
    ORBPathShapeTypeRect = 0,
    ORBPathShapeTypeEllipse = 1,
    ORBPathShapeTypeLine = 2,
    ORBPathShapeTypeFixedRoundedRect = 3,
    ORBPathShapeTypeUnevenRoundedRect = 4,
    ORBPathShapeTypeNested = 5,
    ORBPathShapeTypeStroked = 6,
    ORBPathShapeTypeTrimmed = 7,
    ORBPathShapeTypeOffset = 8,
};

typedef struct ORBPathRectShape {
    CGAffineTransform transform;
    CGRect rect;
} ORBPathRectShape;

typedef struct ORBPathFixedRoundedRectShape {
    CGAffineTransform transform;
    CGRect rect;
    CGSize cornerSize;
    ORBPathRoundedCornerStyle style;
} ORBPathFixedRoundedRectShape;

typedef struct ORBPathUnevenRoundedRectShape {
    CGAffineTransform transform;
    CGRect rect;
    CGFloat topLeftRadius;
    CGFloat topRightRadius;
    CGFloat bottomRightRadius;
    CGFloat bottomLeftRadius;
    ORBPathRoundedCornerStyle style;
} ORBPathUnevenRoundedRectShape;

typedef struct ORBPathLineShape {
    CGPoint start;
    CGPoint end;
} ORBPathLineShape;

typedef struct ORBPathStrokeStyle {
    CGFloat lineWidth;
    int32_t lineCap;
    int32_t lineJoin;
    CGFloat miterLimit;
    CGFloat dashPhase;
    uint64_t dashCount;
    const CGFloat * _Nullable dash;
} ORBPathStrokeStyle;

typedef struct ORBPathStrokedShape {
    ORBPath path;
    ORBPathStrokeStyle style;
} ORBPathStrokedShape;

typedef struct ORBPathNestedShape {
    ORBPath path;
    CGAffineTransform transform;
} ORBPathNestedShape;

typedef struct ORBPathTrimmedShape {
    ORBPath path;
    CGFloat from;
    CGFloat to;
} ORBPathTrimmedShape;

typedef union ORBPathShape {
    ORBPathRectShape rect;
    ORBPathRectShape ellipse;
    ORBPathLineShape line;
    ORBPathFixedRoundedRectShape fixedRoundedRect;
    ORBPathUnevenRoundedRectShape unevenRoundedRect;
    ORBPathStrokedShape stroked;
    ORBPathNestedShape nested;
    ORBPathTrimmedShape trimmed;
} ORBPathShape;

#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
static_assert(sizeof(ORBPath) == 0x10);
static_assert(offsetof(ORBPathUnevenRoundedRectShape, rect) == 0x30);
static_assert(offsetof(ORBPathUnevenRoundedRectShape, topLeftRadius) == 0x50);
static_assert(offsetof(ORBPathUnevenRoundedRectShape, style) == 0x70);
static_assert(sizeof(ORBPathShape) == 0x78);
#endif

typedef void (*ORBPathApplyShapeCallback)(void * info, ORBPathShapeType type, const ORBPathShape *shape);

/// Global empty path (storage = null, callbacks = &ORBPathEmptyCallbacks)
ORB_EXPORT
const ORBPath ORBPathEmpty ORB_SWIFT_NAME(ORBPath.empty);

/// Global null path (storage = 0x1, callbacks = &ORBPathEmptyCallbacks)
ORB_EXPORT
const ORBPath ORBPathNull ORB_SWIFT_NAME(ORBPath.null);

ORB_EXPORT
ORBPath ORBPathRetain(ORBPath path) ORB_SWIFT_NAME(ORBPath.retain(self:));

ORB_EXPORT
void ORBPathRelease(ORBPath path) ORB_SWIFT_NAME(ORBPath.release(self:));

// MARK: - Path Creation

#if ORB_TARGET_OS_DARWIN
ORB_EXPORT
ORBPath ORBPathMakeWithCGPath(CGPathRef _Nullable cgPath) ORB_SWIFT_NAME(ORBPath.init(cgPath:));

ORB_EXPORT
ORBPath ORBPathMakeRect(CGRect rect, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(rect:transform:));

ORB_EXPORT
ORBPath ORBPathMakeEllipse(CGRect rect, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(ellipseIn:transform:));

ORB_EXPORT
ORBPath ORBPathMakeRoundedRect(CGRect rect, CGFloat cornerWidth, CGFloat cornerHeight, ORBPathRoundedCornerStyle style, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(roundedRect:cornerWidth:cornerHeight:style:transform:));

ORB_EXPORT
ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat topRightRadius, CGFloat bottomRightRadius, CGFloat bottomLeftRadius, ORBPathRoundedCornerStyle style, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.init(roundedRect:topLeftRadius:topRightRadius:bottomRightRadius:bottomLeftRadius:style:transform:));
#endif

ORB_EXPORT
ORBPath ORBPathCopyTransformedPath(ORBPath path, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.copy(self:transform:));

ORB_EXPORT
ORBPath ORBPathCopyStrokedPath(ORBPath path, CGFloat lineWidth, int32_t lineCap, int32_t lineJoin, CGFloat miterLimit, CGFloat dashPhase, const CGFloat * _Nullable dash, size_t dashCount) ORB_SWIFT_NAME(ORBPath.stroked(self:lineWidth:lineCap:lineJoin:miterLimit:dashPhase:dash:dashCount:));

ORB_EXPORT
ORBPath ORBPathCopyTrimmedPath(ORBPath path, CGFloat from, CGFloat to) ORB_SWIFT_NAME(ORBPath.trimmed(self:from:to:));

static ORB_INLINE bool ORBPathIsNull(ORBPath path) ORB_SWIFT_NAME(getter:ORBPath.isNull(self:)) {
    return path.storage == ORBPathNull.storage && path.callbacks == ORBPathNull.callbacks;
}


// MARK: - Path Operation

ORB_EXPORT
bool ORBPathIsEmpty(ORBPath path) ORB_SWIFT_NAME(getter:ORBPath.isEmpty(self:));

ORB_EXPORT
bool ORBPathIsSingleElement(ORBPath path) ORB_SWIFT_NAME(getter:ORBPath.isSingleElement(self:));

ORB_EXPORT
size_t ORBPathGetBezierOrder(ORBPath path) ORB_SWIFT_NAME(getter:ORBPath.bezierOrder(self:));

ORB_EXPORT
bool ORBPathApplyElements(ORBPath path, void * info, _Nullable ORBPathApplyCallback callback) ORB_SWIFT_NAME(ORBPath.apply(self:info:callback:));

ORB_EXPORT
bool ORBPathApplyShape(ORBPath path, bool allowLine, void * info, ORBPathApplyShapeCallback callback) ORB_SWIFT_NAME(ORBPath.applyShape(self:allowLine:info:callback:));

ORB_EXPORT
bool ORBPathEqualToPath(ORBPath lhs, ORBPath rhs) ORB_SWIFT_NAME(ORBPath.isEqual(self:to:));

// MARK: - CGPath Interoperability

#if ORB_TARGET_OS_DARWIN
ORB_EXPORT
CGPathRef ORBPathCopyCGPath(ORBPath path) ORB_SWIFT_NAME(getter:ORBPath.cgPath(self:));

ORB_EXPORT
CGRect ORBPathGetBoundingRect(ORBPath path) ORB_SWIFT_NAME(getter:ORBPath.boundingRect(self:));
#endif

// MARK: - Point Containment
#if ORB_TARGET_OS_DARWIN
ORB_EXPORT
bool ORBPathContainsPoint(ORBPath path, CGPoint point, bool eoFill) ORB_SWIFT_NAME(ORBPath.contains(self:point:eoFill:));

ORB_EXPORT
uint64_t ORBPathContainsPoints(ORBPath path, uint64_t count, const CGPoint *points, bool eoFill, const CGAffineTransform * _Nullable transform) ORB_SWIFT_NAME(ORBPath.containsPoints(self:count:points:eoFill:transform:));
#endif

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

#endif /* OPENRENDERBOX_CF_CGTYPES */
