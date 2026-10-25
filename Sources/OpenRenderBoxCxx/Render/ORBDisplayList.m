//
//  ORBDisplayList.m
//  OpenRenderBox

#include <OpenRenderBoxObjC/Render/ORBDisplayList.h>
#include <OpenRenderBoxObjC/Render/Private/_ORBDisplayListContents.h>

#if ORB_OBJC_FOUNDATION

ORBDisplayListRenderKey const ORBDisplayListRenderRasterizationScale = @"rasterizationscale";

@interface ORBDisplayList () <_ORBDisplayListContents>
@end

@implementation ORBDisplayList

// TODO

@end

ORBDrawingState ORBDisplayListGetState(ORBDisplayList *list) {
    // The drawing state is not implemented yet.
    [NSException raise:NSInternalInconsistencyException format:@"ORBDisplayListGetState is not implemented"];
    __builtin_trap();
}

#endif /* ORB_OBJC_FOUNDATION */
