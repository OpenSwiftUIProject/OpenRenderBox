//
//  PathCallbacksTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(Darwin)
import CoreGraphics

@Suite
struct PathCallbacksTests {
    @Test
    func cgPathBezierOrder() {
        let cgPath = CGMutablePath()
        cgPath.move(to: .zero)
        cgPath.addCurve(to: CGPoint(x: 10, y: 0), control1: CGPoint(x: 0, y: 10), control2: CGPoint(x: 10, y: 10))
        let path = ORBPath(cgPath: cgPath)
        defer { path.release() }
        #expect(path.bezierOrder == 3)
    }

    @Test
    func cgPathCallbacks() {
        let rectPath = ORBPath(cgPath: CGPath(rect: CGRect(x: 0, y: 0, width: 10, height: 10), transform: nil))
        defer { rectPath.release() }
        #expect(rectPath.callbacks == ORBPath.Callbacks.cgPath)

        let callbacks = ORBPath.Callbacks.cgPath.pointee
        #expect(callbacks.flags.isExtended == false)
        #expect(callbacks.retain != nil)
        #expect(callbacks.release != nil)
        #expect(callbacks.apply != nil)
        #expect(callbacks.isEqual != nil)
        #expect(callbacks.isEmpty != nil)
        #expect(callbacks.isSingleElement != nil)
        #expect(callbacks.boundingRect != nil)
        #expect(callbacks.cgPath != nil)
        #expect(callbacks.next == nil)
    }
}

#endif

