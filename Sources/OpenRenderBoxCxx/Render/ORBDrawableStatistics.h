//
//  ORBDrawableStatistics.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>

ORB_ASSUME_NONNULL_BEGIN

@protocol ORBDrawableStatistics

@required

@property (readonly, copy, nonatomic, nullable) NSDictionary *statistics;
@property (copy, nonatomic, nullable) id /* block */ statisticsHandler;

- (void)resetStatistics:(NSUInteger)statistics alpha:(double)alpha;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
