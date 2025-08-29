//
//  BoxShims.swift
//  OpenRenderBoxShims

#if OPENRENDERBOX_COMPATIBILITY_TEST
@_exported public import RenderBox
public typealias ORBUUID = RBUUID
public let compatibilityTestEnabled = true
#else
@_exported import OpenRenderBox
public let compatibilityTestEnabled = false
#endif
