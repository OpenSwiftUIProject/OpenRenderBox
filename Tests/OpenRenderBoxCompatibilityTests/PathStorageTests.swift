//
//  PathStorageTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics

@Suite
struct PathStorageTests {
    private struct ClipKeyframe: Equatable {
        var tag: Int32
        var value: Float
        var start: Float
        var end: Float
    }

    private struct ClipInfo {
        var flags: Int32
        var cap: Int32
        var count: Int
        var keys: UnsafePointer<ClipKeyframe>?
    }
    @Test
    func storageIsEmpty() {
        let path = ORBPath(rect: CGRect(x: 0, y: 0, width: 100, height: 100), transform: nil)
        defer { path.release() }
        withPathStorage { storage in
            #expect(storage.isEmpty)
            #expect(storage.append(path: path))
            #expect(!storage.isEmpty)
            storage.clear()
            #expect(storage.isEmpty)
            #expect(pathElements(storage).isEmpty)
        }
    }

    @Test
    func storageIsSingleElement() {
        let path = ORBPath(rect: CGRect(x: 0, y: 0, width: 100, height: 100), transform: nil)
        defer { path.release() }
        withPathStorage { storage in
            #expect(!storage.isSingleElement)
            #expect(storage.append(path: path))
            #expect(storage.isSingleElement)
            #expect(storage.append(path: path))
            #expect(!storage.isSingleElement)
        }
    }

    @Test
    func storageBezierOrder() {
        withPathStorage { storage in
            #expect(storage.bezierOrder == 1)
            storage.append(element: .moveToPoint, points: [0, 0], userInfo: nil)
            storage.append(element: .addQuadCurveToPoint, points: [1, 2, 3, 4], userInfo: nil)
            #expect(storage.bezierOrder == 2)
            storage.append(element: .addCurveToPoint, points: [5, 6, 7, 8, 9, 10], userInfo: nil)
            #expect(storage.bezierOrder == 3)
            storage.clear()
            #expect(storage.bezierOrder == 1)
        }
    }

    @Test
    func storageBoundingRect() {
        let rect = CGRect(x: 10, y: 20, width: 100, height: 50)
        let path = ORBPath(rect: rect, transform: nil)
        defer { path.release() }
        withPathStorage { storage in
            #expect(storage.append(path: path))
            #expect(storage.boundingRect == rect)
            #expect(pathElements(storage) == pathElements(path))
        }
    }

    @Test(arguments: [
        (CGRect(x: 0, y: 0, width: 100, height: 100), CGRect(x: 0, y: 0, width: 100, height: 100), true),
        (CGRect(x: 0, y: 0, width: 100, height: 100), CGRect(x: 0, y: 0, width: 200, height: 200), false),
        (CGRect(x: 10, y: 20, width: 50, height: 50), CGRect(x: 10, y: 20, width: 50, height: 50), true),
    ])
    func storageEquality(rect1: CGRect, rect2: CGRect, expectedEqual: Bool) {
        let path1 = ORBPath(rect: rect1, transform: nil)
        let path2 = ORBPath(rect: rect2, transform: nil)
        defer {
            path1.release()
            path2.release()
        }
        withPathStorage { storage1 in
            withPathStorage { storage2 in
                #expect(storage1.append(path: path1))
                #expect(storage2.append(path: path2))
                #expect(storage1.isEqual(to: storage2) == expectedEqual)
            }
        }
    }

    @Test(arguments: [64, 256], [1, 40])
    func storageCopyPreservesElements(capacity: Int, lineCount: Int) {
        withPathStorage(capacity: capacity) { source in
            source.append(element: .moveToPoint, points: [1, 2], userInfo: nil)
            for index in 1...lineCount {
                source.append(element: .addLineToPoint, points: [CGFloat(index), CGFloat(index * 2)], userInfo: nil)
            }
            let expected = pathElements(source)
            for destinationCapacity in [64, 256] {
                withPathStorage(capacity: destinationCapacity, source: source) { copy in
                    #expect(pathElements(copy) == expected)
                    #expect(copy.isEqual(to: source))
                    copy.append(element: .addLineToPoint, points: [100, 200], userInfo: nil)
                    #expect(pathElements(source) == expected)
                }
            }
            withPathStorage(capacity: 64, source: source) { copy in
                source.clear()
                #expect(pathElements(copy) == expected)
            }
        }
    }

