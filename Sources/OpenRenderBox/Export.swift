//
//  Export.swift
//  OpenRenderBox

#if SWIFT_PACKAGE
#if OPENRENDERBOX_COREGRAPHICS
@_exported public import OpenRenderBoxCxx
#if canImport(ObjectiveC)
@_exported public import OpenRenderBoxCxx.ObjC
#endif
#else
@_exported public import OpenRenderBoxCxx.C
#endif
#endif
