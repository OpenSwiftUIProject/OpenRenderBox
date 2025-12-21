//
//  PathTests.swift
//  OpenRenderBox
//
//  Created by Kyle on 12/21/25.
//

#if canImport(Darwin)
import OpenRenderBoxCxx.Path
import Testing

@Suite
struct PathTests {
    @Test
    func demo() {
        let storage = ORB.Path.Storage(3)
    }
}
#endif
