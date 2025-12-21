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

    @Test
    func pathContainsPoint() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(rect: rect, transform: nil)
        let insidePoint = CGPoint(x: 50, y: 50)
        let outsidePoint = CGPoint(x: 150, y: 150)
        #expect(path.contains(point: insidePoint, eoFill: false) == true)
        #expect(path.contains(point: outsidePoint, eoFill: false) == false)
        path.release()
        
    }
}

#endif

