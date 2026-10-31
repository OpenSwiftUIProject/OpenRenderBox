//
//  ORBAnimation_Private.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBoxObjC/Animation/ORBAnimation.h>

#if ORB_OBJC_FOUNDATION && defined(__cplusplus)
#include <OpenRenderBoxCxx/Animation/Animation.hpp>

@interface ORBAnimation ()
- (const ORB::Animation &)orb_animation __attribute__((objc_direct));
- (void)setORBAnimation:(const ORB::Animation &)animation __attribute__((objc_direct));
@end
#endif
