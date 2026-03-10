//
//  AnimationTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics)
import CoreGraphics

@Suite(.enabled(if: compatibilityTestEnabled))
struct AnimationTests {
    @Test
    func basicAnimation() {
        let animation = ORBAnimation()

        // activeDuration should be 0 for empty animation
        #expect(animation.activeDuration == 0)

        // removeAll on empty animation should not crash
        animation.removeAll()

        // evaluateAtTime should return a value
        let value = animation.evaluate(atTime: 0)
        _ = value
    }

    @Test
    func addBezier() {
        let animation = ORBAnimation()
        animation.addBezierDuration(0.3, controlPoint1: CGPoint(x: 0.42, y: 0), controlPoint2: CGPoint(x: 0.58, y: 1))
        #expect(animation.activeDuration > 0)
    }

    @Test
    func addDelay() {
        let animation = ORBAnimation()
        animation.addDelay(0.5)
        #expect(animation.activeDuration == 0.5)
    }

    @Test
    func addSpeed() {
        let animation = ORBAnimation()
        animation.addBezierDuration(1.0, controlPoint1: .zero, controlPoint2: CGPoint(x: 1, y: 1))
        let baseDuration = animation.activeDuration
        animation.addSpeed(2.0)
        _ = baseDuration
    }

    @Test
    func addSpring() {
        let animation = ORBAnimation()
        animation.addSpringDuration(0.5, mass: 1.0, stiffness: 100, damping: 10, initialVelocity: 0)
        #expect(animation.activeDuration > 0)
    }

    @Test
    func addRepeat() {
        let animation = ORBAnimation()
        animation.addBezierDuration(0.3, controlPoint1: .zero, controlPoint2: CGPoint(x: 1, y: 1))
        animation.addRepeatCount(3, autoreverses: true)
        #expect(animation.activeDuration > 0)
    }

    @Test
    func addPreset() {
        let animation = ORBAnimation()
        animation.addPreset(0, duration: 0.5)
        #expect(animation.activeDuration > 0)
    }

    @Test
    func copyAndEqual() {
        let animation = ORBAnimation()
        animation.addDelay(0.5)
        let copy = animation.copy() as! ORBAnimation
        #expect(animation.isEqual(copy))
        #expect(copy.activeDuration == animation.activeDuration)
    }
}

#endif
