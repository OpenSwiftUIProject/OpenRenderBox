//
//  BoxShims.swift
//  OpenBoxShims

#if OPENBOX_COMPATIBILITY_TEST
@_exported public import RenderBox
public typealias OBUUID = RBUUID
public let compatibilityTestEnabled = true
#else
@_exported import OpenBox
public let compatibilityTestEnabled = false
#endif
