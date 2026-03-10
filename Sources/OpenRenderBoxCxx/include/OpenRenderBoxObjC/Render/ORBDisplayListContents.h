//
//  ORBDisplayListContents.h
//  OpenRenderBox
//
//  Audited for 6.5.4
//  Status: Complete

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBoxObjC/Encoding/ORBEncodable.h>
#include <OpenRenderBoxObjC/Encoding/ORBDecodable.h>
#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

struct _ORBDrawingState;

@protocol ORBDisplayListContents <NSObject, ORBEncodable, ORBDecodable>

@required

@property (readonly, nonatomic) BOOL empty;
@property (readonly, nonatomic) CGRect boundingRect;
@property (readonly, copy, nonatomic) NSString *xmlDescription;

- (BOOL)isEmpty;
- (void)drawInState:(struct _ORBDrawingState *)state;
- (void)renderInContext:(CGContextRef)context options:(nullable id)options;

@optional

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
