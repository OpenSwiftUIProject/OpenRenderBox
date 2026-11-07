//
//  ORBPathGeometry.swift
//  OpenRenderBox

#if !OPENRENDERBOX_CF_CGTYPES
import Foundation
import OpenCoreGraphicsShims

func _orbRectValues(_ rect: CGRect) -> [CGFloat] {
    [rect.origin.x, rect.origin.y, rect.size.width, rect.size.height]
}

func _orbRect(_ values: [CGFloat]) -> CGRect {
    CGRect(x: values[0], y: values[1], width: values[2], height: values[3])
}

func _orbPoint(_ values: [CGFloat], _ index: Int) -> CGPoint {
    CGPoint(x: values[index * 2], y: values[index * 2 + 1])
}

func _orbRecords(_ path: CGPath) -> [_ORBPathRecord] {
    var records: [_ORBPathRecord] = []
    #if OPENRENDERBOX_COREGRAPHICS
    path.applyWithBlock { pointer in
        let value = pointer.pointee
        let element: ORBPath.Element
        let count: Int
        switch value.type {
        case .moveToPoint: element = .moveToPoint; count = 1
        case .addLineToPoint: element = .addLineToPoint; count = 1
        case .addQuadCurveToPoint: element = .addQuadCurveToPoint; count = 2
        case .addCurveToPoint: element = .addCurveToPoint; count = 3
        case .closeSubpath: element = .closeSubpath; count = 0
        @unknown default: return
        }
        records.append(_ORBPathRecord(element: element, values: (0..<count).flatMap { [value.points[$0].x, value.points[$0].y] }))
    }
    #else
    for element in path.elements {
        switch element {
        case let .moveToPoint(point): records.append(.init(element: .moveToPoint, values: [point.x, point.y]))
        case let .addLineToPoint(point): records.append(.init(element: .addLineToPoint, values: [point.x, point.y]))
        case let .addQuadCurveToPoint(control, point): records.append(.init(element: .addQuadCurveToPoint, values: [control.x, control.y, point.x, point.y]))
        case let .addCurveToPoint(a, b, point): records.append(.init(element: .addCurveToPoint, values: [a.x, a.y, b.x, b.y, point.x, point.y]))
        case .closeSubpath: records.append(.init(element: .closeSubpath, values: []))
        }
    }
    #endif
    return records
}

func _orbMakeCGPath(_ records: [_ORBPathRecord]) -> CGPath {
    let path = CGMutablePath()
    for record in _orbExpandedRecords(records) {
        switch record.element {
        case .moveToPoint: path.move(to: _orbPoint(record.values, 0))
        case .addLineToPoint: path.addLine(to: _orbPoint(record.values, 0))
        case .addQuadCurveToPoint: path.addQuadCurve(to: _orbPoint(record.values, 1), control: _orbPoint(record.values, 0))
        case .addCurveToPoint: path.addCurve(to: _orbPoint(record.values, 2), control1: _orbPoint(record.values, 0), control2: _orbPoint(record.values, 1))
        case .closeSubpath: path.closeSubpath()
        default: preconditionFailure("Unsupported path element")
        }
    }
    return path
}

struct _ORBBezierSegment {
    var points: [CGPoint]

    func point(_ t: CGFloat) -> CGPoint {
        var values = points
        for level in stride(from: values.count - 1, through: 1, by: -1) {
            for index in 0..<level {
                values[index] = CGPoint(x: values[index].x + (values[index + 1].x - values[index].x) * t,
                                        y: values[index].y + (values[index + 1].y - values[index].y) * t)
            }
        }
        return values[0]
    }

    func extrema<Scalar: BinaryFloatingPoint>(_ keyPath: KeyPath<CGPoint, Scalar>) -> [CGFloat] {
        let p = points.map { CGFloat($0[keyPath: keyPath]) }
        let roots: [CGFloat]
        switch p.count {
        case 3:
            let a = p[0] - 2 * p[1] + p[2]
            roots = a == 0 ? [] : [(p[0] - p[1]) / a]
        case 4:
            roots = _orbQuadraticRoots(-p[0] + 3 * p[1] - 3 * p[2] + p[3], 2 * (p[0] - 2 * p[1] + p[2]), p[1] - p[0])
        default: roots = []
        }
        return roots.filter { $0 > 0 && $0 < 1 }.sorted()
    }
}

