//
//  LayerTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(Darwin)
import QuartzCore

@MainActor
@Suite(.enabled(if: compatibilityTestEnabled))
struct LayerTests {
    @Test
    func testLayerProperties() {
        let layer = ORBLayer()

        // device
        layer.device = ORBDevice.sharedDefault()
        #expect(layer.device != nil)

        // rendersAsynchronously
        layer.rendersAsynchronously = true
        #expect(layer.rendersAsynchronously == true)
        layer.rendersAsynchronously = false
        #expect(layer.rendersAsynchronously == false)

        // colorMode
        layer.colorMode = 0
        #expect(layer.colorMode == 0)
        layer.colorMode = 1
        #expect(layer.colorMode == 1)

        // promotesFramebuffer
        layer.promotesFramebuffer = true
        #expect(layer.promotesFramebuffer == true)
        layer.promotesFramebuffer = false
        #expect(layer.promotesFramebuffer == false)

        // clearsBackground
        layer.clearsBackground = true
        #expect(layer.clearsBackground == true)
        layer.clearsBackground = false
        #expect(layer.clearsBackground == false)

        // maxDrawableCount
        layer.maxDrawableCount = 3
        #expect(layer.maxDrawableCount == 3)

        // allowsPackedDrawable
        layer.allowsPackedDrawable = true
        #expect(layer.allowsPackedDrawable == true)
        layer.allowsPackedDrawable = false
        #expect(layer.allowsPackedDrawable == false)

        // allowsBottomLeftOrigin
        layer.allowsBottomLeftOrigin = true
        #expect(layer.allowsBottomLeftOrigin == true)
        layer.allowsBottomLeftOrigin = false
        #expect(layer.allowsBottomLeftOrigin == false)

        // needsSynchronousUpdate
        layer.needsSynchronousUpdate = true
        #expect(layer.needsSynchronousUpdate == true)
        layer.needsSynchronousUpdate = false
        #expect(layer.needsSynchronousUpdate == false)

        _ = layer.isDrawableAvailable
        _ = layer.statistics
    }
}

#endif

