//
//  ORBEncoderDelegate.h
//  OpenRenderBox
//
//  Audited for 6.5.4
//  Status: Complete

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBoxObjC/Encoding/ORBEncoderSet.h>
#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

@protocol ORBEncoderDelegate <NSObject>

@required

@optional

@property (readonly, nonatomic) ORBEncoderSet *encoderSet;

// TODO: data type
- (nullable NSData *)encodedShaderLibraryData:(nullable NSData *)data error:(NSError *_Nullable *_Nullable)error;
- (nullable NSData *)encodedMetadata;
- (nullable NSData *)encodedCGFontData:(CGFontRef)data error:(NSError *_Nullable *_Nullable)error;
- (nullable NSData *)encodedFontData:(nullable NSData *)data cgFont:(CGFontRef)font error:(NSError *_Nullable *_Nullable)error;
- (nullable NSData *)encodedFontSubsetData:(nullable NSData *)data cgFont:(CGFontRef)font error:(NSError *_Nullable *_Nullable)error;
- (nullable NSData *)encodedImageData:(const void *)data error:(NSError *_Nullable *_Nullable)error;
- (BOOL)shouldEncodeFontSubset:(CGFontRef)subset;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
