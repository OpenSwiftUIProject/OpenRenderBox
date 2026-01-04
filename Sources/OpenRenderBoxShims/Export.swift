//
//  Export.swift
//  OpenRenderBoxShims

#if OPENRENDERBOX_RENDERBOX
@_exported public import RenderBox
public typealias ORBColor = RBColor
public typealias ORBColorMode = RBColorMode
public typealias ORBColorSpace = RBColorSpace
public typealias ORBDevice = RBDevice
public typealias ORBLayer = RBLayer
public typealias ORBPath = RBPath
public typealias ORBUUID = RBUUID
public let renderBoxEnabled = true
#else
@_exported import OpenRenderBox
public let renderBoxEnabled = false
#endif
