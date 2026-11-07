//
//  ORBPath.swift
//  OpenRenderBox

#if !OPENRENDERBOX_CF_CGTYPES
public import OpenCoreGraphicsShims

public typealias ORBPathApplyCallback = (UnsafeMutableRawPointer, ORBPath.Element, UnsafePointer<CGFloat>, UnsafeRawPointer?) -> Bool
public typealias ORBPathApplyShapeCallback = (UnsafeMutableRawPointer, ORBPathShapeType, UnsafePointer<ORBPathShape>) -> Void

public enum ORBPathShapeType: Int32, @unchecked Sendable {
    case rect = 0
    case ellipse = 1
    case line = 2
    case fixedRoundedRect = 3
    case unevenRoundedRect = 4
    case nested = 5
    case stroked = 6
    case trimmed = 7
    case offset = 8
}

public struct ORBPathRectShape: @unchecked Sendable {
    public var transform: CGAffineTransform
    public var rect: CGRect

    public init(transform: CGAffineTransform, rect: CGRect) {
        self.transform = transform
        self.rect = rect
    }
}

public struct ORBPathFixedRoundedRectShape: @unchecked Sendable {
    public var transform: CGAffineTransform
    public var rect: CGRect
    public var cornerSize: CGSize
    public var style: ORBPath.RoundedCornerStyle

    public init(transform: CGAffineTransform, rect: CGRect, cornerSize: CGSize, style: ORBPath.RoundedCornerStyle) {
        self.transform = transform
        self.rect = rect
        self.cornerSize = cornerSize
        self.style = style
    }
}

public struct ORBPathUnevenRoundedRectShape: @unchecked Sendable {
    public var transform: CGAffineTransform
    public var rect: CGRect
    public var topLeftRadius: CGFloat
    public var topRightRadius: CGFloat
    public var bottomRightRadius: CGFloat
    public var bottomLeftRadius: CGFloat
    public var style: ORBPath.RoundedCornerStyle

    public init(transform: CGAffineTransform, rect: CGRect, topLeftRadius: CGFloat, topRightRadius: CGFloat, bottomRightRadius: CGFloat, bottomLeftRadius: CGFloat, style: ORBPath.RoundedCornerStyle) {
        self.transform = transform
        self.rect = rect
        self.topLeftRadius = topLeftRadius
        self.topRightRadius = topRightRadius
        self.bottomRightRadius = bottomRightRadius
        self.bottomLeftRadius = bottomLeftRadius
        self.style = style
    }
}

public struct ORBPathLineShape: @unchecked Sendable {
    public var start: CGPoint
    public var end: CGPoint

    public init(start: CGPoint, end: CGPoint) {
        self.start = start
        self.end = end
    }
}

public struct ORBPathStrokeStyle: @unchecked Sendable {
    public var lineWidth: CGFloat
    public var lineCap: Int32
    public var lineJoin: Int32
    public var miterLimit: CGFloat
    public var dashPhase: CGFloat
    public var dashCount: UInt64
    public var dash: UnsafePointer<CGFloat>?

    public init(
        lineWidth: CGFloat,
        lineCap: Int32,
        lineJoin: Int32,
        miterLimit: CGFloat,
        dashPhase: CGFloat,
        dashCount: UInt64,
        dash: UnsafePointer<CGFloat>?
    ) {
        self.lineWidth = lineWidth
        self.lineCap = lineCap
        self.lineJoin = lineJoin
        self.miterLimit = miterLimit
        self.dashPhase = dashPhase
        self.dashCount = dashCount
        self.dash = dash
    }
}

public struct ORBPathStrokedShape: @unchecked Sendable {
    public var path: ORBPath
    public var style: ORBPathStrokeStyle

    public init(path: ORBPath, style: ORBPathStrokeStyle) {
        self.path = path
        self.style = style
    }
}

public struct ORBPathNestedShape: @unchecked Sendable {
    public var path: ORBPath
    public var transform: CGAffineTransform

    public init(path: ORBPath, transform: CGAffineTransform) {
        self.path = path
        self.transform = transform
    }
}

