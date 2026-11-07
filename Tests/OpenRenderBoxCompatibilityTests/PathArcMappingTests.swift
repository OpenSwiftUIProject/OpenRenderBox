//
//  PathArcMappingTests.swift
//  OpenRenderBoxCompatibilityTests

#if canImport(CoreGraphics) && (OPENRENDERBOX_CF_CGTYPES || OPENRENDERBOX_COMPATIBILITY_TEST)
import CoreGraphics
import Foundation
import Testing

struct PathArcMappingTests {
    @Test(arguments: [ORBPath.Element.arc, .arcCounterclockwise])
    func quarterTurnUsesStableSegments(element: ORBPath.Element) {
        withPathStorage { storage in
            storage.append(element: element, points: [0, 0, 10, 0, .pi / 2], userInfo: nil)
            withStoragePath(storage) { path in
                let elements = cgPathElements(path.cgPath)
                #expect(elements.count == (element == .arc ? 4 : 2))
                #expect(elements.first?.element == 0)
                let end = path.cgPath.currentPoint
                #expect(abs(end.x) < 0.00001)
                #expect(abs(end.y - 10) < 0.00001)
            }
        }
    }

    @Test(arguments: [ORBPath.Element.arc, .arcCounterclockwise])
    func fullTurnClosesTheCircle(element: ORBPath.Element) {
        withPathStorage { storage in
            storage.append(element: element, points: [0, 0, 10, 0, 2 * .pi], userInfo: nil)
            withStoragePath(storage) { path in
                let elements = cgPathElements(path.cgPath)
                #expect(elements.count == 6)
                #expect(elements.last?.element == 4)
                #expect(path.boundingRect == CGRect(x: -10, y: -10, width: 20, height: 20))
            }
        }
    }

    @Test
    func relativeArcPreservesTwoTurns() {
        withPathStorage { storage in
            storage.append(element: .relativeArc, points: [0, 0, 10, 0, 4 * .pi], userInfo: nil)
            withStoragePath(storage) { path in
                #expect(cgPathElements(path.cgPath).count == 9)
                #expect(path.boundingRect == CGRect(x: -10, y: -10, width: 20, height: 20))
            }
        }
    }

    @Test
    func relativeArcOmitsSmallResiduals() {
        withPathStorage { storage in
            storage.append(element: .relativeArc, points: [0, 0, 10, 0, 0.000001], userInfo: nil)
            withStoragePath(storage) { path in
                #expect(cgPathElements(path.cgPath) == [PathElementRecord(element: 0, points: [10, 0])])
            }
        }
    }
}
#endif
