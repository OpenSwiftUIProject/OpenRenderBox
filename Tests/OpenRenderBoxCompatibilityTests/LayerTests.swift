//
//  LayerTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(Darwin)
import QuartzCore

@MainActor
@Suite(.enabled(if: compatibilityTestEnabled))
struct ORBLayerTests {
    @Test
    func init_layer() {
        let layer = ORBLayer()
        #expect(layer != nil)
    }

    @Test
    func deviceProperty() {
        let layer = ORBLayer()!
        layer.device = ORBDevice.sharedDefault()
        #expect(layer.device != nil)
    }

    @Test
    func rendersAsynchronouslyProperty() {
        let layer = ORBLayer()!
        layer.rendersAsynchronously = true
        #expect(layer.rendersAsynchronously == true)
        layer.rendersAsynchronously = false
        #expect(layer.rendersAsynchronously == false)
    }

    @Test
    func colorModeProperty() {
        let layer = ORBLayer()!
        layer.colorMode = 0
        #expect(layer.colorMode == 0)
        layer.colorMode = 1
        #expect(layer.colorMode == 1)
    }

    @Test
    func promotesFramebufferProperty() {
        let layer = ORBLayer()!
        layer.promotesFramebuffer = true
        #expect(layer.promotesFramebuffer == true)
        layer.promotesFramebuffer = false
        #expect(layer.promotesFramebuffer == false)
    }

    @Test
    func clearsBackgroundProperty() {
        let layer = ORBLayer()!
        layer.clearsBackground = true
        #expect(layer.clearsBackground == true)
        layer.clearsBackground = false
        #expect(layer.clearsBackground == false)
    }

    @Test
    func maxDrawableCountProperty() {
        let layer = ORBLayer()!
        layer.maxDrawableCount = 3
        #expect(layer.maxDrawableCount == 3)
    }

    @Test
    func allowsPackedDrawableProperty() {
        let layer = ORBLayer()!
        layer.allowsPackedDrawable = true
        #expect(layer.allowsPackedDrawable == true)
        layer.allowsPackedDrawable = false
        #expect(layer.allowsPackedDrawable == false)
    }

    @Test
    func allowsBottomLeftOriginProperty() {
        let layer = ORBLayer()!
        layer.allowsBottomLeftOrigin = true
        #expect(layer.allowsBottomLeftOrigin == true)
        layer.allowsBottomLeftOrigin = false
        #expect(layer.allowsBottomLeftOrigin == false)
    }

    @Test
    func needsSynchronousUpdateProperty() {
        let layer = ORBLayer()!
        layer.needsSynchronousUpdate = true
        #expect(layer.needsSynchronousUpdate == true)
        layer.needsSynchronousUpdate = false
        #expect(layer.needsSynchronousUpdate == false)
    }

    @Test
    func drawableAvailableProperty() {
        let layer = ORBLayer()!
        // Just check it doesn't crash - value depends on device state
        _ = layer.drawableAvailable
    }

    @Test
    func statisticsProperty() {
        let layer = ORBLayer()!
        // Just check it doesn't crash - may be nil
        _ = layer.statistics
    }
}

#endif