func _orbQuadraticRoots(_ a: CGFloat, _ b: CGFloat, _ c: CGFloat) -> [CGFloat] {
    if a == 0 { return b == 0 ? [] : [-c / b] }
    let discriminant = b * b - 4 * a * c
    if discriminant < 0 { return [] }
    if discriminant == 0 { return [-b / (2 * a)] }
    let root = discriminant.squareRoot()
    let q = -(b + (b >= 0 ? root : -root)) / 2
    return [q / a, c / q]
}

func _orbSegments(_ records: [_ORBPathRecord], closeOpenSubpaths: Bool) -> [_ORBBezierSegment] {
    var segments: [_ORBBezierSegment] = []
    var current: CGPoint?
    var start: CGPoint?
    func close() {
        if let current, let start, current != start {
            segments.append(.init(points: [current, start]))
        }
    }
    for record in records {
        switch record.element {
        case .moveToPoint:
            if closeOpenSubpaths { close() }
            current = _orbPoint(record.values, 0)
            start = current
        case .addLineToPoint, .addQuadCurveToPoint, .addCurveToPoint:
            let points = stride(from: 0, to: record.values.count, by: 2).map { CGPoint(x: record.values[$0], y: record.values[$0 + 1]) }
            if let current {
                segments.append(.init(points: [current] + points))
            } else {
                start = points.last
            }
            current = points.last
        case .closeSubpath:
            close()
            current = start
        default: break
        }
    }
    if closeOpenSubpaths { close() }
    return segments
}

func _orbBounds(_ records: [_ORBPathRecord]) -> CGRect {
    var bounds = CGRect.null
    func include(_ point: CGPoint) {
        guard point.x.isFinite && point.y.isFinite else { return }
        let rect = CGRect(origin: point, size: .zero)
        bounds = bounds.isNull ? rect : bounds.union(rect)
    }
    for record in records where record.element == .moveToPoint {
        include(_orbPoint(record.values, 0))
    }
    for segment in _orbSegments(records, closeOpenSubpaths: false) {
        include(segment.points[0])
        include(segment.points.last!)
        for t in segment.extrema(\.x) + segment.extrema(\.y) { include(segment.point(t)) }
    }
    return bounds
}

func _orbRecognizeRect(_ records: [_ORBPathRecord]) -> CGRect? {
    guard records.count == 5, records[0].element == .moveToPoint,
          records[1...3].allSatisfy({ $0.element == .addLineToPoint }),
          records[4].element == .closeSubpath else { return nil }
    let points = records.prefix(4).map { _orbPoint($0.values, 0) }
    let bounds = _orbBounds(records)
    guard !bounds.isNull else { return nil }
    let corners = [CGPoint(x: bounds.minX, y: bounds.minY), CGPoint(x: bounds.maxX, y: bounds.minY), CGPoint(x: bounds.maxX, y: bounds.maxY), CGPoint(x: bounds.minX, y: bounds.maxY)]
    guard points.allSatisfy({ corners.contains($0) }) else { return nil }
    for index in 0..<4 {
        let a = points[index]
        let b = points[(index + 1) % 4]
        if a == b || (a.x != b.x && a.y != b.y) { return nil }
    }
    return bounds
}

func _orbClampRadii(_ input: [CGFloat], width: CGFloat, height: CGFloat) -> [CGFloat] {
    let width = abs(width)
    let height = abs(height)
    let limit = max(width, height)
    var radii = input.map { min(max($0, 0), limit) }
    for (a, b, dimension) in [(0, 1, width), (3, 2, width), (0, 3, height), (1, 2, height)] {
        var deficit = dimension - (radii[a] + radii[b])
        if deficit < 0, radii[a] != radii[b] {
            let reduction = min(-deficit, abs(radii[a] - radii[b]))
            if radii[a] > radii[b] { radii[a] -= reduction } else { radii[b] -= reduction }
            deficit += reduction
        }
        if deficit < 0 {
            let change = radii[a] * deficit / (radii[a] + radii[b])
            radii[a] += change
            radii[b] += deficit - change
        }
    }
    return radii
}

