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
    // MARK: - ORBColorMake

    @Test("ORBColorMake", arguments: [
        (Float(0.0), Float(0.0), Float(0.0), Float(1.0), ORBColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0)),
        (Float(1.0), Float(1.0), Float(1.0), Float(1.0), ORBColor(red: 1.0, green: 1.0, blue: 1.0, alpha: 1.0)),
        (Float(0.5), Float(0.25), Float(0.75), Float(0.8), ORBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 0.8)),
        (Float(0.2), Float(0.4), Float(0.6), Float(0.0), ORBColor(red: 0.2, green: 0.4, blue: 0.6, alpha: 0.0)),
    ])
    func colorMake(red: Float, green: Float, blue: Float, alpha: Float, expectColor: ORBColor) {
        let color = ORBColor(red: red, green: green, blue: blue, alpha: alpha)
        #expect(color.red == expectColor.red)
        #expect(color.green == expectColor.green)
        #expect(color.blue == expectColor.blue)
        #expect(color.alpha == expectColor.alpha)
    }

    // MARK: - ORBColorMakeLinear

    @Test("ORBColorMakeLinear", arguments: [
        (Float(0.5), Float(0.25), Float(0.75), Float(1.0), ORBColor(linearRed: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)),
        (Float(0.2), Float(0.4), Float(0.6), Float(0.5), ORBColor(linearRed: 0.2, green: 0.4, blue: 0.6, alpha: 0.5)),
    ])
    func colorMakeLinear(red: Float, green: Float, blue: Float, alpha: Float, expectColor: ORBColor) {
        let color = ORBColor(linearRed: red, green: green, blue: blue, alpha: alpha)
        #expect(color.red == expectColor.red)
        #expect(color.green == expectColor.green)
        #expect(color.blue == expectColor.blue)
        #expect(color.alpha == expectColor.alpha)
    }

    // MARK: - ORBColorToLinear

    @Test("ORBColorToLinear", arguments: [
        (Float(0.5), Float(0.25), Float(0.75), Float(0.8)),
        (Float(0.3), Float(0.6), Float(0.9), Float(1.0)),
    ])
    func colorToLinear(red: Float, green: Float, blue: Float, alpha: Float) {
        let srgbColor = ORBColor(red: red, green: green, blue: blue, alpha: alpha)
        let linearColor = srgbColor.linear
        #expect(linearColor.alpha == srgbColor.alpha)
        #expect(!linearColor.red.isApproximatelyEqual(to: srgbColor.red))
        #expect(!linearColor.green.isApproximatelyEqual(to: srgbColor.green))
        #expect(!linearColor.blue.isApproximatelyEqual(to: srgbColor.blue))
    }

    // MARK: - ORBColorFromLinear

    @Test("ORBColorFromLinear", arguments: [
        (Float(0.2), Float(0.1), Float(0.5), Float(0.9)),
        (Float(0.05), Float(0.15), Float(0.25), Float(1.0)),
    ])
    func colorFromLinear(red: Float, green: Float, blue: Float, alpha: Float) {
        let linearColor = ORBColor(red: red, green: green, blue: blue, alpha: alpha)
        let srgbColor = linearColor.fromLinear()
        #expect(srgbColor.alpha == linearColor.alpha)
        #expect(!srgbColor.red.isApproximatelyEqual(to: linearColor.red))
        #expect(!srgbColor.green.isApproximatelyEqual(to: linearColor.green))
        #expect(!srgbColor.blue.isApproximatelyEqual(to: linearColor.blue))
    }

    // MARK: - Round Trip (toLinear -> fromLinear)

    @Test("ORBColor Round Trip", arguments: [
        (Float(0.5), Float(0.25), Float(0.75), Float(1.0)),
        (Float(0.0), Float(0.0), Float(0.0), Float(1.0)),
        (Float(1.0), Float(1.0), Float(1.0), Float(0.5)),
        (Float(0.3), Float(0.6), Float(0.9), Float(0.8)),
    ])
    func colorRoundTrip(red: Float, green: Float, blue: Float, alpha: Float) {
        let originalColor = ORBColor(red: red, green: green, blue: blue, alpha: alpha)
        let linearColor = originalColor.linear
        let roundTripColor = linearColor.fromLinear()

        #expect(roundTripColor.red.isApproximatelyEqual(to: originalColor.red))
        #expect(roundTripColor.green.isApproximatelyEqual(to: originalColor.green))
        #expect(roundTripColor.blue.isApproximatelyEqual(to: originalColor.blue))
        #expect(roundTripColor.alpha == originalColor.alpha)
    }

    // MARK: - ORBColorEqualToColor

    @Test("ORBColorEqualToColor - equal")
    func colorEqualToColor_equal() {
        let color1 = ORBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        let color2 = ORBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        #expect(color1.isEqual(to: color2))
    }

    @Test("ORBColorEqualToColor - not equal", arguments: [
        (Float(0.6), Float(0.25), Float(0.75), Float(1.0)),
        (Float(0.5), Float(0.35), Float(0.75), Float(1.0)),
        (Float(0.5), Float(0.25), Float(0.85), Float(1.0)),
        (Float(0.5), Float(0.25), Float(0.75), Float(0.5)),
    ])
    func colorEqualToColor_notEqual(r2: Float, g2: Float, b2: Float, a2: Float) {
        let color1 = ORBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        let color2 = ORBColor(red: r2, green: g2, blue: b2, alpha: a2)
        #expect(!color1.isEqual(to: color2))
    }

    // MARK: - Color Constants

    @Test("ORBColor Constant", arguments: [
        (ORBColor.clear, Float(0.0), Float(0.0), Float(0.0), Float(0.0)),
        (ORBColor.black, Float(0.0), Float(0.0), Float(0.0), Float(1.0)),
        (ORBColor.white, Float(1.0), Float(1.0), Float(1.0), Float(1.0)),
        (ORBColor.null, Float(-32768.0), Float(-32768.0), Float(-32768.0), Float(-32768.0)),
    ])
    func constant(_ color: ORBColor, expectedRed: Float, expectedGreen: Float, expectedBlue: Float, expectedAlpha: Float) {
        #expect(color.red.isApproximatelyEqual(to: expectedRed))
        #expect(color.green.isApproximatelyEqual(to: expectedGreen))
        #expect(color.blue.isApproximatelyEqual(to: expectedBlue))
        #expect(color.alpha.isApproximatelyEqual(to: expectedAlpha))
    }

    @Test("ORBColorInvalidComponent")
    func colorInvalidComponent() {
        let value = ORBColor.invalidComponent
        #expect(value.isApproximatelyEqual(to: -32768.0))
    }

    // MARK: - ORBColorCopyCGColor

    #if canImport(CoreGraphics)
    @Test("ORBColorCopyCGColor", arguments: [
        (ORBColor(red: 1.0, green: 0.0, blue: 0.0, alpha: 1.0), ORBColor.ColorSpace.SRGB),
        (ORBColor(red: 0.0, green: 1.0, blue: 0.0, alpha: 1.0), ORBColor.ColorSpace.linearSRGB),
        (ORBColor(red: 0.0, green: 0.0, blue: 1.0, alpha: 0.5), ORBColor.ColorSpace.displayP3),
        (ORBColor(red: 0.5, green: 0.5, blue: 0.5, alpha: 1.0), ORBColor.ColorSpace.linearDisplayP3),
    ])
    func colorCopyCGColor(color: ORBColor, colorSpace: ORBColor.ColorSpace) throws {
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
        // MARK: - ORBColorModeWorkingColorSpace

        @Test("ORBColorModeWorkingColorSpace", arguments: [
            (ORBColor.Mode.mode0, ORBColor.ColorSpace.SRGB),
            (ORBColor.Mode.mode1, ORBColor.ColorSpace.linearSRGB),
            (ORBColor.Mode.mode2, ORBColor.ColorSpace.linearSRGB),
            (ORBColor.Mode.mode3, ORBColor.ColorSpace.SRGB),
            (ORBColor.Mode.mode4, ORBColor.ColorSpace.SRGB),
            (ORBColor.Mode.mode5, ORBColor.ColorSpace.SRGB),
            (ORBColor.Mode.mode6, ORBColor.ColorSpace.linearSRGB),
            (ORBColor.Mode.mode7, ORBColor.ColorSpace.linearSRGB),
            (ORBColor.Mode.mode8, ORBColor.ColorSpace.linearSRGB),
            (ORBColor.Mode.mode9, ORBColor.ColorSpace.SRGB),
            (ORBColor.Mode.mode10, ORBColor.ColorSpace.linearSRGB),
            (ORBColor.Mode.mode11, ORBColor.ColorSpace.SRGB),
            (ORBColor.Mode.mode12, ORBColor.ColorSpace.SRGB),
            (ORBColor.Mode.mode13, ORBColor.ColorSpace.linearSRGB),
            (ORBColor.Mode.mode14, ORBColor.ColorSpace.SRGB),
            (ORBColor.Mode.mode15, ORBColor.ColorSpace.linearSRGB),
        ])
        func colorModeWorkingColorSpace(mode: ORBColor.Mode, expectedColorSpace: ORBColor.ColorSpace) {
            #expect(mode.workingColorSpace == expectedColorSpace)
        }

        // MARK: - ORBColorModeHasExtendedRange

        @Test("ORBColorModeHasExtendedRange", arguments: [
            (ORBColor.Mode.mode0, false),
            (ORBColor.Mode.mode1, false),
            (ORBColor.Mode.mode2, true),
            (ORBColor.Mode.mode3, false),
            (ORBColor.Mode.mode4, false),
            (ORBColor.Mode.mode5, false),
            (ORBColor.Mode.mode6, false),
            (ORBColor.Mode.mode7, false),
            (ORBColor.Mode.mode8, false),
            (ORBColor.Mode.mode9, false),
            (ORBColor.Mode.mode10, false),
            (ORBColor.Mode.mode11, true),
            (ORBColor.Mode.mode12, true),
            (ORBColor.Mode.mode13, true),
            (ORBColor.Mode.mode14, false),
            (ORBColor.Mode.mode15, false),
        ])
        func colorModeHasExtendedRange(mode: ORBColor.Mode, expectedResult: Bool) {
            #expect(mode.hasExtendedRange == expectedResult)
        }
    }
    #endif
}
