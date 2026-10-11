//
//  ORBDisplayListTransform.swift
//  OpenRenderBox

#if !canImport(ObjectiveC)
public import Foundation

public class ORBDisplayListTransform: NSObject {
    public func removeAll() {
        _openRenderBoxUnimplementedFailure()
    }

    public func addColorReplacement(from fromColor: ORBColor, to toColor: ORBColor, colorSpace: ORBColor.ColorSpace) {
        _openRenderBoxUnimplementedFailure()
    }

    public func copyApplying(toDisplayList list: any ORBDisplayListContents) -> any ORBDisplayListContents {
        _openRenderBoxUnimplementedFailure()
    }
}
#endif
