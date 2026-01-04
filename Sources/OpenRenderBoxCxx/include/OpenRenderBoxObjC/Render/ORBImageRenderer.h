//
//  ORBImageRenderer.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

@protocol ORBImageRenderer <NSObject>
@required
- (CGImageRef)renderImageInRect:(CGRect)rect options:(id)options renderer:(id /* block */)renderer;
- (void)renderImageInRect:(CGRect)rect options:(id)options renderer:(id /* block */)renderer completionQueue:(id)queue handler:(id /* block */)handler;
@end

#endif /* ORB_OBJC_FOUNDATION */
