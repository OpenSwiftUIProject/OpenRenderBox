//
//  PathTests.swift
//  OpenRenderBox
//
//  Created by Kyle on 12/21/25.
//

#if canImport(Darwin) && OPENRENDERBOX_CF_CGTYPES
import Foundation
import OpenRenderBox
import OpenRenderBoxCxx.Path
import Testing

@Suite
struct PathTests {
    private struct ElementCapture {
        var element: ORBPath.Element?
        var hasTransform = false
    }

    private struct ShapeCapture {
        var type: ORBPathShapeType?
        var transform: CGAffineTransform = .identity
        var rect: CGRect = .zero
        var cornerSize: CGSize = .zero
        var style: ORBPath.RoundedCornerStyle = .circular
    }

    @Test
    func replacesStorageCapacityBits() {
        let flags = ORB.Path.StorageFlags().withCapacity(15).withCapacity(16)
        #expect(flags.capacity() == 16)
    }

    @Test
    func appliesRectangleShape() {
        let rect = CGRect(x: 10, y: 20, width: 80, height: 40)
        var transform = CGAffineTransform(translationX: 5, y: 7)
        let path = ORBPath(rect: rect, transform: &transform)
        defer { path.release() }

        var capture = ShapeCapture()
        let recognized = path.applyShape(allowLine: true, info: &capture) { info, type, shape in
            let capture = info.assumingMemoryBound(to: ShapeCapture.self)
            capture.pointee.type = type
            capture.pointee.transform = shape.pointee.rect.transform
            capture.pointee.rect = shape.pointee.rect.rect
        }

        #expect(recognized)
        #expect(capture.type == .rect)
        #expect(capture.transform == transform)
        #expect(capture.rect == rect)
        #expect(path.isSingleElement)
    }

    @Test
    func appliesEllipseShape() {
        let rect = CGRect(x: 1, y: 2, width: 30, height: 20)
        let path = ORBPath(ellipseIn: rect, transform: nil)
        defer { path.release() }

        var capture = ShapeCapture()
        let recognized = path.applyShape(allowLine: true, info: &capture) { info, type, shape in
            let capture = info.assumingMemoryBound(to: ShapeCapture.self)
            capture.pointee.type = type
            capture.pointee.transform = shape.pointee.ellipse.transform
            capture.pointee.rect = shape.pointee.ellipse.rect
        }

        #expect(recognized)
        #expect(capture.type == .ellipse)
        #expect(capture.transform.isIdentity)
        #expect(capture.rect == rect)
    }

    @Test
    func appliesFixedRoundedRectangleShape() {
        let rect = CGRect(x: 3, y: 4, width: 50, height: 30)
        let path = ORBPath(
            roundedRect: rect,
            cornerWidth: 8,
            cornerHeight: 6,
            style: .continuous,
            transform: nil
        )
        defer { path.release() }

        var capture = ShapeCapture()
        let recognized = path.applyShape(allowLine: true, info: &capture) { info, type, shape in
            let capture = info.assumingMemoryBound(to: ShapeCapture.self)
            capture.pointee.type = type
            capture.pointee.transform = shape.pointee.fixedRoundedRect.transform
            capture.pointee.rect = shape.pointee.fixedRoundedRect.rect
            capture.pointee.cornerSize = shape.pointee.fixedRoundedRect.cornerSize
            capture.pointee.style = shape.pointee.fixedRoundedRect.style
        }

        #expect(recognized)
        #expect(capture.type == .fixedRoundedRect)
        #expect(capture.transform.isIdentity)
        #expect(capture.rect == rect)
        #expect(capture.cornerSize == CGSize(width: 8, height: 6))
        #expect(capture.style == .continuous)
    }

