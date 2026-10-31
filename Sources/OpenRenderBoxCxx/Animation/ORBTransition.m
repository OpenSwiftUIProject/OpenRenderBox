//
//  ORBTransition.m
//  OpenRenderBox

#include <OpenRenderBoxObjC/Animation/ORBTransition.h>

#if ORB_OBJC_FOUNDATION

#include <dispatch/dispatch.h>
#include <math.h>
#include <string.h>

static uint8_t ORBTransitionTime(float value) {
    if (isnan(value)) return 0;
    return (uint8_t)fmaf(fminf(fmaxf(value, 0), 1), 255, 0.5f);
}

@interface ORBTransitionEffect () {
    uint16_t _bits;
    uint8_t _begin;
    uint8_t _duration;
    uint32_t _arguments[2];
}
- (ORBTransitionEffect *)orb_snapshot;
@end

@implementation ORBTransitionEffect

- (instancetype)init {
    if ((self = [super init])) {
        _bits = 0x301;
        _duration = 255;
    }
    return self;
}

- (ORBTransitionEffectType)type { return _bits & 127; }
- (void)setType:(ORBTransitionEffectType)value { _bits = (_bits & ~127) | (value & 127); }
- (ORBTransitionEffectFlags)flags { return _bits >> 14; }
- (void)setFlags:(ORBTransitionEffectFlags)value { _bits = (_bits & 0x3fff) | (value << 14); }
- (ORBTransitionEvents)events { return (_bits >> 8) & 63; }
- (void)setEvents:(ORBTransitionEvents)value { _bits = (_bits & 0xc0ff) | ((value & 63) << 8); }

- (float)beginTime { return (_bits & 128) ? 0 : _begin * (1.0f / 255); }
- (void)setBeginTime:(float)value {
    if (_bits & 128) {
        _bits &= ~128;
        _duration = 255;
    }
    _begin = ORBTransitionTime(value);
}

- (float)duration { return (_bits & 128) ? 1 : _duration * (1.0f / 255); }
- (void)setDuration:(float)value {
    if (_bits & 128) {
        _bits &= ~128;
        _begin = 0;
    }
    _duration = ORBTransitionTime(value);
}

- (NSUInteger)animationIndex { return (_bits & 128) && _begin == _duration ? _begin : 0; }
- (void)setAnimationIndex:(NSUInteger)value {
    self.insertAnimationIndex = value;
    self.removeAnimationIndex = value;
}
- (NSUInteger)insertAnimationIndex { return (_bits & 128) ? _begin : 0; }
- (void)setInsertAnimationIndex:(NSUInteger)value {
    if (!(_bits & 128)) {
        _bits |= 128;
        _duration = 0;
    }
    _begin = value;
}
- (NSUInteger)removeAnimationIndex { return (_bits & 128) ? _duration : 0; }
- (void)setRemoveAnimationIndex:(NSUInteger)value {
    if (!(_bits & 128)) {
        _bits |= 128;
        _begin = 0;
    }
    _duration = value;
}

- (float)argumentValueAtIndex:(NSUInteger)index {
    float value = 0;
    if (index < 2) memcpy(&value, &_arguments[index], sizeof(value));
    return value;
}
- (void)setArgumentValue:(float)value atIndex:(NSUInteger)index {
    if (index < 2) memcpy(&_arguments[index], &value, sizeof(value));
}
- (uint32_t)integerArgumentValueAtIndex:(NSUInteger)index { return index < 2 ? _arguments[index] : 0; }
- (void)setIntegerArgumentValue:(uint32_t)value atIndex:(NSUInteger)index {
    if (index < 2) _arguments[index] = value;
}

- (ORBTransitionEffect *)orb_snapshot {
    ORBTransitionEffect *copy = [ORBTransitionEffect new];
    copy->_bits = _bits;
    copy->_begin = _begin;
    copy->_duration = _duration;
    static const uint8_t argumentCounts[] = {0, 0, 1, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0};
    NSUInteger type = self.type;
    NSUInteger count = type < sizeof(argumentCounts) ? argumentCounts[type] : 2;
    memcpy(copy->_arguments, _arguments, count * sizeof(uint32_t));
    return copy;
}

