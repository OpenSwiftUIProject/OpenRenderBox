//
//  ORBTransition.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBTransition.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>
#include <OpenRenderBoxObjC/Animation/ORBAnimation.h>

ORB_ASSUME_NONNULL_BEGIN

@interface ORBTransitionEffect : NSObject

@property (nonatomic) ORBTransitionEffectType type;
@property (nonatomic) ORBTransitionEffectFlags flags;
@property (nonatomic) ORBTransitionEvents events;
@property (nonatomic) float beginTime;
@property (nonatomic) float duration;
@property (nonatomic) NSUInteger animationIndex;
@property (nonatomic) NSUInteger insertAnimationIndex;
@property (nonatomic) NSUInteger removeAnimationIndex;

- (float)argumentValueAtIndex:(NSUInteger)index;
- (void)setArgumentValue:(float)value atIndex:(NSUInteger)index;
- (uint32_t)integerArgumentValueAtIndex:(NSUInteger)index;
- (void)setIntegerArgumentValue:(uint32_t)value atIndex:(NSUInteger)index;

@end

@interface ORBTransition : NSObject <NSCopying>

@property (class, readonly, nonatomic) ORBTransition *defaultTransition;
@property (nonatomic) ORBTransitionMethod method;
@property (nonatomic, strong, nullable) ORBAnimation *animation;
@property (nonatomic) uint32_t maxChanges;
@property (nonatomic, getter=isReplaceable) BOOL replaceable;
@property (readonly, nonatomic, copy) NSArray<ORBTransitionEffect *> *effects;
@property (nonatomic) float addRemoveDuration;

- (void)addEffect:(nullable ORBTransitionEffect *)effect;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