    @Test
    func primitiveCallbacksMatchSingleElementLayout() {
        let rect = CGRect(x: 2, y: 3, width: 40, height: 20)
        var identity = CGAffineTransform.identity
        let path = ORBPath(rect: rect, transform: &identity)
        defer { path.release() }

        #expect(path.callbacks.pointee.boundingRect == nil)
        #expect(path.callbacks.pointee.cgPath == nil)

        var capture = ElementCapture()
        let applied = path.apply(info: &capture) { info, element, _, userInfo in
            let capture = info.assumingMemoryBound(to: ElementCapture.self)
            capture.pointee.element = element
            capture.pointee.hasTransform = userInfo != nil
            return true
        }
        #expect(applied)
        #expect(capture.element == .rect)
        #expect(!capture.hasTransform)
    }

    @Test
    func identityAndNilTransformsProduceEqualPrimitives() {
        let rect = CGRect(x: 2, y: 3, width: 40, height: 20)
        var identity = CGAffineTransform.identity
        let lhs = ORBPath(rect: rect, transform: nil)
        let rhs = ORBPath(rect: rect, transform: &identity)
        defer {
            lhs.release()
            rhs.release()
        }

        #expect(lhs.isEqual(to: rhs))
    }

    @Test
    func appliesCoreGraphicsRectangleShape() {
        let rect = CGRect(x: 10, y: 20, width: 80, height: 40)
        var transform = CGAffineTransform(translationX: 5, y: 7)
        let cgPath = CGPath(rect: rect, transform: &transform)
        let path = ORBPath(cgPath: cgPath)
        defer { path.release() }

        var capture = ShapeCapture()
        #expect(path.applyShape(allowLine: true, info: &capture) { info, type, shape in
            let capture = info.assumingMemoryBound(to: ShapeCapture.self)
            capture.pointee.type = type
            capture.pointee.transform = shape.pointee.rect.transform
            capture.pointee.rect = shape.pointee.rect.rect
        })
        #expect(capture.type == .rect)
        #expect(capture.rect.applying(capture.transform) == cgPath.boundingBoxOfPath)
    }

    @Test
    func zeroCornerRoundedRectangleUsesRoundedRectElement() {
        let rect = CGRect(x: 3, y: 4, width: 50, height: 30)
        let path = ORBPath(
            roundedRect: rect,
            cornerWidth: 0,
            cornerHeight: 0,
            style: .continuous,
            transform: nil
        )
        defer { path.release() }

        var capture = ElementCapture()
        #expect(path.apply(info: &capture) { info, element, _, _ in
            info.assumingMemoryBound(to: ElementCapture.self).pointee.element = element
            return true
        })
        #expect(capture.element == .roundedRect)

        var shape = ShapeCapture()
        #expect(path.applyShape(allowLine: true, info: &shape) { info, type, _ in
            info.assumingMemoryBound(to: ShapeCapture.self).pointee.type = type
        })
        #expect(shape.type == .rect)
    }

    @Test
    func continuousRoundedRectangleUsesContinuousGeometry() {
        let rect = CGRect(x: 0, y: 0, width: 80, height: 50)
        let circular = ORBPath(
            roundedRect: rect,
            cornerWidth: 12,
            cornerHeight: 10,
            style: .circular,
            transform: nil
        )
        let continuous = ORBPath(
            roundedRect: rect,
            cornerWidth: 12,
            cornerHeight: 10,
            style: .continuous,
            transform: nil
        )
        defer {
            circular.release()
            continuous.release()
        }

        #expect(circular.cgPath != continuous.cgPath)
    }

    @Test
    func primitiveBoundingRectUsesPathAPI() {
        let rect = CGRect(x: 1, y: 2, width: 30, height: 20)
        var transform = CGAffineTransform(rotationAngle: .pi / 4)
        let path = ORBPath(ellipseIn: rect, transform: &transform)
        defer { path.release() }

        let expected = rect.applying(transform)
        let actual = path.boundingRect
        #expect(abs(actual.origin.x - expected.origin.x) < 0.0001)
        #expect(abs(actual.origin.y - expected.origin.y) < 0.0001)
        #expect(abs(actual.width - expected.width) < 0.0001)
        #expect(abs(actual.height - expected.height) < 0.0001)
    }
}
#endif
