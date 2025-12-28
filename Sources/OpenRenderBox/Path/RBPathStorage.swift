//
//  RBPathStorage.swift
//  OpenRenderBox

#if !OPENRENDERBOX_CF_CGTYPE
public import OpenCoreGraphicsShims

extension RBPath.Storage {
    public func initialize(capacity: UInt32, source: RBPath.Storage?) {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func destroy() {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func clear() {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func append(element: RBPath.Element, points: UnsafePointer<CGFloat>, userInfo: UnsafeRawPointer?) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func append(path: RBPath) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func apply(info: UnsafeMutableRawPointer, callback: RBPathApplyCallback?) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var isEmpty: Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func isEqual(to rhs: RBPath.Storage) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var isSingleElement: Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var bezierOrder: UInt32 {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var boundingRect: CGRect {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var cgPath: CGPath? {
        _openRenderBoxUnimplementedFailure()
    }
}

#endif

