//
//  PathWrapperTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics

@Suite
struct PathWrapperTests {
    private func makeLine() -> ORBPath {
        let cgPath = CGMutablePath()
        cgPath.move(to: .zero)
        cgPath.addLine(to: CGPoint(x: 10, y: 0))
        return ORBPath(cgPath: cgPath)
    }

    @Test
    func transformedPathRetainsSource() {
        let source = makeLine()
        var transform = CGAffineTransform(a: 2, b: 0, c: 0, d: 3, tx: 20, ty: 30)
        let path = source.copy(transform: &transform)
        transform = .identity
        source.release()
        defer { path.release() }
        #expect(path.isSingleElement)
        #expect(pathElements(path).map(\.element) == [17, 0, 1, 16])
        #expect(cgPathElements(path.cgPath) == [
            PathElementRecord(element: 0, points: [20, 30]),
            PathElementRecord(element: 1, points: [40, 30]),
        ])
    }

    @Test
    func retainedNestedShapeSurvivesWrapperRelease() {
        let source = makeLine()
        var transform = CGAffineTransform(translationX: 20, y: 30)
        let path = source.copy(transform: &transform)
        source.release()
        var nested = ORBPath.empty
        var type = ORBPathShapeType.rect
        #expect(path.applyShape(allowLine: true, info: &type) { info, type, _ in
            info.assumingMemoryBound(to: ORBPathShapeType.self).pointee = type
        })
        #expect(type == .nested)
        #expect(path.applyShape(allowLine: true, info: &nested) { info, type, shape in
            if type == .nested {
                info.assumingMemoryBound(to: ORBPath.self).pointee = shape.pointee.nested.path.retain()
            }
        })
        path.release()
        defer { nested.release() }
        #expect(cgPathElements(nested.cgPath) == [
            PathElementRecord(element: 0, points: [0, 0]),
            PathElementRecord(element: 1, points: [10, 0]),
        ])
    }

    @Test
    func nestedWrappersKeepChildDepth() {
        let source = makeLine()
        let trimmed = source.trimmed(from: 0.25, to: 0.75)
        source.release()
        var transform = CGAffineTransform(translationX: 20, y: 30)
        let path = trimmed.copy(transform: &transform)
        trimmed.release()
        #expect(path.isSingleElement)
        #expect(pathElements(path).map(\.element) == [17, 18, 0, 1, 16, 16])
        #expect(cgPathElements(path.cgPath) == [
            PathElementRecord(element: 0, points: [22.5, 30]),
            PathElementRecord(element: 1, points: [27.5, 30]),
        ])
        var child = ORBPath.empty
        #expect(path.applyShape(allowLine: true, info: &child) { info, type, shape in
            if type == .nested {
                info.assumingMemoryBound(to: ORBPath.self).pointee = shape.pointee.nested.path.retain()
            }
        })
        path.release()
        defer { child.release() }
        var shapeType = ORBPathShapeType.rect
        #expect(child.applyShape(allowLine: true, info: &shapeType) { info, type, _ in
            info.assumingMemoryBound(to: ORBPathShapeType.self).pointee = type
        })
        #expect(shapeType == .trimmed)
        #expect(cgPathElements(child.cgPath) == [
            PathElementRecord(element: 0, points: [2.5, 0]),
            PathElementRecord(element: 1, points: [7.5, 0]),
        ])
    }

    @Test
    func trimmedPathPreservesSegmentBoundaries() {
        let cgPath = CGMutablePath()
        cgPath.move(to: .zero)
        cgPath.addLine(to: CGPoint(x: 10, y: 0))
        cgPath.addLine(to: CGPoint(x: 10, y: 10))
        let source = ORBPath(cgPath: cgPath)
        let path = source.trimmed(from: 0.25, to: 0.75)
        source.release()
        defer { path.release() }
        #expect(path.isSingleElement)
        #expect(pathElements(path).map(\.element) == [18, 0, 1, 1, 16])
        #expect(cgPathElements(path.cgPath) == [
            PathElementRecord(element: 0, points: [5, 0]),
            PathElementRecord(element: 1, points: [10, 0]),
            PathElementRecord(element: 1, points: [10, 5]),
        ])
        var shapeType = ORBPathShapeType.rect
        #expect(path.applyShape(allowLine: true, info: &shapeType) { info, type, _ in
            info.assumingMemoryBound(to: ORBPathShapeType.self).pointee = type
        })
        #expect(shapeType == .trimmed)
    }

    @Test(.enabled(if: referencePathGeometryEnabled), arguments: [false, true])
    func trimmedCurvesMatchSegmentGeometry(quadratic: Bool) {
        let cgPath = CGMutablePath()
        cgPath.move(to: .zero)
        if quadratic {
            cgPath.addQuadCurve(to: CGPoint(x: 10, y: 0), control: CGPoint(x: 5, y: 10))
        } else {
            cgPath.addCurve(to: CGPoint(x: 10, y: 0), control1: CGPoint(x: 0, y: 10), control2: CGPoint(x: 10, y: 10))
        }
        let source = ORBPath(cgPath: cgPath)
        let path = source.trimmed(from: 0.25, to: 0.75)
        source.release()
        defer { path.release() }
        let elements = cgPathElements(path.cgPath)
        #expect(elements.map(\.element) == [0, 3])
        let actual = elements.flatMap(\.points)
        let expected: [CGFloat] = quadratic
            ? [1.9837460658183548, 3.1804424409067495,
               3.9947165820760118, 5.6066815333809306,
               6.0056870983336692, 5.6065191755005594,
               8.0166576145913258, 3.1799553672656353]
            : [1.0906342200107548, 4.8932050683032262,
               3.0138364775468443, 8.3689316438989252,
               6.9861635224531566, 8.3689316438989252,
               8.9093657799892458, 4.8932050683032253]
        #expect(actual.count == expected.count)
        for (actual, expected) in zip(actual, expected) {
            #expect(abs(actual - expected) < 1e-10)
        }
    }

    @Test(arguments: [(0.75, 0.25), (0.0, 0.0), (1.0, 1.0)])
    func emptyTrimProducesNoGeometry(from: Double, to: Double) {
        let source = makeLine()
        let path = source.trimmed(from: from, to: to)
        source.release()
        defer { path.release() }
        #expect(path.cgPath.isEmpty)
    }

    @Test(arguments: [Int32(0), 1, 2])
    func strokedPathUsesLineCap(cap: Int32) {
        let source = makeLine()
        let path = source.stroked(lineWidth: 4, lineCap: cap, lineJoin: 0, miterLimit: 10, dashPhase: 0, dash: nil, dashCount: 0)
        source.release()
        defer { path.release() }
        #expect(path.isSingleElement)
        #expect(pathElements(path).map(\.element) == [19, 0, 1, 16])
        let bounds = cap == 0 ? CGRect(x: 0, y: -2, width: 10, height: 4) : CGRect(x: -2, y: -2, width: 14, height: 4)
        #expect(path.cgPath.boundingBoxOfPath == bounds)
        var shapeType = ORBPathShapeType.rect
        #expect(path.applyShape(allowLine: true, info: &shapeType) { info, type, _ in
            info.assumingMemoryBound(to: ORBPathShapeType.self).pointee = type
        })
        #expect(shapeType == .stroked)
        var hasEmptyDashBuffer = false
        #expect(path.applyShape(allowLine: true, info: &hasEmptyDashBuffer) { info, type, shape in
            if type == .stroked {
                let style = shape.pointee.stroked.style
                info.assumingMemoryBound(to: Bool.self).pointee = style.dashCount == 0 && style.dash != nil
            }
        })
        #expect(hasEmptyDashBuffer)
    }
}
#endif
