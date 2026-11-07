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

    @Test(arguments: [CGFloat(0), 0.25, 1])
    func clippedStrokeUsesProgressAndRoundCaps(progress: CGFloat) {
        let keys = [
            ClipKeyframe(tag: 0, value: 0, start: 0, end: 0.5),
            ClipKeyframe(tag: 0, value: 1, start: 0.5, end: 1),
        ]
        withPathStorage { storage in
            keys.withUnsafeBufferPointer { keys in
                var metadata = ClipInfo(flags: 0, cap: 1, count: keys.count, keys: keys.baseAddress)
                withUnsafePointer(to: &metadata) { metadata in
                    storage.append(element: .clipStroke, points: [progress], userInfo: metadata)
                }
            }
            storage.append(element: .rect, points: [0, 0, 10, 10], userInfo: nil)
            storage.append(element: .end, points: [], userInfo: nil)
            withStoragePath(storage) { path in
                let elements = cgPathElements(path.cgPath)
                if progress == 0 {
                    #expect(elements.isEmpty)
                } else if progress == 1 {
                    #expect(elements == [
                        PathElementRecord(element: 0, points: [0, 0]),
                        PathElementRecord(element: 1, points: [10, 0]),
                        PathElementRecord(element: 1, points: [10, 10]),
                        PathElementRecord(element: 1, points: [0, 10]),
                        PathElementRecord(element: 4, points: []),
                    ])
                } else {
                    #expect(elements == [
                        PathElementRecord(element: 0, points: [0, 0]),
                        PathElementRecord(element: 1, points: [5, 0]),
                        PathElementRecord(element: 3, points: [5, -1.3807119131088257, 6.1192879676818848, -2.5, 7.5, -2.5]),
                        PathElementRecord(element: 3, points: [8.880711555480957, -2.5, 10, -1.3807119131088257, 10, 0]),
                        PathElementRecord(element: 1, points: [10, 5]),
                        PathElementRecord(element: 1, points: [10, 10]),
                        PathElementRecord(element: 1, points: [5, 10]),
                        PathElementRecord(element: 3, points: [5, 11.380711555480957, 3.8807117938995361, 12.5, 2.5, 12.5]),
                        PathElementRecord(element: 3, points: [1.1192880868911743, 12.5, 0, 11.380711555480957, 0, 10]),
                        PathElementRecord(element: 1, points: [0, 5]),
                        PathElementRecord(element: 1, points: [0, 0]),
                        PathElementRecord(element: 4, points: []),
                    ])
                }
            }
        }
    }

    @Test
    func clippedStrokeUsesCustomKeyframeCap() {
        let keys = [
            ClipKeyframe(tag: 2, value: 0, start: 0, end: 0.25),
            ClipKeyframe(tag: 0, value: 1, start: 0.5, end: 0.75),
        ]
        expectClippedRectangle(keys: keys, progress: 0.25, expected: [
            PathElementRecord(element: 0, points: [0, 0]),
            PathElementRecord(element: 1, points: [10, 0]),
            PathElementRecord(element: 1, points: [10, 2.5]),
            PathElementRecord(element: 1, points: [0, 2.5]),
            PathElementRecord(element: 4, points: []),
        ])
    }

    @Test
    func clippedStrokeWrapsTaggedKeyframe() {
        let keys = [
            ClipKeyframe(tag: 0, value: 0, start: 0, end: 0.1),
            ClipKeyframe(tag: 1, value: 0.5, start: 0.8, end: 0.7),
            ClipKeyframe(tag: 0, value: 1, start: 0.5, end: 0.6),
        ]
        expectClippedRectangle(keys: keys, progress: 0.75, expected: [
            PathElementRecord(element: 0, points: [4, 10]),
            PathElementRecord(element: 1, points: [0, 10]),
            PathElementRecord(element: 1, points: [0, 8]),
            PathElementRecord(element: 1, points: [5, 9]),
            PathElementRecord(element: 3, points: [5.3186259269714355, 9.0637245178222656, 5.525263786315918, 9.3736820220947266, 5.4615383148193359, 9.6923074722290039]),
            PathElementRecord(element: 3, points: [5.3978133201599121, 10.010933876037598, 5.0878567695617676, 10.217571258544922, 4.769230842590332, 10.15384578704834]),
            PathElementRecord(element: 4, points: []),
            PathElementRecord(element: 0, points: [0, 0]),
            PathElementRecord(element: 1, points: [4, 0]),
            PathElementRecord(element: 1, points: [2, 10]),
            PathElementRecord(element: 1, points: [0, 10]),
            PathElementRecord(element: 4, points: []),
        ])
    }

    @Test(.enabled(if: referencePathGeometryEnabled))
    func clippedStrokeExtendsBelowProgressRange() {
        let keys = [
            ClipKeyframe(tag: 0, value: 0, start: 0, end: 0.5),
            ClipKeyframe(tag: 0, value: 1, start: 0.5, end: 1),
        ]
        expectClippedRectangle(keys: keys, flags: 4, progress: -0.25, expected: [
            PathElementRecord(element: 0, points: [8.821488380432129, 11.178511619567871]),
            PathElementRecord(element: 1, points: [-1.178511381149292, 11.178511619567871]),
            PathElementRecord(element: 1, points: [-1.178511381149292, 1.178511381149292]),
            PathElementRecord(element: 1, points: [8.821488380432129, 1.178511381149292]),
            PathElementRecord(element: 1, points: [8.821488380432129, 11.178511619567871]),
            PathElementRecord(element: 1, points: [-1.178511381149292, 11.178511619567871]),
            PathElementRecord(element: 1, points: [-1.178511381149292, 1.178511381149292]),
            PathElementRecord(element: 1, points: [8.821488380432129, 1.178511381149292]),
            PathElementRecord(element: 3, points: [11.582912445068359, 1.178511381149292, 13.821488380432129, 3.4170875549316406, 13.821488380432129, 6.178511142730713]),
            PathElementRecord(element: 3, points: [13.821488380432129, 8.939934730529785, 11.582912445068359, 11.178511619567871, 8.821488380432129, 11.178511619567871]),
            PathElementRecord(element: 4, points: []),
        ])
    }

    private func expectClippedRectangle(
        keys: [ClipKeyframe], flags: Int32 = 0, progress: CGFloat, expected: [PathElementRecord]
    ) {
        withPathStorage { storage in
            keys.withUnsafeBufferPointer { keys in
                var metadata = ClipInfo(flags: flags, cap: 1, count: keys.count, keys: keys.baseAddress)
                withUnsafePointer(to: &metadata) { metadata in
                    storage.append(element: .clipStroke, points: [progress], userInfo: metadata)
                }
            }
            storage.append(element: .rect, points: [0, 0, 10, 10], userInfo: nil)
            storage.append(element: .end, points: [], userInfo: nil)
            withStoragePath(storage) { path in
                let actual = normalizedClipElements(cgPathElements(path.cgPath))
                #expect(actual.map(\.element) == expected.map(\.element))
                for (actual, expected) in zip(actual, expected) {
                    #expect(actual.points.count == expected.points.count)
                    for (actual, expected) in zip(actual.points, expected.points) {
                        #expect(abs(actual - expected) < 1e-6)
                    }
                }
            }
        }
    }

    @Test
    func glyphDilationPreservesClosedGeometry() {
        withPathStorage { storage in
            storage.append(element: .dilateGlyph, points: [2, 3], userInfo: nil)
            storage.append(element: .rect, points: [0, 0, 10, 10], userInfo: nil)
            storage.append(element: .end, points: [], userInfo: nil)
            withStoragePath(storage) { path in
                let actual = cgPathElements(path.cgPath)
                #expect(actual == [
                    PathElementRecord(element: 0, points: [0, 0]),
                    PathElementRecord(element: 1, points: [10, 0]),
                    PathElementRecord(element: 1, points: [12, 3]),
                    PathElementRecord(element: 1, points: [12, 13]),
                    PathElementRecord(element: 1, points: [10, 16]),
                    PathElementRecord(element: 1, points: [0, 16]),
                    PathElementRecord(element: 1, points: [-2, 13]),
                    PathElementRecord(element: 1, points: [-2, 3]),
                    PathElementRecord(element: 4, points: []),
                ])
            }
        }
    }

    @Test(arguments: [CGFloat(2), -2])
    func offsetCurveUsesMappedStorage(amount: CGFloat) {
        withPathStorage(capacity: 64) { storage in
            storage.append(element: .offset, points: [amount], userInfo: nil)
            storage.append(element: .moveToPoint, points: [10, 20], userInfo: nil)
            storage.append(element: .addCurveToPoint, points: [10, 30, 20, 30, 20, 20], userInfo: nil)
            storage.append(element: .end, points: [], userInfo: nil)
            withStoragePath(storage) { path in
                let cgPath = path.cgPath
                let expected: [PathElementRecord] = amount > 0
                    ? [PathElementRecord(element: 0, points: [8, 20]),
                       PathElementRecord(element: 3, points: [8, 32.666667938232422, 22, 32.666667938232422, 22, 20])]
                    : [PathElementRecord(element: 0, points: [12, 20]),
                       PathElementRecord(element: 3, points: [12, 27.333333969116211, 18, 27.333333969116211, 18, 20])]
                #expect(cgPathElements(cgPath) == expected)
                storage.append(element: .rect, points: [100, 100, 10, 10], userInfo: nil)
                #expect(path.cgPath.boundingBoxOfPath.maxX == 110)
                #expect(cgPathElements(cgPath) == expected)
            }
        }
    }

    @Test(arguments: [ORBPath.Element.boolean, .offset, .transformed])
    func nestedBooleanPreservesPathBoundaries(inner: ORBPath.Element) {
        withPathStorage { storage in
            let intersection: [UInt8] = [1, 0]
            intersection.withUnsafeBufferPointer { flags in
                storage.append(element: .boolean, points: [], userInfo: flags.baseAddress)
            }
            if inner == .boolean {
                let normalize: [UInt8] = [0, 0]
                normalize.withUnsafeBufferPointer { flags in
                    storage.append(element: .boolean, points: [], userInfo: flags.baseAddress)
                }
            } else if inner == .offset {
                storage.append(element: .offset, points: [0], userInfo: nil)
            } else {
                var transform = CGAffineTransform.identity
                withUnsafePointer(to: &transform) { transform in
                    storage.append(element: .transformed, points: [], userInfo: transform)
                }
            }
            storage.append(element: .rect, points: [0, 0, 10, 10], userInfo: nil)
            storage.append(element: .end, points: [], userInfo: nil)
            storage.append(element: .rect, points: [5, 5, 10, 10], userInfo: nil)
            storage.append(element: .end, points: [], userInfo: nil)
            withStoragePath(storage) { path in
                let cgPath = path.cgPath
                let coalescesPaths = inner == .transformed
                #expect(cgPath.boundingBoxOfPath == (coalescesPaths
                    ? CGRect(x: 0, y: 0, width: 15, height: 15)
                    : CGRect(x: 5, y: 5, width: 5, height: 5)))
                #expect(cgPath.contains(CGPoint(x: 7, y: 7)))
                #expect(cgPath.contains(CGPoint(x: 2, y: 2)) == coalescesPaths)
                #expect(cgPath.contains(CGPoint(x: 12, y: 12)) == coalescesPaths)
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
