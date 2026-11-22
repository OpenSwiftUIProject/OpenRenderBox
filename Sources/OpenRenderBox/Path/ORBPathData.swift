//
//  ORBPathData.swift
//  OpenRenderBox

#if !canImport(Darwin)
import OpenCoreGraphicsShims

private typealias _ORBCGFloat = CGFloat
private typealias _ORBCGAffineTransform = CGAffineTransform

private let _orbElementArgumentCounts = [2, 2, 4, 6, 0, 4, 4, 4, 6, 6, 8, 8, 5, 5, 5, 5, 0, 0, 2, 3, 0, 0, 2, 1, 1]

private func _orbArgumentCount(_ element: ORBPath.Element) -> Int {
    let index = Int(element.rawValue)
    return _orbElementArgumentCounts.indices.contains(index) ? _orbElementArgumentCounts[index] : 0
}

private func _orbMetadataKind(_ element: UInt32) -> UInt32 {
    switch element {
    case 4, 16, 18, 22, 23: return 0
    case 19: return 2
    case 20: return 3
    case 24: return 4
    default: return 1
    }
}

private struct _ORBWireEncoder {
    var bytes: [UInt8] = []

    mutating func appendVarint(_ value: UInt64) {
        var current = value
        repeat {
            let byte = UInt8(current & 0x7f)
            current >>= 7
            bytes.append(byte | (current != 0 ? 0x80 : 0))
        } while current != 0
    }

    mutating func appendInteger(_ field: UInt32, _ value: UInt64) {
        guard value != 0 else { return }
        appendVarint(UInt64(field) << 3)
        appendVarint(value)
    }

    mutating func appendFloatScalar(_ value: Float) {
        var bits = value.bitPattern.littleEndian
        withUnsafeBytes(of: &bits) { bytes.append(contentsOf: $0) }
    }

    mutating func appendDoubleScalar(_ value: Double) {
        var bits = value.bitPattern.littleEndian
        withUnsafeBytes(of: &bits) { bytes.append(contentsOf: $0) }
    }

    mutating func appendReal(_ field: UInt32, _ value: Float, defaultValue: Float = 0) {
        guard value != defaultValue else { return }
        appendVarint((UInt64(field) << 3) | 5)
        appendFloatScalar(value)
    }

    mutating func appendData(_ field: UInt32, _ value: [UInt8]) {
        appendVarint((UInt64(field) << 3) | 2)
        appendVarint(UInt64(value.count))
        bytes.append(contentsOf: value)
    }
}

private struct _ORBWireField {
    var number: UInt32 = 0
    var wire: UInt32 = 0
    var integer: UInt64 = 0
    var slice: ArraySlice<UInt8> = []
}

private struct _ORBWireDecoder {
    let bytes: [UInt8]
    var offset: Int = 0
    var valid: Bool = true

    init(_ bytes: [UInt8]) {
        self.bytes = bytes
    }

    mutating func readVarint(_ result: inout UInt64) -> Bool {
        result = 0
        var shift: UInt64 = 0
        while shift < 70 {
            guard offset < bytes.count else {
                valid = false
                return false
            }
            let byte = bytes[offset]
            offset += 1
            if shift == 63 && byte > 1 {
                valid = false
                return false
            }
            result |= UInt64(byte & 0x7f) << shift
            if (byte & 0x80) == 0 {
                return true
            }
            shift += 7
        }
        valid = false
        return false
    }

    mutating func next(_ field: inout _ORBWireField) -> Bool {
        guard valid, offset < bytes.count else { return false }
        var key: UInt64 = 0
        guard readVarint(&key), (key >> 3) != 0, (key >> 3) <= 0x1fff_ffff else {
            valid = false
            return false
        }
        field = _ORBWireField(number: UInt32(key >> 3), wire: UInt32(key & 7))
        if field.wire == 0 {
            return readVarint(&field.integer)
        }
        let count: UInt64
        switch field.wire {
        case 1:
            count = 8
        case 5:
            count = 4
        case 2:
            var length: UInt64 = 0
            guard readVarint(&length) else { return false }
            count = length
        default:
            valid = false
            return false
        }
        guard count <= UInt64(bytes.count - offset) else {
            valid = false
            return false
        }
        let length = Int(count)
        field.slice = bytes[offset..<(offset + length)]
        offset += length
        return true
    }
}

