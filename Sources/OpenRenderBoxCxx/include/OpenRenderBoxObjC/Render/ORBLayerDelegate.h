//
//  ORBLayerDelegate.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBoxObjC/Render/ORBDisplayList.h>

#include <Foundation/Foundation.h>
#include <QuartzCore/QuartzCore.h>

ORB_ASSUME_NONNULL_BEGIN

@class ORBLayer;

@protocol ORBLayerDelegate <CALayerDelegate>

@required

@optional

- (void)RBLayer:(ORBLayer *)layer draw:(ORBDisplayList *)inDisplayList;
- (nullable id)RBLayerDefaultDevice:(ORBLayer *)layer;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */

