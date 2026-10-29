//
//  PathRegressionTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics

@Suite
struct PathRegressionTests {
    @Test
    func retainUsesReturnedStorage() {
        var destination: UInt64 = 0
        withUnsafePointer(to: &destination) { destination in
            var source = UnsafeRawPointer(destination)
            var callbacks = ORBPath.Callbacks()
            callbacks.retain = { object in
                object.assumingMemoryBound(to: UnsafeRawPointer.self).pointee
            }
            withUnsafePointer(to: &source) { source in
                withUnsafePointer(to: &callbacks) { callbacks in
                    let path = ORBPath(storage: unsafeBitCast(source, to: ORBPath.Storage.self), callbacks: callbacks)
                    let retained = path.retain()
                    #expect(unsafeBitCast(retained.storage, to: UnsafeRawPointer.self) == UnsafeRawPointer(destination))
                    #expect(retained.callbacks == callbacks)
                }
            }
        }
    }
}

#endif
