//
//  DeviceTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(Darwin)

@MainActor
@Suite(.enabled(if: compatibilityTestEnabled))
struct ORBDeviceTests {
    @Test
    func classMethods() {
        let device = ORBDevice.sharedDefault()
        #expect(device != nil)

        let devices = ORBDevice.allDevices()
        #expect(devices != nil)

        let supported = ORBDevice.isSupported()
        #expect(supported == true)

        let priority = ORBDevice.defaultGPUPriority()
        #expect(priority >= 0)

        let backgroundPriority = ORBDevice.defaultBackgroundGPUPriority()
        #expect(backgroundPriority >= 0)

        let _ = ORBDevice.allowsRenderingInBackground()
    }
}

#endif

