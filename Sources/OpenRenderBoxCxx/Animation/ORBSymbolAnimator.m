//
//  ORBSymbolAnimator.m
//  OpenRenderBox

#include <OpenRenderBoxObjC/Animation/ORBSymbolAnimator.h>

#if ORB_OBJC_FOUNDATION

@implementation ORBSymbolAnimator

// TODO

- (ORBSymbolUpdate)beginUpdateWithRenderingMode:(ORBSymbolRenderingMode)mode {
    return [self beginUpdateWithRenderingMode:mode position:NULL size:NULL flags:0];
}

- (ORBSymbolUpdate)beginUpdateWithRenderingMode:(ORBSymbolRenderingMode)mode position:(const CGPoint *)position size:(const CGSize *)size flags:(ORBSymbolUpdateFlags)flags {
    [self doesNotRecognizeSelector:_cmd];
    __builtin_trap();
}

- (void)endUpdate:(ORBSymbolUpdate)update {}

@end

#endif /* ORB_OBJC_FOUNDATION */
