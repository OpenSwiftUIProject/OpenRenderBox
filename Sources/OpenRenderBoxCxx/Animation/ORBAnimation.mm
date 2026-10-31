//
//  ORBAnimation.mm
//  OpenRenderBox

#include <OpenRenderBoxObjC/Animation/ORBAnimation.h>

#if ORB_OBJC_FOUNDATION

#include "ORBAnimation_Private.h"

@implementation ORBAnimation {
    ORB::Animation _animation;
}

- (id)copyWithZone:(NSZone *)zone {
    ORBAnimation *copy = [ORBAnimation new];
    copy->_animation = _animation;
    return copy;
}

- (BOOL)isEqual:(id)object {
    return [object isKindOfClass:[ORBAnimation class]]
        && _animation == ((ORBAnimation *)object)->_animation;
}

- (NSUInteger)hash { return _animation.hash(); }
- (BOOL)orb_isEmpty { return _animation.empty(); }
- (void)removeAll { _animation.clear(); }

- (double)activeDuration {
    return _animation.active_duration();
}

- (void)addSpringDuration:(double)duration mass:(double)mass stiffness:(double)stiffness damping:(double)damping initialVelocity:(double)velocity {
    _animation.add_spring(duration, mass, stiffness, damping, velocity);
}

- (const ORB::Animation &)orb_animation { return _animation; }
- (void)setORBAnimation:(const ORB::Animation &)animation { _animation = animation; }

// Other animation terms and evaluation require the animation engine.

@end

#endif /* ORB_OBJC_FOUNDATION */
