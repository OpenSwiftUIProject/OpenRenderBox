//
//  ORBDisplayListPredicate.swift
//  OpenRenderBox

#if !canImport(ObjectiveC)
public import Foundation

public class ORBDisplayListPredicate: NSObject {
    public var invertsResult: Bool = false

    public func removeAll() {
        _openRenderBoxUnimplementedFailure()
    }

    public func addCondition(withFill color: ORBColor, colorSpace: ORBColor.ColorSpace) {
        _openRenderBoxUnimplementedFailure()
    }

    public func copyFilteredDisplayList(_ list: any ORBDisplayListContents) -> any ORBDisplayListContents {
        _openRenderBoxUnimplementedFailure()
    }
}
#endif
