//
//  RBPathCallbacks.swift
//  OpenRenderBox

#if !OPENRENDERBOX_CF_CGTYPE
import OpenCoreGraphics

/// Flags for path callbacks
public struct RBPathCallbacksFlags {
    public var unknown0: UInt8
    public var unknown1: UInt8
    public var isExtended: Bool
    public var padding: (UInt8, UInt8, UInt8, UInt8, UInt8)
    
    public init() {
        self.unknown0 = 0
        self.unknown1 = 0
        self.isExtended = false
        self.padding = (0, 0, 0, 0, 0)
    }
    
    public init(unknown0: UInt8, unknown1: UInt8, isExtended: Bool, padding: (UInt8, UInt8, UInt8, UInt8, UInt8)) {
        self.unknown0 = unknown0
        self.unknown1 = unknown1
        self.isExtended = isExtended
        self.padding = padding
    }
}

// MARK: - RBPath.Callbacks

extension RBPath {
    /// Callbacks structure for path operations
    /// This allows different path storage types (CGPath, custom storage, etc.) to provide their own implementations
    public struct Callbacks {
        public var flags: RBPathCallbacksFlags
        public var retain: (@convention(c) (UnsafeRawPointer) -> UnsafeRawPointer)?
        public var release: (@convention(c) (UnsafeRawPointer) -> Void)?
        public var apply: (@convention(c) (UnsafeRawPointer, UnsafeMutableRawPointer, RBPathApplyCallback?) -> Bool)?
        public var isEqual: (@convention(c) (UnsafeRawPointer, UnsafeRawPointer) -> Bool)?
        public var isEmpty: (@convention(c) (UnsafeRawPointer) -> Bool)?
        public var isSingleElement: (@convention(c) (UnsafeRawPointer) -> Bool)?
        public var bezierOrder: (@convention(c) (UnsafeRawPointer) -> UInt32)?
        public var boundingRect: (@convention(c) (UnsafeRawPointer) -> CGRect)?
        public var cgPath: (@convention(c) (UnsafeRawPointer) -> Unmanaged<CGPath>?)?
        public var next: (@convention(c) (UnsafeRawPointer) -> UnsafePointer<RBPath.Callbacks>?)?
        
        public init() {
            self.flags = RBPathCallbacksFlags()
            self.retain = nil
            self.release = nil
            self.apply = nil
            self.isEqual = nil
            self.isEmpty = nil
            self.isSingleElement = nil
            self.bezierOrder = nil
            self.boundingRect = nil
            self.cgPath = nil
            self.next = nil
        }
        
        public init(
            flags: RBPathCallbacksFlags,
            retain: (@convention(c) (UnsafeRawPointer) -> UnsafeRawPointer)?,
            release: (@convention(c) (UnsafeRawPointer) -> Void)?,
            apply: (@convention(c) (UnsafeRawPointer, UnsafeMutableRawPointer, RBPathApplyCallback?) -> Bool)?,
            isEqual: (@convention(c) (UnsafeRawPointer, UnsafeRawPointer) -> Bool)?,
            isEmpty: (@convention(c) (UnsafeRawPointer) -> Bool)?,
            isSingleElement: (@convention(c) (UnsafeRawPointer) -> Bool)?,
            bezierOrder: (@convention(c) (UnsafeRawPointer) -> UInt32)?,
            boundingRect: (@convention(c) (UnsafeRawPointer) -> CGRect)?,
            cgPath: (@convention(c) (UnsafeRawPointer) -> Unmanaged<CGPath>?)?,
            next: (@convention(c) (UnsafeRawPointer) -> UnsafePointer<RBPath.Callbacks>?)?
        ) {
            self.flags = flags
            self.retain = retain
            self.release = release
            self.apply = apply
            self.isEqual = isEqual
            self.isEmpty = isEmpty
            self.isSingleElement = isSingleElement
            self.bezierOrder = bezierOrder
            self.boundingRect = boundingRect
            self.cgPath = cgPath
            self.next = next
        }
    }
}

// MARK: - RBPath.Callbacks Static Properties

extension RBPath.Callbacks {
    /// Global callbacks for CGPath-backed paths
    public static var cgPath: RBPath.Callbacks {
        _openRenderBoxUnimplementedFailure()
    }
}

// MARK: - RBPath.CallbacksExtended

extension RBPath {
    /// Extended callbacks structure with additional extended callbacks argument
    public struct CallbacksExtended {
        public var flags: RBPathCallbacksFlags
        public var retain: (@convention(c) (UnsafeRawPointer) -> UnsafeRawPointer)?
        public var release: (@convention(c) (UnsafeRawPointer) -> Void)?
        public var apply: (@convention(c) (UnsafeRawPointer, UnsafeMutableRawPointer, RBPathApplyCallback?, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?
        public var isEqual: (@convention(c) (UnsafeRawPointer, UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?
        public var isEmpty: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?
        public var isSingleElement: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?
        public var bezierOrder: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> UInt32)?
        public var boundingRect: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> CGRect)?
        public var cgPath: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Unmanaged<CGPath>?)?
        public var next: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> UnsafePointer<RBPath.CallbacksExtended>?)?
        
        public init() {
            self.flags = RBPathCallbacksFlags()
            self.retain = nil
            self.release = nil
            self.apply = nil
            self.isEqual = nil
            self.isEmpty = nil
            self.isSingleElement = nil
            self.bezierOrder = nil
            self.boundingRect = nil
            self.cgPath = nil
            self.next = nil
        }

        public init(
            flags: RBPathCallbacksFlags,
            retain: (@convention(c) (UnsafeRawPointer) -> UnsafeRawPointer)?,
            release: (@convention(c) (UnsafeRawPointer) -> Void)?,
            apply: (@convention(c) (UnsafeRawPointer, UnsafeMutableRawPointer, RBPathApplyCallback?, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?,
            isEqual: (@convention(c) (UnsafeRawPointer, UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?,
            isEmpty: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?,
            isSingleElement: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?,
            bezierOrder: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> UInt32)?,
            boundingRect: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> CGRect)?,
            cgPath: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Unmanaged<CGPath>?)?,
            next: (@convention(c) (UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> UnsafePointer<RBPath.CallbacksExtended>?)?
        ) {
            self.flags = flags
            self.retain = retain
            self.release = release
            self.apply = apply
            self.isEqual = isEqual
            self.isEmpty = isEmpty
            self.isSingleElement = isSingleElement
            self.bezierOrder = bezierOrder
            self.boundingRect = boundingRect
            self.cgPath = cgPath
            self.next = next
        }
    }
}

#endif

