//
//  PathStorageTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(CoreGraphics)
import CoreGraphics

@Suite(.enabled(if: compatibilityTestEnabled))
struct PathStorageTests {
    @Test
    func storageIsEmpty() {
        let path = ORBPath(rect: CGRect(x: 0, y: 0, width: 100, height: 100), transform: nil)
        let storage = path.storage
        storage.initialize(capacity: 96, source: nil)
        #expect(storage.isEmpty == true)
        storage.append(path: path)
        #expect(storage.isEmpty == false)
        storage.destroy()
        path.release()
    }

    @Test
    func storageIsSingleElement() {
        let path = ORBPath(rect: CGRect(x: 0, y: 0, width: 100, height: 100), transform: nil)
        let storage = path.storage
        storage.initialize(capacity: 96, source: nil)
        #expect(storage.isSingleElement == false)
        storage.append(path: path)
        #expect(storage.isSingleElement == true)
        storage.destroy()
        path.release()
    }
    
    @Test
    func storageBezierOrder() {
        let path = ORBPath(rect: CGRect(x: 0, y: 0, width: 100, height: 100), transform: nil)
        let order = path.storage.bezierOrder
        #expect(order == 1)
        path.release()
    }
    
    @Test
    func storageBoundingRect() {
        let rect = CGRect(x: 10, y: 20, width: 100, height: 50)
        let path = ORBPath(rect: rect, transform: nil)
        let storage = path.storage
        storage.initialize(capacity: 96, source: nil)
        storage.append(path: path)
        let boundingRect = path.storage.boundingRect
        #expect(boundingRect.origin.x == rect.origin.x)
        #expect(boundingRect.origin.y == rect.origin.y)
        #expect(boundingRect.size.width == rect.size.width)
        #expect(boundingRect.size.height == rect.size.height)
        storage.destroy()
        path.release()
    }
    
    @Test
    func storageEqualToStorage() {
        let rect = CGRect(x: 0, y: 0, width: 100, height: 100)
        let path1 = ORBPath(rect: rect, transform: nil)
        let path2 = ORBPath(rect: rect, transform: nil)
        let storage1 = path1.storage
        let storage2 = path2.storage
        #expect(storage1.isEqual(to: storage2) == true)
        storage1.initialize(capacity: 96, source: nil)
        storage1.append(path: path1)
        storage2.initialize(capacity: 96, source: nil)
        storage2.append(path: path2)
        #expect(storage1.isEqual(to: storage2) == true)
        storage1.destroy()
        storage2.destroy()
        path1.release()
        path2.release()
    }
    
    @Test
    func storageNotEqual() {
        let path1 = ORBPath(rect: CGRect(x: 0, y: 0, width: 100, height: 100), transform: nil)
        let path2 = ORBPath(rect: CGRect(x: 0, y: 0, width: 200, height: 200), transform: nil)
        let storage1 = path1.storage
        let storage2 = path2.storage
        #expect(storage1.isEqual(to: storage2) == true)
        storage1.initialize(capacity: 96, source: nil)
        storage1.append(path: path1)
        storage2.initialize(capacity: 96, source: nil)
        storage2.append(path: path2)
        #expect(storage1.isEqual(to: storage2) == false)
        storage1.destroy()
        storage2.destroy()
        path1.release()
        path2.release()
    }
}

#endif

