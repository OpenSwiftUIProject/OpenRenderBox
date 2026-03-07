//
//  Shims.swift
//  OpenRenderBoxShims

#if OPENRENDERBOX_COMPATIBILITY_TEST
@_exported public import RenderBox
public typealias ORBColor = RBColor
public typealias ORBDevice = RBDevice
public typealias ORBLayer = RBLayer
public typealias ORBPath = RBPath
public typealias ORBUUID = RBUUID
public typealias ORBAnimation = RBAnimation
public typealias ORBSymbolAnimator = RBSymbolAnimator
public typealias ORBSymbolAnimatorObserver = RBSymbolAnimatorObserver
public let compatibilityTestEnabled = true
#else
@_exported import OpenRenderBox
@_exported import OpenRenderBoxCxx
public let compatibilityTestEnabled = false
#endif
