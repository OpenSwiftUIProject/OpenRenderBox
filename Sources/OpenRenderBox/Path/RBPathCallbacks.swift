//
//  RBPathCallbacks.swift
//  OpenRenderBox

#if !OPENRENDERBOX_CF_CGTYPE
public import OpenCoreGraphicsShims

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
        public var retain: ((UnsafeRawPointer) -> UnsafeRawPointer)?
        public var release: ((UnsafeRawPointer) -> Void)?
        public var apply: ((UnsafeRawPointer, UnsafeMutableRawPointer, RBPathApplyCallback?) -> Bool)?
        public var isEqual: ((UnsafeRawPointer, UnsafeRawPointer) -> Bool)?
        public var isEmpty: ((UnsafeRawPointer) -> Bool)?
        public var isSingleElement: ((UnsafeRawPointer) -> Bool)?
        public var bezierOrder: ((UnsafeRawPointer) -> UInt32)?
        public var boundingRect: ((UnsafeRawPointer) -> CGRect)?
        public var cgPath: ((UnsafeRawPointer) -> Unmanaged<CGPath>?)?
        public var next: ((UnsafeRawPointer) -> UnsafePointer<RBPath.Callbacks>?)?
        
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
            retain: ((UnsafeRawPointer) -> UnsafeRawPointer)?,
            release: ((UnsafeRawPointer) -> Void)?,
            apply: ((UnsafeRawPointer, UnsafeMutableRawPointer, RBPathApplyCallback?) -> Bool)?,
            isEqual: ((UnsafeRawPointer, UnsafeRawPointer) -> Bool)?,
            isEmpty: ((UnsafeRawPointer) -> Bool)?,
            isSingleElement: ((UnsafeRawPointer) -> Bool)?,
            bezierOrder: ((UnsafeRawPointer) -> UInt32)?,
            boundingRect: ((UnsafeRawPointer) -> CGRect)?,
            cgPath: ((UnsafeRawPointer) -> Unmanaged<CGPath>?)?,
            next: ((UnsafeRawPointer) -> UnsafePointer<RBPath.Callbacks>?)?
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
        public var retain: ((UnsafeRawPointer) -> UnsafeRawPointer)?
        public var release: ((UnsafeRawPointer) -> Void)?
        public var apply: ((UnsafeRawPointer, UnsafeMutableRawPointer, RBPathApplyCallback?, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?
        public var isEqual: ((UnsafeRawPointer, UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?
        public var isEmpty: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?
        public var isSingleElement: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?
        public var bezierOrder: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> UInt32)?
        public var boundingRect: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> CGRect)?
        public var cgPath: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Unmanaged<CGPath>?)?
        public var next: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> UnsafePointer<RBPath.CallbacksExtended>?)?
        
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
            retain: ((UnsafeRawPointer) -> UnsafeRawPointer)?,
            release: ((UnsafeRawPointer) -> Void)?,
            apply: ((UnsafeRawPointer, UnsafeMutableRawPointer, RBPathApplyCallback?, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?,
            isEqual: ((UnsafeRawPointer, UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?,
            isEmpty: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?,
            isSingleElement: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Bool)?,
            bezierOrder: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> UInt32)?,
            boundingRect: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> CGRect)?,
            cgPath: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> Unmanaged<CGPath>?)?,
            next: ((UnsafeRawPointer, UnsafePointer<RBPath.CallbacksExtended>) -> UnsafePointer<RBPath.CallbacksExtended>?)?
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

