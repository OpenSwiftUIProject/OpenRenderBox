//
//  ORBTransition.mm
//  OpenRenderBox

#include <OpenRenderBoxObjC/Animation/ORBTransition.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBoxCxx/Animation/Transition.hpp>
#include <OpenRenderBoxCxx/Util/refcounted_ptr.hpp>
#include <OpenRenderBoxCxx/Util/assert.hpp>
#include "ORBAnimation_Private.h"
#include <cstring>

@interface ORBTransitionEffect () {
@package
    ORB::Transition::Effect _effect;
    uint32_t _args[2];
}
@end

@implementation ORBTransitionEffect

- (ORBTransitionEffectType)type { return static_cast<ORBTransitionEffectType>(_effect.type()); }
- (void)setType:(ORBTransitionEffectType)value { _effect.set_type(value); }
- (ORBTransitionEffectFlags)flags { return _effect.flags(); }
- (void)setFlags:(ORBTransitionEffectFlags)value { _effect.set_flags(value); }
- (ORBTransitionEvents)events { return _effect.events(); }
- (void)setEvents:(ORBTransitionEvents)value { _effect.set_events(value); }

- (float)beginTime { return _effect.begin_time(); }
- (void)setBeginTime:(float)value { _effect.set_begin_time(value); }
- (float)duration { return _effect.duration_value(); }
- (void)setDuration:(float)value { _effect.set_duration(value); }

- (NSUInteger)animationIndex { return _effect.animation_index(); }
- (void)setAnimationIndex:(NSUInteger)value { _effect.set_animation_index(value); }
- (NSUInteger)insertAnimationIndex { return _effect.insert_animation(); }
- (void)setInsertAnimationIndex:(NSUInteger)value { _effect.set_insert_animation(value); }
- (NSUInteger)removeAnimationIndex { return _effect.remove_animation(); }
- (void)setRemoveAnimationIndex:(NSUInteger)value { _effect.set_remove_animation(value); }

- (float)argumentValueAtIndex:(NSUInteger)index {
    float value = 0;
    if (index < 2) std::memcpy(&value, &_args[index], sizeof(value));
    return value;
}
- (void)setArgumentValue:(float)value atIndex:(NSUInteger)index {
    if (index < 2) std::memcpy(&_args[index], &value, sizeof(value));
}
- (uint32_t)integerArgumentValueAtIndex:(NSUInteger)index { return index < 2 ? _args[index] : 0; }
- (void)setIntegerArgumentValue:(uint32_t)value atIndex:(NSUInteger)index {
    if (index < 2) _args[index] = value;
}

- (BOOL)isEqual:(id)object {
    if (![object isKindOfClass:[ORBTransitionEffect class]]) return NO;
    ORBTransitionEffect *other = object;
    return _effect.bits == other->_effect.bits && _effect.begin == other->_effect.begin
        && _effect.duration == other->_effect.duration && _args[0] == other->_args[0] && _args[1] == other->_args[1];
}

- (NSUInteger)hash {
    NSUInteger hash = _effect.hash() * 16777619 ^ _args[0];
    return hash * 16777619 ^ _args[1];
}

@end

static ORB::Transition::Method ORBTransitionMethodValue(ORBTransitionMethod method) {
    using Method = ORB::Transition::Method;
    switch (method) {
        case ORBTransitionMethodDiff: return Method::Diff;
        case ORBTransitionMethodForwards: return Method::Forwards;
        case ORBTransitionMethodBackwards: return Method::Backwards;
        case ORBTransitionMethodPrefixAndSuffix: return Method::PrefixSuffix;
        case ORBTransitionMethodPrefix: return Method::Prefix;
        case ORBTransitionMethodSuffix: return Method::Suffix;
        case ORBTransitionMethodBinary: return Method::Binary;
        case ORBTransitionMethodNone: return Method::None;
        default: return Method::Default;
    }
}

static ORBTransitionMethod ORBTransitionMethodFromValue(ORB::Transition::Method method) {
    using Method = ORB::Transition::Method;
    switch (method) {
        case Method::Diff: return ORBTransitionMethodDiff;
        case Method::Forwards: return ORBTransitionMethodForwards;
        case Method::Backwards: return ORBTransitionMethodBackwards;
        case Method::PrefixSuffix: return ORBTransitionMethodPrefixAndSuffix;
        case Method::Prefix: return ORBTransitionMethodPrefix;
        case Method::Suffix: return ORBTransitionMethodSuffix;
        case Method::Binary: return ORBTransitionMethodBinary;
        case Method::None: return ORBTransitionMethodNone;
        default: return ORBTransitionMethodEmpty;
    }
}

