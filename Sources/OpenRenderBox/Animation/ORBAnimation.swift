//
//  ORBAnimation.swift
//  OpenRenderBox
//

#if !canImport(ObjectiveC)
public import Foundation

public class ORBAnimation: NSObject {
    public private(set) var activeDuration: Double = 0

    public override init() {
        super.init()
    }

    public func removeAll() {
        _openRenderBoxUnimplementedFailure()
    }

    public func evaluate(atTime time: Double) -> Float {
        _openRenderBoxUnimplementedFailure()
    }

    public func addBezierDuration(_ duration: Double, controlPoint1 point1: CGPoint, controlPoint2 point2: CGPoint) {
        _openRenderBoxUnimplementedFailure()
    }

    public func addDelay(_ delay: Double) {
        _openRenderBoxUnimplementedFailure()
    }

    public func addPreset(_ preset: UInt32, duration: Double) {
        _openRenderBoxUnimplementedFailure()
    }

    public func addRepeatCount(_ count: Double, autoreverses: Bool) {
        _openRenderBoxUnimplementedFailure()
    }

    public func addSampledFunction(withDuration duration: Double, count: UInt64, values: UnsafePointer<Float>) {
        _openRenderBoxUnimplementedFailure()
    }

    public func addSpeed(_ speed: Double) {
        _openRenderBoxUnimplementedFailure()
    }

    public func addSpringDuration(_ duration: Double, mass: Double, stiffness: Double, damping: Double, initialVelocity velocity: Double) {
        _openRenderBoxUnimplementedFailure()
    }
}
#endif