    @Test
    func appendingStorageToItselfUsesSnapshot() {
        withPathStorage { storage in
            storage.append(element: .moveToPoint, points: [1, 2], userInfo: nil)
            storage.append(element: .addLineToPoint, points: [3, 4], userInfo: nil)
            let original = pathElements(storage)
            withStoragePath(storage) { path in
                #expect(storage.append(path: path))
            }
            #expect(pathElements(storage) == original + original)
        }
    }

    @Test
    func storageOwnsClipStrokeKeyframes() {
        let expected = [
            ClipKeyframe(tag: 0, value: 0, start: 0, end: 0.5),
            ClipKeyframe(tag: 0, value: 1, start: 0.5, end: 1),
        ]
        #expect(MemoryLayout<ClipKeyframe>.size == 16)
        #expect(MemoryLayout<ClipInfo>.size == 24)
        withPathStorage(capacity: 64) { storage in
            var keys = expected
            keys.withUnsafeBufferPointer { keys in
                var metadata = ClipInfo(flags: 2, cap: 1, count: keys.count, keys: keys.baseAddress)
                withUnsafePointer(to: &metadata) { metadata in
                    storage.append(element: .clipStroke, points: [0.25], userInfo: metadata)
                }
            }
            keys[0].value = 42
            keys.removeAll()
            storage.append(element: .rect, points: [0, 0, 10, 10], userInfo: nil)
            storage.append(element: .end, points: [], userInfo: nil)
            withPathStorage(capacity: 64, source: storage) { copy in
                storage.clear()
                var result: [ClipKeyframe] = []
                withUnsafeMutablePointer(to: &result) { result in
                    _ = copy.apply(info: result) { info, element, _, metadata in
                        if element == .clipStroke, let metadata {
                            let clip = metadata.assumingMemoryBound(to: ClipInfo.self).pointee
                            info.assumingMemoryBound(to: [ClipKeyframe].self).pointee = Array(UnsafeBufferPointer(start: clip.keys, count: clip.count))
                        }
                        return true
                    }
                }
                #expect(result == expected)
            }
        }
    }

    @Test
    func storageStopsEnumeration() {
        withPathStorage { storage in
            storage.append(element: .moveToPoint, points: [1, 2], userInfo: nil)
            storage.append(element: .addLineToPoint, points: [3, 4], userInfo: nil)
            var count = 0
            let completed = storage.apply(info: &count) { info, _, _, _ in
                info.assumingMemoryBound(to: Int.self).pointee += 1
                return false
            }
            #expect(!completed)
            #expect(count == 1)
        }
    }

    @Test("Cached paths follow storage mutation", arguments: 1...4)
    func storageGetCGPath(iteration: Int) throws {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path = ORBPath(rect: rect, transform: nil)
        defer { path.release() }
        try withPathStorage(capacity: 64) { storage in
            #expect(storage.append(path: path))
            for _ in 1...7 {
                storage.append(element: .moveToPoint, points: [50, 50], userInfo: nil)
                storage.append(element: .addLineToPoint, points: [100, 50], userInfo: nil)
                storage.append(element: .closeSubpath, points: [], userInfo: nil)
            }
            let cached = try #require(storage.cgPath)
            #expect(cached.boundingBoxOfPath == rect)
            storage.append(element: .moveToPoint, points: [200, 200], userInfo: nil)
            storage.append(element: .addLineToPoint, points: [300, 300], userInfo: nil)
            #expect(storage.cgPath?.boundingBoxOfPath == CGRect(x: 0, y: 0, width: 300, height: 300))
            #expect(cached.boundingBoxOfPath == rect)
        }
    }
}

#endif
