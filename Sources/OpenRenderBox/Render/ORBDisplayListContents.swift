//
//  ORBDisplayListContents.swift
//  OpenRenderBox
//

#if !canImport(ObjectiveC)
public import Foundation
public import OpenCoreGraphicsShims

public struct ORBDisplayListRenderKey: RawRepresentable, Hashable, Sendable {
    public var rawValue: String

    public init(rawValue: String) {
        self.rawValue = rawValue
    }

    public static let rasterizationScale = ORBDisplayListRenderKey(rawValue: "rasterizationscale")
}

public protocol ORBDisplayListContents: AnyObject {
    var empty: Bool { get }
    var boundingRect: CGRect { get }
    var xmlDescription: String { get }
    func isEmpty() -> Bool
    func render(in context: CGContext, options: Any?)
}
#endif
