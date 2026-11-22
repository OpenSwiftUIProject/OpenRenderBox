//
//  PortablePathTests.swift
//  OpenRenderBoxTests

#if !canImport(Darwin)
import OpenCoreGraphicsShims
import OpenRenderBox
import Testing

@Suite
struct PortablePathTests {
    private func withStorage<Result>(capacity: Int = 96, source: ORBPath.Storage? = nil, _ body: (ORBPath.Storage) -> Result) -> Result {
        let bytes = UnsafeMutableRawPointer.allocate(byteCount: capacity, alignment: 16)
        let storage = ORBPath.Storage(OpaquePointer(bytes))
        storage.initialize(capacity: capacity, source: source)
        defer {
            storage.destroy()
            bytes.deallocate()
        }
        return body(storage)
    }

    private func elementCount(_ storage: ORBPath.Storage) -> Int {
        var count = 0
        _ = storage.apply(info: &count) { info, _, _, _ in
            info.assumingMemoryBound(to: Int.self).pointee += 1
            return true
        }
        return count
    }

    @Test
    func primitiveQueries() {
        let rect = CGRect(x: 1, y: 2, width: 100, height: 80)
        let path = ORBPath(rect: rect, transform: nil)
        defer { path.release() }
        #expect(path.boundingRect == rect)
        #expect(!path.isEmpty)
        #expect(path.isSingleElement)
        #expect(path.contains(point: CGPoint(x: 50, y: 40), eoFill: false))
        #expect(!path.contains(point: CGPoint(x: -1, y: 40), eoFill: false))
        var type = ORBPathShapeType.line
        #expect(path.applyShape(allowLine: true, info: &type) { info, type, _ in
            info.assumingMemoryBound(to: ORBPathShapeType.self).pointee = type
        })
        #expect(type == .rect)
    }

    @Test
    func retainedPathUsesReturnedStorage() {
        var destination: UInt64 = 0
        withUnsafePointer(to: &destination) { destination in
            var source = UnsafeRawPointer(destination)
            var callbacks = ORBPath.Callbacks()
            callbacks.retain = { object in
                object.assumingMemoryBound(to: UnsafeRawPointer.self).pointee
            }
            withUnsafePointer(to: &source) { source in
                withUnsafePointer(to: &callbacks) { callbacks in
                    let path = ORBPath(storage: ORBPath.Storage(OpaquePointer(source)), callbacks: callbacks)
                    let retained = path.retain()
                    #expect(retained.storage.rawValue == OpaquePointer(destination))
                }
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
    func containmentPreservesMaskBits() {
        let path = ORBPath(rect: CGRect(x: 0, y: 0, width: 10, height: 10), transform: nil)
        defer { path.release() }
        var points = Array(repeating: CGPoint(x: 20, y: 20), count: 65)
        points[0] = CGPoint(x: 5, y: 5)
        points[63] = CGPoint(x: 5, y: 5)
        points[64] = CGPoint(x: 5, y: 5)
        #expect(path.containsPoints(count: 65, points: points, eoFill: false, transform: nil) == (UInt64(1) | (UInt64(1) << 63)))
    }

    @Test(arguments: [64, 256])
    func storageCopiesAndClearsIndependently(capacity: Int) {
        withStorage(capacity: capacity) { source in
            source.append(element: .moveToPoint, points: [0, 0], userInfo: nil)
            for index in 1...40 {
                source.append(element: .addLineToPoint, points: [CGFloat(index), CGFloat(index * 2)], userInfo: nil)
            }
            #expect(elementCount(source) == 41)
            withStorage(capacity: 64, source: source) { copy in
                #expect(copy.isEqual(to: source))
                source.clear()
                #expect(source.isEmpty)
                #expect(elementCount(copy) == 41)
                #expect(copy.boundingRect == CGRect(x: 0, y: 0, width: 40, height: 80))
            }
        }
    }

    @Test
    func storageUsesBezierOrderAndStopsEnumeration() {
        withStorage { storage in
            storage.append(element: .moveToPoint, points: [0, 0], userInfo: nil)
            storage.append(element: .addQuadCurveToPoint, points: [1, 2, 3, 4], userInfo: nil)
            #expect(storage.bezierOrder == 2)
            storage.append(element: .addCurveToPoint, points: [5, 6, 7, 8, 9, 10], userInfo: nil)
            #expect(storage.bezierOrder == 3)
            var count = 0
            #expect(!storage.apply(info: &count) { info, _, _, _ in
                info.assumingMemoryBound(to: Int.self).pointee += 1
                return false
            })
            #expect(count == 1)
        }
    }

    @Test
    func roundedStylesProduceDifferentElements() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 80)
        for style in [ORBPath.RoundedCornerStyle.circular, .continuous] {
            let path = ORBPath(roundedRect: rect, topLeftRadius: 5, topRightRadius: 10, bottomRightRadius: 15, bottomLeftRadius: 20, style: style, transform: nil)
            defer { path.release() }
            var element = ORBPath.Element.invalid
            #expect(path.apply(info: &element) { info, element, _, _ in
                info.assumingMemoryBound(to: ORBPath.Element.self).pointee = element
                return true
            })
            #expect(element == (style == .circular ? .unevenRoundedRectCircular : .unevenRoundedRectContinuous))
            #expect(path.contains(point: CGPoint(x: 50, y: 40), eoFill: false))
            var actualStyle = ORBPath.RoundedCornerStyle.circular
            #expect(path.applyShape(allowLine: true, info: &actualStyle) { info, type, shape in
                if type == .unevenRoundedRect {
                    info.assumingMemoryBound(to: ORBPath.RoundedCornerStyle.self).pointee = shape.pointee.unevenRoundedRect.style
                }
            })
            #expect(actualStyle == style)
        }
    }

    #if !OPENRENDERBOX_COREGRAPHICS
    @Test
    func trimmedCubicGeometry() {
        let cgPath = CGMutablePath()
        cgPath.move(to: .zero)
        cgPath.addCurve(to: CGPoint(x: 10, y: 0), control1: CGPoint(x: 0, y: 10), control2: CGPoint(x: 10, y: 10))
        let source = ORBPath(cgPath: cgPath)
        let path = source.trimmed(from: 0.25, to: 0.75)
        source.release()
        defer { path.release() }
        let elements = path.cgPath.elements
        #expect(elements.count == 2)
        var actual: [CGFloat] = []
        for element in elements {
            switch element {
            case let .moveToPoint(point): actual += [point.x, point.y]
            case let .addCurveToPoint(first, second, end): actual += [first.x, first.y, second.x, second.y, end.x, end.y]
            default: Issue.record("Unexpected trimmed curve element")
            }
        }
        let expected: [CGFloat] = [1.0906342200107548, 4.8932050683032262,
                                   3.0138364775468443, 8.3689316438989252,
                                   6.9861635224531566, 8.3689316438989252,
                                   8.9093657799892458, 4.8932050683032253]
        #expect(actual.count == expected.count)
        for (actual, expected) in zip(actual, expected) {
            #expect(abs(actual - expected) < 1e-10)
        }
    }

    @Test
    func relativeArcGeometry() {
        withStorage { storage in
            storage.append(element: .relativeArc, points: [0, 0, 10, 0, .pi / 2], userInfo: nil)
            guard let path = storage.cgPath else {
                Issue.record("Missing arc path")
                return
            }
            #expect(path.elements.count == 2)
            #expect(path.elements.first == .moveToPoint(CGPoint(x: 10, y: 0)))
            if case let .addCurveToPoint(_, _, end) = path.elements.last {
                #expect(abs(end.x) < 1e-6)
                #expect(abs(end.y - 10) < 1e-6)
            } else {
                Issue.record("Missing arc curve")
            }
        }
    }

    @Test(arguments: [ORBPath.RoundedCornerStyle.circular, .continuous])
    func roundedRectangleClampsBothAxes(style: ORBPath.RoundedCornerStyle) {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 20)
        let oversized = ORBPath(roundedRect: rect, cornerWidth: 50, cornerHeight: 50, style: style, transform: nil)
        let clamped = ORBPath(roundedRect: rect, cornerWidth: 10, cornerHeight: 10, style: style, transform: nil)
        let singleAxis = ORBPath(roundedRect: rect, cornerWidth: 0, cornerHeight: 10, style: style, transform: nil)
        let square = ORBPath(roundedRect: rect, cornerWidth: 0, cornerHeight: 0, style: style, transform: nil)
        defer {
            oversized.release()
            clamped.release()
            singleAxis.release()
            square.release()
        }
        #expect(oversized.cgPath == clamped.cgPath)
        #expect(singleAxis.cgPath.elements.count == (style == .circular ? 10 : 18))
        #expect(square.cgPath.elements.count == 6)
    }

    @Test
    func continuousRoundedRectangleGeometry() {
        let path = ORBPath(roundedRect: CGRect(x: 0, y: 0, width: 100, height: 80), cornerWidth: 10, cornerHeight: 20, style: .continuous, transform: nil)
        defer { path.release() }
        let elements = path.cgPath.elements
        #expect(elements.count == 18)
        #expect(elements[0] == .moveToPoint(CGPoint(x: 100, y: 40)))
        #expect(elements[1] == .addLineToPoint(CGPoint(x: 100, y: 49.426701068878174)))
        #expect(elements[2] == .addCurveToPoint(
            CGPoint(x: 100, y: 58.230199813842773),
            CGPoint(x: 100, y: 62.63185977935791),
            CGPoint(x: 99.250885993242264, y: 67.370120286941528)
        ))
    }
    #endif

    @Test
    func nestedShapeSurvivesSourceRelease() {
        let source = ORBPath(rect: CGRect(x: 1, y: 2, width: 10, height: 20), transform: nil)
        var transform = CGAffineTransform(translationX: 20, y: 30)
        let path = source.copy(transform: &transform)
        source.release()
        var nested = ORBPath.empty
        #expect(path.isSingleElement)
        #expect(path.boundingRect == CGRect(x: 21, y: 32, width: 10, height: 20))
        #expect(path.applyShape(allowLine: true, info: &nested) { info, type, shape in
            if type == .nested {
                info.assumingMemoryBound(to: ORBPath.self).pointee = shape.pointee.nested.path.retain()
            }
        })
        path.release()
        defer { nested.release() }
        #expect(nested.boundingRect == CGRect(x: 1, y: 2, width: 10, height: 20))
    }

    @Test
    func applyCGPathElementsTransformsAndStopsOnCancellation() {
        let cgPath = CGMutablePath()
        cgPath.move(to: CGPoint(x: 1, y: 2))
        cgPath.addQuadCurve(to: CGPoint(x: 7, y: 8), control: CGPoint(x: 5, y: 6))
        cgPath.addCurve(to: CGPoint(x: 13, y: 14), control1: CGPoint(x: 9, y: 10), control2: CGPoint(x: 11, y: 12))
        cgPath.closeSubpath()
        let path = ORBPath(cgPath: cgPath)
        defer { path.release() }

        var transform = CGAffineTransform(a: 2, b: 0, c: 0, d: 3, tx: 10, ty: 20)
        var recorded: [(Int32, [CGFloat])] = []
        let completed = withUnsafeMutablePointer(to: &recorded) { recordedPtr in
            path.applyCGPathElements(transform: &transform, info: recordedPtr) { info, elementPtr in
                let element = elementPtr.pointee
                let count: Int
                switch element.type {
                case .moveToPoint, .addLineToPoint: count = 1
                case .addQuadCurveToPoint: count = 2
                case .addCurveToPoint: count = 3
                case .closeSubpath: count = 0
                @unknown default: count = 0
                }
                let scalars = (0..<count).flatMap { [element.points[$0].x, element.points[$0].y] }
                info.assumingMemoryBound(to: [(Int32, [CGFloat])].self).pointee.append((element.type.rawValue, scalars))
                return true
            }
        }
        #expect(completed)
        #expect(recorded.count == 4)
        #expect(recorded[0].0 == 0 && recorded[0].1 == [12, 26])
        #expect(recorded[1].0 == 2 && recorded[1].1 == [20, 38, 24, 44])
        #expect(recorded[2].0 == 3 && recorded[2].1 == [28, 50, 32, 56, 36, 62])
        #expect(recorded[3].0 == 4 && recorded[3].1.isEmpty)

        var count = 0
        #expect(!path.applyCGPathElements(transform: nil, info: &count) { info, _ in
            let countPtr = info.assumingMemoryBound(to: Int.self)
            countPtr.pointee += 1
            return countPtr.pointee < 2
        })
        #expect(count == 2)
    }

    @Test
    func interpolationPromotesRoundedRectAndPreservesEndpoints() {
        let lhs = ORBPath(rect: CGRect(x: 1, y: 2, width: 10, height: 20), transform: nil)
        let rhs = ORBPath(roundedRect: CGRect(x: 3, y: 4, width: 30, height: 40), cornerWidth: 5, cornerHeight: 6, style: .continuous, transform: nil)
        defer {
            lhs.release()
            rhs.release()
        }
        #expect(lhs.canInterpolate(to: rhs))
        #expect(rhs.canInterpolate(to: lhs))

        struct ElementCapture {
            var element: ORBPath.Element = .invalid
            var values: [CGFloat] = []
        }

        let promoted = lhs.interpolated(to: rhs, fraction: 0.25)
        defer { promoted.release() }
        var promotedCapture = ElementCapture()
        let promotedApplied = withUnsafeMutablePointer(to: &promotedCapture) { ptr in
            promoted.apply(info: ptr) { info, element, points, _ in
                info.assumingMemoryBound(to: ElementCapture.self).pointee = ElementCapture(
                    element: element,
                    values: Array(UnsafeBufferPointer(start: points, count: 6))
                )
                return true
            }
        }
        #expect(promotedApplied)
        #expect(promotedCapture.element == .fixedRoundedRectContinuous)
        #expect(promotedCapture.values == [1.5, 2.5, 15, 25, 1.25, 1.5])

        let demoted = rhs.interpolated(to: lhs, fraction: 0.75)
        defer { demoted.release() }
        var demotedCapture = ElementCapture()
        let demotedApplied = withUnsafeMutablePointer(to: &demotedCapture) { ptr in
            demoted.apply(info: ptr) { info, element, points, _ in
                info.assumingMemoryBound(to: ElementCapture.self).pointee = ElementCapture(
                    element: element,
                    values: Array(UnsafeBufferPointer(start: points, count: 6))
                )
                return true
            }
        }
        #expect(demotedApplied)
        #expect(demotedCapture.element == .fixedRoundedRectContinuous)
        #expect(demotedCapture.values == [1.5, 2.5, 15, 25, 1.25, 1.5])

        for fraction: Float in [0, 1] {
            let endpoint = lhs.interpolated(to: rhs, fraction: fraction)
            defer { endpoint.release() }
            let expected = fraction == 0 ? lhs : rhs
            #expect(endpoint.storage == expected.storage)
            #expect(endpoint.callbacks == expected.callbacks)
        }

        let ellipse = ORBPath(ellipseIn: CGRect(x: 0, y: 0, width: 10, height: 10), transform: nil)
        defer { ellipse.release() }
        #expect(!lhs.canInterpolate(to: ellipse))
        let incompatible = lhs.interpolated(to: ellipse, fraction: 0.25)
        defer { incompatible.release() }
        #expect(incompatible.isNull)
    }

    @Test
    func interpolatedStrokeAndTransforms() {
        func makeStroke(rect: CGRect, width: CGFloat, cap: Int32, dash: [CGFloat]) -> ORBPath {
            let source = ORBPath(rect: rect, transform: nil)
            defer { source.release() }
            return dash.withUnsafeBufferPointer {
                source.stroked(lineWidth: width, lineCap: cap, lineJoin: 1, miterLimit: 10, dashPhase: 0, dash: $0.baseAddress, dashCount: $0.count)
            }
        }
        let lhs = makeStroke(rect: CGRect(x: 1, y: 2, width: 10, height: 20), width: 1, cap: 0, dash: [2, 4])
        let rhs = makeStroke(rect: CGRect(x: 3, y: 4, width: 30, height: 40), width: 3, cap: 2, dash: [3, 5, 7])
        #expect(lhs.canInterpolate(to: rhs))
        let mixedStroke = lhs.interpolated(to: rhs, fraction: 0.25)
        lhs.release()
        rhs.release()
        struct StrokeResult { var width: CGFloat = 0; var cap: CGLineCap = .butt; var dash: [CGFloat] = []; var child = ORBPath.empty }
        var result = StrokeResult()
        let strokeRecognized = withUnsafeMutablePointer(to: &result) { resultPtr in
            mixedStroke.applyShape(allowLine: true, info: resultPtr) { info, type, shape in
                if type == .stroked {
                    let stroke = shape.pointee.stroked
                    let out = info.assumingMemoryBound(to: StrokeResult.self)
                    out.pointee.width = stroke.style.lineWidth
                    out.pointee.cap = stroke.style.lineCap
                    out.pointee.dash = Array(UnsafeBufferPointer(start: stroke.style.dash, count: Int(stroke.style.dashCount)))
                    out.pointee.child = stroke.path.retain()
                }
            }
        }
        #expect(strokeRecognized)
        mixedStroke.release()
        defer { result.child.release() }
        #expect(result.width == 1.5)
        #expect(result.cap == .square)
        #expect(result.dash == [3, 5, 7])
        #expect(result.child.boundingRect == CGRect(x: 1.5, y: 2.5, width: 15, height: 25))

        var t1 = CGAffineTransform(rotationAngle: 0).translatedBy(x: 0, y: 10)
        var t2 = CGAffineTransform(rotationAngle: .pi / 2).translatedBy(x: 20, y: 30)
        let r1 = ORBPath(rect: CGRect(x: 0, y: 0, width: 10, height: 20), transform: &t1)
        let r2 = ORBPath(rect: CGRect(x: 0, y: 0, width: 10, height: 20), transform: &t2)
        defer {
            r1.release()
            r2.release()
        }
        let mid = r1.interpolated(to: r2, fraction: 0.5)
        defer { mid.release() }
        #expect(!mid.isNull)
        var capturedTransform = CGAffineTransform.identity
        let rectRecognized = withUnsafeMutablePointer(to: &capturedTransform) { ptr in
            mid.applyShape(allowLine: true, info: ptr) { info, type, shape in
                if type == .rect {
                    info.assumingMemoryBound(to: CGAffineTransform.self).pointee = shape.pointee.rect.transform
                }
            }
        }
        #expect(rectRecognized)
        let invSqrt2 = CGFloat(0.5).squareRoot()
        #expect(abs(capturedTransform.a - invSqrt2) < 1e-10)
        #expect(abs(capturedTransform.b - invSqrt2) < 1e-10)
        #expect(abs(capturedTransform.c - (-invSqrt2)) < 1e-10)
        #expect(abs(capturedTransform.d - invSqrt2) < 1e-10)

        let trimmedRect = r1.trimmed(from: 0, to: 0.5)
        let rounded = ORBPath(roundedRect: CGRect(x: 0, y: 0, width: 10, height: 20), cornerWidth: 4, cornerHeight: 4, style: .circular, transform: nil)
        let trimmedRounded = rounded.trimmed(from: 0, to: 0.5)
        rounded.release()
        defer {
            trimmedRect.release()
            trimmedRounded.release()
        }
        #expect(!trimmedRect.canInterpolate(to: trimmedRounded))
    }

    @Test
    func serializedDataWireFormatAndInvalidInputs() throws {
        let path = ORBPath(rect: CGRect(x: 1, y: 2, width: 10, height: 20), transform: nil)
        defer { path.release() }
        let data = try #require(path.data)
        #expect(testCFDataBytes(data) == [0x1a, 1, 5, 0x12, 0x10, 0, 0, 0x80, 0x3f, 0, 0, 0, 0x40, 0, 0, 0x20, 0x41, 0, 0, 0xa0, 0x41])
        let decoded = ORBPath(data: data)
        defer { decoded.release() }
        #expect(decoded.isEqual(to: path))

        let emptyData = try #require(ORBPath.empty.data)
        #expect(testCFDataBytes(emptyData).isEmpty)
        let decodedEmpty = ORBPath(data: emptyData)
        defer { decodedEmpty.release() }
        #expect(decodedEmpty.isEmpty)
        #expect(!decodedEmpty.isNull)

        let nilPath = ORBPath(data: nil)
        defer { nilPath.release() }
        #expect(nilPath.isEmpty)
        #expect(nilPath.isNull)

        for (bytes, expectedNull) in [
            ([UInt8](), false),
            ([0x80], false),
            ([0x0f], false),
            ([0x1a, 1, 0], true),
            ([0x1a, 1, 0x10], true),
            ([0x1a, 1, 0x91], true),
        ] {
            let candidate = ORBPath(data: makeTestCFData(bytes))
            defer { candidate.release() }
            #expect(candidate.isEmpty)
            #expect(candidate.isNull == expectedNull)
        }
    }
}
#endif