func _orbExpandedRecords(_ records: [_ORBPathRecord]) -> [_ORBPathRecord] {
    var result: [_ORBPathRecord] = []
    var frames: [(_ORBPathRecord, [_ORBPathRecord])] = []
    for record in records {
        switch record.element {
        case .transformed, .trimmed, .stroked:
            frames.append((record, result))
            result = []
            continue
        case .end:
            if let (header, parent) = frames.popLast() {
                result = parent + _orbFinishFrame(header, result)
            }
            continue
        default: break
        }
        let transform = record.transform ?? .identity
        var expanded: [_ORBPathRecord]
        switch record.element {
        case .moveToPoint, .addLineToPoint, .addQuadCurveToPoint, .addCurveToPoint, .closeSubpath:
            expanded = [record]
        case .rect:
            let rect = _orbRect(record.values)
            expanded = [
                .init(element: .moveToPoint, values: [rect.minX, rect.minY]),
                .init(element: .addLineToPoint, values: [rect.maxX, rect.minY]),
                .init(element: .addLineToPoint, values: [rect.maxX, rect.maxY]),
                .init(element: .addLineToPoint, values: [rect.minX, rect.maxY]),
                .init(element: .closeSubpath, values: [])
            ]
        case .ellipse:
            let rect = _orbRect(record.values)
            let rx = rect.width / 2
            let ry = rect.height / 2
            let cx = rect.midX
            let cy = rect.midY
            let k = CGFloat(0.5522847498)
            expanded = [
                .init(element: .moveToPoint, values: [cx + rx, cy]),
                .init(element: .addCurveToPoint, values: [cx + rx, cy + ry * k, cx + rx * k, cy + ry, cx, cy + ry]),
                .init(element: .addCurveToPoint, values: [cx - rx * k, cy + ry, cx - rx, cy + ry * k, cx - rx, cy]),
                .init(element: .addCurveToPoint, values: [cx - rx, cy - ry * k, cx - rx * k, cy - ry, cx, cy - ry]),
                .init(element: .addCurveToPoint, values: [cx + rx * k, cy - ry, cx + rx, cy - ry * k, cx + rx, cy]),
                .init(element: .closeSubpath, values: [])
            ]
        case .roundedRect, .fixedRoundedRectCircular, .fixedRoundedRectContinuous, .unevenRoundedRectCircular, .unevenRoundedRectContinuous:
            expanded = _orbRoundedRecords(record)
        case .relativeArc, .arc, .arcCounterclockwise, .arcTo:
            let current = _orbCurrentPoint(result).map { $0.applying(transform.inverted()) }
            expanded = _orbArcRecords(record, current: current)
        default:
            preconditionFailure("Unsupported path element")
        }
        if !transform.isIdentity {
            expanded = expanded.map { entry in
                var entry = entry
                for index in stride(from: 0, to: entry.values.count, by: 2) {
                    let point = CGPoint(x: entry.values[index], y: entry.values[index + 1]).applying(transform)
                    entry.values[index] = point.x
                    entry.values[index + 1] = point.y
                }
                entry.transform = nil
                return entry
            }
        }
        result.append(contentsOf: expanded)
    }
    while let (header, parent) = frames.popLast() {
        result = parent + _orbFinishFrame(header, result)
    }
    return result
}