- (BOOL)isEqual:(id)object {
    if (![object isKindOfClass:[ORBTransitionEffect class]]) return NO;
    ORBTransitionEffect *other = object;
    return _bits == other->_bits && _begin == other->_begin && _duration == other->_duration
        && _arguments[0] == other->_arguments[0] && _arguments[1] == other->_arguments[1];
}

- (NSUInteger)hash {
    NSUInteger hash = (_bits & 0x3fff) | ((_bits >> 2) & 0x3000) | ((NSUInteger)_begin << 16) | ((NSUInteger)_duration << 24);
    hash = hash * 16777619 ^ _arguments[0];
    return hash * 16777619 ^ _arguments[1];
}

@end

@interface ORBAnimation (TransitionStorage)
- (BOOL)orb_isEmpty;
@end

@interface ORBTransition () {
    ORBTransitionMethod _method;
    ORBAnimation *_animation;
    NSMutableArray<ORBTransitionEffect *> *_effects;
    uint8_t _addRemoveDuration;
    BOOL _isDefault;
}
@end

@implementation ORBTransition

+ (ORBTransition *)defaultTransition {
    static ORBTransition *transition;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        transition = [ORBTransition new];
        transition->_isDefault = YES;
    });
    return transition;
}

- (instancetype)init {
    if ((self = [super init])) {
        _maxChanges = UINT32_MAX;
        _addRemoveDuration = 32;
        _effects = [NSMutableArray new];
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    ORBTransition *copy = [ORBTransition new];
    copy->_method = _method;
    copy->_animation = [_animation copy];
    copy->_effects = [_effects mutableCopy];
    copy->_maxChanges = _maxChanges;
    copy->_replaceable = _replaceable;
    copy->_addRemoveDuration = _addRemoveDuration;
    return copy;
}

- (ORBTransitionMethod)method { return _method; }
- (void)setMethod:(ORBTransitionMethod)value {
    value = value <= ORBTransitionMethodPrefixAndSuffix ? value : ORBTransitionMethodEmpty;
    if (_method != value) {
        NSAssert(!_isDefault, @"The default transition cannot be modified.");
        _method = value;
    }
}
- (ORBAnimation *)animation { return [_animation orb_isEmpty] ? nil : [_animation copy]; }
- (void)setAnimation:(ORBAnimation *)value {
    NSAssert(!_isDefault, @"The default transition cannot be modified.");
    _animation = [value copy];
}
- (NSArray<ORBTransitionEffect *> *)effects {
    NSMutableArray *effects = [NSMutableArray arrayWithCapacity:_effects.count];
    for (ORBTransitionEffect *effect in _effects) [effects addObject:[effect orb_snapshot]];
    return effects;
}
- (void)addEffect:(ORBTransitionEffect *)effect {
    NSAssert(!_isDefault, @"The default transition cannot be modified.");
    if (effect) [_effects addObject:[effect orb_snapshot]];
}
- (float)addRemoveDuration { return _addRemoveDuration * (1.0f / 255); }
- (void)setAddRemoveDuration:(float)value { _addRemoveDuration = ORBTransitionTime(value); }

- (BOOL)isEqual:(id)object {
    if (![object isKindOfClass:[ORBTransition class]]) return NO;
    ORBTransition *other = object;
    ORBAnimation *animation = self.animation;
    ORBAnimation *otherAnimation = other.animation;
    return _method == other->_method && _maxChanges == other->_maxChanges
        && _replaceable == other->_replaceable && _addRemoveDuration == other->_addRemoveDuration
        && [_effects isEqual:other->_effects]
        && (animation == otherAnimation || [animation isEqual:otherAnimation]);
}
- (NSUInteger)hash {
    NSUInteger hash = _method ^ _maxChanges ^ _replaceable ^ _addRemoveDuration;
    hash = hash * 33 ^ self.animation.hash;
    for (ORBTransitionEffect *effect in _effects) hash = hash * 33 ^ effect.hash;
    return hash;
}

@end

#endif /* ORB_OBJC_FOUNDATION */
