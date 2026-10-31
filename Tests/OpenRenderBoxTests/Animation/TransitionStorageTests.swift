//
//  TransitionStorageTests.swift
//  OpenRenderBoxTests

#if canImport(ObjectiveC)
import Foundation
import ObjectiveC
import OpenRenderBox
import Testing

struct TransitionStorageTests {
    @Test
    func objectiveCStorageLayout() throws {
        var count: UInt32 = 0
        let ivars = try #require(class_copyIvarList(ORBTransition.self, &count))
        defer { free(ivars) }
        let fields = (0..<Int(count)).map { index in
            (String(cString: ivar_getName(ivars[index])!), ivar_getOffset(ivars[index]))
        }
        #expect(fields.map(\.0) == ["_transition", "_is_default"])
        #expect(fields.map(\.1) == [8, 16])
    }

    @Test
    func defaultCopyIsMutable() throws {
        let copy = try #require(ORBTransition.default.copy() as? ORBTransition)
        copy.method = .diff
        copy.maxChanges = 3
        copy.isReplaceable = true
        copy.addRemoveDuration = 1
        copy.addEffect(ORBTransitionEffect())
        #expect(ORBTransition.default.method == .empty)
        #expect(ORBTransition.default.maxChanges == .max)
        #expect(!ORBTransition.default.isReplaceable)
        #expect(ORBTransition.default.addRemoveDuration == Float(32) * (1 / 255))
        #expect(ORBTransition.default.effects.isEmpty)
    }

    @Test
    func heapBackedCopySurvivesSourceAndDetaches() throws {
        let survivor = autoreleasepool {
            let source = ORBTransition()
            for value: Float in [1, 2, 3, 4] {
                let effect = ORBTransitionEffect()
                effect.type = .translation
                effect.setArgumentValue(value, at: 0)
                effect.setArgumentValue(-value, at: 1)
                source.addEffect(effect)
            }
            let animation = ORBAnimation()
            animation.addSpringDuration(0.5, mass: 1, stiffness: 344, damping: 37, initialVelocity: 0)
            animation.addSpringDuration(0.8, mass: 2, stiffness: 470, damping: 34, initialVelocity: 1)
            source.animation = animation
            return source.copy() as! ORBTransition
        }
        let sibling = try #require(survivor.copy() as? ORBTransition)
        survivor.method = .prefix
        sibling.maxChanges = 2
        sibling.isReplaceable = true
        sibling.addRemoveDuration = 1
        sibling.addEffect(ORBTransitionEffect())
        #expect(survivor.effects.count == 4)
        #expect(sibling.effects.count == 5)
        #expect(survivor.effects.map { $0.argumentValue(at: 0) } == [1, 2, 3, 4])
        #expect(sibling.effects.prefix(4).map { $0.argumentValue(at: 1) } == [-1, -2, -3, -4])
        #expect(survivor.maxChanges == .max)
        #expect(!survivor.isReplaceable)
        #expect(survivor.addRemoveDuration == Float(32) * (1 / 255))
        #expect(sibling.method == .empty)
        #expect(survivor.animation?.activeDuration == 0.5)
        #expect(survivor.animation?.isEqual(sibling.animation) == true)
    }
}
#endif
