//
//  ORBLayer.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBoxObjC/Device/ORBDevice.h>
#include <OpenRenderBoxObjC/Render/ORBDrawableStatistics.h>
#include <OpenRenderBox/ORBColor.h>

//#include "RBImageQueueLayer.h"
//#include "RBSurfaceContentsLayer.h"
//#include "_RBDrawableDelegate-Protocol.h"
//#include "_RBSharedSurfaceOwner-Protocol.h"

#include <QuartzCore/QuartzCore.h>
#include <Foundation/Foundation.h>

@class CAContext, CALayer;

ORB_ASSUME_NONNULL_BEGIN

@interface ORBLayer: CALayer </*_RBDrawableDelegate, _RBSharedSurfaceOwner,*/ ORBDrawableStatistics>

@property (retain, nonatomic, nullable) ORBDevice *device;
@property (nonatomic) BOOL rendersAsynchronously;
@property (nonatomic) int colorMode;
@property (nonatomic) BOOL promotesFramebuffer;
@property (nonatomic) NSUInteger pixelFormat;
@property (nonatomic) BOOL clearsBackground;
@property (nonatomic) ORBColor clearColor;
@property (nonatomic) NSInteger maxDrawableCount;
@property (nonatomic) BOOL allowsPackedDrawable;
@property (nonatomic) BOOL allowsBottomLeftOrigin;
@property (readonly, nonatomic, getter=isDrawableAvailable) BOOL drawableAvailable;
@property (nonatomic) BOOL needsSynchronousUpdate;
@property (readonly) NSUInteger hash;
@property (readonly) Class superclass;
@property (readonly, copy) NSString *description;
@property (readonly, copy, nullable) NSString *debugDescription;
@property (readonly, copy, nonatomic, nullable) NSDictionary *statistics;
@property (copy, nonatomic, nullable) id /* block */ statisticsHandler;

/* class methods */
+ (nullable id)defaultValueForKey:(NSString *)key;

/* instance methods */
- (void)dealloc;
- (instancetype)init;
- (nullable instancetype)initWithCoder:(NSCoder *)coder;
- (void)display;
- (void)layoutSublayers;
- (void)displayIfNeeded;
- (void)_renderForegroundInContext:(CGContextRef)context;
- (nullable id<CAAction>)actionForKey:(NSString *)key;
- (instancetype)initWithLayer:(id)layer;
- (BOOL)isDrawableAvailable;
- (void)layerDidBecomeVisible:(BOOL)visible;
- (void)renderInContext:(CGContextRef)context;
- (void)setBounds:(CGRect)bounds;
- (void)setContents:(nullable id)contents;
- (void)_RBDrawableStatisticsDidChange;
- (void)_moveSubsurface:(void *)subsurface;
- (BOOL)_willMoveSubsurface:(unsigned int)subsurface;
- (void)copyImageInRect:(CGRect)rect options:(nullable id)options completionQueue:(nullable id)queue handler:(nullable id /* block */)handler;
- (BOOL)displayWithBounds:(CGRect)bounds callback:(nullable id /* block */)callback;
- (void)drawInDisplayList:(nullable id)list;
- (void)resetStatistics:(NSUInteger)statistics alpha:(double)alpha;
- (void)waitUntilAsyncRenderingCompleted;

@end

ORB_ASSUME_NONNULL_END

#endif
