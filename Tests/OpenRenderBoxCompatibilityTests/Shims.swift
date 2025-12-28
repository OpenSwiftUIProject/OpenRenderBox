//
//  Shims.swift
//  OpenRenderBoxShims

#if OPENRENDERBOX_COMPATIBILITY_TEST
@_exported public import RenderBox
public typealias ORBUUID = RBUUID
public typealias ORBPath = RBPath
public let compatibilityTestEnabled = true
#else
@_exported import OpenRenderBox
@_exported import OpenRenderBoxCxx
public let compatibilityTestEnabled = false
#endif
