//
//  TransitionTests.swift
//  OpenRenderBoxCompatibilityTests

import Foundation
import Testing

struct TransitionTests {
    @Test
    func defaults() {
        let effect = ORBTransitionEffect()
        #expect(effect.type == .alpha)
        #expect(effect.events == .addRemove)
        #expect(effect.flags.rawValue == 0)
        #expect(effect.beginTime == 0)
        #expect(effect.duration == 1)
        let transition = ORBTransition()
        #expect(transition.method == .empty)
        #expect(transition.maxChanges == .max)
        #expect(!transition.isReplaceable)
        #expect(transition.animation == nil)
        #expect(transition.effects.isEmpty)
        #expect(transition.addRemoveDuration == Float(32) * (1 / 255))
        #expect(ORBTransition.default === ORBTransition.default)
        #expect(ORBTransition.default.isEqual(transition))
    }

    @Test(arguments: [
        (Float(-1), Float(0)), (0, 0), (0.15, Float(38) * (1 / 255)),
        (0.5, Float(128) * (1 / 255)), (2, 1), (.nan, 0),
    ] as [(Float, Float)])
    func timeQuantization(input: Float, expected: Float) {
        let effect = ORBTransitionEffect()
        effect.beginTime = input
        effect.duration = input
        #expect(effect.beginTime == expected)
        #expect(effect.duration == expected)
        let transition = ORBTransition()
        transition.addRemoveDuration = input
        #expect(transition.addRemoveDuration == expected)
    }

    @Test
    func animationIndicesReplaceTimeline() {
        let effect = ORBTransitionEffect()
        effect.beginTime = 0.5
        effect.duration = 0.25
        effect.insertAnimationIndex = 258
        #expect(effect.insertAnimationIndex == 2)
        #expect(effect.removeAnimationIndex == 0)
        #expect(effect.animationIndex == 0)
        #expect(effect.beginTime == 0)
        #expect(effect.duration == 1)
        effect.removeAnimationIndex = 2
        #expect(effect.animationIndex == 2)
        effect.beginTime = 0.25
        #expect(effect.animationIndex == 0)
        #expect(effect.duration == 1)
        effect.animationIndex = 7
        effect.duration = 0.5
        #expect(effect.insertAnimationIndex == 0)
        #expect(effect.removeAnimationIndex == 0)
        #expect(effect.beginTime == 0)
        #expect(effect.duration == Float(128) * (1 / 255))
    }

    @Test
    func argumentsShareBitsAndIgnoreInvalidIndices() {
        let effect = ORBTransitionEffect()
        effect.setIntegerArgumentValue(0x3f800000, at: 0)
        #expect(effect.argumentValue(at: 0) == 1)
        effect.setArgumentValue(-0.0, at: 1)
        #expect(effect.integerArgumentValue(at: 1) == 0x80000000)
        effect.setArgumentValue(4, at: 2)
        #expect(effect.argumentValue(at: 2) == 0)
        #expect(effect.integerArgumentValue(at: .max) == 0)
        effect.type = ORBTransitionEffectType(rawValue: 129)!
        effect.flags = ORBTransitionEffectFlags(rawValue: 3)
        effect.events = ORBTransitionEvents(rawValue: .max)
        #expect(effect.type == .alpha)
        #expect(effect.flags.rawValue == 3)
        #expect(effect.events.rawValue == 63)
        if !compatibilityTestEnabled {
            // The 2024 descriptor stores two flag bits; newer system versions add a third bit.
            effect.flags = ORBTransitionEffectFlags(rawValue: .max)
            #expect(effect.flags.rawValue == 3)
        }
    }

    @Test
    func transitionCopiesEffectsAndDropsUnusedArguments() throws {
        let effect = ORBTransitionEffect()
        effect.type = .scaleCenter
        effect.setArgumentValue(0.5, at: 0)
        effect.setArgumentValue(0.75, at: 1)
        let transition = ORBTransition()
        transition.addEffect(effect)
        effect.setArgumentValue(1, at: 0)
        let stored = try #require(transition.effects.first)
        #expect(stored.argumentValue(at: 0) == 0.5)
        #expect(stored.argumentValue(at: 1) == 0)
        stored.setArgumentValue(2, at: 0)
        #expect(transition.effects[0].argumentValue(at: 0) == 0.5)
        let copy = try #require(transition.copy() as? ORBTransition)
        #expect(copy.isEqual(transition))
        #expect(copy.hash == transition.hash)
        copy.method = .diff
        copy.isReplaceable = true
        copy.maxChanges = 10
        copy.addEffect(effect)
        #expect(transition.method == .empty)
        #expect(!transition.isReplaceable)
        #expect(transition.maxChanges == .max)
        #expect(transition.effects.count == 1)
        #expect(!copy.isEqual(transition))
    }

    @Test
    func animationIsStoredByValue() throws {
        let animation = ORBAnimation()
        animation.addSpringDuration(0.5, mass: 1, stiffness: 344, damping: 37, initialVelocity: 0)
        animation.addSpringDuration(0.8, mass: 2, stiffness: 470, damping: 34, initialVelocity: 0)
        let transition = ORBTransition()
        transition.animation = animation
        let stored = try #require(transition.animation)
        #expect(stored !== animation)
        #expect(stored.isEqual(animation))
        #expect(stored.activeDuration == 0.5)
        animation.removeAll()
        stored.removeAll()
        #expect(transition.animation?.activeDuration == 0.5)
        let copy = try #require(transition.copy() as? ORBTransition)
        #expect(copy.isEqual(transition))
        copy.animation = animation
        #expect(copy.animation == nil)
        #expect(transition.animation != nil)
    }
}
