//
//  PathTestSupport.swift
//  OpenRenderBoxCompatibilityTests

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics
import Foundation

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
