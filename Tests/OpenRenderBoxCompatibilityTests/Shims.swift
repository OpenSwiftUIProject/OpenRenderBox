//
//  Shims.swift
//  OpenRenderBoxShims

#if OPENRENDERBOX_COMPATIBILITY_TEST
@_exported public import RenderBox
public let compatibilityTestEnabled = true
#else
@_exported import OpenRenderBox
@_exported import OpenRenderBoxCxx

public typealias RBColor = ORBColor
public typealias RBUUID = ORBUUID
public typealias RBDisplayList = ORBDisplayList
public typealias RBAnimation = ORBAnimation
public typealias RBSymbolAnimator = ORBSymbolAnimator
public typealias RBSymbolAnimatorObserver = ORBSymbolAnimatorObserver
public typealias RBDevice = ORBDevice
public typealias RBLayer = ORBLayer
public typealias RBLayerDelegate = ORBLayerDelegate
public typealias RBPath = ORBPath
public typealias RBPathApplyCallback = ORBPathApplyCallback

public let compatibilityTestEnabled = false
#endif
