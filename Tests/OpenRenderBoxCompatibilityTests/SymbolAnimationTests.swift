//
//  SymbolAnimationTests.swift
//  OpenRenderBoxCompatibilityTests

import Foundation
import Testing

struct SymbolAnimationTests {
    @Test
    func optionDictionaryBridging() {
        let options: [ORBSymbolAnimation.OptionKey: Any] = [
            .timing: "customTiming",
            .clampsSpeed: false,
            .wiggleOptions: ORBSymbolAnimation.WiggleOptions([.rotational, .continuous]).rawValue,
            .symbolDefaults: ["wiggleStyle": 3],
        ]
        let dictionary = Dictionary(
            uniqueKeysWithValues: options.map { ($0.key.rawValue, $0.value) }
        ) as NSDictionary

        #expect(dictionary["timingObject"] as? String == "customTiming")
        #expect(dictionary["clampsSpeed"] as? Bool == false)
        #expect((dictionary["wiggleOptions"] as? NSNumber)?.uint32Value == 0x103)
        #expect((dictionary["symbolDefaults"] as? NSDictionary)?["wiggleStyle"] as? Int == 3)
    }

    @Test(arguments: [
        (ORBSymbolAnimation.OptionKey.priority, "priority"),
        (.speed, "speed"),
        (.clampsSpeed, "clampsSpeed"),
        (.repeatCount, "repeatCount"),
        (.repeatDelay, "repeatDelay"),
        (.byLayer, "byLayer"),
        (.timing, "timingObject"),
        (.onCompletion, "onCompletion"),
        (.pulseOptions, "pulseOptions"),
        (.breatheOptions, "breatheOptions"),
        (.bounceOptions, "bounceOptions"),
        (.wiggleAngle, "wiggleAngle"),
        (.wiggleOptions, "wiggleOptions"),
        (.rotateOptions, "rotateOptions"),
        (.variableColorOptions, "variableColorOptions"),
        (.replaceOptions, "replaceOptions"),
        (.appearDisappearOptions, "appearDisappearOptions"),
        (.interpolateOptions, "interpolateOptions"),
        (.symbolDefaults, "symbolDefaults"),
    ])
    func optionKeyRawValue(key: ORBSymbolAnimation.OptionKey, expected: String) {
        #expect(key.rawValue == expected)
        #expect(ORBSymbolAnimation.OptionKey(rawValue: expected) == key)
    }
}