@interface ORBTransition () {
    ORB::refcounted_ptr<ORB::Transition> _transition;
    BOOL _is_default;
}
- (void)_copyOnWrite __attribute__((objc_direct));
- (ORB::Transition *)orb_transition __attribute__((objc_direct));
- (void)setTransition:(ORB::Transition *)transition __attribute__((objc_direct));
@end

@implementation ORBTransition

+ (ORBTransition *)defaultTransition {
    static ORBTransition *default_transition = [] {
        ORBTransition *transition = [ORBTransition new];
        transition->_is_default = YES;
        return transition;
    }();
    return default_transition;
}

- (instancetype)init {
    if ((self = [super init])) {
        _transition = ORB::refcounted_ptr<ORB::Transition>(new ORB::Transition);
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    ORBTransition *copy = [ORBTransition new];
    copy->_transition = _transition;
    return copy;
}

- (void)_copyOnWrite {
    if (self && !_transition->is_unique()) {
        _transition = ORB::refcounted_ptr<ORB::Transition>(new ORB::Transition(*_transition));
    }
}

- (ORBTransitionMethod)method { return ORBTransitionMethodFromValue(_transition->method()); }
- (void)setMethod:(ORBTransitionMethod)value {
    ORB::Transition::Method method = ORBTransitionMethodValue(value);
    if (_transition->method() != method) {
        if (_is_default) ORB::precondition_failure("modifying default transition");
        [self _copyOnWrite];
        _transition->set_method(method);
    }
}

- (ORBAnimation *)animation {
    if (_transition->animation().empty()) return nil;
    ORBAnimation *animation = [ORBAnimation new];
    [animation setORBAnimation:_transition->animation()];
    return animation;
}
- (void)setAnimation:(ORBAnimation *)value {
    if (_is_default) ORB::precondition_failure("modifying default transition");
    [self _copyOnWrite];
    if (value) _transition->set_animation([value orb_animation]);
    else _transition->clear_animation();
}

- (uint32_t)maxChanges { return _transition->max_changes(); }
- (void)setMaxChanges:(uint32_t)value {
    if (_transition->max_changes() != value) {
        [self _copyOnWrite];
        _transition->set_max_changes(value);
    }
}
- (BOOL)isReplaceable { return _transition->replaceable(); }
- (void)setReplaceable:(BOOL)value {
    if (_transition->replaceable() != value) {
        [self _copyOnWrite];
        _transition->set_replaceable(value);
    }
}
- (float)addRemoveDuration { return _transition->add_remove_duration(); }
- (void)setAddRemoveDuration:(float)value {
    [self _copyOnWrite];
    _transition->set_add_remove_duration(value);
}

- (NSArray<ORBTransitionEffect *> *)effects {
    const auto &values = _transition->effects();
    if (values.empty()) return @[];
    NSMutableArray<ORBTransitionEffect *> *effects = [NSMutableArray array];
    for (uint32_t index = 0; index < values.size();) {
        ORBTransitionEffect *effect = [ORBTransitionEffect new];
        effect->_effect = values[index++].effect;
        uint8_t count = effect->_effect.argument_count();
        for (uint8_t i = 0; i < count; ++i) effect->_args[i] = values[index++].argument;
        [effects addObject:effect];
    }
    return effects;
}
- (void)addEffect:(ORBTransitionEffect *)effect {
    if (_is_default) ORB::precondition_failure("modifying default transition");
    if (effect) {
        [self _copyOnWrite];
        _transition->add_effect(effect->_effect, effect->_args);
    }
}

- (ORB::Transition *)orb_transition { return _transition.get(); }
- (void)setTransition:(ORB::Transition *)transition {
    if (!self) return;
    if (_is_default) ORB::precondition_failure("modifying default transition");
    if (_transition.get() != transition) {
        transition->retain();
        _transition = ORB::refcounted_ptr<ORB::Transition>(transition);
    }
}

- (BOOL)isEqual:(id)object {
    return [object isKindOfClass:[ORBTransition class]]
        && *_transition == *((ORBTransition *)object)->_transition;
}
- (NSUInteger)hash { return _transition->hash(); }

@end

#endif /* ORB_OBJC_FOUNDATION */
