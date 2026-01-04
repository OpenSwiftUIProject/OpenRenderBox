//
//  ORBDrawableStatistics.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>

@protocol ORBDrawableStatistics
@required
@property (readonly, copy, nonatomic) NSDictionary *statistics;
@property (copy, nonatomic) id /* block */ statisticsHandler;
- (void)resetStatistics:(unsigned long long)statistics alpha:(double)alpha;
@end

#endif /* ORB_OBJC_FOUNDATION */
