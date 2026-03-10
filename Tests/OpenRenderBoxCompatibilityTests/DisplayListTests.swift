//
//  DisplayListTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(Darwin)
import CoreGraphics
#if canImport(AppKit)
import AppKit
#endif

@Suite(.enabled(if: compatibilityTestEnabled))
struct DisplayListTests {
    @Test
    func testDisplayListInit() {
        let displayList = ORBDisplayList()
        #expect(displayList.isEmpty())
    }

    @Test
    func testDisplayListProperties() {
        let displayList = ORBDisplayList()

        // defaultColorSpace
        displayList.defaultColorSpace = .SRGB
        #expect(displayList.defaultColorSpace == .SRGB)
        displayList.defaultColorSpace = .default
        #expect(displayList.defaultColorSpace == .default)

        // linearColors
        displayList.linearColors = true
        #expect(displayList.linearColors == true)
        displayList.linearColors = false
        #expect(displayList.linearColors == false)

        // deviceScale
        displayList.deviceScale = 2.0
        #expect(displayList.deviceScale == 2.0)

        // contentRect
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        displayList.contentRect = rect
        #expect(displayList.contentRect == rect)
    }

    @Test
    func testBeginEndCGContext() {
        let displayList = ORBDisplayList()
        let context = displayList.beginCGContext(withAlpha: 1.0)
        context.setFillColor(CGColor(red: 1, green: 0, blue: 0, alpha: 1))
        context.fill([CGRect(x: 0, y: 0, width: 10, height: 10)])
        displayList.endCGContext()
        #expect(!displayList.isEmpty())
    }

    @Test
    func testMoveContents() {
        let displayList = ORBDisplayList()
        displayList.defaultColorSpace = .SRGB
        let context = displayList.beginCGContext(withAlpha: 1.0)
        context.setFillColor(CGColor(red: 1, green: 0, blue: 0, alpha: 1))
        context.fill([CGRect(x: 0, y: 0, width: 10, height: 10)])
        displayList.endCGContext()

        let contents = displayList.moveContents()
        #expect(contents != nil)
        #expect(displayList.isEmpty())
    }

    #if canImport(AppKit)
    @Test
    func testNSImageDisplayList() throws {
        let size = CGSize(width: 16, height: 16)
        let image = NSImage(size: size, flipped: false) { rect in
            NSColor.red.setFill()
            rect.fill()
            return true
        }

        let displayList = ORBDisplayList()
        displayList.defaultColorSpace = .SRGB
        var rect = CGRect(origin: .zero, size: size)
        let scale: CGFloat = 2.0
        let cgImage = try #require(
            image.cgImage(
                forProposedRect: &rect,
                context: nil,
                hints: [.ctm: AffineTransform(scaleByX: scale, byY: scale)]
            )
        )
        let context = displayList.beginCGContext(withAlpha: 1.0)
        context.draw(cgImage, in: CGRect(origin: .zero, size: size))
        displayList.endCGContext()

        let contents = displayList.moveContents()
        #expect(contents != nil)
        #expect(contents!.isEmpty() == false)
    }
    #endif
}

#endif
