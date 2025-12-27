//
//  PathCallbacksTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics)
import CoreGraphics

@Suite(.enabled(if: compatibilityTestEnabled))
struct PathCallbacksTests {
    @Test
    func cgPathCallbacks() {
        let callbacks = ORBPath.Callbacks.cgPath
        #expect(callbacks.flags.isExtended == false)
        #expect(callbacks.retain != nil)
        #expect(callbacks.release != nil)
        #expect(callbacks.apply != nil)
        #expect(callbacks.isEqual != nil)
        #expect(callbacks.isEmpty != nil)
        #expect(callbacks.isSingleElement != nil)
        #expect(callbacks.bezierOrder != nil)
        #expect(callbacks.boundingRect != nil)
        #expect(callbacks.cgPath != nil)
        #expect(callbacks.next == nil)
    }
}

#endif

