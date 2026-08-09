//
//  Export.swift
//  OpenRenderBox

@_exported public import OpenRenderBoxCxx

#if !canImport(ObjectiveC)
extension ORBColor {
    public typealias ColorSpace = ORBColorSpace
}
#endif
