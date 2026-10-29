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
    @Test
    func replacesStorageCapacityBits() {
        let flags = ORB.Path.StorageFlags().withCapacity(15).withCapacity(16)
        #expect(flags.capacity() == 16)
    }
}

#endif