private func _orbReadFloats(_ field: _ORBWireField, _ append: (Float) -> Void) -> Bool {
    guard field.wire == 2 || field.wire == 5, field.slice.count % 4 == 0 else { return false }
    var index = field.slice.startIndex
    while index < field.slice.endIndex {
        let b0 = UInt32(field.slice[index])
        let b1 = UInt32(field.slice[index + 1]) << 8
        let b2 = UInt32(field.slice[index + 2]) << 16
        let b3 = UInt32(field.slice[index + 3]) << 24
        append(Float(bitPattern: b0 | b1 | b2 | b3))
        index += 4
    }
    return true
}

private func _orbReadDoubles(_ field: _ORBWireField, _ append: (Double) -> Void) -> Bool {
    guard field.wire == 2 || field.wire == 1, field.slice.count % 8 == 0 else { return false }
    var index = field.slice.startIndex
    while index < field.slice.endIndex {
        var bits: UInt64 = 0
        for shift in 0..<8 {
            bits |= UInt64(field.slice[index + shift]) << (UInt64(shift) * 8)
        }
        append(Double(bitPattern: bits))
        index += 8
    }
    return true
}

private struct _ORBDecodedMetadata {
    var kind: UInt32 = 0
    var affine: _ORBCGAffineTransform = .identity
    var stroke = _ORBPathStrokeInfo(lineCap: 0, lineJoin: 0, dash: [])
    var boolean: [UInt8] = [0, 0]
    var clip = _ORBPathClipInfo(flags: 0, cap: 1, records: [])
}

private func _orbDecodeClipKey(_ message: _ORBWireField, _ key: inout _ORBPathClipRecord) -> Bool {
    guard message.wire == 2 else { return false }
    var decoder = _ORBWireDecoder(Array(message.slice))
    var field = _ORBWireField()
    while decoder.next(&field) {
        switch field.number {
        case 1:
            guard _orbReadFloats(field, { key.x = $0 }) else { return false }
        case 2:
            guard _orbReadFloats(field, { key.y = $0 }) else { return false }
        case 3:
            guard _orbReadFloats(field, { key.z = $0 }) else { return false }
        case 4, 5:
            guard field.wire == 0 else { return false }
            if field.integer != 0 {
                key.type |= field.number == 4 ? 1 : 2
            }
        default:
            break
        }
    }
    return decoder.valid
}

private func _orbDecodeMetadata(_ message: _ORBWireField, _ metadata: inout _ORBDecodedMetadata) -> Bool {
    guard message.wire == 2 else { return false }
    var outer = _ORBWireDecoder(Array(message.slice))
    var variant = _ORBWireField()
    while outer.next(&variant) {
        if variant.number > 4 || metadata.kind != 0 { continue }
        guard variant.wire == 2 else { return false }
        metadata.kind = variant.number
        var decoder = _ORBWireDecoder(Array(variant.slice))
        var field = _ORBWireField()
        while decoder.next(&field) {
            switch metadata.kind {
            case 1:
                switch field.number {
                case 1: guard _orbReadFloats(field, { metadata.affine.a = _ORBCGFloat($0) }) else { return false }
                case 2: guard _orbReadFloats(field, { metadata.affine.b = _ORBCGFloat($0) }) else { return false }
                case 3: guard _orbReadFloats(field, { metadata.affine.c = _ORBCGFloat($0) }) else { return false }
                case 4: guard _orbReadFloats(field, { metadata.affine.d = _ORBCGFloat($0) }) else { return false }
                case 5: guard _orbReadFloats(field, { metadata.affine.tx = _ORBCGFloat($0) }) else { return false }
                case 6: guard _orbReadFloats(field, { metadata.affine.ty = _ORBCGFloat($0) }) else { return false }
                default: break
                }
            case 2:
                if field.number <= 2 {
                    guard field.wire == 0 else { return false }
                    if field.number == 1 && field.integer <= 6 { metadata.stroke.lineCap = Int32(field.integer) }
                    if field.number == 2 && field.integer <= 2 { metadata.stroke.lineJoin = Int32(field.integer) }
                } else if field.number == 3 {
                    guard _orbReadFloats(field, { metadata.stroke.dash.append(_ORBCGFloat($0)) }) else { return false }
                }
            case 3:
                if field.number <= 2 {
                    guard field.wire == 0 else { return false }
                    if field.number == 1 && field.integer <= 6 { metadata.boolean[0] = UInt8(field.integer) }
                    if field.number == 2 { metadata.boolean[1] = field.integer != 0 ? 1 : 0 }
                }
            default:
                if field.number == 5 {
                    var key = _ORBPathClipRecord(type: 0, x: 0, y: 0, z: 0)
                    guard _orbDecodeClipKey(field, &key) else { return false }
                    metadata.clip.records.append(key)
                } else if field.number <= 6 {
                    guard field.wire == 0 else { return false }
                    if field.number == 1 {
                        if field.integer <= 6 { metadata.clip.cap = Int32(field.integer) }
                    } else if field.integer != 0 {
                        let flags: [Int32] = [0, 0, 4, 1, 2, 0, 8]
                        metadata.clip.flags |= flags[Int(field.number)]
                    }
                }
            }
        }
        guard decoder.valid else { return false }
    }
    return outer.valid
}

