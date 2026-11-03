//
//  ORBAnimation_Private.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBAnimation.h>

#if ORB_OBJC_FOUNDATION && defined(__cplusplus)
#include "Animation.hpp"

@interface ORBAnimation ()
- (const ORB::Animation &)orb_animation __attribute__((objc_direct));
- (void)setORBAnimation:(const ORB::Animation &)animation __attribute__((objc_direct));
@end
#endif
