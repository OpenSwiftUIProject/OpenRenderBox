//
//  ColorTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing
import Numerics

@Suite
struct ColorTests {
    // MARK: - ORBColorMake

    @Test(arguments: [
        (red: Float(0.0), green: Float(0.0), blue: Float(0.0), alpha: Float(1.0)),
        (red: Float(1.0), green: Float(1.0), blue: Float(1.0), alpha: Float(1.0)),
        (red: Float(0.5), green: Float(0.25), blue: Float(0.75), alpha: Float(0.8)),
        (red: Float(0.2), green: Float(0.4), blue: Float(0.6), alpha: Float(0.0)),
    ])
    func colorMake(red: Float, green: Float, blue: Float, alpha: Float) {
        let color = ORBColor(red: red, green: green, blue: blue, alpha: alpha)
        #expect(color.red == red)
        #expect(color.green == green)
        #expect(color.blue == blue)
        #expect(color.alpha == alpha)
    }

    // MARK: - ORBColorMakeLinear

    @Test(arguments: [
        (red: Float(0.5), green: Float(0.25), blue: Float(0.75), alpha: Float(1.0)),
        (red: Float(0.2), green: Float(0.4), blue: Float(0.6), alpha: Float(0.5)),
    ])
    func colorMakeLinear(red: Float, green: Float, blue: Float, alpha: Float) {
        let color = ORBColor(linearRed: red, green: green, blue: blue, alpha: alpha)
        // Alpha should be unchanged
        #expect(color.alpha == alpha)
        // RGB values should be converted to linear (smaller for typical sRGB values > threshold)
        #expect(!color.red.isApproximatelyEqual(to: red, absoluteTolerance: 0.0001))
        #expect(!color.green.isApproximatelyEqual(to: green, absoluteTolerance: 0.0001))
        #expect(!color.blue.isApproximatelyEqual(to: blue, absoluteTolerance: 0.0001))
    }

    // MARK: - ORBColorToLinear

    @Test(arguments: [
        (red: Float(0.5), green: Float(0.25), blue: Float(0.75), alpha: Float(0.8)),
        (red: Float(0.3), green: Float(0.6), blue: Float(0.9), alpha: Float(1.0)),
    ])
    func colorToLinear(red: Float, green: Float, blue: Float, alpha: Float) {
        let srgbColor = ORBColor(red: red, green: green, blue: blue, alpha: alpha)
        let linearColor = srgbColor.linear

        // Alpha should be unchanged
        #expect(linearColor.alpha == srgbColor.alpha)
        // Linear values should differ from sRGB for values > threshold
        #expect(!linearColor.red.isApproximatelyEqual(to: srgbColor.red, absoluteTolerance: 0.0001))
        #expect(!linearColor.green.isApproximatelyEqual(to: srgbColor.green, absoluteTolerance: 0.0001))
        #expect(!linearColor.blue.isApproximatelyEqual(to: srgbColor.blue, absoluteTolerance: 0.0001))
    }

    // MARK: - ORBColorFromLinear

    @Test(arguments: [
        (red: Float(0.2), green: Float(0.1), blue: Float(0.5), alpha: Float(0.9)),
        (red: Float(0.05), green: Float(0.15), blue: Float(0.25), alpha: Float(1.0)),
    ])
    func colorFromLinear(red: Float, green: Float, blue: Float, alpha: Float) {
        let linearColor = ORBColor(red: red, green: green, blue: blue, alpha: alpha)
        let srgbColor = linearColor.fromLinear()

        // Alpha should be unchanged
        #expect(srgbColor.alpha == linearColor.alpha)
        // sRGB values should differ from linear for values > threshold
        #expect(!srgbColor.red.isApproximatelyEqual(to: linearColor.red, absoluteTolerance: 0.0001))
        #expect(!srgbColor.green.isApproximatelyEqual(to: linearColor.green, absoluteTolerance: 0.0001))
        #expect(!srgbColor.blue.isApproximatelyEqual(to: linearColor.blue, absoluteTolerance: 0.0001))
    }

    // MARK: - Round Trip (toLinear -> fromLinear)

    @Test(arguments: [
        (red: Float(0.5), green: Float(0.25), blue: Float(0.75), alpha: Float(1.0)),
        (red: Float(0.0), green: Float(0.0), blue: Float(0.0), alpha: Float(1.0)),
        (red: Float(1.0), green: Float(1.0), blue: Float(1.0), alpha: Float(0.5)),
        (red: Float(0.3), green: Float(0.6), blue: Float(0.9), alpha: Float(0.8)),
    ])
    func colorRoundTrip(red: Float, green: Float, blue: Float, alpha: Float) {
        let originalColor = ORBColor(red: red, green: green, blue: blue, alpha: alpha)
        let linearColor = originalColor.linear
        let roundTripColor = linearColor.fromLinear()

        #expect(roundTripColor.red.isApproximatelyEqual(to: originalColor.red, absoluteTolerance: 0.0001))
        #expect(roundTripColor.green.isApproximatelyEqual(to: originalColor.green, absoluteTolerance: 0.0001))
        #expect(roundTripColor.blue.isApproximatelyEqual(to: originalColor.blue, absoluteTolerance: 0.0001))
        #expect(roundTripColor.alpha == originalColor.alpha)
    }

    // MARK: - ORBColorEqualToColor

    @Test
    func colorEqualToColor_equal() {
        let color1 = ORBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        let color2 = ORBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        #expect(color1.isEqual(to: color2))
    }

    @Test(arguments: [
        (r2: Float(0.6), g2: Float(0.25), b2: Float(0.75), a2: Float(1.0)),  // different red
        (r2: Float(0.5), g2: Float(0.35), b2: Float(0.75), a2: Float(1.0)),  // different green
        (r2: Float(0.5), g2: Float(0.25), b2: Float(0.85), a2: Float(1.0)),  // different blue
        (r2: Float(0.5), g2: Float(0.25), b2: Float(0.75), a2: Float(0.5)),  // different alpha
    ])
    func colorEqualToColor_notEqual(r2: Float, g2: Float, b2: Float, a2: Float) {
        let color1 = ORBColor(red: 0.5, green: 0.25, blue: 0.75, alpha: 1.0)
        let color2 = ORBColor(red: r2, green: g2, blue: b2, alpha: a2)
        #expect(!color1.isEqual(to: color2))
    }
}