private func _orbEncodeMetadata(_ element: UInt32, _ info: UnsafeRawPointer?) -> [UInt8] {
    guard let info, element != 4, element != 16, element != 18, element != 22, element != 23 else {
        return []
    }
    var result = _ORBWireEncoder()
    var value = _ORBWireEncoder()
    switch element {
    case 19:
        let stroke = info.load(as: _ORBPathStrokePayload.self)
        if stroke.lineCap == 0 && stroke.lineJoin == 0 && stroke.dashCount == 0 { return [] }
        value.appendInteger(1, UInt64(bitPattern: Int64(stroke.lineCap)))
        value.appendInteger(2, UInt64(bitPattern: Int64(stroke.lineJoin)))
        if stroke.dashCount > 0, let dashPtr = stroke.dash {
            var dash = _ORBWireEncoder()
            for i in 0..<stroke.dashCount {
                dash.appendFloatScalar(Float(dashPtr[i]))
            }
            value.appendData(3, dash.bytes)
        }
        result.appendData(2, value.bytes)
    case 20:
        let flags = info.assumingMemoryBound(to: UInt8.self)
        if flags[0] == 0 && flags[1] != 1 { return [] }
        value.appendInteger(1, UInt64(flags[0]))
        value.appendInteger(2, flags[1] == 1 ? 1 : 0)
        result.appendData(3, value.bytes)
    case 24:
        let clip = info.load(as: _ORBPathClipPayload.self)
        if clip.cap == 1 && clip.count == 0 && clip.flags == 0 { return [] }
        if clip.cap != 1 {
            value.appendVarint(8)
            value.appendVarint(UInt64(bitPattern: Int64(clip.cap)))
        }
        value.appendInteger(2, (clip.flags & 4) != 0 ? 1 : 0)
        value.appendInteger(3, (clip.flags & 1) != 0 ? 1 : 0)
        value.appendInteger(4, (clip.flags & 2) != 0 ? 1 : 0)
        if clip.count > 0, let keys = clip.records {
            for i in 0..<clip.count {
                let key = keys[i]
                var item = _ORBWireEncoder()
                item.appendReal(1, key.x)
                item.appendReal(2, key.y)
                item.appendReal(3, key.z)
                item.appendInteger(4, (key.type & 1) != 0 ? 1 : 0)
                item.appendInteger(5, (key.type & 2) != 0 ? 1 : 0)
                value.appendData(5, item.bytes)
            }
        }
        value.appendInteger(6, (clip.flags & 8) != 0 ? 1 : 0)
        result.appendData(4, value.bytes)
    default:
        let affine = info.load(as: _ORBCGAffineTransform.self)
        if affine.isIdentity { return [] }
        value.appendReal(1, Float(affine.a), defaultValue: 1)
        value.appendReal(2, Float(affine.b))
        value.appendReal(3, Float(affine.c))
        value.appendReal(4, Float(affine.d), defaultValue: 1)
        value.appendReal(5, Float(affine.tx))
        value.appendReal(6, Float(affine.ty))
        result.appendData(1, value.bytes)
    }
    return result.bytes
}

