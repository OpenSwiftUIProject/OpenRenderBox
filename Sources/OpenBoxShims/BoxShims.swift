//
//  BoxShims.swift
//  OpenBoxShims

#if OPENBOX_RENDERBOX
@_exported public import RenderBox
public typealias OBUUID = RBUUID
public let renderBoxEnabled = true
#else
@_exported import OpenBox
public let renderBoxEnabled = false
#endif
