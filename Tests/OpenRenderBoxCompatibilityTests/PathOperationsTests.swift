//
//  PathOperationsTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics
import Foundation

@Suite
struct PathOperationsTests {
    private func curve(quadratic: Bool) -> ORBPath {
        let path = CGMutablePath()
        path.move(to: CGPoint(x: 0.1, y: 0.2))
        if quadratic {
            path.addQuadCurve(to: CGPoint(x: 10, y: 0), control: CGPoint(x: 5, y: 10))
        } else {
            path.addCurve(to: CGPoint(x: 10, y: 0), control1: CGPoint(x: 0, y: 10), control2: CGPoint(x: 10, y: 10))
        }
        path.closeSubpath()
        return ORBPath(cgPath: path)
    }

    @Test
    func applyTransformsElementsAndStopsOnCancellation() {
        let path = curve(quadratic: false)
        defer { path.release() }
        var transform = CGAffineTransform(a: 2, b: 0, c: 0, d: 3, tx: 7, ty: 11)
        var elements: [PathElementRecord] = []
        let completed = withUnsafeMutablePointer(to: &elements) {
            path.applyCGPathElements(transform: &transform, info: $0) { info, element in
                let element = element.pointee
                let count = element.type == .addCurveToPoint ? 3 : element.type == .closeSubpath ? 0 : 1
                let points = UnsafeBufferPointer(start: element.points, count: count).flatMap { [$0.x, $0.y] }
                info.assumingMemoryBound(to: [PathElementRecord].self).pointee.append(
                    PathElementRecord(element: element.type.rawValue, points: points)
                )
                return true
            }
        }
        #expect(completed)
        #expect(elements == [
            PathElementRecord(element: 0, points: [7.2, 11.6]),
            PathElementRecord(element: 3, points: [7, 41, 27, 41, 27, 11]),
            PathElementRecord(element: 4, points: []),
        ])
        var count = 0
        #expect(!path.applyCGPathElements(transform: nil, info: &count) { info, _ in
            let count = info.assumingMemoryBound(to: Int.self)
            count.pointee += 1
            return count.pointee < 2
        })
        #expect(count == 2)
        count = 0
        #expect(!path.applyLines(tolerance: 0.5, info: &count) { info, _, _ in
            let count = info.assumingMemoryBound(to: Int.self)
            count.pointee += 1
            return count.pointee < 2
        })
        #expect(count == 2)
    }

    @Test(.enabled(if: referencePathGeometryEnabled), arguments: [false, true])
    func flattenedCurvesPreserveReferenceStepsAndLength(quadratic: Bool) {
        let path = curve(quadratic: quadratic)
        defer { path.release() }
        var elements: [PathElementRecord] = []
        let completed = withUnsafeMutablePointer(to: &elements) {
            path.applyLines(tolerance: 0.5, info: $0) { info, element, point in
                let points: [CGFloat] = element.rawValue == 2 ? [] : [point.pointee.x, point.pointee.y]
                info.assumingMemoryBound(to: [PathElementRecord].self).pointee.append(
                    PathElementRecord(element: element.rawValue, points: points)
                )
                return true
            }
        }
        #expect(completed)
        let expected: [CGFloat] = quadratic
            ? [0.1, 0.2, 2.5562498569488525, 3.8625001907348633, 5.0249996185302734, 5.0500001907348633,
               7.5062494277954102, 3.762500524520874, 10, 0]
            : [0.1, 0.2, 0.79861116409301758, 4.282407283782959, 2.6222224235534668, 6.7259254455566406,
               5.0125002861022949, 7.5249991416931152, 7.4111113548278809, 6.6740727424621582,
               9.2597227096557617, 4.1675910949707031, 10, 0]
        #expect(elements.map(\.element) == [0] + Array(repeating: 1, count: quadratic ? 4 : 6) + [2])
        #expect(elements.flatMap(\.points) == expected)
        let expectedLength = quadratic ? 24.360695925004599 : 29.409184003419121
        #expect(abs(path.length(tolerance: 0.5, transform: nil) - expectedLength) < 1e-10)
        var transform = CGAffineTransform(a: 2, b: 0, c: 0, d: 3, tx: 7, ty: 11)
        let transformedLength = quadratic ? 56.859833567655286 : 72.313997446559654
        #expect(abs(path.length(tolerance: 0.5, transform: &transform) - transformedLength) < 1e-10)
    }

    @Test(arguments: [false, true])
    func applyPreservesRawMoveAndLineCommands(hasInitialMove: Bool) {
        withPathStorage { storage in
            if hasInitialMove {
                storage.append(element: .moveToPoint, points: [1, 2], userInfo: nil)
                storage.append(element: .moveToPoint, points: [3, 4], userInfo: nil)
                storage.append(element: .addLineToPoint, points: [3, 4], userInfo: nil)
                storage.append(element: .closeSubpath, points: [], userInfo: nil)
                storage.append(element: .closeSubpath, points: [], userInfo: nil)
            } else {
                storage.append(element: .addLineToPoint, points: [3, 4], userInfo: nil)
                storage.append(element: .addLineToPoint, points: [5, 6], userInfo: nil)
            }
            withStoragePath(storage) { path in
                var kinds: [Int32] = []
                let completed = withUnsafeMutablePointer(to: &kinds) {
                    path.applyCGPathElements(transform: nil, info: $0) { info, element in
                        info.assumingMemoryBound(to: [Int32].self).pointee.append(element.pointee.type.rawValue)
                        return true
                    }
                }
                #expect(completed)
                #expect(kinds == (hasInitialMove ? [0, 0, 1, 4] : [1, 1]))
                kinds.removeAll()
                let linesCompleted = withUnsafeMutablePointer(to: &kinds) {
                    path.applyLines(tolerance: 0.5, info: $0) { info, element, _ in
                        info.assumingMemoryBound(to: [Int32].self).pointee.append(element.rawValue)
                        return true
                    }
                }
                #expect(linesCompleted)
                #expect(kinds == (hasInitialMove ? [0, 0, 1, 2] : [1, 1]))
            }
        }
    }

    @Test
    func closedLineLengthIncludesClosingEdge() {
        let cgPath = CGMutablePath()
        cgPath.move(to: .zero)
        cgPath.addLine(to: CGPoint(x: 3, y: 4))
        cgPath.closeSubpath()
        let path = ORBPath(cgPath: cgPath)
        defer { path.release() }
        #expect(path.length(tolerance: 0.5, transform: nil) == 10)
        var transform = CGAffineTransform(scaleX: 2, y: 3)
        #expect(abs(path.length(tolerance: 0.5, transform: &transform) - 26.83281572999748) < 1e-12)
    }

    @Test
    func interpolationPromotesRoundedRectAndPreservesEndpoints() {
        let lhs = ORBPath(rect: CGRect(x: 1, y: 2, width: 10, height: 20), transform: nil)
        let rhs = ORBPath(roundedRect: CGRect(x: 3, y: 4, width: 30, height: 40), cornerWidth: 5, cornerHeight: 6, style: .continuous, transform: nil)
        defer { lhs.release(); rhs.release() }
        #expect(lhs.canInterpolate(to: rhs))
        let mixed = lhs.interpolated(to: rhs, fraction: 0.25)
        defer { mixed.release() }
        #expect(pathElements(mixed) == [PathElementRecord(element: 9, points: [1.5, 2.5, 15, 25, 1.25, 1.5])])
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
        #expect(unsafeBitCast(incompatible.storage, to: UInt.self) == 1)
    }

    @Test
    func interpolatedStrokeOwnsMetadataAndChild() {
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
        let path = lhs.interpolated(to: rhs, fraction: 0.25)
        lhs.release()
        rhs.release()
        struct Result { var width: CGFloat = 0; var cap: Int32 = -1; var dash: [CGFloat] = []; var child = ORBPath.empty }
        var result = Result()
        #expect(path.applyShape(allowLine: true, info: &result) { info, type, shape in
            if type == .stroked {
                let stroke = shape.pointee.stroked
                let result = info.assumingMemoryBound(to: Result.self)
                result.pointee.width = stroke.style.lineWidth
                result.pointee.cap = stroke.style.lineCap
                result.pointee.dash = Array(UnsafeBufferPointer(start: stroke.style.dash, count: Int(stroke.style.dashCount)))
                result.pointee.child = stroke.path.retain()
            }
        })
        path.release()
        defer { result.child.release() }
        #expect(result.width == 1.5)
        #expect(result.cap == 2)
        #expect(result.dash == [3, 5, 7])
        #expect(result.child.boundingRect == CGRect(x: 1.5, y: 2.5, width: 15, height: 25))
    }

    @Test
    func serializedRectUsesCompatibleWireFormat() {
        let path = ORBPath(rect: CGRect(x: 1, y: 2, width: 10, height: 20), transform: nil)
        defer { path.release() }
        let data = path.data as Data
        #expect(data == Data([0x1a, 1, 5, 0x12, 0x10, 0, 0, 0x80, 0x3f, 0, 0, 0, 0x40, 0, 0, 0x20, 0x41, 0, 0, 0xa0, 0x41]))
        let copy = ORBPath(data: data as CFData)
        defer { copy.release() }
        #expect(copy.isEqual(to: path))
        #expect(path.debugDescription as String == "RBPath:\n  rect 1 2 10 20\n")
        let fractional = ORBPath(rect: CGRect(x: 0.1, y: 0.2, width: 10.3, height: 20.4), transform: nil)
        let decoded = ORBPath(data: fractional.data)
        fractional.release()
        defer { decoded.release() }
        #expect(pathElements(decoded) == [PathElementRecord(element: 5, points: [CGFloat(Float(0.1)), CGFloat(Float(0.2)), CGFloat(Float(10.3)), CGFloat(Float(20.4))])])
    }

    @Test(arguments: [
        ([UInt8](), false), ([0x80], false), ([0x0f], false),
        ([0x1a, 1, 0], true), ([0x1a, 1, 0x10], true), ([0x1a, 1, 0x91], true),
    ])
    func dataDecoderDistinguishesEmptyMessagesFromInvalidPaths(bytes: [UInt8], isNull: Bool) {
        let path = ORBPath(data: Data(bytes) as CFData)
        defer { path.release() }
        #expect(path.isEmpty)
        #expect((unsafeBitCast(path.storage, to: UInt.self) == 1) == isNull)
        if !isNull {
            #expect(unsafeBitCast(path.storage, to: UInt.self) > 1)
        }
    }
}
#endif
