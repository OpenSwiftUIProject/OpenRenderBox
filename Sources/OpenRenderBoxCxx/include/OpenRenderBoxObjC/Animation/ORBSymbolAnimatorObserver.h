//
//  ORBSymbolAnimatorObserver.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>

ORB_ASSUME_NONNULL_BEGIN

@class ORBSymbolAnimator;

@protocol ORBSymbolAnimatorObserver <NSObject>

@required

- (void)symbolAnimatorDidChange:(ORBSymbolAnimator *)animator;

@optional

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
