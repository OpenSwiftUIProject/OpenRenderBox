//
//  ORBEncodable.h
//  OpenRenderBox
//
//  Audited for 6.5.4
//  Status: Complete

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBoxObjC/Encoding/ORBEncoderDelegate.h>
#include <Foundation/Foundation.h>

ORB_ASSUME_NONNULL_BEGIN

@protocol ORBEncodable <NSObject>

@required

- (nullable NSData *)encodedDataForDelegate:(nullable id<ORBEncoderDelegate>)delegate error:(NSError *_Nullable *_Nullable)error;

@optional

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
