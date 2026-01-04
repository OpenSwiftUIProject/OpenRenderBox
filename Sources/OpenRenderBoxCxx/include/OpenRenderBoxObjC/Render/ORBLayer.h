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

@interface ORBLayer: CALayer </*_RBDrawableDelegate, _RBSharedSurfaceOwner,*/ ORBDrawableStatistics>
@property (retain, nonatomic) ORBDevice *device;
@property (nonatomic) BOOL rendersAsynchronously;
@property (nonatomic) int colorMode;
@property (nonatomic) BOOL promotesFramebuffer;
@property (nonatomic) unsigned long long pixelFormat;
@property (nonatomic) BOOL clearsBackground;
@property (nonatomic) ORBColor clearColor;
@property (nonatomic) long long maxDrawableCount;
@property (nonatomic) BOOL allowsPackedDrawable;
@property (nonatomic) BOOL allowsBottomLeftOrigin;
@property (readonly, nonatomic) BOOL drawableAvailable;
@property (nonatomic) BOOL needsSynchronousUpdate;
@property (readonly) unsigned long long hash;
@property (readonly) Class superclass;
@property (readonly, copy) NSString *description;
@property (readonly, copy) NSString *debugDescription;
@property (readonly, copy, nonatomic) NSDictionary *statistics;
@property (copy, nonatomic) id /* block */ statisticsHandler;

/* class methods */
+ (id)defaultValueForKey:(id)key;

/* instance methods */
- (void)dealloc;
- (id)init;
- (id)initWithCoder:(id)coder;
- (void)display;
- (void)layoutSublayers;
- (void)displayIfNeeded;
- (void)_renderForegroundInContext:(CGContextRef)context;
- (id)actionForKey:(id)key;
- (id)initWithLayer:(id)layer;
- (BOOL)isDrawableAvailable;
- (void)layerDidBecomeVisible:(BOOL)visible;
- (void)renderInContext:(CGContextRef)context;
- (void)setBounds:(CGRect)bounds;
- (void)setContents:(id)contents;
- (void)_RBDrawableStatisticsDidChange;
- (void)_moveSubsurface:(void *)subsurface;
- (BOOL)_willMoveSubsurface:(unsigned int)subsurface;
- (void)copyImageInRect:(CGRect)rect options:(id)options completionQueue:(id)queue handler:(id /* block */)handler;
- (BOOL)displayWithBounds:(CGRect)bounds callback:(id /* block */)callback;
- (void)drawInDisplayList:(id)list;
- (void)resetStatistics:(unsigned long long)statistics alpha:(double)alpha;
- (void)waitUntilAsyncRenderingCompleted;

@end

#endif