public struct ORBPathTrimmedShape: @unchecked Sendable {
    public var path: ORBPath
    public var from: CGFloat
    public var to: CGFloat

    public init(path: ORBPath, from: CGFloat, to: CGFloat) {
        self.path = path
        self.from = from
        self.to = to
    }
}

public struct ORBPathShape: @unchecked Sendable {
    public var rect: ORBPathRectShape
    public var ellipse: ORBPathRectShape
    public var line: ORBPathLineShape
    public var fixedRoundedRect: ORBPathFixedRoundedRectShape
    public var unevenRoundedRect: ORBPathUnevenRoundedRectShape
    public var nested: ORBPathNestedShape
    public var trimmed: ORBPathTrimmedShape
    public var stroked: ORBPathStrokedShape

    public init(
        rect: ORBPathRectShape,
        ellipse: ORBPathRectShape,
        line: ORBPathLineShape = .init(start: .zero, end: .zero),
        fixedRoundedRect: ORBPathFixedRoundedRectShape,
        stroked: ORBPathStrokedShape,
        unevenRoundedRect: ORBPathUnevenRoundedRectShape = .init(transform: .identity, rect: .zero, topLeftRadius: 0, topRightRadius: 0, bottomRightRadius: 0, bottomLeftRadius: 0, style: .circular)
    ) {
        self.rect = rect
        self.ellipse = ellipse
        self.line = line
        self.fixedRoundedRect = fixedRoundedRect
        self.nested = .init(path: stroked.path, transform: .identity)
        self.trimmed = .init(path: stroked.path, from: 0, to: 1)
        self.stroked = stroked
        self.unevenRoundedRect = unevenRoundedRect
    }
}

public struct ORBPath {
    public var storage: ORBPath.Storage
    public var callbacks: UnsafePointer<ORBPath.Callbacks>
    
    public init(storage: ORBPath.Storage, callbacks: UnsafePointer<ORBPath.Callbacks>) {
        self.storage = storage
        self.callbacks = callbacks
    }
}

// MARK: - Element

extension ORBPath {
    /// Path element type for path enumeration
    public enum Element: Int32, @unchecked Sendable {
        case moveToPoint = 0
        case addLineToPoint = 1
        case addQuadCurveToPoint = 2
        case addCurveToPoint = 3
        case closeSubpath = 4
        case rect = 5
        case roundedRect = 6
        case ellipse = 7
        case fixedRoundedRectCircular = 8
        case fixedRoundedRectContinuous = 9
        case unevenRoundedRectCircular = 10
        case unevenRoundedRectContinuous = 11
        case relativeArc = 12
        case arc = 13
        case arcCounterclockwise = 14
        case arcTo = 15
        case end = 16
        case transformed = 17
        case trimmed = 18
        case stroked = 19
        case boolean = 20
        case booleanSeparator = 21
        case dilateGlyph = 22
        case offset = 23
        case clipStroke = 24
        case invalid = 25
    }
}

// MARK: - RoundedCornerStyle

extension ORBPath {
    /// Defines the shape of a rounded rectangle's corners.
    public enum RoundedCornerStyle: Int32, @unchecked Sendable {
        /// Quarter-circle rounded rect corners.
        case circular = 0
        /// Continuous curvature rounded rect corners.
        case continuous = 1
    }
}

// MARK: - Storage

extension ORBPath {
    public struct Storage: Hashable, Equatable, RawRepresentable, @unchecked Sendable {
        public var rawValue: OpaquePointer
        
        public init(_ rawValue: OpaquePointer) {
            self.rawValue = rawValue
        }
        
        public init(rawValue: OpaquePointer) {
            self.rawValue = rawValue
        }
    }
}

// MARK: - Portable Storage

struct _ORBPathStrokeInfo: Equatable {
    var lineCap: Int32
    var lineJoin: Int32
    var dash: [CGFloat]
}

struct _ORBPathStrokePayload {
    var lineCap: Int32
    var lineJoin: Int32
    var dashCount: Int
    var dash: UnsafePointer<CGFloat>?
}

struct _ORBPathClipRecord: Equatable {
    var type: Int32
    var x: Float
    var y: Float
    var z: Float
}

