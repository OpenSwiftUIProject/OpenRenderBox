//
//  Shims.swift
//  OpenRenderBoxShims

#if OPENRENDERBOX_RENDERBOX
@_exported public import RenderBox
public typealias ORBUUID = RBUUID
public typealias ORBPath = RBPath
public let renderBoxEnabled = true
#else
@_exported import OpenRenderBox
public let renderBoxEnabled = false
#endif
