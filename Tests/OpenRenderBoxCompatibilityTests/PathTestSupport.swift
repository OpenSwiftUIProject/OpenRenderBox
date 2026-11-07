//
//  PathTestSupport.swift
//  OpenRenderBoxCompatibilityTests

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics
import Foundation

// The curve reference data comes from RenderBox in iOS 18.5.
let referencePathGeometryEnabled: Bool = {
    #if OPENRENDERBOX_COMPATIBILITY_TEST
    #if os(iOS)
    let version = ProcessInfo.processInfo.operatingSystemVersion
    return version.majorVersion == 18 && version.minorVersion == 5
    #else
    return false
    #endif
    #else
    return true
    #endif
}()

func withPathStorage<Result>(
    capacity: Int = 96,
    source: ORBPath.Storage? = nil,
    _ body: (ORBPath.Storage) throws -> Result
) rethrows -> Result {
    let bytes = UnsafeMutableRawPointer.allocate(byteCount: capacity, alignment: 16)
    let storage = unsafeBitCast(bytes, to: ORBPath.Storage.self)
    storage.initialize(capacity: capacity, source: source)
    defer {
        storage.destroy()
        bytes.deallocate()
    }
    return try body(storage)
}

func withStoragePath<Result>(_ storage: ORBPath.Storage, _ body: (ORBPath) throws -> Result) rethrows -> Result {
    var callbacks = ORBPath.Callbacks()
    callbacks.apply = { object, info, callback in
        let storage = unsafeBitCast(object, to: ORBPath.Storage.self)
        return storage.apply(info: info, callback: callback)
    }
    callbacks.isEmpty = { object in
        unsafeBitCast(object, to: ORBPath.Storage.self).isEmpty
    }
    callbacks.isSingleElement = { object in
        unsafeBitCast(object, to: ORBPath.Storage.self).isSingleElement
    }
    return try withUnsafePointer(to: &callbacks) { callbacks in
        try body(ORBPath(storage: storage, callbacks: callbacks))
    }
}

struct PathElementRecord: Equatable {
    var element: Int32
    var points: [CGFloat]
}

func recordPathElement(
    info: UnsafeMutableRawPointer,
    element: ORBPath.Element,
    points: UnsafePointer<CGFloat>,
    userInfo: UnsafeRawPointer?
) -> Bool {
    let count: Int
    switch element {
    case .moveToPoint, .addLineToPoint: count = 2
    case .addQuadCurveToPoint, .rect, .roundedRect, .ellipse: count = 4
    case .addCurveToPoint, .fixedRoundedRectCircular, .fixedRoundedRectContinuous: count = 6
    case .unevenRoundedRectCircular, .unevenRoundedRectContinuous: count = 8
    default: count = 0
    }
    info.assumingMemoryBound(to: [PathElementRecord].self).pointee.append(
        PathElementRecord(element: element.rawValue, points: Array(UnsafeBufferPointer(start: points, count: count)))
    )
    return true
}

func pathElements(_ storage: ORBPath.Storage) -> [PathElementRecord] {
    var elements: [PathElementRecord] = []
    _ = withUnsafeMutablePointer(to: &elements) {
        storage.apply(info: $0, callback: recordPathElement)
    }
    return elements
}

func pathElements(_ path: ORBPath) -> [PathElementRecord] {
    var elements: [PathElementRecord] = []
    _ = withUnsafeMutablePointer(to: &elements) {
        path.apply(info: $0, callback: recordPathElement)
    }
    return elements
}
#endif

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
func cgPathElements(_ path: CGPath) -> [PathElementRecord] {
    var elements: [PathElementRecord] = []
    path.applyWithBlock { element in
        let element = element.pointee
        let count: Int
        switch element.type {
        case .moveToPoint, .addLineToPoint: count = 1
        case .addQuadCurveToPoint: count = 2
        case .addCurveToPoint: count = 3
        case .closeSubpath: count = 0
        @unknown default: count = 0
        }
        let values = UnsafeBufferPointer(start: element.points, count: count).flatMap { [$0.x, $0.y] }
        elements.append(PathElementRecord(element: element.type.rawValue, points: values))
    }
    return elements
}

// Ignore commands that do not change the geometry of a closed clip result.
func normalizedClipElements(_ elements: [PathElementRecord]) -> [PathElementRecord] {
    func near(_ lhs: CGPoint, _ rhs: CGPoint) -> Bool {
        abs(lhs.x - rhs.x) < 1e-8 && abs(lhs.y - rhs.y) < 1e-8
    }
    func endpoint(_ element: PathElementRecord) -> CGPoint {
        CGPoint(x: element.points[element.points.count - 2], y: element.points[element.points.count - 1])
    }
    var result: [PathElementRecord] = []
    var current = CGPoint.zero
    var start = CGPoint.zero
    for element in elements {
        if element.element == 0 {
            start = endpoint(element)
        } else if element.element == 1 {
            let next = endpoint(element)
            if near(current, next) { continue }
            if result.count >= 2, result.last?.element == 1,
               !result[result.count - 2].points.isEmpty {
                let previous = endpoint(result[result.count - 2])
                let a = CGPoint(x: current.x - previous.x, y: current.y - previous.y)
                let b = CGPoint(x: next.x - current.x, y: next.y - current.y)
                if abs(a.x * b.y - a.y * b.x) < 1e-8 && a.x * b.x + a.y * b.y >= 0 {
                    result.removeLast()
                }
            }
        } else if element.element == 2 || element.element == 3 {
            let isDegenerate = stride(from: 0, to: element.points.count, by: 2).allSatisfy {
                near(current, CGPoint(x: element.points[$0], y: element.points[$0 + 1]))
            }
            if isDegenerate { continue }
        } else if element.element == 4 {
            if result.last?.element == 1, near(current, start) {
                result.removeLast()
            }
            current = start
            result.append(element)
            continue
        }
        current = endpoint(element)
        result.append(element)
    }
    return result
}
#endif