func _orbRoundedRecords(_ record: _ORBPathRecord) -> [_ORBPathRecord] {
    let rect = _orbRect(record.values).standardized
    let continuous = record.element == .fixedRoundedRectContinuous || record.element == .unevenRoundedRectContinuous
    let rx: [CGFloat]
    let ry: [CGFloat]
    if record.values.count == 8 {
        rx = _orbClampRadii(Array(record.values[4...7]), width: rect.width, height: rect.height)
        ry = rx
    } else if record.values.count == 6 {
        let limit = min(rect.width, rect.height) / 2
        rx = Array(repeating: min(max(record.values[4], 0), limit), count: 4)
        ry = Array(repeating: min(max(record.values[5], 0), limit), count: 4)
    } else {
        rx = Array(repeating: 0, count: 4)
        ry = rx
    }
    var result: [_ORBPathRecord] = [.init(element: .moveToPoint, values: [rect.maxX, (rect.minY + ry[1] + rect.maxY - ry[2]) / 2])]
    if rx.allSatisfy({ $0 == 0 }) && ry.allSatisfy({ $0 == 0 }) {
        result += [
            .init(element: .addLineToPoint, values: [rect.maxX, rect.maxY]),
            .init(element: .addLineToPoint, values: [rect.minX, rect.maxY]),
            .init(element: .addLineToPoint, values: [rect.minX, rect.minY]),
            .init(element: .addLineToPoint, values: [rect.maxX, rect.minY]),
            .init(element: .closeSubpath, values: [])
        ]
        return result
    }
    // The sequence begins at the right edge, then visits BR, BL, TL, and TR.
    let corners = [2, 3, 0, 1]
    let vertices = [CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY)]
    let incoming = [CGPoint(x: 0, y: -1), CGPoint(x: 1, y: 0), CGPoint(x: 0, y: 1), CGPoint(x: -1, y: 0)]
    let outgoing = [CGPoint(x: -1, y: 0), CGPoint(x: 0, y: -1), CGPoint(x: 1, y: 0), CGPoint(x: 0, y: 1)]
    let k = CGFloat(0.5522847498)
    for index in 0..<4 {
        let corner = corners[index]
        let vertex = vertices[index]
        let incomingRadius = index & 1 == 0 ? ry[corner] : rx[corner]
        let outgoingRadius = index & 1 == 0 ? rx[corner] : ry[corner]
        func point(_ a: CGFloat, _ b: CGFloat) -> [CGFloat] {
            [vertex.x + incoming[index].x * incomingRadius * a + outgoing[index].x * outgoingRadius * b,
             vertex.y + incoming[index].y * incomingRadius * a + outgoing[index].y * outgoingRadius * b]
        }
        if continuous {
            let previous = corners[(index + 3) % 4]
            let next = corners[(index + 1) % 4]
            let incomingDimension = index & 1 == 0 ? rect.height : rect.width
            let outgoingDimension = index & 1 == 0 ? rect.width : rect.height
            let previousRadius = index & 1 == 0 ? ry[previous] : rx[previous]
            let nextRadius = index & 1 == 0 ? rx[next] : ry[next]
            let a = _orbContinuousCoefficients(incomingDimension, incomingRadius, previousRadius)
            let b = _orbContinuousCoefficients(outgoingDimension, outgoingRadius, nextRadius)
            let u = CGFloat(Float(0.0749114))
            let v = CGFloat(Float(0.631494))
            let c = CGFloat(Float(0.16906))
            let d = CGFloat(Float(0.372824))
            result.append(.init(element: .addLineToPoint, values: point(a.0, 0)))
            result.append(.init(element: .addCurveToPoint, values: point(a.1, 0) + point(a.2, 0) + point(v, u)))
            result.append(.init(element: .addCurveToPoint, values: point(d, c) + point(c, d) + point(u, v)))
            result.append(.init(element: .addCurveToPoint, values: point(0, b.2) + point(0, b.1) + point(0, b.0)))
        } else {
            result.append(.init(element: .addLineToPoint, values: point(1, 0)))
            result.append(.init(element: .addCurveToPoint, values: point(1 - k, 0) + point(0, 1 - k) + point(0, 1)))
        }
    }
    result.append(.init(element: .closeSubpath, values: []))
    return result
}

func _orbContinuousCoefficients(_ length: CGFloat, _ a: CGFloat, _ b: CGFloat) -> (CGFloat, CGFloat, CGFloat) {
    let sum = Float(a) + Float(b)
    let delta = Float(bitPattern: 0x3f075696)
    let ratio = (Float(abs(length)) - sum) / (sum * delta)
    if ratio < 1 {
        let t = min(max(ratio, 0), 1)
        return (CGFloat(Float(1).addingProduct(t, delta)),
                CGFloat(Float(0.96).addingProduct(t, Float(bitPattern: 0x3e0392e4))),
                CGFloat(Float(0.82).addingProduct(t, Float(bitPattern: 0x3d464670))))
    }
    return (CGFloat(Float(1.528665)), CGFloat(Float(1.08849)), CGFloat(Float(0.868407)))
}

func _orbMix(_ a: CGPoint, _ b: CGPoint, _ t: CGFloat) -> CGPoint {
    CGPoint(x: a.x.addingProduct(b.x - a.x, t), y: a.y.addingProduct(b.y - a.y, t))
}

func _orbDistance(_ a: CGPoint, _ b: CGPoint) -> CGFloat {
    let x = b.x - a.x, y = b.y - a.y
    return (x * x + y * y).squareRoot()
}

struct _ORBCubic {
    var points: [CGPoint]

    func split(_ t: CGFloat) -> (_ORBCubic, _ORBCubic) {
        let a = _orbMix(points[0], points[1], t)
        let b = _orbMix(points[1], points[2], t)
        let c = _orbMix(points[2], points[3], t)
        let d = _orbMix(a, b, t), e = _orbMix(b, c, t), f = _orbMix(d, e, t)
        return (.init(points: [points[0], a, d, f]), .init(points: [f, e, c, points[3]]))
    }

