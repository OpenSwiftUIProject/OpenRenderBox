//
//  ORBSymbolUpdate.m
//  OpenRenderBox

#include <OpenRenderBox/ORBSymbolUpdate.h>

#if ORB_OBJC_FOUNDATION

#include <CoreGraphics/CoreGraphics.h>
#include <Foundation/Foundation.h>
#include <math.h>
#include <stdint.h>

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
ORBPath ORBSymbolUpdateLayerPath(ORBSymbolUpdateLayer layer, CGAffineTransform *transform, ORBSymbolUpdateLayerFlags flags) {
    *transform = CGAffineTransformIdentity;
    return ORBPathNull;
}
void ORBSymbolUpdateLayerDraw(ORBSymbolUpdateLayer layer, ORBDrawingState state, ORBColor color, float opacity, ORBBlendMode blendMode, ORBSymbolUpdateLayerFlags flags) {}

#endif /* ORB_OBJC_FOUNDATION */
