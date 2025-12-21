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
        #expect(callbacks.reserved == nil)
        #expect(callbacks.retain != nil)
        #expect(callbacks.release != nil)
        #expect(callbacks.apply != nil)
        #expect(callbacks.isEqual != nil)
        #expect(callbacks.isEmpty != nil)
        #expect(callbacks.isSingleRect != nil)
        #expect(callbacks.bezierOrder != nil)
        #expect(callbacks.boundingBox != nil)
        #expect(callbacks.cgPath != nil)
        #expect(callbacks.reserved2 == nil)
    }

    @Test
    func callbacksStructSize() {
        #expect(MemoryLayout<ORBPath.Callbacks>.size == 11 * MemoryLayout<UnsafeRawPointer>.size)
    }
}

#endif

