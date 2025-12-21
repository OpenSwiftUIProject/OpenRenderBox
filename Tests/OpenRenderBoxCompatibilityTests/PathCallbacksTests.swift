//
//  PathCallbacksTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics)
import CoreGraphics

@Suite(.enabled(if: compatibilityTestEnabled))
struct PathCallbacksTests {
    @Test
    func emptyCallbacks() {
        let callbacks = ORBPathEmptyCallbacks
        #expect(callbacks.reserved == nil)
        #expect(callbacks.retain == nil)
        #expect(callbacks.release == nil)
        #expect(callbacks.apply == nil)
        #expect(callbacks.isEqual == nil)
        #expect(callbacks.isEmpty == nil)
        #expect(callbacks.isSingleRect == nil)
        #expect(callbacks.bezierOrder == nil)
        #expect(callbacks.boundingBox == nil)
        #expect(callbacks.cgPath == nil)
        #expect(callbacks.reserved2 == nil)
    }
    
    @Test
    func cgPathCallbacks() {
        let callbacks = ORBPathCGPathCallbacks
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
    func pathUsesCallbacks() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(rect: rect, transform: nil)
        #expect(path.callbacks == withUnsafePointer(to: ORBPathCGPathCallbacks) { $0 })
        path.release()
    }
    
    @Test
    func emptyPathUsesEmptyCallbacks() {
        let emptyPath = ORBPathEmpty
        #expect(emptyPath.callbacks == withUnsafePointer(to: ORBPathEmptyCallbacks) { $0 })
    }
    
    @Test
    func nullPathUsesEmptyCallbacks() {
        let nullPath = ORBPathNull
        #expect(nullPath.callbacks == withUnsafePointer(to: ORBPathEmptyCallbacks) { $0 })
    }
    
    @Test
    func callbacksStructSize() {
        // ORBPathCallbacks should be 11 pointers (0x58 bytes on 64-bit)
        let expectedSize = 11 * MemoryLayout<UnsafeRawPointer?>.size
        #expect(MemoryLayout<ORBPathCallbacks>.size == expectedSize)
    }
}

#endif

