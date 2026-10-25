//
//  ORBSymbolAnimator.swift
//  OpenRenderBox
//

#if !canImport(ObjectiveC)
public import Foundation
public import OpenRenderBoxCxx

// MARK: - ORBSymbolAnimatorObserver

public protocol ORBSymbolAnimatorObserver: AnyObject {
    func symbolAnimatorDidChange(_ animator: ORBSymbolAnimator)
}

// MARK: - ORBSymbolAnimator

public class ORBSymbolAnimator: NSObject {
    // MARK: - Properties

    public var glyph: AnyObject?
    public var renderingMode: ORBSymbolRenderingMode = .preferred
    public var flipsRightToLeft: Bool = false
    public var variableValue: Double = 0
    public var opacities: ORBColor = .clear
    public var isHidden: Bool = false
    public var scaleLevel: ORBSymbolScaleLevel = .normal
    public var anchorPoint: CGPoint = .zero
    public var position: CGPoint = .zero
    public var size: CGSize = .zero
    public var presentationPosition: CGPoint = .zero
    public var currentTime: Double = 0
    public private(set) var maxVelocity: Double = 0
    public private(set) var version: UInt32 = 0
    public private(set) var isAnimating: Bool = false
    public private(set) var alignmentRect: CGRect = .zero
    public private(set) var unroundedAlignmentRect: CGRect = .zero
    public private(set) var styleMask: UInt32 = 0
    public var depth: UInt32 = 0
    public private(set) var boundingRect: CGRect = .zero

    public override init() {
        super.init()
    }

    // MARK: - Animation Management

    public func addAnimation(_ animation: ORBSymbolAnimation, options: Any?) -> UInt32 {
        _openRenderBoxUnimplementedFailure()
    }

    public func removeAllAnimations() {
        _openRenderBoxUnimplementedFailure()
    }

    public func removeAnimation(_ animation: ORBSymbolAnimation) {
        _openRenderBoxUnimplementedFailure()
    }

    public func removeAnimation(withID animationID: UInt32) {
        _openRenderBoxUnimplementedFailure()
    }

    public func cancelAllAnimations() {
        _openRenderBoxUnimplementedFailure()
    }

    public func cancelAnimation(_ animation: ORBSymbolAnimation) {
        _openRenderBoxUnimplementedFailure()
    }

    public func cancelAnimation(withID animationID: UInt32) {
        _openRenderBoxUnimplementedFailure()
    }

    public func setPriority(_ priority: Float, ofAnimationWithID animationID: UInt32) {
        _openRenderBoxUnimplementedFailure()
    }

    // MARK: - Update

    public func beginUpdate(with mode: ORBSymbolRenderingMode) -> ORBSymbolUpdate {
        _openRenderBoxUnimplementedFailure()
    }

    public func beginUpdate(with mode: ORBSymbolRenderingMode, position: UnsafePointer<CGPoint>?, size: UnsafePointer<CGSize>?, flags: ORBSymbolUpdateFlags) -> ORBSymbolUpdate {
        _openRenderBoxUnimplementedFailure()
    }

    public func end(_ update: ORBSymbolUpdate) {}

    public func copyDebugDescription(for update: ORBSymbolUpdate) -> Any? {
        _openRenderBoxUnimplementedFailure()
    }

    // MARK: - Color

    public func color(forStyle style: UInt32) -> ORBColor {
        _openRenderBoxUnimplementedFailure()
    }

    public func setColor(_ color: ORBColor, forStyle style: UInt32) {
        _openRenderBoxUnimplementedFailure()
    }

    // MARK: - Observer

    public func addObserver(_ observer: any ORBSymbolAnimatorObserver) {
        _openRenderBoxUnimplementedFailure()
    }

    public func removeObserver(_ observer: any ORBSymbolAnimatorObserver) {
        _openRenderBoxUnimplementedFailure()
    }
}
#endif