    func measure(_ requested: CGFloat = 0, findIndex: Bool = false) -> CGFloat {
        var curve = self
        var edge0 = _orbDistance(curve.points[0], curve.points[1])
        var edge2 = _orbDistance(curve.points[2], curve.points[3])
        if !(edge0 + edge2).isFinite || (findIndex && requested == 0) { return 0 }
        var pending: [(_ORBCubic, CGFloat)] = []
        var interval: CGFloat = 1, parameter: CGFloat = 0, total: CGFloat = 0
        for _ in 0..<1024 {
            edge0 = _orbDistance(curve.points[0], curve.points[1])
            let edge1 = _orbDistance(curve.points[1], curve.points[2])
            edge2 = _orbDistance(curve.points[2], curve.points[3])
            let polygon = edge0 + edge2 + edge1
            let chord = _orbDistance(curve.points[0], curve.points[3])
            var flat = polygon - chord <= 0.5 * interval
            if findIndex { flat = flat && abs(edge0 - edge1) + abs(edge1 - edge2) < 0.5 }
            if flat || pending.count == 29 {
                let length = (polygon + chord) * 0.5
                if findIndex && total + length >= requested {
                    return min(parameter + (requested - total) * interval / length, 1)
                }
                total += length
                guard let next = pending.popLast() else { return findIndex ? 1 : total }
                parameter += interval
                curve = next.0
                interval = next.1
            } else {
                let (left, right) = curve.split(0.5)
                interval *= 0.5
                pending.append((right, interval))
                curve = left
            }
        }
        return 1
    }
}

func _orbTrimmedRecords(_ records: [_ORBPathRecord], from: Float, to: Float) -> [_ORBPathRecord] {
    if from <= 0 && to >= 1 { return records }
    guard from < to else { return [] }
    struct Segment {
        var element: ORBPath.Element
        var curve: _ORBCubic
        var length: CGFloat
    }
    var segments: [Segment] = []
    var current = CGPoint.zero, start = CGPoint.zero
    var total: CGFloat = 0
    for record in records {
        var curve = _ORBCubic(points: [current, .zero, .zero, .zero])
        var length: CGFloat = 0
        switch record.element {
        case .moveToPoint:
            current = _orbPoint(record.values, 0)
            start = current
            curve.points[0] = current
        case .addLineToPoint:
            curve.points[3] = _orbPoint(record.values, 0)
            length = _orbDistance(current, curve.points[3])
            current = curve.points[3]
        case .addQuadCurveToPoint:
            let control = _orbPoint(record.values, 0), end = _orbPoint(record.values, 1)
            curve.points[1] = CGPoint(x: (current.x + 2 * control.x) / 3, y: (current.y + 2 * control.y) / 3)
            curve.points[2] = CGPoint(x: (end.x + 2 * control.x) / 3, y: (end.y + 2 * control.y) / 3)
            curve.points[3] = end
            length = curve.measure()
            current = end
        case .addCurveToPoint:
            curve.points = [current, _orbPoint(record.values, 0), _orbPoint(record.values, 1), _orbPoint(record.values, 2)]
            length = curve.measure()
            current = curve.points[3]
        case .closeSubpath:
            curve.points[3] = start
            length = _orbDistance(current, start)
            current = start
        default: continue
        }
        total += length
        segments.append(.init(element: record.element, curve: curve, length: length))
    }
    let fromDistance = total * CGFloat(from), toDistance = total * CGFloat(to)
    var consumed: Float = 0
    var needsMove = true, includesStart = false
    var result: [_ORBPathRecord] = []
    func move(_ point: CGPoint) {
        result.append(.init(element: .moveToPoint, values: [point.x, point.y]))
    }
    for segment in segments {
        if segment.element == .moveToPoint {
            needsMove = true
            includesStart = false
            continue
        }
        let begin = CGFloat(consumed), end = begin + segment.length
        consumed = Float(end)
        if segment.length == 0 {
            if segment.element == .closeSubpath && includesStart && begin <= toDistance {
                result.append(.init(element: .closeSubpath, values: []))
            }
            continue
        }
        var first = fromDistance >= begin ? max((fromDistance - begin) / segment.length, 0) : 0
        var last = toDistance < end ? min((toDistance - begin) / segment.length, 1) : 1
        guard first < last else { continue }
        if needsMove { includesStart = first == 0 }
        if segment.element == .addLineToPoint || segment.element == .closeSubpath {
            let start = _orbMix(segment.curve.points[0], segment.curve.points[3], first)
            let finish = _orbMix(segment.curve.points[0], segment.curve.points[3], last)
            if needsMove { move(start); needsMove = false }
            result.append(.init(element: .addLineToPoint, values: [finish.x, finish.y]))
            if segment.element == .closeSubpath && last == 1 && includesStart {
                result.append(.init(element: .closeSubpath, values: []))
            }
        } else {
            var curve = segment.curve
            if first > 0 || last < 1 {
                if first > 0.001 { first = curve.measure(first * segment.length, findIndex: true) }
                if last < 0.999 { last = curve.measure(last * segment.length, findIndex: true) }
                if first > 0.001 {
                    curve = curve.split(first).1
                    last = (last - first) / (1 - first)
                }
                if last < 0.999 { curve = curve.split(last).0 }
            }
            if needsMove { move(curve.points[0]); needsMove = false }
            result.append(.init(element: .addCurveToPoint, values: curve.points.dropFirst().flatMap { [$0.x, $0.y] }))
        }
    }
    return result
}

