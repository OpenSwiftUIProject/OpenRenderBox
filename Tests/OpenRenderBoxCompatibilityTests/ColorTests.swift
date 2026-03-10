//
//  ColorTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing
import Numerics
#if canImport(CoreGraphics)
import CoreGraphics
#endif

@Suite
struct ColorTests {
    // MARK: - RBColorMake

    @Test("RBColorMake", arguments: [
        (Float(0.0), Float(0.0), Float(0.0), Float(1.0), RBColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0)),
        (Float(1.0), Float(1.0), Float(1.0), Float(1.0), RBColor(red: 1.0, green: 1.0, blue: 1.0, alpha: 1.0)),
        (Float(0.5), Float(0.25), Float(0.75), Float(0.8), RBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 0.8)),
        (Float(0.2), Float(0.4), Float(0.6), Float(0.0), RBColor(red: 0.2, green: 0.4, blue: 0.6, alpha: 0.0)),
    ])
    func colorMake(red: Float, green: Float, blue: Float, alpha: Float, expectColor: RBColor) {
        let color = RBColor(red: red, green: green, blue: blue, alpha: alpha)
        #expect(color.red == expectColor.red)
        #expect(color.green == expectColor.green)
        #expect(color.blue == expectColor.blue)
        #expect(color.alpha == expectColor.alpha)
    }

    // MARK: - RBColorMakeLinear

    @Test("RBColorMakeLinear", arguments: [
        (Float(0.5), Float(0.25), Float(0.75), Float(1.0), RBColor(linearRed: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)),
        (Float(0.2), Float(0.4), Float(0.6), Float(0.5), RBColor(linearRed: 0.2, green: 0.4, blue: 0.6, alpha: 0.5)),
    ])
    func colorMakeLinear(red: Float, green: Float, blue: Float, alpha: Float, expectColor: RBColor) {
        let color = RBColor(linearRed: red, green: green, blue: blue, alpha: alpha)
        #expect(color.red == expectColor.red)
        #expect(color.green == expectColor.green)
        #expect(color.blue == expectColor.blue)
        #expect(color.alpha == expectColor.alpha)
    }

    // MARK: - RBColorToLinear

    @Test("RBColorToLinear", arguments: [
        (Float(0.5), Float(0.25), Float(0.75), Float(0.8)),
        (Float(0.3), Float(0.6), Float(0.9), Float(1.0)),
    ])
    func colorToLinear(red: Float, green: Float, blue: Float, alpha: Float) {
        let srgbColor = RBColor(red: red, green: green, blue: blue, alpha: alpha)
        let linearColor = srgbColor.linear
        #expect(linearColor.alpha == srgbColor.alpha)
        #expect(!linearColor.red.isApproximatelyEqual(to: srgbColor.red))
        #expect(!linearColor.green.isApproximatelyEqual(to: srgbColor.green))
        #expect(!linearColor.blue.isApproximatelyEqual(to: srgbColor.blue))
    }

    // MARK: - RBColorFromLinear

    @Test("RBColorFromLinear", arguments: [
        (Float(0.2), Float(0.1), Float(0.5), Float(0.9)),
        (Float(0.05), Float(0.15), Float(0.25), Float(1.0)),
    ])
    func colorFromLinear(red: Float, green: Float, blue: Float, alpha: Float) {
        let linearColor = RBColor(red: red, green: green, blue: blue, alpha: alpha)
        let srgbColor = linearColor.fromLinear()
        #expect(srgbColor.alpha == linearColor.alpha)
        #expect(!srgbColor.red.isApproximatelyEqual(to: linearColor.red))
        #expect(!srgbColor.green.isApproximatelyEqual(to: linearColor.green))
        #expect(!srgbColor.blue.isApproximatelyEqual(to: linearColor.blue))
    }

    // MARK: - Round Trip (toLinear -> fromLinear)

    @Test("RBColor Round Trip", arguments: [
        (Float(0.5), Float(0.25), Float(0.75), Float(1.0)),
        (Float(0.0), Float(0.0), Float(0.0), Float(1.0)),
        (Float(1.0), Float(1.0), Float(1.0), Float(0.5)),
        (Float(0.3), Float(0.6), Float(0.9), Float(0.8)),
    ])
    func colorRoundTrip(red: Float, green: Float, blue: Float, alpha: Float) {
        let originalColor = RBColor(red: red, green: green, blue: blue, alpha: alpha)
        let linearColor = originalColor.linear
        let roundTripColor = linearColor.fromLinear()

        #expect(roundTripColor.red.isApproximatelyEqual(to: originalColor.red))
        #expect(roundTripColor.green.isApproximatelyEqual(to: originalColor.green))
        #expect(roundTripColor.blue.isApproximatelyEqual(to: originalColor.blue))
        #expect(roundTripColor.alpha == originalColor.alpha)
    }

    // MARK: - RBColorEqualToColor

    @Test("RBColorEqualToColor - equal")
    func colorEqualToColor_equal() {
        let color1 = RBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        let color2 = RBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        #expect(color1.isEqual(to: color2))
    }

    @Test("RBColorEqualToColor - not equal", arguments: [
        (Float(0.6), Float(0.25), Float(0.75), Float(1.0)),
        (Float(0.5), Float(0.35), Float(0.75), Float(1.0)),
        (Float(0.5), Float(0.25), Float(0.85), Float(1.0)),
        (Float(0.5), Float(0.25), Float(0.75), Float(0.5)),
    ])
    func colorEqualToColor_notEqual(r2: Float, g2: Float, b2: Float, a2: Float) {
        let color1 = RBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        let color2 = RBColor(red: r2, green: g2, blue: b2, alpha: a2)
        #expect(!color1.isEqual(to: color2))
    }

    // MARK: - Color Constants

    @Test("RBColor Constant", arguments: [
        (RBColor.clear, Float(0.0), Float(0.0), Float(0.0), Float(0.0)),
        (RBColor.black, Float(0.0), Float(0.0), Float(0.0), Float(1.0)),
        (RBColor.white, Float(1.0), Float(1.0), Float(1.0), Float(1.0)),
        (RBColor.null, Float(-32768.0), Float(-32768.0), Float(-32768.0), Float(-32768.0)),
    ])
    func constant(_ color: RBColor, expectedRed: Float, expectedGreen: Float, expectedBlue: Float, expectedAlpha: Float) {
        #expect(color.red.isApproximatelyEqual(to: expectedRed))
        #expect(color.green.isApproximatelyEqual(to: expectedGreen))
        #expect(color.blue.isApproximatelyEqual(to: expectedBlue))
        #expect(color.alpha.isApproximatelyEqual(to: expectedAlpha))
    }

    @Test("RBColorInvalidComponent")
    func colorInvalidComponent() {
        let value = RBColor.invalidComponent
        #expect(value.isApproximatelyEqual(to: -32768.0))
    }

    // MARK: - RBColorCopyCGColor

    #if canImport(CoreGraphics)
    @Test("RBColorCopyCGColor", arguments: [
        (RBColor(red: 1.0, green: 0.0, blue: 0.0, alpha: 1.0), RBColor.ColorSpace.SRGB),
        (RBColor(red: 0.0, green: 1.0, blue: 0.0, alpha: 1.0), RBColor.ColorSpace.linearSRGB),
        (RBColor(red: 0.0, green: 0.0, blue: 1.0, alpha: 0.5), RBColor.ColorSpace.displayP3),
        (RBColor(red: 0.5, green: 0.5, blue: 0.5, alpha: 1.0), RBColor.ColorSpace.linearDisplayP3),
    ])
    func colorCopyCGColor(color: RBColor, colorSpace: RBColor.ColorSpace) throws {
        let cgColor = try #require(color.cgColor(colorSpace: colorSpace))
        #expect(cgColor.numberOfComponents == 4)
        let components = cgColor.components!
        #expect(components[0].isApproximatelyEqual(to: CGFloat(color.red)))
        #expect(components[1].isApproximatelyEqual(to: CGFloat(color.green)))
        #expect(components[2].isApproximatelyEqual(to: CGFloat(color.blue)))
        #expect(components[3].isApproximatelyEqual(to: CGFloat(color.alpha)))
    }
    #endif

    #if compiler(>=6.2) // old Xcode version swift-testing bug
    @Suite
    struct ColorModeTests {
        // MARK: - RBColorModeWorkingColorSpace

        @Test("RBColorModeWorkingColorSpace", arguments: [
            (RBColor.Mode.mode0, RBColor.ColorSpace.SRGB),
            (RBColor.Mode.mode1, RBColor.ColorSpace.linearSRGB),
            (RBColor.Mode.mode2, RBColor.ColorSpace.linearSRGB),
            (RBColor.Mode.mode3, RBColor.ColorSpace.SRGB),
            (RBColor.Mode.mode4, RBColor.ColorSpace.SRGB),
            (RBColor.Mode.mode5, RBColor.ColorSpace.SRGB),
            (RBColor.Mode.mode6, RBColor.ColorSpace.linearSRGB),
            (RBColor.Mode.mode7, RBColor.ColorSpace.linearSRGB),
            (RBColor.Mode.mode8, RBColor.ColorSpace.linearSRGB),
            (RBColor.Mode.mode9, RBColor.ColorSpace.SRGB),
            (RBColor.Mode.mode10, RBColor.ColorSpace.linearSRGB),
            (RBColor.Mode.mode11, RBColor.ColorSpace.SRGB),
            (RBColor.Mode.mode12, RBColor.ColorSpace.SRGB),
            (RBColor.Mode.mode13, RBColor.ColorSpace.linearSRGB),
            (RBColor.Mode.mode14, RBColor.ColorSpace.SRGB),
            (RBColor.Mode.mode15, RBColor.ColorSpace.linearSRGB),
        ])
        func colorModeWorkingColorSpace(mode: RBColor.Mode, expectedColorSpace: RBColor.ColorSpace) {
            #expect(mode.workingColorSpace == expectedColorSpace)
        }

        // MARK: - RBColorModeHasExtendedRange

        @Test("RBColorModeHasExtendedRange", arguments: [
            (RBColor.Mode.mode0, false),
            (RBColor.Mode.mode1, false),
            (RBColor.Mode.mode2, true),
            (RBColor.Mode.mode3, false),
            (RBColor.Mode.mode4, false),
            (RBColor.Mode.mode5, false),
            (RBColor.Mode.mode6, false),
            (RBColor.Mode.mode7, false),
            (RBColor.Mode.mode8, false),
            (RBColor.Mode.mode9, false),
            (RBColor.Mode.mode10, false),
            (RBColor.Mode.mode11, true),
            (RBColor.Mode.mode12, true),
            (RBColor.Mode.mode13, true),
            (RBColor.Mode.mode14, false),
            (RBColor.Mode.mode15, false),
        ])
        func colorModeHasExtendedRange(mode: RBColor.Mode, expectedResult: Bool) {
            #expect(mode.hasExtendedRange == expectedResult)
        }
    }
    #endif
}
