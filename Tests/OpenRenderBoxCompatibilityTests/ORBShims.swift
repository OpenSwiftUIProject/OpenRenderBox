//
//  ORBShims.swift
//  OpenRenderBoxShims

#if OPENRENDERBOX_COMPATIBILITY_TEST
@_exported public import RenderBox

public typealias ORBAnimation = RBAnimation
public typealias ORBColor = RBColor
public typealias ORBDevice = RBDevice
public typealias ORBDisplayList = RBDisplayList
public typealias ORBDisplayListContents = RBDisplayListContents
public typealias ORBDisplayListInterpolator = RBDisplayListInterpolator
public typealias ORBLayer = RBLayer
public typealias ORBLayerDelegate = RBLayerDelegate
public typealias ORBPath = RBPath
public typealias ORBPathApplyCallback = RBPathApplyCallback
public typealias ORBUUID = RBUUID
public typealias ORBSymbolAnimator = RBSymbolAnimator
public typealias ORBSymbolAnimatorObserver = RBSymbolAnimatorObserver

public let compatibilityTestEnabled = true
#else
@_exported import OpenRenderBox
@_exported import OpenRenderBoxCxx

public let compatibilityTestEnabled = false
#endif