func _orbFinishFrame(_ header: _ORBPathRecord, _ records: [_ORBPathRecord]) -> [_ORBPathRecord] {
    switch header.element {
    case .transformed:
        let transform = header.transform ?? .identity
        return records.map { record in
            var record = record
            for index in stride(from: 0, to: record.values.count, by: 2) {
                let point = CGPoint(x: record.values[index], y: record.values[index + 1]).applying(transform)
                record.values[index] = point.x
                record.values[index + 1] = point.y
            }
            return record
        }
    case .trimmed:
        return _orbTrimmedRecords(records, from: Float(header.values[0]), to: Float(header.values[1]))
    case .stroked:
        #if OPENRENDERBOX_COREGRAPHICS
        var path = _orbMakeCGPath(records)
        if let stroke = header.stroke, !stroke.dash.isEmpty {
            path = path.copy(dashingWithPhase: CGFloat(Float(header.values[2])), lengths: stroke.dash)
        }
        let cap = CGLineCap(rawValue: header.stroke?.lineCap ?? 0) ?? .butt
        let join = CGLineJoin(rawValue: header.stroke?.lineJoin ?? 0) ?? .miter
        return _orbRecords(path.copy(strokingWithWidth: CGFloat(Float(header.values[0])), lineCap: cap, lineJoin: join, miterLimit: CGFloat(Float(header.values[1]))))
        #else
        preconditionFailure("Stroke conversion requires CoreGraphics")
        #endif
    default:
        preconditionFailure("Unsupported path wrapper")
    }
}

func _orbCurrentPoint(_ records: [_ORBPathRecord]) -> CGPoint? {
    var current: CGPoint?
    var start: CGPoint?
    for record in records {
        switch record.element {
        case .moveToPoint: current = _orbPoint(record.values, 0); start = current
        case .addLineToPoint, .addQuadCurveToPoint, .addCurveToPoint:
            current = _orbPoint(record.values, record.values.count / 2 - 1)
        case .closeSubpath: current = start
        default: break
        }
    }
    return current
}

