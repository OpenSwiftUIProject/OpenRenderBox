//
//  PortableCFDataTestSupport.swift
//  OpenRenderBoxTests

#if !OPENRENDERBOX_CF_CGTYPES
import CoreFoundation

func makeTestCFData(_ bytes: [UInt8]) -> CFData {
    bytes.withUnsafeBufferPointer { buffer in
        CFDataCreate(nil, buffer.baseAddress, buffer.count)!
    }
}

func testCFDataBytes(_ data: CFData) -> [UInt8] {
    let length = CFDataGetLength(data)
    guard length > 0 else {
        return []
    }
    if let ptr = CFDataGetBytePtr(data) {
        return Array(UnsafeBufferPointer(start: ptr, count: length))
    }
    return [UInt8](unsafeUninitializedCapacity: length) { buffer, initializedCount in
        CFDataGetBytes(data, CFRange(location: 0, length: length), buffer.baseAddress)
        initializedCount = length
    }
}
#endif
