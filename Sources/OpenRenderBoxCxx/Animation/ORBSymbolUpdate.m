//
//  ORBSymbolUpdate.m
//  OpenRenderBox

#include <OpenRenderBox/ORBSymbolUpdate.h>

#if ORB_OBJC_FOUNDATION

#include <math.h>

// Symbol updates are placeholders until the symbol renderer is implemented.
double ORBSymbolUpdateNextTime(ORBSymbolUpdate update) { return INFINITY; }
uint32_t ORBSymbolUpdateStyleMask(ORBSymbolUpdate update) { return 0; }
intptr_t ORBSymbolUpdateGroupCount(ORBSymbolUpdate update) { return 0; }
intptr_t ORBSymbolUpdateGroupSize(ORBSymbolUpdate update, intptr_t group) { return 0; }
CGImageRef ORBSymbolUpdateTemplateImage(ORBSymbolUpdate update, double *scale, float *opacity) { return NULL; }
void ORBSymbolUpdateDrawWithStyleResolver(ORBSymbolUpdate update, ORBDrawingState state, ORBColor (^resolver)(ORBSymbolStyle, NSString *)) {}
float ORBSymbolUpdateLayerDrawingOpacity(ORBSymbolUpdateLayer layer) { return 0; }
ORBSymbolStyle ORBSymbolUpdateLayerStyle(ORBSymbolUpdateLayer layer) { return ORBSymbolStylePrimary; }
NSString *ORBSymbolUpdateLayerColorName(ORBSymbolUpdateLayer layer) { return nil; }
CGColorRef ORBSymbolUpdateLayerDefaultColor(ORBSymbolUpdateLayer layer) { return NULL; }
#if OPENRENDERBOX_CF_CGTYPES
ORBPath ORBSymbolUpdateLayerPath(ORBSymbolUpdateLayer layer, CGAffineTransform *transform, ORBSymbolUpdateLayerFlags flags) {
    *transform = CGAffineTransformIdentity;
    return ORBPathNull;
}
#endif
void ORBSymbolUpdateLayerDraw(ORBSymbolUpdateLayer layer, ORBDrawingState state, ORBColor color, float opacity, ORBBlendMode blendMode, ORBSymbolUpdateLayerFlags flags) {}

#endif /* ORB_OBJC_FOUNDATION */
