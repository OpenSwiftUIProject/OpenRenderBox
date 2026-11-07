//
//  ORBPathStorage.swift
//  OpenRenderBox

#if !OPENRENDERBOX_CF_CGTYPES
public import OpenCoreGraphicsShims

extension ORBPath.Storage {
    private var _slot: UnsafeMutablePointer<UnsafeMutableRawPointer?> {
        UnsafeMutableRawPointer(rawValue).assumingMemoryBound(to: UnsafeMutableRawPointer?.self)
    }

    private var _records: _ORBPathRecords {
        Unmanaged<_ORBPathRecords>.fromOpaque(_slot.pointee!).takeUnretainedValue()
    }

    public func initialize(capacity: Int, source: ORBPath.Storage?) {
        precondition(capacity >= MemoryLayout<UnsafeMutableRawPointer?>.stride)
        let records = source?._records.records ?? []
        _slot.initialize(to: Unmanaged.passRetained(_ORBPathRecords(records)).toOpaque())
    }

    public func destroy() {
        if let object = _slot.pointee {
            _slot.pointee = nil
            Unmanaged<_ORBPathRecords>.fromOpaque(object).release()
        }
        _slot.deinitialize(count: 1)
    }

    public func clear() {
        _records.records.removeAll(keepingCapacity: true)
    }

    public func append(element: ORBPath.Element, points: UnsafePointer<CGFloat>, userInfo: UnsafeRawPointer?) {
        guard let record = _ORBPathRecord(element: element, points: points, userInfo: userInfo) else { return }
        _records.records.append(record)
    }

    @discardableResult
    public func append(path: ORBPath) -> Bool {
        guard let records = path._records() else { return false }
        _records.records.append(contentsOf: records)
        return true
    }

    public func apply(info: UnsafeMutableRawPointer, callback: ORBPathApplyCallback?) -> Bool {
        _records.apply(info: info, callback: callback)
    }

    public var isEmpty: Bool { _records.records.isEmpty }

    public func isEqual(to rhs: ORBPath.Storage) -> Bool {
        _records.records == rhs._records.records
    }

    public var isSingleElement: Bool { _orbIsSingleElement(_records.records) }

    public var bezierOrder: Int { _records.records.map(\.order).max() ?? 1 }

    public var boundingRect: CGRect {
        let path = ORBPath._fromRecords(_records.records)
        defer { path.release() }
        return path.boundingRect
    }

    public var cgPath: CGPath? { _orbMakeCGPath(_records.records) }
}

#endif
