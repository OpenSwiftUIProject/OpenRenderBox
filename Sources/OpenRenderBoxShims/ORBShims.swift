//
//  ORBShims.swift
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

public typealias ORBAnimation = RBAnimation
public typealias ORBColor = RBColor
public typealias ORBDevice = RBDevice
public typealias ORBDisplayList = RBDisplayList
public typealias ORBDisplayListContents = RBDisplayListContents
public typealias ORBDisplayListInterpolator = RBDisplayListInterpolator
public typealias ORBLayer = RBLayer
public typealias ORBLayerDelegate = RBLayerDelegate
public typealias ORBPath = RBPath
public typealias ORBPathApplyCallback = RBPathApplyCallback
public typealias ORBUUID = RBUUID
public typealias ORBSymbolAnimator = RBSymbolAnimator
public typealias ORBSymbolAnimatorObserver = RBSymbolAnimatorObserver

public let renderBoxVendor = RenderBoxVendor.rb
#else
@_exported import OpenRenderBox

public let renderBoxVendor = RenderBoxVendor.orb
#endif
