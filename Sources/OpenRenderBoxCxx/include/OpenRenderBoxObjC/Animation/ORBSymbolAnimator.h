//
//  ORBSymbolAnimator.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBox/ORBColor.h>
#include <OpenRenderBoxObjC/Animation/ORBSymbolAnimatorObserver.h>
#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

@class CUINamedVectorGlyph;
struct _ORBSymbolUpdate;

// TODO
@interface ORBSymbolAnimator : NSObject

@property (retain, nonatomic, nullable) CUINamedVectorGlyph *glyph;
@property (nonatomic) unsigned int renderingMode;
@property (nonatomic) BOOL flipsRightToLeft;
@property (nonatomic) double variableValue;
@property (nonatomic) ORBColor opacities;
@property (nonatomic, getter=isHidden) BOOL hidden;
@property (nonatomic) int scaleLevel;
@property (nonatomic) CGPoint anchorPoint;
@property (nonatomic) CGPoint position;
@property (nonatomic) CGSize size;
@property (nonatomic) CGPoint presentationPosition;
@property (nonatomic) double currentTime;
@property (readonly, nonatomic) double maxVelocity;
@property (readonly, nonatomic) unsigned int version;
@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (readonly, nonatomic) CGRect alignmentRect;
@property (readonly, nonatomic) CGRect unroundedAlignmentRect;
@property (readonly, nonatomic) unsigned int styleMask;
@property (nonatomic) unsigned int depth;
@property (readonly, nonatomic) CGRect boundingRect;

/* instance methods */
- (unsigned int)addAnimation:(unsigned int)animation options:(nullable id)options;
- (struct _ORBSymbolUpdate *)beginUpdateWithRenderingMode:(unsigned int)mode;
- (void)endUpdate:(struct _ORBSymbolUpdate *)update;
- (void)removeAllAnimations;
- (void)removeAnimation:(unsigned int)animation;
- (struct _ORBSymbolUpdate *)beginUpdateWithRenderingMode:(unsigned int)mode position:(nullable const CGPoint *)position size:(nullable const CGSize *)size flags:(unsigned int)flags;
- (void)cancelAllAnimations;
- (void)cancelAnimation:(unsigned int)animation;
- (void)cancelAnimationWithID:(unsigned int)animationID;
- (ORBColor)colorForStyle:(unsigned int)style;
- (nullable id)copyDebugDescriptionForUpdate:(struct _ORBSymbolUpdate *)update;
- (void)removeAnimationWithID:(unsigned int)animationID;
- (void)setColor:(ORBColor)color forStyle:(unsigned int)style;
- (void)setPriority:(float)priority ofAnimationWithID:(unsigned int)animationID;
- (void)addObserver:(id<ORBSymbolAnimatorObserver>)observer;
- (void)removeObserver:(id<ORBSymbolAnimatorObserver>)observer;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
