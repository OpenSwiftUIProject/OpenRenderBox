//
//  RBPath.swift
//  OpenRenderBox

#if !OPENRENDERBOX_CF_CGTYPE
public import OpenCoreGraphicsShims

public typealias RBPathApplyCallback = (UnsafeMutableRawPointer, RBPath.Element, UnsafePointer<CGFloat>, UnsafeRawPointer?) -> Bool

public struct RBPath {
    public var storage: RBPath.Storage
    public var callbacks: UnsafePointer<RBPath.Callbacks>
    
    public init(storage: RBPath.Storage, callbacks: UnsafePointer<RBPath.Callbacks>) {
        self.storage = storage
        self.callbacks = callbacks
    }
}

// MARK: - Element

extension RBPath {
    /// Path element type for path enumeration
    public enum Element: Int32, @unchecked Sendable {
        case moveToPoint = 0
        case addLineToPoint = 1
        case addQuadCurveToPoint = 2
        case addCurveToPoint = 3
        case closeSubpath = 4
        case rect = 5
        case roundedRect = 6
        case fixedRoundedRectCircular = 8
        case fixedRoundedRectContinuous = 9
        case invalid = 25
    }
}

// MARK: - RoundedCornerStyle

extension RBPath {
    /// Defines the shape of a rounded rectangle's corners.
    public enum RoundedCornerStyle: Int32, @unchecked Sendable {
        /// Quarter-circle rounded rect corners.
        case circular = 0
        /// Continuous curvature rounded rect corners.
        case continuous = 1
    }
}

// MARK: - Storage

extension RBPath {
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

// MARK: - Static Properties

extension RBPath {
    /// Global empty path (storage = null, callbacks = &RBPathEmptyCallbacks)
    public static var empty: RBPath {
        _openRenderBoxUnimplementedFailure()
    }
    
    /// Global null path (storage = 0x1, callbacks = &RBPathEmptyCallbacks)
    public static var null: RBPath {
        _openRenderBoxUnimplementedFailure()
    }
}

// MARK: - Memory Management

extension RBPath {
    public func retain() {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func release() {
        _openRenderBoxUnimplementedFailure()
    }
}

// MARK: - Initializers

extension RBPath {
    public init(cgPath: CGPath) {
        _openRenderBoxUnimplementedFailure()
    }

    public init(rect: CGRect, transform: UnsafePointer<CGAffineTransform>?) {
        _openRenderBoxUnimplementedFailure()
    }

    public init(ellipseIn rect: CGRect, transform: UnsafePointer<CGAffineTransform>?) {
        _openRenderBoxUnimplementedFailure()
    }

    public init(roundedRect rect: CGRect, cornerWidth: CGFloat, cornerHeight: CGFloat, style: RBPath.RoundedCornerStyle, transform: UnsafePointer<CGAffineTransform>?) {
        _openRenderBoxUnimplementedFailure()
    }

    public init(roundedRect rect: CGRect, topLeftRadius: CGFloat, bottomLeftRadius: CGFloat, bottomRightRadius: CGFloat, topRightRadius: CGFloat, style: RBPath.RoundedCornerStyle, transform: UnsafePointer<CGAffineTransform>?) {
        _openRenderBoxUnimplementedFailure()
    }
}

// MARK: - Properties

extension RBPath {
    public var isEmpty: Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var cgPath: CGPath {
        _openRenderBoxUnimplementedFailure()
    }
}

// MARK: - Methods

extension RBPath {
    public func apply(info: UnsafeMutableRawPointer, callback: RBPathApplyCallback?) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func isEqual(to rhs: RBPath) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func contains(point: CGPoint, eoFill: Bool) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func containsPoints(count: UInt64, points: UnsafePointer<CGPoint>, eoFill: Bool, transform: UnsafePointer<CGAffineTransform>?) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
}

#endif

