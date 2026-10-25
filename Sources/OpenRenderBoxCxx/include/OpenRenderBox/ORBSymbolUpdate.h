//
//  ORBSymbolUpdate.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBBlendMode.h>
#include <OpenRenderBox/ORBColor.h>
#include <OpenRenderBox/ORBDrawingState.h>
#include <OpenRenderBox/ORBPath.h>

#if ORB_OBJC_FOUNDATION
#include <Foundation/Foundation.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

typedef ORB_OPTIONS(uint32_t, ORBSymbolUpdateFlags) {
    ORBSymbolUpdateFlagsSwapAxes = 1 << 0,
    ORBSymbolUpdateFlagsFlipX = 1 << 1,
    ORBSymbolUpdateFlagsFlipY = 1 << 2,
    ORBSymbolUpdateFlagsMergeLayers = 1 << 4,
    // Apply the glyph's vertical alignment offset to each layer transform.
    ORBSymbolUpdateFlagsApplyVerticalOffset = 1 << 5,
};

typedef ORB_OPTIONS(uint32_t, ORBSymbolUpdateLayerFlags) {
    ORBSymbolUpdateLayerFlagsIncludeClips = 1 << 0,
};

typedef ORB_ENUM(uint32_t, ORBSymbolStyle) {
    ORBSymbolStylePrimary = 0,
    ORBSymbolStyleSecondary = 5,
    ORBSymbolStyleTertiary = 6,
    ORBSymbolStyleQuaternary = 7,
    ORBSymbolStyleQuinary = 8,
    ORBSymbolStyleNamedColor = 9,
};

typedef struct _ORBSymbolUpdate *ORBSymbolUpdate ORB_SWIFT_STRUCT;

// The update owns all layer data. A layer is valid until endUpdate:.
typedef struct ORBSymbolUpdateLayer {
    ORBSymbolUpdate update;
    intptr_t group;
    intptr_t index;
} ORBSymbolUpdateLayer;

#if ORB_OBJC_FOUNDATION

ORB_EXTERN_C_BEGIN

ORB_EXPORT double ORBSymbolUpdateNextTime(ORBSymbolUpdate update) ORB_SWIFT_NAME(getter:ORBSymbolUpdate.nextTime(self:));
ORB_EXPORT uint32_t ORBSymbolUpdateStyleMask(ORBSymbolUpdate update) ORB_SWIFT_NAME(getter:ORBSymbolUpdate.styleMask(self:));
ORB_EXPORT intptr_t ORBSymbolUpdateGroupCount(ORBSymbolUpdate update) ORB_SWIFT_NAME(getter:ORBSymbolUpdate.groupCount(self:));
ORB_EXPORT intptr_t ORBSymbolUpdateGroupSize(ORBSymbolUpdate update, intptr_t group) ORB_SWIFT_NAME(ORBSymbolUpdate.groupSize(self:group:));
ORB_EXPORT CGImageRef _Nullable ORBSymbolUpdateTemplateImage(ORBSymbolUpdate update, double * _Nullable scale, float * _Nullable opacity) CF_RETURNS_NOT_RETAINED ORB_SWIFT_NAME(ORBSymbolUpdate.templateImage(self:scale:opacity:));
ORB_EXPORT void ORBSymbolUpdateDrawWithStyleResolver(ORBSymbolUpdate update, ORBDrawingState state, ORBColor (^resolver)(ORBSymbolStyle, NSString * _Nullable)) ORB_SWIFT_NAME(ORBSymbolUpdate.draw(self:in:resolver:));

ORB_EXPORT float ORBSymbolUpdateLayerDrawingOpacity(ORBSymbolUpdateLayer layer) ORB_SWIFT_NAME(getter:ORBSymbolUpdateLayer.drawingOpacity(self:));
ORB_EXPORT ORBSymbolStyle ORBSymbolUpdateLayerStyle(ORBSymbolUpdateLayer layer) ORB_SWIFT_NAME(getter:ORBSymbolUpdateLayer.style(self:));
ORB_EXPORT NSString * _Nullable ORBSymbolUpdateLayerColorName(ORBSymbolUpdateLayer layer) ORB_SWIFT_NAME(getter:ORBSymbolUpdateLayer.colorName(self:));
ORB_EXPORT CGColorRef _Nullable ORBSymbolUpdateLayerDefaultColor(ORBSymbolUpdateLayer layer) CF_RETURNS_NOT_RETAINED ORB_SWIFT_NAME(getter:ORBSymbolUpdateLayer.defaultColor(self:));
// The update owns the returned path. Copy it before the update ends.
#if OPENRENDERBOX_CF_CGTYPES
ORB_EXPORT ORBPath ORBSymbolUpdateLayerPath(ORBSymbolUpdateLayer layer, CGAffineTransform *transform, ORBSymbolUpdateLayerFlags flags) ORB_SWIFT_NAME(ORBSymbolUpdateLayer.path(self:transform:flags:));
#endif
ORB_EXPORT void ORBSymbolUpdateLayerDraw(ORBSymbolUpdateLayer layer, ORBDrawingState state, ORBColor color, float opacity, ORBBlendMode blendMode, ORBSymbolUpdateLayerFlags flags) ORB_SWIFT_NAME(ORBSymbolUpdateLayer.draw(self:in:color:opacity:blendMode:flags:));

ORB_EXTERN_C_END

#endif /* ORB_OBJC_FOUNDATION */

ORB_ASSUME_NONNULL_END