struct _ORBPathClipInfo: Equatable {
    var flags: Int32
    var cap: Int32
    var records: [_ORBPathClipRecord]
}

struct _ORBPathClipPayload {
    var flags: Int32
    var cap: Int32
    var count: Int
    var records: UnsafePointer<_ORBPathClipRecord>?
}

struct _ORBPathRecord: Equatable {
    var element: ORBPath.Element
    var values: [CGFloat]
    var transform: CGAffineTransform?
    var stroke: _ORBPathStrokeInfo?
    var boolean: [UInt8]?
    var clipStroke: _ORBPathClipInfo?

    init(element: ORBPath.Element, values: [CGFloat], transform: CGAffineTransform? = nil) {
        self.element = element
        self.values = values
        self.transform = transform?.isIdentity == false ? transform : nil
    }

    init?(element: ORBPath.Element, points: UnsafePointer<CGFloat>, userInfo: UnsafeRawPointer?) {
        let counts = [2, 2, 4, 6, 0, 4, 4, 4, 6, 6, 8, 8, 5, 5, 5, 5, 0, 0, 2, 3, 0, 0, 2, 1, 1]
        let index = Int(element.rawValue)
        guard counts.indices.contains(index) else { return nil }
        self.init(
            element: element,
            values: Array(UnsafeBufferPointer(start: points, count: counts[index])),
            transform: ((0...15).contains(index) && index != 4) || index == 17 || index == 21 ? userInfo?.load(as: CGAffineTransform.self) : nil
        )
        if element == .stroked {
            let payload = userInfo?.load(as: _ORBPathStrokePayload.self)
            stroke = .init(lineCap: payload?.lineCap ?? 0, lineJoin: payload?.lineJoin ?? 0,
                           dash: payload?.dash.map { Array(UnsafeBufferPointer(start: $0, count: max(0, payload!.dashCount))) } ?? [])
        }
        if element == .boolean {
            boolean = userInfo.map { Array(UnsafeRawBufferPointer(start: $0, count: 2)) } ?? [0, 0]
        }
        if element == .clipStroke, let userInfo {
            let payload = userInfo.load(as: _ORBPathClipPayload.self)
            clipStroke = .init(flags: payload.flags, cap: payload.cap,
                records: payload.records.map { Array(UnsafeBufferPointer(start: $0, count: max(0, payload.count))) } ?? [])
        }
    }

    func apply(info: UnsafeMutableRawPointer, callback: ORBPathApplyCallback) -> Bool {
        // The non-null callback pointer is valid even for zero-argument elements.
        var scalars = values.isEmpty ? [CGFloat(0)] : values
        return scalars.withUnsafeMutableBufferPointer { points in
            if let boolean {
                return boolean.withUnsafeBufferPointer { callback(info, element, points.baseAddress!, $0.baseAddress) }
            }
            if let clipStroke {
                return clipStroke.records.withUnsafeBufferPointer { records in
                    var payload = _ORBPathClipPayload(flags: clipStroke.flags, cap: clipStroke.cap, count: records.count, records: records.baseAddress)
                    return withUnsafePointer(to: &payload) { callback(info, element, points.baseAddress!, UnsafeRawPointer($0)) }
                }
            }
            if let stroke {
                let values = stroke.dash.isEmpty ? [CGFloat(0)] : stroke.dash
                return values.withUnsafeBufferPointer { dash in
                    var payload = _ORBPathStrokePayload(lineCap: stroke.lineCap, lineJoin: stroke.lineJoin, dashCount: stroke.dash.count, dash: dash.baseAddress)
                    return withUnsafePointer(to: &payload) {
                        callback(info, element, points.baseAddress!, UnsafeRawPointer($0))
                    }
                }
            }
            if var transform {
                return withUnsafePointer(to: &transform) {
                    callback(info, element, points.baseAddress!, UnsafeRawPointer($0))
                }
            }
            return callback(info, element, points.baseAddress!, nil)
        }
    }

    static func == (lhs: Self, rhs: Self) -> Bool {
        guard lhs.element == rhs.element, lhs.values == rhs.values, lhs.transform == rhs.transform,
              lhs.stroke == rhs.stroke, lhs.boolean == rhs.boolean else { return false }
        let defaultClip = _ORBPathClipInfo(flags: 0, cap: 1, records: [])
        return (lhs.clipStroke ?? defaultClip) == (rhs.clipStroke ?? defaultClip)
    }

    var order: Int {
        switch element {
        case .stroked:
            return stroke?.lineJoin == 1 || stroke?.lineCap == 1 ? 3 : 1
        case .clipStroke:
            return clipStroke?.cap == 1 ? 3 : 1
        case .offset: return 3
        case .addQuadCurveToPoint: return 2
        case .addCurveToPoint, .ellipse, .fixedRoundedRectCircular,
             .fixedRoundedRectContinuous, .unevenRoundedRectCircular,
             .unevenRoundedRectContinuous, .relativeArc, .arc,
             .arcCounterclockwise, .arcTo: return 3
        default: return 1
        }
    }
}

final class _ORBPathRecords {
    var records: [_ORBPathRecord]

    init(_ records: [_ORBPathRecord] = []) {
        self.records = records
    }

    func apply(info: UnsafeMutableRawPointer, callback: ORBPathApplyCallback?) -> Bool {
        guard let callback else { return true }
        for record in records {
            if !record.apply(info: info, callback: callback) { return false }
        }
        return true
    }
}

func _orbCallbackPointer(_ value: ORBPath.Callbacks) -> UnsafePointer<ORBPath.Callbacks> {
    let pointer = UnsafeMutablePointer<ORBPath.Callbacks>.allocate(capacity: 1)
    pointer.initialize(to: value)
    return UnsafePointer(pointer)
}

private let _orbEmptyCallbacks = _orbCallbackPointer(ORBPath.Callbacks())
private let _orbRecordCallbacks: UnsafePointer<ORBPath.Callbacks> = {
    var callbacks = ORBPath.Callbacks()
    callbacks.retain = { object in
        _ = Unmanaged<_ORBPathRecords>.fromOpaque(object).retain()
        return object
    }
    callbacks.release = { Unmanaged<_ORBPathRecords>.fromOpaque($0).release() }
    callbacks.apply = { object, info, callback in
        Unmanaged<_ORBPathRecords>.fromOpaque(object).takeUnretainedValue().apply(info: info, callback: callback)
    }
    callbacks.isEqual = { lhs, rhs in
        Unmanaged<_ORBPathRecords>.fromOpaque(lhs).takeUnretainedValue().records
            == Unmanaged<_ORBPathRecords>.fromOpaque(rhs).takeUnretainedValue().records
    }
    callbacks.isEmpty = { Unmanaged<_ORBPathRecords>.fromOpaque($0).takeUnretainedValue().records.isEmpty }
    callbacks.isSingleElement = { _orbIsSingleElement(Unmanaged<_ORBPathRecords>.fromOpaque($0).takeUnretainedValue().records) }
    callbacks.bezierOrder = { Unmanaged<_ORBPathRecords>.fromOpaque($0).takeUnretainedValue().records.map(\.order).max() ?? 1 }
    return _orbCallbackPointer(callbacks)
}()

let _orbCGPathCallbacks: UnsafePointer<ORBPath.Callbacks> = {
    var callbacks = ORBPath.Callbacks()
    callbacks.retain = { object in
        _ = Unmanaged<CGPath>.fromOpaque(object).retain()
        return object
    }
    callbacks.release = { Unmanaged<CGPath>.fromOpaque($0).release() }
    callbacks.apply = { object, info, callback in
        let path = Unmanaged<CGPath>.fromOpaque(object).takeUnretainedValue()
        return _ORBPathRecords(_orbRecords(path)).apply(info: info, callback: callback)
    }
    callbacks.isEqual = { lhs, rhs in
        Unmanaged<CGPath>.fromOpaque(lhs).takeUnretainedValue() == Unmanaged<CGPath>.fromOpaque(rhs).takeUnretainedValue()
    }
    callbacks.isEmpty = { Unmanaged<CGPath>.fromOpaque($0).takeUnretainedValue().isEmpty }
    callbacks.bezierOrder = { _orbRecords(Unmanaged<CGPath>.fromOpaque($0).takeUnretainedValue()).map(\.order).max() ?? 1 }
    callbacks.cgPath = { Unmanaged<CGPath>.fromOpaque($0) }
    return _orbCallbackPointer(callbacks)
}()

extension ORBPath {
    static func _fromRecords(_ records: [_ORBPathRecord]) -> ORBPath {
        let object = Unmanaged.passRetained(_ORBPathRecords(records)).toOpaque()
        return ORBPath(storage: Storage(OpaquePointer(object)), callbacks: _orbRecordCallbacks)
    }

    var _object: UnsafeRawPointer { UnsafeRawPointer(storage.rawValue) }

    var _extended: UnsafePointer<CallbacksExtended> {
        UnsafeRawPointer(callbacks).assumingMemoryBound(to: CallbacksExtended.self)
    }

    func _records() -> [_ORBPathRecord]? {
        var records: [_ORBPathRecord] = []
        let success = withUnsafeMutablePointer(to: &records) { records in
            apply(info: UnsafeMutableRawPointer(records)) { info, element, points, userInfo in
                guard let record = _ORBPathRecord(element: element, points: points, userInfo: userInfo) else { return false }
                info.assumingMemoryBound(to: [_ORBPathRecord].self).pointee.append(record)
                return true
            }
        }
        return success ? records : nil
    }

    public static var empty: ORBPath {
        ORBPath(storage: Storage(unsafeBitCast(UInt(0), to: OpaquePointer.self)), callbacks: _orbEmptyCallbacks)
    }

    public static var null: ORBPath {
        ORBPath(storage: Storage(OpaquePointer(bitPattern: 1)!), callbacks: _orbEmptyCallbacks)
    }

    @discardableResult
    public func retain() -> ORBPath {
        guard let retain = callbacks.pointee.retain else { return self }
        return ORBPath(storage: Storage(OpaquePointer(retain(_object))), callbacks: callbacks)
    }

    public func release() {
        callbacks.pointee.release?(_object)
    }

    public init(cgPath: CGPath?) {
        guard let cgPath else { self = .null; return }
        self.init(storage: Storage(OpaquePointer(Unmanaged.passRetained(cgPath).toOpaque())), callbacks: _orbCGPathCallbacks)
    }

    public init(rect: CGRect, transform: UnsafePointer<CGAffineTransform>?) {
        self = rect.isNull ? .empty : Self._fromRecords([_ORBPathRecord(element: .rect, values: _orbRectValues(rect), transform: transform?.pointee)])
    }

    public init(ellipseIn rect: CGRect, transform: UnsafePointer<CGAffineTransform>?) {
        self = rect.isNull ? .empty : Self._fromRecords([_ORBPathRecord(element: .ellipse, values: _orbRectValues(rect), transform: transform?.pointee)])
    }

    public init(roundedRect rect: CGRect, cornerWidth: CGFloat, cornerHeight: CGFloat, style: RoundedCornerStyle, transform: UnsafePointer<CGAffineTransform>?) {
        guard !rect.isNull else { self = .empty; return }
        let zero = cornerWidth == 0 && cornerHeight == 0
        self = Self._fromRecords([_ORBPathRecord(
            element: zero ? .roundedRect : (style == .circular ? .fixedRoundedRectCircular : .fixedRoundedRectContinuous),
            values: _orbRectValues(rect) + (zero ? [] : [cornerWidth, cornerHeight]),
            transform: transform?.pointee
        )])
    }

    public init(roundedRect rect: CGRect, topLeftRadius: CGFloat, topRightRadius: CGFloat, bottomRightRadius: CGFloat, bottomLeftRadius: CGFloat, style: RoundedCornerStyle, transform: UnsafePointer<CGAffineTransform>?) {
        guard !rect.isNull else { self = .empty; return }
        let radii = [topLeftRadius, topRightRadius, bottomRightRadius, bottomLeftRadius]
        let zero = radii.allSatisfy { $0 == 0 }
        self = Self._fromRecords([_ORBPathRecord(
            element: zero ? .roundedRect : (style == .circular ? .unevenRoundedRectCircular : .unevenRoundedRectContinuous),
            values: _orbRectValues(rect) + (zero ? [] : radii),
            transform: transform?.pointee
        )])
    }

    public var isNull: Bool {
        storage == Self.null.storage && callbacks == Self.null.callbacks
    }

    public var isEmpty: Bool {
        if callbacks.pointee.flags.isExtended {
            if let isEmpty = _extended.pointee.isEmpty { return isEmpty(_object, _extended) }
        } else if let isEmpty = callbacks.pointee.isEmpty {
            return isEmpty(_object)
        }
        var empty = true
        _ = apply(info: &empty) { info, _, _, _ in
            info.assumingMemoryBound(to: Bool.self).pointee = false
            return false
        }
        return empty
    }

    public var isSingleElement: Bool {
        if callbacks.pointee.flags.isExtended {
            if let isSingleElement = _extended.pointee.isSingleElement { return isSingleElement(_object, _extended) }
        } else if let isSingleElement = callbacks.pointee.isSingleElement {
            return isSingleElement(_object)
        }
        return _records().map(_orbIsSingleElement) ?? false
    }

    public var bezierOrder: Int {
        if callbacks.pointee.flags.isExtended {
            if let order = _extended.pointee.bezierOrder { return order(_object, _extended) }
        } else if let order = callbacks.pointee.bezierOrder {
            return order(_object)
        }
        return _records()?.map(\.order).max() ?? 1
    }

    public var cgPath: CGPath {
        if callbacks.pointee.flags.isExtended {
            if let path = _extended.pointee.cgPath?(_object, _extended) { return path.takeUnretainedValue() }
        } else if let path = callbacks.pointee.cgPath?(_object) {
            return path.takeUnretainedValue()
        }
        guard let records = _records() else { preconditionFailure("Unsupported path element") }
        return _orbMakeCGPath(records)
    }

    public var boundingRect: CGRect {
        if callbacks.pointee.flags.isExtended {
            if let bounds = _extended.pointee.boundingRect { return bounds(_object, _extended) }
        } else if let bounds = callbacks.pointee.boundingRect {
            return bounds(_object)
        }
        guard let records = _records() else { return .null }
        if records.count == 1, let record = records.first, (5...11).contains(record.element.rawValue) {
            return _orbRect(record.values).applying(record.transform ?? .identity)
        }
        if _orbIsSingleElement(records), let record = records.first, record.element == .transformed {
            let child = Self._fromRecords(Array(records.dropFirst().dropLast()))
            defer { child.release() }
            return child.boundingRect.applying(record.transform ?? .identity)
        }
        return _orbBounds(_orbExpandedRecords(records))
    }

    public func apply(info: UnsafeMutableRawPointer, callback: ORBPathApplyCallback?) -> Bool {
        if callbacks.pointee.flags.isExtended {
            return _extended.pointee.apply?(_object, info, callback, _extended) ?? true
        }
        return callbacks.pointee.apply?(_object, info, callback) ?? true
    }

    public func isEqual(to rhs: ORBPath) -> Bool {
        if callbacks != rhs.callbacks {
            if callbacks == _orbEmptyCallbacks { return storage == Self.null.storage ? false : rhs.isEmpty }
            if rhs.callbacks == _orbEmptyCallbacks { return rhs.storage == Self.null.storage ? false : isEmpty }
        }
        if callbacks == rhs.callbacks {
            if storage == rhs.storage { return true }
            if callbacks.pointee.flags.isExtended {
                if let equal = _extended.pointee.isEqual { return equal(_object, rhs._object, _extended) }
            } else if let equal = callbacks.pointee.isEqual {
                return equal(_object, rhs._object)
            }
        }
        guard let lhsRecords = _records(), let rhsRecords = rhs._records() else { return false }
        return lhsRecords == rhsRecords
    }

    public func contains(point: CGPoint, eoFill: Bool) -> Bool {
        var point = point
        return containsPoints(count: 1, points: &point, eoFill: eoFill, transform: nil) & 1 != 0
    }

