//
//  _ORBDrawableDelegate.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>

ORB_ASSUME_NONNULL_BEGIN

@protocol _ORBDrawableDelegate <NSObject>

@required

- (void)_RBDrawableStatisticsDidChange;

@optional

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */

