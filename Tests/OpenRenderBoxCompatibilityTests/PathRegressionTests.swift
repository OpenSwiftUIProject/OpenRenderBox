//
//  PathRegressionTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics

@Suite
struct PathRegressionTests {
    private struct ElementSource {
        var element: ORBPath.Element
        var points: UnsafePointer<CGFloat>
    }

    private func withElementPath<Result>(
        element: ORBPath.Element,
        points: [CGFloat],
        _ body: (ORBPath) throws -> Result
    ) rethrows -> Result {
        try points.withUnsafeBufferPointer { points in
            var source = ElementSource(element: element, points: points.baseAddress!)
            var callbacks = ORBPath.Callbacks()
            callbacks.apply = { object, info, callback in
                let source = object.assumingMemoryBound(to: ElementSource.self).pointee
                return callback?(info, source.element, source.points, nil) ?? true
            }
            callbacks.isSingleElement = { _ in true }
            return try withUnsafePointer(to: &source) { source in
                try withUnsafePointer(to: &callbacks) { callbacks in
                    let path = ORBPath(storage: unsafeBitCast(source, to: ORBPath.Storage.self), callbacks: callbacks)
                    return try body(path)
                }
            }
        }
    }

    @Test(arguments: [ORBPath.Element.moveToPoint, .addLineToPoint, .closeSubpath])
    func shapeRecognitionAcceptsMinimalPayloads(element: ORBPath.Element) {
        let values: [CGFloat] = element == .closeSubpath ? [0] : [1, 2]
        withElementPath(element: element, points: values) { path in
            var callbackCount = 0
            let recognized = path.applyShape(allowLine: true, info: &callbackCount) { info, _, _ in
                info.assumingMemoryBound(to: Int.self).pointee += 1
            }
            #expect(!recognized)
            #expect(callbackCount == 0)
        }
    }

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

    @Test
    func batchContainmentPreservesAllMaskBits() {
        let path = ORBPath(rect: CGRect(x: 0, y: 0, width: 10, height: 10), transform: nil)
        defer { path.release() }
        var points = Array(repeating: CGPoint(x: 20, y: 20), count: 65)
        points[0] = CGPoint(x: 5, y: 5)
        points[63] = CGPoint(x: 5, y: 5)
        points[64] = CGPoint(x: 5, y: 5)
        #expect(path.containsPoints(count: 0, points: points, eoFill: false, transform: nil) == 0)
        #expect(path.containsPoints(count: 65, points: points, eoFill: false, transform: nil) == (UInt64(1) | (UInt64(1) << 63)))
    }

    @Test
    func batchContainmentTransformsQueryPoints() {
        let path = ORBPath(rect: CGRect(x: 0, y: 0, width: 10, height: 10), transform: nil)
        defer { path.release() }
        var transform = CGAffineTransform(translationX: -10, y: -20)
        let points = [CGPoint(x: 15, y: 25), CGPoint(x: 5, y: 5)]
        #expect(path.containsPoints(count: 2, points: points, eoFill: false, transform: &transform) & 3 == 1)
    }

    @Test
    func containmentUsesTheRequestedWindingRule() {
        let cgPath = CGMutablePath()
        cgPath.addRect(CGRect(x: 0, y: 0, width: 20, height: 20))
        cgPath.addRect(CGRect(x: 5, y: 5, width: 10, height: 10))
        let path = ORBPath(cgPath: cgPath)
        defer { path.release() }
        #expect(path.contains(point: CGPoint(x: 10, y: 10), eoFill: false))
        #expect(!path.contains(point: CGPoint(x: 10, y: 10), eoFill: true))
        #expect(path.contains(point: CGPoint(x: 2, y: 2), eoFill: true))
    }

    @Test
    func nullPrimitiveRectanglesUseEmptyStorage() {
        let paths = [
            ORBPath(rect: .null, transform: nil),
            ORBPath(ellipseIn: .null, transform: nil),
            ORBPath(roundedRect: .null, cornerWidth: 4, cornerHeight: 5, style: .circular, transform: nil),
            ORBPath(roundedRect: .null, topLeftRadius: 1, topRightRadius: 2, bottomRightRadius: 3, bottomLeftRadius: 4, style: .continuous, transform: nil),
        ]
        for path in paths {
            #expect(unsafeBitCast(path.storage, to: UInt.self) == 0)
            #expect(path.isEmpty)
            path.release()
        }
        let null = ORBPath(cgPath: nil)
        #expect(unsafeBitCast(null.storage, to: UInt.self) == 1)
        null.release()
    }

    @Test
    func sentinelEqualityPreservesNullMarker() {
        #expect(ORBPath.empty.isEqual(to: .null))
        #expect(ORBPath.null.isEqual(to: .empty))
        var callbacks = ORBPath.Callbacks()
        callbacks.apply = { _, _, _ in true }
        withUnsafePointer(to: &callbacks) { callbacks in
            var payload: UInt64 = 0
            withUnsafePointer(to: &payload) { payload in
                let custom = ORBPath(storage: unsafeBitCast(payload, to: ORBPath.Storage.self), callbacks: callbacks)
                #expect(ORBPath.empty.isEqual(to: custom))
                #expect(custom.isEqual(to: .empty))
                #expect(!ORBPath.null.isEqual(to: custom))
                #expect(!custom.isEqual(to: .null))
            }
        }
    }

