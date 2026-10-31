//
//  ORBAnimation.swift
//  OpenRenderBox
//

#if !canImport(ObjectiveC)
public import Foundation

public class ORBAnimation: NSObject, NSCopying {
    private var springTerms: [UInt32] = []

    var isEmpty: Bool { springTerms.isEmpty }

    public var activeDuration: Double {
        isEmpty ? 0 : Double(Float(bitPattern: springTerms[1]))
    }

    public override init() {
        super.init()
    }

    public func removeAll() {
        springTerms.removeAll()
    }

    public func copy(with zone: NSZone? = nil) -> Any {
        let copy = ORBAnimation()
        copy.springTerms = springTerms
        return copy
    }

    public override func isEqual(_ object: Any?) -> Bool {
        guard let other = object as? ORBAnimation else { return false }
        return springTerms == other.springTerms
    }

    public override var hash: Int {
        springTerms.hashValue
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
        springTerms.append(velocity == 0 ? 9 : 10)
        springTerms.append(contentsOf: [duration, mass, stiffness, damping].map { Float($0).bitPattern })
        if velocity != 0 { springTerms.append(Float(velocity).bitPattern) }
    }
}
#endif
