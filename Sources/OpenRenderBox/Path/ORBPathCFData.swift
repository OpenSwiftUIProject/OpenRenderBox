//
//  ORBPathCFData.swift
//  OpenRenderBox

#if !canImport(Darwin)
public import CoreFoundation

extension ORBPath {
    public var data: CFData? {
        let bytes = _encodeWireBytes()
        return bytes.withUnsafeBufferPointer { buffer in
            CFDataCreate(nil, buffer.baseAddress, buffer.count)
        }
    }

    public init(data: CFData?) {
        guard let data else {
            self = .null
            return
        }
        let length = CFDataGetLength(data)
        if length <= 0 {
            self.init(_wireBytes: [])
        } else if let ptr = CFDataGetBytePtr(data) {
            self.init(_wireBytes: Array(UnsafeBufferPointer(start: ptr, count: length)))
        } else {
            let bytes = [UInt8](unsafeUninitializedCapacity: length) { buffer, initializedCount in
                CFDataGetBytes(data, CFRange(location: 0, length: length), buffer.baseAddress)
                initializedCount = length
            }
            self.init(_wireBytes: bytes)
        }
    }
}
#endif
