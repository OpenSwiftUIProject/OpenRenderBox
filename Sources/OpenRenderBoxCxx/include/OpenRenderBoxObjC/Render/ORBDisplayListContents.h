//
//  ORBDisplayListContents.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

@protocol ORBDisplayListContents <NSObject>

@required

@property (readonly, nonatomic) BOOL empty;
@property (readonly, nonatomic) CGRect boundingRect;
@property (readonly, copy, nonatomic) NSString *xmlDescription;

- (BOOL)isEmpty;
- (void)renderInContext:(CGContextRef)context options:(nullable id)options;

@optional

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