    public func containsPoints(count: UInt64, points: UnsafePointer<CGPoint>, eoFill: Bool, transform: UnsafePointer<CGAffineTransform>?) -> UInt64 {
        guard count > 0, let records = _records() else { return 0 }
        let queries = (0..<Int(min(count, 64))).map { points[$0].applying(transform?.pointee ?? .identity) }
        return _orbContains(_orbExpandedRecords(records), queries: queries, bounds: boundingRect, eoFill: eoFill)
    }

    public func applyShape(allowLine: Bool, info: UnsafeMutableRawPointer, callback: ORBPathApplyShapeCallback) -> Bool {
        guard let records = _records() else { return false }
        let zeroRect = ORBPathRectShape(transform: .identity, rect: .zero)
        var shape = ORBPathShape(
            rect: zeroRect, ellipse: zeroRect,
            fixedRoundedRect: .init(transform: .identity, rect: .zero, cornerSize: .zero, style: .circular),
            stroked: .init(path: self, style: .init(lineWidth: 0, lineCap: 0, lineJoin: 0, miterLimit: 0, dashPhase: 0, dashCount: 0, dash: nil))
        )
        let type: ORBPathShapeType
        var ownedChild: ORBPath?
        var strokeDash: [CGFloat]?
        defer { ownedChild?.release() }
        if let record = records.first, records.last?.element == .end,
           [.transformed, .trimmed, .stroked].contains(record.element), _orbIsSingleElement(records) {
            let child: ORBPath
            if callbacks == _orbWrapperCallbacks {
                child = Unmanaged<_ORBPathWrapper>.fromOpaque(_object).takeUnretainedValue().child
            } else {
                child = Self._fromRecords(Array(records.dropFirst().dropLast()))
                ownedChild = child
            }
            switch record.element {
            case .transformed:
                type = .nested
                shape.nested = .init(path: child, transform: record.transform ?? .identity)
            case .trimmed:
                type = .trimmed
                shape.trimmed = .init(path: child, from: record.values[0], to: record.values[1])
            default:
                type = .stroked
                strokeDash = record.stroke?.dash ?? []
                shape.stroked = .init(path: child, style: .init(lineWidth: record.values[0], lineCap: record.stroke?.lineCap ?? 0,
                    lineJoin: record.stroke?.lineJoin ?? 0, miterLimit: record.values[1], dashPhase: record.values[2],
                    dashCount: UInt64(strokeDash!.count), dash: nil))
            }
        } else if records.count == 1, let record = records.first {
            switch record.element {
            case .rect, .roundedRect, .ellipse, .fixedRoundedRectCircular, .fixedRoundedRectContinuous, .unevenRoundedRectCircular, .unevenRoundedRectContinuous:
                let rect = _orbRect(record.values)
                let transform = record.transform ?? .identity
                shape.rect = .init(transform: transform, rect: rect)
                shape.ellipse = shape.rect
                switch record.element {
                case .rect, .roundedRect: type = .rect
                case .ellipse: type = .ellipse
                case .fixedRoundedRectCircular, .fixedRoundedRectContinuous:
                    type = .fixedRoundedRect
                    shape.fixedRoundedRect = .init(transform: transform, rect: rect, cornerSize: CGSize(width: record.values[4], height: record.values[5]), style: record.element == .fixedRoundedRectContinuous ? .continuous : .circular)
                default:
                    type = .unevenRoundedRect
                    shape.unevenRoundedRect = .init(transform: transform, rect: rect, topLeftRadius: record.values[4], topRightRadius: record.values[5], bottomRightRadius: record.values[6], bottomLeftRadius: record.values[7], style: record.element == .unevenRoundedRectContinuous ? .continuous : .circular)
                }
            default: return false
            }
        } else if !allowLine, records.count == 2, records[0].element == .moveToPoint, records[1].element == .addLineToPoint {
            type = .line
            shape.line = .init(start: _orbPoint(records[0].values, 0), end: _orbPoint(records[1].values, 0))
        } else if let rect = _orbRecognizeRect(records) {
            type = .rect
            shape.rect = .init(transform: .identity, rect: rect)
        } else {
            return false
        }
        if let strokeDash {
            let values = strokeDash.isEmpty ? [CGFloat(0)] : strokeDash
            values.withUnsafeBufferPointer { dash in
                shape.stroked.style.dash = dash.baseAddress
                withUnsafePointer(to: &shape) { callback(info, type, $0) }
            }
        } else {
            withUnsafePointer(to: &shape) { callback(info, type, $0) }
        }
        return true
    }
}

