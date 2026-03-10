//
//  DeviceTests.swift
//  OpenRenderBoxCompatibilityTests

import Testing

#if canImport(Darwin)

@MainActor
@Suite(.enabled(if: compatibilityTestEnabled))
struct DeviceTests {
    @Test
    func classMethods() {
        let device = RBDevice.sharedDefault()
        #expect(device != nil)

        let devices = RBDevice.allDevices()
        #expect(!devices.isEmpty)

        let supported = RBDevice.isSupported()
        #expect(supported == true)

        let priority = RBDevice.defaultGPUPriority()
        #expect(priority >= 0)

        let backgroundPriority = RBDevice.defaultBackgroundGPUPriority()
        #expect(backgroundPriority >= 0)

        let _ = RBDevice.allowsRenderingInBackground()
    }
}

#endif

