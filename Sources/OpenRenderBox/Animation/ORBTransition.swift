//
//  ORBTransition.swift
//  OpenRenderBox

#if !canImport(ObjectiveC)
public import Foundation
public import OpenRenderBoxCxx

private func transitionTime(_ value: Float) -> UInt8 {
    guard !value.isNaN else { return 0 }
    return UInt8(Float(0.5).addingProduct(min(max(value, 0), 1), 255))
}

public class ORBTransitionEffect: NSObject {
    private var bits: UInt16 = 0x301
    private var begin: UInt8 = 0
    private var length: UInt8 = 255
    private var arguments: [UInt32] = [0, 0]

    public var type: ORBTransitionEffectType {
        get { .init(rawValue: UInt32(bits & 127))! }
        set { bits = (bits & ~127) | UInt16(newValue.rawValue & 127) }
    }
    public var flags: ORBTransitionEffectFlags {
        get { .init(rawValue: UInt32(bits >> 14)) }
        set { bits = (bits & 0x3fff) | UInt16((newValue.rawValue & 3) << 14) }
    }
    public var events: ORBTransitionEvents {
        get { .init(rawValue: UInt32((bits >> 8) & 63)) }
        set { bits = (bits & 0xc0ff) | UInt16((newValue.rawValue & 63) << 8) }
    }
    public var beginTime: Float {
        get { bits & 128 != 0 ? 0 : Float(begin) * (1 / 255) }
        set {
            if bits & 128 != 0 {
                bits &= ~128
                length = 255
            }
            begin = transitionTime(newValue)
        }
    }
    public var duration: Float {
        get { bits & 128 != 0 ? 1 : Float(length) * (1 / 255) }
        set {
            if bits & 128 != 0 {
                bits &= ~128
                begin = 0
            }
            length = transitionTime(newValue)
        }
    }
    public var animationIndex: UInt {
        get { bits & 128 != 0 && begin == length ? UInt(begin) : 0 }
        set {
            insertAnimationIndex = newValue
            removeAnimationIndex = newValue
        }
    }
    public var insertAnimationIndex: UInt {
        get { bits & 128 != 0 ? UInt(begin) : 0 }
        set {
            if bits & 128 == 0 {
                bits |= 128
                length = 0
            }
            begin = UInt8(truncatingIfNeeded: newValue)
        }
    }
    public var removeAnimationIndex: UInt {
        get { bits & 128 != 0 ? UInt(length) : 0 }
        set {
            if bits & 128 == 0 {
                bits |= 128
                begin = 0
            }
            length = UInt8(truncatingIfNeeded: newValue)
        }
    }

    public func argumentValue(at index: UInt) -> Float {
        Float(bitPattern: integerArgumentValue(at: index))
    }
    public func setArgumentValue(_ value: Float, at index: UInt) {
        setIntegerArgumentValue(value.bitPattern, at: index)
    }
    public func integerArgumentValue(at index: UInt) -> UInt32 {
        index < 2 ? arguments[Int(index)] : 0
    }
    public func setIntegerArgumentValue(_ value: UInt32, at index: UInt) {
        if index < 2 { arguments[Int(index)] = value }
    }

    fileprivate func snapshot() -> ORBTransitionEffect {
        let copy = ORBTransitionEffect()
        copy.bits = bits
        copy.begin = begin
        copy.length = length
        let counts = [0, 0, 1, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0]
        let type = Int(type.rawValue)
        let count = type < counts.count ? counts[type] : 2
        for index in 0..<count { copy.arguments[index] = arguments[index] }
        return copy
    }

    public override func isEqual(_ object: Any?) -> Bool {
        guard let other = object as? ORBTransitionEffect else { return false }
        return bits == other.bits && begin == other.begin && length == other.length && arguments == other.arguments
    }
    public override var hash: Int {
        var hash = UInt(bits & 0x3fff) | UInt((bits >> 2) & 0x3000) | (UInt(begin) << 16) | (UInt(length) << 24)
        hash = hash &* 16_777_619 ^ UInt(arguments[0])
        hash = hash &* 16_777_619 ^ UInt(arguments[1])
        return Int(bitPattern: hash)
    }
}

public class ORBTransition: NSObject, NSCopying {
    public static let `default`: ORBTransition = {
        let transition = ORBTransition()
        transition.isDefault = true
        return transition
    }()

    private var isDefault = false
    private var storedMethod: ORBTransitionMethod = .empty
    private var storedAnimation: ORBAnimation?
    private var storedEffects: [ORBTransitionEffect] = []
    private var storedDuration: UInt8 = 32

    public var method: ORBTransitionMethod {
        get { storedMethod }
        set {
            let method: ORBTransitionMethod = newValue.rawValue <= 8 ? newValue : .empty
            if method != storedMethod {
                precondition(!isDefault, "The default transition cannot be modified.")
                storedMethod = method
            }
        }
    }
    public var animation: ORBAnimation? {
        get { storedAnimation?.isEmpty == false ? storedAnimation?.copy() as? ORBAnimation : nil }
        set {
            precondition(!isDefault, "The default transition cannot be modified.")
            storedAnimation = newValue?.copy() as? ORBAnimation
        }
    }
    public var maxChanges: UInt32 = .max
    public var isReplaceable: Bool = false
    public var effects: [ORBTransitionEffect] { storedEffects.map { $0.snapshot() } }
    public var addRemoveDuration: Float {
        get { Float(storedDuration) * (1 / 255) }
        set { storedDuration = transitionTime(newValue) }
    }

    public func addEffect(_ effect: ORBTransitionEffect?) {
        precondition(!isDefault, "The default transition cannot be modified.")
        if let effect { storedEffects.append(effect.snapshot()) }
    }
    public func copy(with zone: NSZone? = nil) -> Any {
        let copy = ORBTransition()
        copy.storedMethod = storedMethod
        copy.storedAnimation = storedAnimation?.copy() as? ORBAnimation
        copy.storedEffects = storedEffects
        copy.maxChanges = maxChanges
        copy.isReplaceable = isReplaceable
        copy.storedDuration = storedDuration
        return copy
    }
    public override func isEqual(_ object: Any?) -> Bool {
        guard let other = object as? ORBTransition else { return false }
        return method == other.method && maxChanges == other.maxChanges
            && isReplaceable == other.isReplaceable && storedDuration == other.storedDuration
            && storedEffects == other.storedEffects && animation == other.animation
    }
    public override var hash: Int {
        var hasher = Hasher()
        hasher.combine(method.rawValue)
        hasher.combine(maxChanges)
        hasher.combine(isReplaceable)
        hasher.combine(storedDuration)
        hasher.combine(storedEffects)
        hasher.combine(animation)
        return hasher.finalize()
    }
}
#endif
