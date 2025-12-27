//
//  PathTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics)
import CoreGraphics

@Suite(.enabled(if: compatibilityTestEnabled))
struct PathTests {
    @Test
    func createRectPath() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(rect: rect, transform: nil)
        path.release()
    }

    @Test
    func createEllipsePath() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 50)
        let path = ORBPath(ellipseIn: rect, transform: nil)
        path.release()
    }

    @Test
    func createRoundedRectPath() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(roundedRect: rect, cornerWidth: 10, cornerHeight: 10, style: .circular, transform: nil)
        path.release()
    }

    @Test
    func createUnevenRoundedRectPath() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(roundedRect: rect, topLeftRadius: 5, bottomLeftRadius: 10, bottomRightRadius: 15, topRightRadius: 20, style: .continuous, transform: nil)
        path.release()
    }

    @Test
    func pathRetainRelease() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(rect: rect, transform: nil)
        path.retain()
        path.release()
        path.release()
    }

    @Test(arguments: [
        (CGPoint(x: 50, y: 50), true),
        (CGPoint(x: 150, y: 150), false),
        (CGPoint(x: 0, y: 0), true),
        (CGPoint(x: 100, y: 100), false),
        (CGPoint(x: -1, y: 50), false),
    ])
    func pathContainsPoint(point: CGPoint, expected: Bool) {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(rect: rect, transform: nil)
        #expect(path.contains(point: point, eoFill: false) == expected)
        path.release()
    }

    @Test
    func pathIsEmpty() {
        let emptyPath = ORBPath.null
        #expect(emptyPath.isEmpty == true)

        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(rect: rect, transform: nil)
        #expect(path.isEmpty == false)
        path.release()
    }

    @Test
    func pathApplyElements() {
        let rect = CGRect(x: 0, y: 0, width: 10, height: 10)
        let cgPath = CGPath(rect: rect, transform: nil)
        let path = ORBPath(cgPath: cgPath)
        var elementCount: Int32 = 0
        path.apply(info: &elementCount) { info, element, points, userInfo in
            let countPtr = info.assumingMemoryBound(to: Int32.self)
            countPtr.pointee += 1
            return true // continue iteration
        }
        #expect(elementCount == 5) // move + 4 lines for rect
        path.release()
    }

    @Test
    func pathEqualToPath() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path1 = ORBPath(rect: rect, transform: nil)
        let path2 = ORBPath(rect: rect, transform: nil)
        #expect(path1.isEqual(to: path2) == true)
        path1.release()
        path2.release()

        let differentRect = CGRect(x: 0, y: 0, width: 200, height: 200)
        let path3 = ORBPath(rect: rect, transform: nil)
        let path4 = ORBPath(rect: differentRect, transform: nil)
        #expect(path3.isEqual(to: path4) == false)
        path3.release()
        path4.release()
    }
}

#endif

