//
//  ORBDisplayListPredicate.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBox/ORBColor.h>
#include <OpenRenderBoxObjC/Render/ORBDisplayListContents.h>
#include <Foundation/Foundation.h>

ORB_ASSUME_NONNULL_BEGIN

@interface ORBDisplayListPredicate : NSObject

@property (nonatomic) BOOL invertsResult;

- (void)removeAll;
- (void)addConditionWithFillColor:(ORBColor)color colorSpace:(ORBColorSpace)colorSpace;
- (id<ORBDisplayListContents>)copyFilteredDisplayList:(id<ORBDisplayListContents>)list;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
