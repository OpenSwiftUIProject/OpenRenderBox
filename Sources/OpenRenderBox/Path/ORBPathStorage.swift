//
//  ORBPathStorage.swift
//  OpenRenderBox

#if !OPENRENDERBOX_CF_CGTYPES
public import OpenCoreGraphicsShims

extension ORBPath.Storage {
    public func initialize(capacity: Int, source: ORBPath.Storage?) {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func destroy() {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func clear() {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func append(element: ORBPath.Element, points: UnsafePointer<CGFloat>, userInfo: UnsafeRawPointer?) {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func append(path: ORBPath) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func apply(info: UnsafeMutableRawPointer, callback: ORBPathApplyCallback?) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var isEmpty: Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public func isEqual(to rhs: ORBPath.Storage) -> Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var isSingleElement: Bool {
        _openRenderBoxUnimplementedFailure()
    }
    
    public var bezierOrder: Int {
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

