//
//  ORBDecoderDelegate.h
//  OpenRenderBox
//
//  Audited for 6.5.4
//  Status: Complete

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

@protocol ORBDecoderDelegate <NSObject>

@required

@optional

// TODO: data type
- (CGFontRef)decodedCGFontWithData:(nullable NSData *)data error:(NSError *_Nullable *_Nullable)error;
- (nullable void *)decodedImageContentsWithData:(nullable NSData *)data type:(int *_Nullable)type error:(NSError *_Nullable *_Nullable)error;
- (void)decodedMetadata:(nullable NSData *)metadata;
- (nullable id)decodedShaderLibraryWithData:(nullable NSData *)data error:(NSError *_Nullable *_Nullable)error;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
