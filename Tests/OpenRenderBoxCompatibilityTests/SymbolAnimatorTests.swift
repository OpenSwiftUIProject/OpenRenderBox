//
//  SymbolAnimatorTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics)
import CoreGraphics

@Suite(.enabled(if: compatibilityTestEnabled))
struct SymbolAnimatorTests {
    @Test
    func basicProperties() {
        let animator = ORBSymbolAnimator()

        // Default state
        #expect(animator.isAnimating == false)
        #expect(animator.isHidden == false)

        // Geometry
        animator.anchorPoint = CGPoint(x: 0.5, y: 0.5)
        #expect(animator.anchorPoint.x == 0.5)
        #expect(animator.anchorPoint.y == 0.5)

        animator.position = CGPoint(x: 100, y: 200)
        #expect(animator.position.x == 100)
        #expect(animator.position.y == 200)

        animator.size = CGSize(width: 44, height: 44)
        #expect(animator.size.width == 44)
        #expect(animator.size.height == 44)
    }

    @Test
    func renderingProperties() {
        let animator = ORBSymbolAnimator()

        animator.renderingMode = 1
        #expect(animator.renderingMode == 1)

        animator.flipsRightToLeft = true
        #expect(animator.flipsRightToLeft == true)

        animator.isHidden = true
        #expect(animator.isHidden == true)

        animator.depth = 2
        #expect(animator.depth == 2)

        animator.scaleLevel = 1
        #expect(animator.scaleLevel == 1)
    }

    @Test
    func animationManagement() {
        let animator = ORBSymbolAnimator()

        // removeAllAnimations on empty animator should not crash
        animator.removeAllAnimations()

        // cancelAllAnimations on empty animator should not crash
        animator.cancelAllAnimations()
    }

    @Test
    func timeAndVelocity() {
        let animator = ORBSymbolAnimator()

        animator.currentTime = 1.5
        #expect(animator.currentTime == 1.5)

        _ = animator.maxVelocity
        _ = animator.version
    }

    @Test
    func rectProperties() {
        let animator = ORBSymbolAnimator()
        _ = animator.alignmentRect
        _ = animator.unroundedAlignmentRect
        _ = animator.boundingRect
        _ = animator.styleMask
    }
}

#endif