func _orbArcRecords(_ record: _ORBPathRecord, current: CGPoint?) -> [_ORBPathRecord] {
    let v = record.values
    #if OPENRENDERBOX_COREGRAPHICS
    let path = CGMutablePath()
    if let current { path.move(to: current) }
    switch record.element {
    case .relativeArc: path.addRelativeArc(center: CGPoint(x: v[0], y: v[1]), radius: v[2], startAngle: v[3], delta: v[4])
    case .arc, .arcCounterclockwise: path.addArc(center: CGPoint(x: v[0], y: v[1]), radius: v[2], startAngle: v[3], endAngle: v[4], clockwise: record.element == .arc)
    case .arcTo: path.addArc(tangent1End: CGPoint(x: v[0], y: v[1]), tangent2End: CGPoint(x: v[2], y: v[3]), radius: v[4])
    default: return []
    }
    let records = _orbRecords(path)
    return current == nil ? records : Array(records.dropFirst())
    #else
    if record.element == .arcTo {
        let corner = CGPoint(x: v[0], y: v[1]), endpoint = CGPoint(x: v[2], y: v[3])
        guard let current else { return [.init(element: .moveToPoint, values: [corner.x, corner.y])] }
        let a = CGPoint(x: current.x - corner.x, y: current.y - corner.y)
        let b = CGPoint(x: endpoint.x - corner.x, y: endpoint.y - corner.y)
        let al = (a.x * a.x + a.y * a.y).squareRoot(), bl = (b.x * b.x + b.y * b.y).squareRoot()
        let cross = a.x * b.y - a.y * b.x
        guard al > 0, bl > 0, cross != 0, v[4] > 0 else {
            return [.init(element: .addLineToPoint, values: [corner.x, corner.y])]
        }
        let u = CGPoint(x: a.x / al, y: a.y / al), w = CGPoint(x: b.x / bl, y: b.y / bl)
        let angle = acos(min(max(u.x * w.x + u.y * w.y, -1), 1))
        let distance = v[4] / tan(angle / 2)
        let first = CGPoint(x: corner.x + u.x * distance, y: corner.y + u.y * distance)
        let last = CGPoint(x: corner.x + w.x * distance, y: corner.y + w.y * distance)
        let sign: CGFloat = cross < 0 ? 1 : -1
        let center = CGPoint(x: first.x + sign * u.y * v[4], y: first.y - sign * u.x * v[4])
        let start = atan2(first.y - center.y, first.x - center.x)
        let finish = atan2(last.y - center.y, last.x - center.x)
        var delta = finish - start
        if sign > 0 { while delta < 0 { delta += 2 * .pi } } else { while delta > 0 { delta -= 2 * .pi } }
        return _orbCircularArc(center: center, radius: v[4], start: start, delta: delta, current: current)
    }
    var delta = v[4]
    if record.element != .relativeArc {
        delta -= v[3]
        if record.element == .arc, delta > 0 {
            delta -= ceil(delta / (2 * .pi)) * (2 * .pi)
        } else if record.element == .arcCounterclockwise, delta < 0 {
            delta += ceil(-delta / (2 * .pi)) * (2 * .pi)
        }
    }
    return _orbCircularArc(center: CGPoint(x: v[0], y: v[1]), radius: v[2], start: v[3], delta: delta, current: current)
    #endif
}

func _orbCircularArc(center: CGPoint, radius: CGFloat, start: CGFloat, delta: CGFloat, current: CGPoint?) -> [_ORBPathRecord] {
    guard radius.isFinite, start.isFinite, delta.isFinite else { return [] }
    let segmentCount = ceil(abs(delta) / (.pi / 2))
    guard segmentCount < CGFloat(Int.max) else { return [] }
    let count = max(1, Int(segmentCount))
    let step = delta / CGFloat(count)
    var angle = start
    let first = CGPoint(x: center.x + cos(angle) * radius, y: center.y + sin(angle) * radius)
    var result: [_ORBPathRecord] = []
    if current == nil || current! != first {
        result.append(.init(element: current == nil ? .moveToPoint : .addLineToPoint, values: [first.x, first.y]))
    }
    if delta == 0 { return result }
    for _ in 0..<count {
        let next = angle + step
        let factor = 4 * tan(step / 4) / 3
        let a = CGPoint(x: center.x + cos(angle) * radius, y: center.y + sin(angle) * radius)
        let b = CGPoint(x: center.x + cos(next) * radius, y: center.y + sin(next) * radius)
        result.append(.init(element: .addCurveToPoint, values: [a.x - sin(angle) * radius * factor, a.y + cos(angle) * radius * factor,
            b.x + sin(next) * radius * factor, b.y - cos(next) * radius * factor, b.x, b.y]))
        angle = next
    }
    return result
}

private struct _ORBPathWinding {
    var queries: [SIMD2<Float>]
    var winding: [Int]
    var minimum: SIMD2<Float>
    var maximum: SIMD2<Float>
    var start = SIMD2<Float>.zero
    var current = SIMD2<Float>.zero

    static func narrow(_ p: CGPoint) -> SIMD2<Float> { SIMD2(Float(p.x), Float(p.y)) }

    mutating func line(_ end: SIMD2<Float>) {
        let from = current
        current = end
        let delta = end - from
        if abs(delta.y) < 0.0001 { return }
        for index in queries.indices {
            let t = (queries[index].y - from.y) / delta.y
            if t >= 0 && t <= 1 && from.x.addingProduct(delta.x, t) > queries[index].x {
                winding[index] += delta.y > 0 ? 1 : -1
            }
        }
    }

