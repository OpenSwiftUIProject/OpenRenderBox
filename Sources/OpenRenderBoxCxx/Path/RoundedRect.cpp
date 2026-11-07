#include <OpenRenderBox/ORBBase.h>
#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN
#include "RoundedRect.hpp"
#include <algorithm>
#include <cmath>

namespace ORB::Path {
namespace {
void constrain_pair(CGFloat &a, CGFloat &b, CGFloat length) {
    CGFloat deficit = length - (a + b);
    if (deficit < 0 && a != b) {
        CGFloat reduction = std::min(-deficit, std::abs(a - b));
        (a > b ? a : b) -= reduction;
        deficit += reduction;
    }
    if (deficit < 0) {
        CGFloat delta = a * deficit / (a + b);
        a += delta;
        b += deficit - delta;
    }
}

struct EdgeCoefficients {
    CGFloat start = static_cast<double>(1.528665f);
    CGFloat control1 = static_cast<double>(1.08849f);
    CGFloat control2 = static_cast<double>(0.868407f);
};

EdgeCoefficients coefficients(CGFloat length, CGFloat a, CGFloat b) {
    float sum = static_cast<float>(a) + static_cast<float>(b);
    float ratio = (static_cast<float>(std::abs(length)) - sum) / (sum * 0.528664947f);
    if (!(ratio < 1)) { return {}; }
    float t = std::max(0.0f, std::min(1.0f, ratio));
    return {std::fma(t, 0.528664947f, 1.0f), std::fma(t, 0.128490031f, 0.96f), std::fma(t, 0.0484070182f, 0.82f)};
}
} // namespace

void add_rounded_rect(CGMutablePathRef path, ORBPathElement element, const CGFloat *values, const CGAffineTransform *transform) {
    CGRect rect = CGRectStandardize(CGRectMake(values[0], values[1], values[2], values[3]));
    CGFloat w = rect.size.width, h = rect.size.height;
    CGFloat rx[4], ry[4];
    if (element == ORBPathElementFixedRoundedRectCircular || element == ORBPathElementFixedRoundedRectContinuous) {
        CGFloat limit = std::min(w, h) * 0.5;
        std::fill(rx, rx + 4, std::min(limit, std::max(CGFloat(0), values[4])));
        std::fill(ry, ry + 4, std::min(limit, std::max(CGFloat(0), values[5])));
    } else {
        CGFloat limit = std::max(w, h);
        for (size_t i = 0; i != 4; ++i) { rx[i] = std::min(limit, std::max(CGFloat(0), values[4 + i])); }
        constrain_pair(rx[0], rx[1], w);
        constrain_pair(rx[3], rx[2], w);
        constrain_pair(rx[0], rx[3], h);
        constrain_pair(rx[1], rx[2], h);
        std::copy(rx, rx + 4, ry);
    }
    CGFloat x0 = CGRectGetMinX(rect), y0 = CGRectGetMinY(rect);
    CGFloat x1 = CGRectGetMaxX(rect), y1 = CGRectGetMaxY(rect);
    bool emptyRadii = true;
    for (size_t i = 0; i != 4; ++i) { emptyRadii &= rx[i] == 0 && ry[i] == 0; }
    CGPathMoveToPoint(path, transform, x1, (y0 + ry[1] + y1 - ry[2]) * 0.5);
    if (emptyRadii) {
        CGPathAddLineToPoint(path, transform, x1, y1);
        CGPathAddLineToPoint(path, transform, x0, y1);
        CGPathAddLineToPoint(path, transform, x0, y0);
        CGPathAddLineToPoint(path, transform, x1, y0);
        CGPathCloseSubpath(path);
        return;
    }
    bool continuous = element == ORBPathElementFixedRoundedRectContinuous || element == ORBPathElementUnevenRoundedRectContinuous;
    // Each corner starts on its incoming edge and ends on the next edge.
    const CGPoint origins[4] = {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};
    const CGPoint incoming[4] = {{0, 1}, {-1, 0}, {0, -1}, {1, 0}};
    const CGPoint outgoing[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    EdgeCoefficients edges[4] = {
        coefficients(w, rx[0], rx[1]), coefficients(h, ry[1], ry[2]),
        coefficients(w, rx[2], rx[3]), coefficients(h, ry[3], ry[0])
    };
    for (size_t turn = 0; turn != 4; ++turn) {
        size_t c = (turn + 2) % 4;
        CGFloat a = incoming[c].x == 0 ? ry[c] : rx[c];
        CGFloat b = outgoing[c].x == 0 ? ry[c] : rx[c];
        auto point = [&](CGFloat u, CGFloat v) -> CGPoint {
            return {origins[c].x + incoming[c].x * (u * a) + outgoing[c].x * (v * b),
                    origins[c].y + incoming[c].y * (u * a) + outgoing[c].y * (v * b)};
        };
        auto line = [&](CGFloat u, CGFloat v) { auto p = point(u, v); CGPathAddLineToPoint(path, transform, p.x, p.y); };
        auto curve = [&](CGFloat u1, CGFloat v1, CGFloat u2, CGFloat v2, CGFloat u3, CGFloat v3) {
            auto p = point(u1, v1), q = point(u2, v2), r = point(u3, v3);
            CGPathAddCurveToPoint(path, transform, p.x, p.y, q.x, q.y, r.x, r.y);
        };
        if (continuous) {
            auto before = edges[(c + 3) % 4], after = edges[c];
            constexpr CGFloat small = static_cast<double>(0.0749114f);
            constexpr CGFloat large = static_cast<double>(0.631494f);
            constexpr CGFloat middle1 = static_cast<double>(0.16906f);
            constexpr CGFloat middle2 = static_cast<double>(0.372824f);
            line(before.start, 0);
            curve(before.control1, 0, before.control2, 0, large, small);
            curve(middle2, middle1, middle1, middle2, small, large);
            curve(0, after.control2, 0, after.control1, 0, after.start);
        } else {
            constexpr CGFloat k = 1 - 0.5522847498;
            line(1, 0);
            curve(k, 0, 0, k, 0, 1);
        }
    }
    CGPathCloseSubpath(path);
}
} // namespace ORB::Path
#endif
