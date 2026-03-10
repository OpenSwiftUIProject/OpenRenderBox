//
//  _ORBDisplayListContents.h
//  OpenRenderBox
//
//  Audited for 6.5.4
//  Status: Complete

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBoxObjC/Render/ORBDisplayListContents.h>
#include <Foundation/Foundation.h>

ORB_ASSUME_NONNULL_BEGIN

@protocol _ORBDisplayListContents <ORBDisplayListContents>

@required

@property (readonly, nonatomic) const void *_rb_contents;
@property (readonly, nonatomic) const void *_rb_xml_document;

- (void)_drawInState:(void *)state alpha:(float)alpha;

@optional

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
