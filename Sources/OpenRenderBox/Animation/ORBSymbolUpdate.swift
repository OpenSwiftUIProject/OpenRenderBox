//
//  ORBSymbolUpdate.swift
//  OpenRenderBox

#if !canImport(ObjectiveC)
public import OpenCoreGraphicsShims
public import OpenRenderBoxCxx.C

extension ORBSymbolUpdate {
    public var nextTime: Double { .infinity }
    public var styleMask: UInt32 { 0 }
    public var groupCount: Int { 0 }

    public func groupSize(group: Int) -> Int { 0 }

    public func templateImage(scale: UnsafeMutablePointer<Double>?, opacity: UnsafeMutablePointer<Float>?) -> CGImage? {
        nil
    }

    public func draw(in state: ORBDrawingState, resolver: @escaping (ORBSymbolStyle, String?) -> ORBColor) {}
}

extension ORBSymbolUpdateLayer {
    public var drawingOpacity: Float { 0 }
    public var style: ORBSymbolStyle { .primary }
    public var colorName: String? { nil }
    public var defaultColor: CGColor? { nil }

    public func draw(in state: ORBDrawingState, color: ORBColor, opacity: Float, blendMode: ORBBlendMode, flags: ORBSymbolUpdateLayerFlags) {}

    public func path(transform: UnsafeMutablePointer<CGAffineTransform>, flags: ORBSymbolUpdateLayerFlags) -> ORBPath {
        transform.pointee = .identity
        return .null
    }
}
#endif