func _orbIsSingleElement(_ records: [_ORBPathRecord]) -> Bool {
    var depth = 0
    var count = 0
    for record in records {
        switch record.element {
        case .end:
            if depth == 0 { return false }
            depth -= 1
        case .transformed, .trimmed, .stroked, .boolean, .dilateGlyph, .offset, .clipStroke:
            if depth == 0 { count += 1 }
            depth += 1
        default:
            if depth == 0 { count += 1 }
        }
        if count > 1 { return false }
    }
    return count == 1 && depth == 0
}

private final class _ORBPathWrapper {
    let child: ORBPath
    let record: _ORBPathRecord

    init(child: ORBPath, record: _ORBPathRecord) {
        self.child = child.retain()
        self.record = record
    }

    deinit { child.release() }
}

private let _orbWrapperCallbacks: UnsafePointer<ORBPath.Callbacks> = {
    var callbacks = ORBPath.Callbacks()
    callbacks.retain = { object in
        _ = Unmanaged<_ORBPathWrapper>.fromOpaque(object).retain()
        return object
    }
    callbacks.release = { Unmanaged<_ORBPathWrapper>.fromOpaque($0).release() }
    callbacks.apply = { object, info, callback in
        guard let callback else { return true }
        let wrapper = Unmanaged<_ORBPathWrapper>.fromOpaque(object).takeUnretainedValue()
        guard wrapper.record.apply(info: info, callback: callback),
              wrapper.child.apply(info: info, callback: callback) else { return false }
        return _ORBPathRecord(element: .end, values: []).apply(info: info, callback: callback)
    }
    callbacks.isEqual = { lhs, rhs in
        let a = Unmanaged<_ORBPathWrapper>.fromOpaque(lhs).takeUnretainedValue()
        let b = Unmanaged<_ORBPathWrapper>.fromOpaque(rhs).takeUnretainedValue()
        return a.record == b.record && a.child.isEqual(to: b.child)
    }
    callbacks.isEmpty = { _ in false }
    callbacks.isSingleElement = { _ in true }
    callbacks.next = { Unmanaged<_ORBPathWrapper>.fromOpaque($0).takeUnretainedValue().child }
    return _orbCallbackPointer(callbacks)
}()

extension ORBPath {
    private func _wrapped(_ record: _ORBPathRecord) -> ORBPath {
        let object = Unmanaged.passRetained(_ORBPathWrapper(child: self, record: record)).toOpaque()
        return ORBPath(storage: Storage(OpaquePointer(object)), callbacks: _orbWrapperCallbacks)
    }

    public func copy(transform: UnsafePointer<CGAffineTransform>?) -> ORBPath {
        if callbacks == _orbEmptyCallbacks { return self }
        return _wrapped(.init(element: .transformed, values: [], transform: transform?.pointee))
    }

    public func trimmed(from: CGFloat, to: CGFloat) -> ORBPath {
        if callbacks == _orbEmptyCallbacks { return .empty }
        return _wrapped(.init(element: .trimmed, values: [from, to]))
    }

    public func stroked(lineWidth: CGFloat, lineCap: Int32, lineJoin: Int32, miterLimit: CGFloat, dashPhase: CGFloat, dash: UnsafePointer<CGFloat>?, dashCount: Int) -> ORBPath {
        if callbacks == _orbEmptyCallbacks { return .empty }
        var record = _ORBPathRecord(element: .stroked, values: [lineWidth, miterLimit, dashPhase])
        record.stroke = .init(lineCap: lineCap, lineJoin: lineJoin, dash: dash.map { Array(UnsafeBufferPointer(start: $0, count: max(0, dashCount))) } ?? [])
        return _wrapped(record)
    }
}


#endif
