//
//  BoxShims.swift
//  OpenBoxShims

#if OPENBOX_RENDERBOX
@_exported public import RenderBox
public typealias OBUUID = RBUUID
public typealias OBPath = RBPath
public let renderBoxEnabled = true
#else
@_exported import OpenBox
public let renderBoxEnabled = false
#endif