extension ORBPath {
    func _encodeWireBytes() -> [UInt8] {
        struct EncodingState {
            var output = _ORBWireEncoder()
            var elements: [UInt8] = []
            var floats = _ORBWireEncoder()
            var doubles = _ORBWireEncoder()
        }
        var state = EncodingState()
        _ = withUnsafeMutablePointer(to: &state) { statePtr in
            apply(info: UnsafeMutableRawPointer(statePtr)) { info, element, points, userInfo in
                guard element.rawValue >= 0 else { return false }
                let state = info.assumingMemoryBound(to: EncodingState.self)
                let type = UInt32(element.rawValue)
                let metadata = _orbEncodeMetadata(type, userInfo)
                var code = UInt8(type)
                if !metadata.isEmpty {
                    code |= 0x80
                    state.pointee.output.appendData(4, metadata)
                }
                let count = _orbArgumentCount(element)
                for i in 0..<count {
                    if !(abs(points[i]) <= 32767) {
                        code |= 0x40
                    }
                }
                state.pointee.elements.append(code)
                for i in 0..<count {
                    if (code & 0x40) != 0 {
                        state.pointee.doubles.appendDoubleScalar(Double(points[i]))
                    } else {
                        state.pointee.floats.appendFloatScalar(Float(points[i]))
                    }
                }
                return true
            }
        }
        if !state.elements.isEmpty { state.output.appendData(3, state.elements) }
        if !state.floats.bytes.isEmpty { state.output.appendData(2, state.floats.bytes) }
        if !state.doubles.bytes.isEmpty { state.output.appendData(5, state.doubles.bytes) }
        return state.output.bytes
    }

    init(_wireBytes bytes: [UInt8]) {
        var decoder = _ORBWireDecoder(bytes)
        var elements: [UInt8] = []
        var floats: [_ORBCGFloat] = []
        var doubles: [_ORBCGFloat] = []
        var metadataList: [_ORBDecodedMetadata] = []
        var field = _ORBWireField()
        while decoder.next(&field) {
            switch field.number {
            case 1, 3:
                guard field.wire == 2 else {
                    decoder.valid = false
                    break
                }
                elements = Array(field.slice)
                if field.number == 1 {
                    for index in elements.indices {
                        elements[index] &= 0x1f
                    }
                }
            case 2:
                guard _orbReadFloats(field, { floats.append(_ORBCGFloat($0)) }) else {
                    self = .null
                    return
                }
            case 4:
                var metadata = _ORBDecodedMetadata()
                guard _orbDecodeMetadata(field, &metadata) else {
                    self = .null
                    return
                }
                metadataList.append(metadata)
            case 5:
                guard _orbReadDoubles(field, { doubles.append(_ORBCGFloat($0)) }) else {
                    self = .null
                    return
                }
            default:
                break
            }
        }

        var floatIndex = 0
        var doubleIndex = 0
        var metadataIndex = 0
        var depth = 0
        for code in elements {
            let type = UInt32(code & 0x3f)
            guard type <= 24 else {
                self = .null
                return
            }
            if (code & 0x80) != 0 {
                guard metadataIndex < metadataList.count,
                      metadataList[metadataIndex].kind == _orbMetadataKind(type) else {
                    self = .null
                    return
                }
                metadataIndex += 1
            }
            let count = _orbElementArgumentCounts[Int(type)]
            if (code & 0x40) != 0 {
                guard count <= doubles.count - doubleIndex else {
                    self = .null
                    return
                }
                doubleIndex += count
            } else {
                guard count <= floats.count - floatIndex else {
                    self = .null
                    return
                }
                floatIndex += count
            }
            if ((UInt32(1) << type) & 0x1de0000) != 0 {
                depth += 1
            } else if type == 16 {
                guard depth > 0 else {
                    self = .null
                    return
                }
                depth -= 1
            }
        }
        guard depth == 0 else {
            self = .null
            return
        }

        floatIndex = 0
        doubleIndex = 0
        metadataIndex = 0
        var records: [_ORBPathRecord] = []
        records.reserveCapacity(elements.count)
        for code in elements {
            let rawType = Int32(code & 0x3f)
            guard let element = Element(rawValue: rawType) else {
                self = .null
                return
            }
            let count = _orbElementArgumentCounts[Int(rawType)]
            let values: [_ORBCGFloat]
            if (code & 0x40) != 0 {
                values = Array(doubles[doubleIndex..<(doubleIndex + count)])
                doubleIndex += count
            } else {
                values = Array(floats[floatIndex..<(floatIndex + count)])
                floatIndex += count
            }
            var record = _ORBPathRecord(element: element, values: values)
            if (code & 0x80) != 0 {
                let meta = metadataList[metadataIndex]
                metadataIndex += 1
                switch meta.kind {
                case 1:
                    record.transform = meta.affine.isIdentity ? nil : meta.affine
                case 2:
                    record.stroke = meta.stroke
                case 3:
                    record.boolean = meta.boolean
                case 4:
                    record.clipStroke = meta.clip
                default:
                    break
                }
            }
            records.append(record)
        }
        self = Self._fromRecords(records)
    }
}
#endif