    mutating func close() {
        if current != start { line(start) }
    }

    mutating func cubic(_ cp1: CGPoint, _ cp2: CGPoint, _ end: CGPoint) {
        var a = current
        let b = Self.narrow(cp1), c = Self.narrow(cp2), d = Self.narrow(end)
        let minX = min(a.x, b.x, c.x, d.x), minY = min(a.y, b.y, c.y, d.y)
        let maxX = max(a.x, b.x, c.x, d.x), maxY = max(a.y, b.y, c.y, d.y)
        if minY > maximum.y || maxX < minimum.x || maxY < minimum.y { current = d; return }
        if minX > maximum.x { line(d); return }
        let e = c - (b + b) + a, f = b - (c + c) + d
        let x = max(abs(e.x), abs(f.x)), y = max(abs(e.y), abs(f.y))
        let estimate = ceil((x * x + y * y).squareRoot().squareRoot())
        guard estimate.isFinite else { current = d; return }
        let segments = Int(min(estimate, 1024))
        if segments >= 2 {
            let first = (b - a) * 3
            var second = SIMD2<Float>.zero, third = SIMD2<Float>.zero
            for axis in 0..<2 {
                second[axis] = (-first[axis]).addingProduct(3, c[axis] - b[axis])
                third[axis] = d[axis] - (a[axis].addingProduct(3, b[axis] - a[axis]) + second[axis])
            }
            let dt = 1 / Float(segments), dt2 = dt * dt, dt3 = dt2 * dt
            var delta1 = SIMD2<Float>.zero, delta2 = SIMD2<Float>.zero
            let delta3 = (third * 6) * dt3
            second *= dt2
            for axis in 0..<2 {
                delta2[axis] = delta3[axis].addingProduct(2, second[axis])
                delta1[axis] = second[axis].addingProduct(first[axis], dt).addingProduct(third[axis], dt3)
            }
            for _ in 1..<segments {
                a += delta1
                delta1 += delta2
                delta2 += delta3
                line(a)
            }
        }
        line(d)
    }
}

func _orbContains(_ records: [_ORBPathRecord], queries: [CGPoint], bounds: CGRect, eoFill: Bool) -> UInt64 {
    var state = _ORBPathWinding(queries: queries.map(_ORBPathWinding.narrow), winding: Array(repeating: 0, count: queries.count),
        minimum: SIMD2(repeating: .infinity), maximum: SIMD2(repeating: -.infinity))
    for query in state.queries {
        for axis in 0..<2 {
            state.minimum[axis] = fmin(state.minimum[axis], query[axis])
            state.maximum[axis] = fmax(state.maximum[axis], query[axis])
        }
    }
    if state.minimum == state.maximum {
        if !bounds.contains(CGPoint(x: CGFloat(state.minimum.x), y: CGFloat(state.minimum.y))) { return 0 }
    } else {
        let queryBounds = CGRect(x: CGFloat(state.minimum.x), y: CGFloat(state.minimum.y),
                                 width: CGFloat(state.maximum.x - state.minimum.x), height: CGFloat(state.maximum.y - state.minimum.y))
        if !bounds.intersects(queryBounds) { return 0 }
    }
    for record in records {
        switch record.element {
        case .moveToPoint:
            state.close()
            state.current = _ORBPathWinding.narrow(_orbPoint(record.values, 0))
            state.start = state.current
        case .addLineToPoint: state.line(_ORBPathWinding.narrow(_orbPoint(record.values, 0)))
        case .addQuadCurveToPoint:
            let control = _orbPoint(record.values, 0), end = _orbPoint(record.values, 1)
            let cp1 = CGPoint(x: CGFloat(state.current.x).addingProduct(2, control.x) / 3,
                              y: CGFloat(state.current.y).addingProduct(2, control.y) / 3)
            let cp2 = CGPoint(x: end.x.addingProduct(2, control.x) / 3, y: end.y.addingProduct(2, control.y) / 3)
            state.cubic(cp1, cp2, end)
        case .addCurveToPoint: state.cubic(_orbPoint(record.values, 0), _orbPoint(record.values, 1), _orbPoint(record.values, 2))
        case .closeSubpath: state.close()
        default: break
        }
    }
    state.close()
    var result: UInt64 = 0
    for index in queries.indices {
        if eoFill ? state.winding[index] & 1 != 0 : state.winding[index] != 0 { result |= UInt64(1) << index }
    }
    return result
}

#endif
