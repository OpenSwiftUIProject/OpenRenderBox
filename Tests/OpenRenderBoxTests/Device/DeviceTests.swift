//
//  DeviceTests.swift
//  OpenRenderBox
//
//  Created by Kyle on 1/4/26.
//

#if canImport(Darwin)
import Testing
import OpenRenderBoxCxx.ObjC

@Suite
struct DeviceTests {
    @Test
    func device() {
        let _ = ORBDevice()
    }
}
#endif
