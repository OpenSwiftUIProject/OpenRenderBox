//
//  Export.swift
//  OpenRenderBoxShims

#if OPENRENDERBOX_RENDERBOX
@_exported public import RenderBox
public typealias ORBColor = RBColor
public typealias ORBDevice = RBDevice
public typealias ORBLayer = RBLayer
public typealias ORBLayerDelegate = RBLayerDelegate
public typealias ORBDisplayList = RBDisplayList
public typealias ORBPath = RBPath
public typealias ORBUUID = RBUUID
public typealias ORBAnimation = RBAnimation
public typealias ORBSymbolAnimator = RBSymbolAnimator
public typealias ORBSymbolAnimatorObserver = RBSymbolAnimatorObserver
public let renderBoxEnabled = true
#else
@_exported import OpenRenderBox
public let renderBoxEnabled = false
#endif
