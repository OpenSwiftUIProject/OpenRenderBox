//
//  PathCallbacksTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics

@Suite
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
        #expect(callbacks.boundingRect != nil)
        #expect(callbacks.cgPath != nil)
        #expect(callbacks.next == nil)
    }
}

#endif

