//
//  ORBImageRenderer.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

@protocol ORBImageRenderer <NSObject>

@required

- (nullable CGImageRef)renderImageInRect:(CGRect)rect options:(nullable id)options renderer:(nullable id /* block */)renderer;
- (void)renderImageInRect:(CGRect)rect options:(nullable id)options renderer:(nullable id /* block */)renderer completionQueue:(nullable id)queue handler:(nullable id /* block */)handler;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
