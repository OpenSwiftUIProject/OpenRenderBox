//
//  ORBSymbolAnimation.swift
//  OpenRenderBox

#if !canImport(ObjectiveC)
public import OpenRenderBoxCxx

extension ORBSymbolAnimation {
    public struct OptionKey: RawRepresentable, Hashable, Sendable {
        public let rawValue: String

        public init(rawValue: String) {
            self.rawValue = rawValue
        }

        public static let timing = Self(rawValue: "timingObject")

        public static let speed = Self(rawValue: "speed")

        public static let repeatCount = Self(rawValue: "repeatCount")

        public static let repeatDelay = Self(rawValue: "repeatDelay")

        public static let priority = Self(rawValue: "priority")

        public static let byLayer = Self(rawValue: "byLayer")

        public static let onCompletion = Self(rawValue: "onCompletion")

        public static let breatheOptions = Self(rawValue: "breatheOptions")

        public static let bounceOptions = Self(rawValue: "bounceOptions")

        public static let wiggleOptions = Self(rawValue: "wiggleOptions")

        public static let wiggleAngle = Self(rawValue: "wiggleAngle")

        public static let rotateOptions = Self(rawValue: "rotateOptions")

        public static let variableColorOptions = Self(rawValue: "variableColorOptions")

        public static let appearDisappearOptions = Self(rawValue: "appearDisappearOptions")

        public static let replaceOptions = Self(rawValue: "replaceOptions")

        public static let interpolateOptions = Self(rawValue: "interpolateOptions")

        public static let clampsSpeed = Self(rawValue: "clampsSpeed")

        public static let symbolDefaults = Self(rawValue: "symbolDefaults")

        public static let pulseOptions = Self(rawValue: "pulseOptions")
    }
}
#endif
