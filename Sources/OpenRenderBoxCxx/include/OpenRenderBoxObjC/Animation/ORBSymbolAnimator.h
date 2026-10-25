//
//  ORBSymbolAnimator.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_OBJC_FOUNDATION

#include <OpenRenderBox/ORBColor.h>
#include <OpenRenderBox/ORBSymbolRenderingMode.h>
#include <OpenRenderBox/ORBSymbolUpdate.h>
#include <OpenRenderBoxObjC/Animation/ORBSymbolAnimatorObserver.h>
#include <OpenRenderBoxObjC/Animation/ORBSymbolAnimation.h>
#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

ORB_ASSUME_NONNULL_BEGIN

@class CUINamedVectorGlyph;

// TODO
@interface ORBSymbolAnimator : NSObject

@property (retain, nonatomic, nullable) CUINamedVectorGlyph *glyph;
@property (nonatomic) ORBSymbolRenderingMode renderingMode;
@property (nonatomic) BOOL flipsRightToLeft;
@property (nonatomic) double variableValue;
@property (nonatomic) ORBColor opacities;
@property (nonatomic, getter=isHidden) BOOL hidden;
@property (nonatomic) ORBSymbolScaleLevel scaleLevel;
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
- (unsigned int)addAnimation:(ORBSymbolAnimation)animation options:(nullable id)options ORB_SWIFT_NAME(addAnimation(_:options:));
- (ORBSymbolUpdate)beginUpdateWithRenderingMode:(ORBSymbolRenderingMode)mode;
- (void)endUpdate:(ORBSymbolUpdate)update;
- (void)removeAllAnimations;
- (void)removeAnimation:(ORBSymbolAnimation)animation ORB_SWIFT_NAME(removeAnimation(_:));
- (ORBSymbolUpdate)beginUpdateWithRenderingMode:(ORBSymbolRenderingMode)mode position:(nullable const CGPoint *)position size:(nullable const CGSize *)size flags:(ORBSymbolUpdateFlags)flags;
- (void)cancelAllAnimations;
- (void)cancelAnimation:(ORBSymbolAnimation)animation ORB_SWIFT_NAME(cancelAnimation(_:));
- (void)cancelAnimationWithID:(unsigned int)animationID;
- (ORBColor)colorForStyle:(unsigned int)style;
- (nullable id)copyDebugDescriptionForUpdate:(ORBSymbolUpdate)update;
- (void)removeAnimationWithID:(unsigned int)animationID;
- (void)setColor:(ORBColor)color forStyle:(unsigned int)style;
- (void)setPriority:(float)priority ofAnimationWithID:(unsigned int)animationID;
- (void)addObserver:(id<ORBSymbolAnimatorObserver>)observer;
- (void)removeObserver:(id<ORBSymbolAnimatorObserver>)observer;

@end

ORB_ASSUME_NONNULL_END

#endif /* ORB_OBJC_FOUNDATION */
