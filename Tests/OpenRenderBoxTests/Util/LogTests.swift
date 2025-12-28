//
//  LogTests.swift
//  OpenRenderBox
//
//  Created by Kyle on 2025/9/8.
//

#if canImport(Darwin)
import OpenRenderBoxCxx.Util
import Testing

@Suite
struct LogTests {
    @Test
    func table() {
        _ = ORB.misc_log()
        _ = ORB.error_log()
    }
}
#endif
