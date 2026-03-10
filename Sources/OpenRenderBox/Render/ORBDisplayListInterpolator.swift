//
//  ORBDisplayListInterpolator.swift
//  OpenRenderBox
//

#if !canImport(ObjectiveC)
public import Foundation

public class ORBDisplayListInterpolator: NSObject {
    // MARK: - Properties

    public var from: any ORBDisplayListContents {
        get { _openRenderBoxUnimplementedFailure() }
        set { _openRenderBoxUnimplementedFailure() }
    }

    public private(set) var to: any ORBDisplayListContents {
        get { _openRenderBoxUnimplementedFailure() }
        set { }
    }

    public private(set) var options: [AnyHashable: Any]? = nil
    public private(set) var isIdentity: Bool = false
    public private(set) var onlyFades: Bool = false
    public private(set) var activeDuration: Double = 0

    // MARK: - Init

    public convenience init(from: any ORBDisplayListContents, to: any ORBDisplayListContents, options: [AnyHashable: Any]?) {
        self.init()
        _openRenderBoxUnimplementedFailure()
    }

    public static func interpolator(withFrom from: any ORBDisplayListContents, to: any ORBDisplayListContents, options: [AnyHashable: Any]?) -> ORBDisplayListInterpolator {
        _openRenderBoxUnimplementedFailure()
    }

    // MARK: - Methods

    public func boundingRect(withProgress progress: Float) -> CGRect {
        _openRenderBoxUnimplementedFailure()
    }

    public func contents(withProgress progress: Float) -> (any ORBDisplayListContents)? {
        _openRenderBoxUnimplementedFailure()
    }

    public func copyContents(withProgress progress: Float) -> (any ORBDisplayListContents)? {
        _openRenderBoxUnimplementedFailure()
    }

    public func maxAbsoluteVelocity(withProgress progress: Float) -> Double {
        _openRenderBoxUnimplementedFailure()
    }
}
#endif
