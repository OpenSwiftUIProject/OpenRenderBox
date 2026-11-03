//
//  ORBEncoderSet.h
//  OpenRenderBox
//
//  Audited for 6.5.4
//  Status: Complete

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>

ORB_ASSUME_NONNULL_BEGIN

@interface ORBEncoderSet : NSObject

-(instancetype)init;
- (void)commit;
- (void)addDisplayList:(id)list;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
