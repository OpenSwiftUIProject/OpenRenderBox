//
//  ORBDisplayListTransform.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBox/ORBColor.h>
#include <OpenRenderBoxObjC/Render/ORBDisplayListContents.h>
#include <Foundation/Foundation.h>

ORB_ASSUME_NONNULL_BEGIN

@interface ORBDisplayListTransform : NSObject

- (void)removeAll;
- (void)addColorReplacementFrom:(ORBColor)fromColor to:(ORBColor)toColor colorSpace:(ORBColorSpace)colorSpace;
- (id<ORBDisplayListContents>)copyApplyingToDisplayList:(id<ORBDisplayListContents>)list;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
