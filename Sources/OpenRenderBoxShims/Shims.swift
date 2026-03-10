//
//  Shims.swift
//  OpenRenderBoxShims

/// A type that identifies the underlying RenderBox implementation vendor.
///
/// Use ``renderBoxVendor`` to check which vendor is active at runtime.
public struct RenderBoxVendor: RawRepresentable, CaseIterable {
    public let rawValue: String

    public init(rawValue: String) {
        self.rawValue = rawValue
    }

    /// OpenRenderBox — the open source implementation by OpenSwiftUIProject.
    public static let orb = RenderBoxVendor(rawValue: "org.OpenSwiftUIProject.OpenRenderBox")

    /// Apple's private RenderBox framework.
    public static let rb = RenderBoxVendor(rawValue: "com.apple.RenderBox")

    public static var allCases: [RenderBoxVendor] { [.orb, .rb] }
}

#if OPENRENDERBOX_RENDERBOX
@_exported public import RenderBox

public let renderBoxVendor = RenderBoxVendor.rb
#else
@_exported import OpenRenderBox

public typealias RBColor = ORBColor
public typealias RBUUID = ORBUUID
public typealias RBDisplayList = ORBDisplayList
public typealias RBAnimation = ORBAnimation
public typealias RBSymbolAnimator = ORBSymbolAnimator
public typealias RBSymbolAnimatorObserver = ORBSymbolAnimatorObserver

#if canImport(ObjectiveC)
public typealias RBDevice = ORBDevice
public typealias RBLayer = ORBLayer
public typealias RBLayerDelegate = ORBLayerDelegate
#endif

#if !OPENRENDERBOX_CF_CGTYPES
public typealias RBPath = ORBPath
public typealias RBPathApplyCallback = ORBPathApplyCallback
#endif

public let renderBoxVendor = RenderBoxVendor.orb
#endif