    @Test(arguments: [
        (CGPoint(x: 5, y: 0), false),
        (CGPoint(x: 10, y: 5), false),
        (CGPoint(x: 5, y: 10), false),
        (CGPoint(x: 0, y: 5), true),
        (CGPoint(x: 5, y: 5), true),
    ])
    func ellipseBoundaryContainment(point: CGPoint, expected: Bool) {
        let path = ORBPath(ellipseIn: CGRect(x: 0, y: 0, width: 10, height: 10), transform: nil)
        defer { path.release() }
        #expect(path.contains(point: point, eoFill: false) == expected)
    }

    @Test
    func equalityFallsBackToElementContents() {
        let values: [CGFloat] = [1, 2, 30, 40]
        withElementPath(element: .rect, points: values) { lhs in
            withElementPath(element: .rect, points: values) { rhs in
                #expect(lhs.callbacks != rhs.callbacks)
                #expect(lhs.isEqual(to: rhs))
                let sameCallbacks = ORBPath(storage: rhs.storage, callbacks: lhs.callbacks)
                #expect(lhs.isEqual(to: sameCallbacks))
            }
            withElementPath(element: .rect, points: [1, 2, 30, 41]) { rhs in
                #expect(!lhs.isEqual(to: rhs))
            }
            withElementPath(element: .ellipse, points: values) { rhs in
                #expect(!lhs.isEqual(to: rhs))
            }
        }
    }

    @Test
    func genericEqualityUsesCompactStoragePrecision() {
        let values: [CGFloat] = [0.1, 0.2, 30.3, 40.4]
        let compactValues = values.map { CGFloat(Float($0)) }
        withElementPath(element: .rect, points: values) { original in
            withElementPath(element: .rect, points: compactValues) { compact in
                #expect(original.isEqual(to: compact))
                #expect(!compact.isEqual(to: original))
            }
            withElementPath(element: .rect, points: values) { other in
                #expect(!original.isEqual(to: other))
            }
        }
    }

    @Test(arguments: [ORBPath.RoundedCornerStyle.circular, .continuous])
    func unevenRoundedRectanglePreservesPrimitiveStyle(style: ORBPath.RoundedCornerStyle) {
        let rect = CGRect(x: 1, y: 2, width: 100, height: 80)
        let path = ORBPath(roundedRect: rect, topLeftRadius: 5, topRightRadius: 10, bottomRightRadius: 15, bottomLeftRadius: 20, style: style, transform: nil)
        defer { path.release() }
        let elements = pathElements(path)
        #expect(elements.count == 1)
        #expect(elements.first?.element == (style == .circular ? ORBPath.Element.unevenRoundedRectCircular.rawValue : ORBPath.Element.unevenRoundedRectContinuous.rawValue))
        #expect(path.boundingRect == rect)
        #expect(path.contains(point: CGPoint(x: 50, y: 40), eoFill: false))
        #expect(!path.contains(point: CGPoint(x: 1.01, y: 2.01), eoFill: false))
        var actualStyle = ORBPath.RoundedCornerStyle.circular
        var actualType = ORBPathShapeType.rect
        #expect(path.applyShape(allowLine: true, info: &actualStyle) { info, type, shape in
            if type == .unevenRoundedRect {
                info.assumingMemoryBound(to: ORBPath.RoundedCornerStyle.self).pointee = shape.pointee.unevenRoundedRect.style
            }
        })
        #expect(actualStyle == style)
        #expect(path.applyShape(allowLine: true, info: &actualType) { info, type, _ in
            info.assumingMemoryBound(to: ORBPathShapeType.self).pointee = type
        })
        #expect(actualType == .unevenRoundedRect)
    }

    @Test
    func continuousRoundedRectangleGeometry() {
        let path = ORBPath(roundedRect: CGRect(x: 0, y: 0, width: 100, height: 80), cornerWidth: 10, cornerHeight: 20, style: .continuous, transform: nil)
        defer { path.release() }
        let elements = cgPathElements(path.cgPath)
        #expect(elements.count == 18)
        #expect(elements.prefix(3).map(\.element) == [0, 1, 3])
        let actual = elements.prefix(3).flatMap(\.points)
        let expected: [CGFloat] = [100, 40, 100, 49.426701068878174,
                                  100, 58.230199813842773, 100, 62.63185977935791,
                                  99.250885993242264, 67.370120286941528]
        #expect(actual.count == expected.count)
        for (actual, expected) in zip(actual, expected) {
            #expect(abs(actual - expected) < 1e-10)
        }
    }

    @Test
    func unevenRoundedRectangleStylesHaveDifferentGeometry() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 80)
        let circular = ORBPath(roundedRect: rect, topLeftRadius: 5, topRightRadius: 10, bottomRightRadius: 15, bottomLeftRadius: 20, style: .circular, transform: nil)
        let continuous = ORBPath(roundedRect: rect, topLeftRadius: 5, topRightRadius: 10, bottomRightRadius: 15, bottomLeftRadius: 20, style: .continuous, transform: nil)
        defer {
            circular.release()
            continuous.release()
        }
        #expect(circular.cgPath != continuous.cgPath)
    }
}
#endif
