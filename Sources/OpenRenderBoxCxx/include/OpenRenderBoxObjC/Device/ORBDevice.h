//
//  ORBDevice.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include <OpenRenderBoxObjC/Render/ORBImageRenderer.h>
#include <dispatch/dispatch.h>

@protocol MTLCaptureScope, MTLDevice, OS_dispatch_queue;

@interface ORBDevice: NSObject <ORBImageRenderer>

@property (readonly, nonatomic) id <MTLDevice> device;
@property (readonly, nonatomic) NSObject<OS_dispatch_queue> *queue;
@property (readonly, nonatomic) id <MTLCaptureScope> captureScope;
@property NSUInteger GPUPriority;
@property NSUInteger backgroundGPUPriority;

///* class methods */
+ (instancetype)sharedDefaultDevice;
+ (NSArray<ORBDevice *> *)allDevices;
+ (BOOL)isSupported;
+ (void)setAllowsRenderingInBackground:(BOOL)background;
+ (BOOL)allowsRenderingInBackground;
+ (NSUInteger)defaultBackgroundGPUPriority;
+ (NSUInteger)defaultGPUPriority;
+ (BOOL)isRunningInBackground;
+ (void)setDefaultBackgroundGPUPriority:(NSUInteger)gpupriority;
+ (void)setDefaultGPUPriority:(NSUInteger)gpupriority;
+ (instancetype)sharedDevice:(id<MTLDevice>)device;
+ (instancetype)sharedDeviceForDisplay:(unsigned int)display;

/* instance methods */
- (void)dealloc;
- (instancetype)initWithDevice:(id<MTLDevice>)device;
- (CGImageRef)renderImageInRect:(CGRect)rect options:(id)options renderer:(id /* block */)renderer;
- (void)collectResources;
- (void)compileShader:(id)shader completionQueue:(id)queue handler:(id /* block */)handler;
- (BOOL)compileShader:(id)shader error:(id *)error;
- (id)pipelineDescriptions:(id)descriptions extraColorFormats:(id)formats;
- (void)renderImageInRect:(CGRect)rect options:(id)options renderer:(id /* block */)renderer completionQueue:(id)queue handler:(id /* block */)handler;

@end

#endif /* ORB_OBJC_FOUNDATION */
