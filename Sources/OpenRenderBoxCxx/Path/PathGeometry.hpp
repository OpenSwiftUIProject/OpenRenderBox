//
//  PathGeometry.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include <OpenRenderBox/ORBPath.h>
#include <cmath>

namespace ORB::Path::Geometry {

inline CGPoint mix(CGPoint a, CGPoint b, CGFloat t) {
    return {std::fma(b.x - a.x, t, a.x), std::fma(b.y - a.y, t, a.y)};
}

inline CGFloat distance(CGPoint a, CGPoint b) {
    CGFloat x = b.x - a.x, y = b.y - a.y;
    return std::sqrt(x * x + y * y);
}

struct Cubic {
    CGPoint p[4];

    void split(CGFloat t, Cubic &left, Cubic &right) const {
        CGPoint a = mix(p[0], p[1], t), b = mix(p[1], p[2], t), c = mix(p[2], p[3], t);
        CGPoint d = mix(a, b, t), e = mix(b, c, t), f = mix(d, e, t);
        left = {{p[0], a, d, f}};
        right = {{f, e, c, p[3]}};
    }
};

struct Quadratic {
    CGPoint p[3];

    void split(CGFloat t, Quadratic &left, Quadratic &right) const {
        CGPoint a = mix(p[0], p[1], t), b = mix(p[1], p[2], t);
        CGPoint middle = mix(a, b, t);
        left = {{p[0], a, middle}};
        right = {{middle, b, p[2]}};
    }
};

inline CGFloat quadratic_length(Quadratic curve, unsigned depth = 0) {
    CGFloat x = (curve.p[0].x - curve.p[1].x) + (curve.p[2].x - curve.p[1].x);
    CGFloat y = (curve.p[0].y - curve.p[1].y) + (curve.p[2].y - curve.p[1].y);
    CGFloat curvature = x * x + y * y;
    if (depth > 32 || curvature < 4 || !std::isfinite(curvature)) {
        return distance(curve.p[0], curve.p[2]);
    }
    Quadratic left, right;
    curve.split(0.5, left, right);
    return quadratic_length(left, depth + 1) + quadratic_length(right, depth + 1);
}

inline CGFloat quadratic_measure(Quadratic curve, CGFloat requested, bool findIndex) {
    if (!findIndex) { return quadratic_length(curve); }
    if (requested <= 0) { return 0; }
    if (quadratic_length(curve) <= requested) { return 1; }
    CGFloat lower = 0, upper = 1;
    do {
        CGFloat middle = (lower + upper) * 0.5;
        Quadratic left, right;
        curve.split(middle, left, right);
        if (quadratic_length(left) > requested) {
            upper = middle;
        } else {
            lower = middle;
        }
    } while (upper - lower > 0.0001);
    return (lower + upper) * 0.5;
}

struct CurveSpan {
    Cubic curve;
    CGFloat interval;
};

inline CGFloat curve_measure(Cubic curve, CGFloat requested, bool findIndex) {
    CGFloat edge0 = distance(curve.p[0], curve.p[1]);
    CGFloat edge2 = distance(curve.p[2], curve.p[3]);
    if (!std::isfinite(edge0 + edge2) || (findIndex && requested == 0)) {
        return 0;
    }
    CurveSpan pending[29];
    size_t count = 0;
    CGFloat interval = 1, parameter = 0, total = 0;
    for (unsigned iteration = 0; iteration < 1024; ++iteration) {
        edge0 = distance(curve.p[0], curve.p[1]);
        CGFloat edge1 = distance(curve.p[1], curve.p[2]);
        edge2 = distance(curve.p[2], curve.p[3]);
        CGFloat polygon = edge0 + edge2 + edge1;
        CGFloat chord = distance(curve.p[0], curve.p[3]);
        bool flat = polygon - chord <= 0.5 * interval;
        if (findIndex) {
            flat = flat && std::abs(edge0 - edge1) + std::abs(edge1 - edge2) < 0.5;
        }
        if (flat || count == 29) {
            CGFloat length = (polygon + chord) * 0.5;
            if (findIndex && total + length >= requested) {
                return std::fmin(parameter + (requested - total) * interval / length, 1);
            }
            total += length;
            if (count == 0) {
                return findIndex ? 1 : total;
            }
            parameter += interval;
            auto span = pending[--count];
            curve = span.curve;
            interval = span.interval;
        } else {
            Cubic left, right;
            curve.split(0.5, left, right);
            interval *= 0.5;
            pending[count++] = {right, interval};
            curve = left;
        }
    }
    return 1;
}

} // namespace ORB::Path::Geometry

#endif
