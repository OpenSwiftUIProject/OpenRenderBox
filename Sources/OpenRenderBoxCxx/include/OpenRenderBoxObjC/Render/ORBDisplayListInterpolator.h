//
//  ORBDisplayListInterpolator.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBoxObjC/Render/ORBDisplayListContents.h>
#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

// TODO
@interface ORBDisplayListInterpolator : NSObject <NSCopying>

@property (retain, nonatomic) id<ORBDisplayListContents> from;
@property (readonly, nonatomic) id<ORBDisplayListContents> to;
@property (readonly, copy, nonatomic, nullable) NSDictionary *options;
@property (readonly, nonatomic, getter=isIdentity) BOOL identity;
@property (readonly, nonatomic) BOOL onlyFades;
@property (readonly, nonatomic) double activeDuration;

+ (instancetype)interpolatorWithFrom:(id<ORBDisplayListContents>)from to:(id<ORBDisplayListContents>)to options:(nullable NSDictionary *)options;

- (instancetype)initWithFrom:(id<ORBDisplayListContents>)from to:(id<ORBDisplayListContents>)to options:(nullable NSDictionary *)options;
- (id)copyWithZone:(nullable NSZone *)zone;
- (void)drawInState:(void *)state by:(float)by;
- (CGRect)boundingRectWithProgress:(float)progress;
- (nullable id<ORBDisplayListContents>)contentsWithProgress:(float)progress;
- (nullable id<ORBDisplayListContents>)copyContentsWithProgress:(float)progress;
- (double)maxAbsoluteVelocityWithProgress:(float)progress;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
