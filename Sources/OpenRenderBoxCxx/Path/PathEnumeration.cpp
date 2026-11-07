//
//  PathEnumeration.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include <OpenRenderBox/ORBPath.h>
#include "PathMapper.hpp"
#include <algorithm>
#include <cmath>
#if defined(__aarch64__)
#include <arm_neon.h>
#endif

namespace {

template <typename T> struct Point {
    T x, y;
};

template <typename T> struct CubicIterator {
    Point<T> point, first, second, third;

    CubicIterator(const Point<T> (&p)[4], int count) : point(p[0]) {
        float step = 1.0f / count;
        float square = step * step, cube = square * step;
        auto initialize = [=](T p0, T p1, T p2, T p3, T &d1, T &d2, T &d3) {
            T a = (p1 - p0) * T(3);
            T b = std::fma(T(3), p2 - p1, -a);
            T c = p3 - (std::fma(T(3), p1 - p0, p0) + b);
            d3 = (c * T(6)) * T(cube);
            b *= T(square);
            d2 = std::fma(T(2), b, d3);
            d1 = std::fma(c, T(cube), std::fma(a, T(step), b));
        };
        initialize(p[0].x, p[1].x, p[2].x, p[3].x, first.x, second.x, third.x);
        initialize(p[0].y, p[1].y, p[2].y, p[3].y, first.y, second.y, third.y);
    }

    void advance() {
        point.x += first.x;
        point.y += first.y;
        first.x += second.x;
        first.y += second.y;
        second.x += third.x;
        second.y += third.y;
    }
};

CGPoint quadratic_control(CGPoint end, CGPoint control) {
    return {std::fma(control.x, 2.0, end.x) * (1.0 / 3.0),
            std::fma(control.y, 2.0, end.y) * (1.0 / 3.0)};
}

double distance(CGPoint first, CGPoint second) {
    double x = second.x - first.x, y = second.y - first.y;
    return std::sqrt(x * x + y * y);
}

float reciprocal(float value) {
#if defined(__aarch64__)
    float32x2_t input = vset_lane_f32(value, vdup_n_f32(0), 0);
    float32x2_t result = vrecpe_f32(input);
    result = vmul_f32(vrecps_f32(input, result), result);
    result = vmul_f32(result, vrecps_f32(input, result));
    return vget_lane_f32(result, 0);
#else
    return 1.0f / value;
#endif
}

struct ApplyContext {
    const CGAffineTransform *transform;
    void *info;
    ORBPathApplierCallback callback;
    bool active = true;

    void apply(const CGPathElement &element) {
        if (!active) { return; }
        CGPoint points[3];
        static constexpr size_t counts[] = {1, 1, 2, 3, 0};
        size_t count = counts[element.type];
        for (size_t i = 0; i < count; ++i) {
            CGPoint point = element.points[i];
            if (transform) {
                points[i] = {
                    std::fma(transform->c, point.y, std::fma(transform->a, point.x, transform->tx)),
                    std::fma(transform->d, point.y, std::fma(transform->b, point.x, transform->ty))
                };
            } else {
                points[i] = point;
            }
        }
        CGPathElement mapped{element.type, count ? points : nullptr};
        active = callback(info, &mapped);
    }
};

struct Flattener {
    void *info;
    ORBPathLineApplyCallback callback;
    float factor;
    bool active = true;
    CGPoint start = CGPointZero, current = CGPointZero;

    void emit(ORBPathLineElement element, CGPoint point) {
        if (active) { active = callback(info, element, &point); }
        current = point;
    }

    void cubic(CGPoint first, CGPoint second, CGPoint end) {
        if (!active) { current = end; return; }
        Point<float> p[] = {{float(current.x), float(current.y)}, {float(first.x), float(first.y)},
                            {float(second.x), float(second.y)}, {float(end.x), float(end.y)}};
        auto curvature = [](float p0, float p1, float p2, float p3) {
            return std::fmax(std::abs((p2 - (p1 + p1)) + p0), std::abs((p1 - (p2 + p2)) + p3));
        };
        float x = curvature(p[0].x, p[1].x, p[2].x, p[3].x);
        float y = curvature(p[0].y, p[1].y, p[2].y, p[3].y);
        float estimate = std::ceil(std::sqrt(std::sqrt(x * x + y * y)) * factor);
        if (!std::isfinite(estimate)) { current = end; return; }
        if (estimate < 1 && p[0].x == p[3].x && p[0].y == p[3].y) { return; }
        int count = int(std::fmin(std::fmax(estimate, 1), 1024));
        if (count >= 2) {
            CubicIterator<float> iterator(p, count);
            for (int i = 1; i < count && active; ++i) {
                iterator.advance();
                emit(ORBPathLineElementAddLineToPoint, {iterator.point.x, iterator.point.y});
            }
        }
        if (active) { emit(ORBPathLineElementAddLineToPoint, end); }
    }

    void apply(const CGPathElement &element) {
        switch (element.type) {
            case kCGPathElementMoveToPoint:
                start = element.points[0];
                emit(ORBPathLineElementMoveToPoint, start);
                break;
            case kCGPathElementAddLineToPoint:
                emit(ORBPathLineElementAddLineToPoint, element.points[0]);
                break;
            case kCGPathElementAddQuadCurveToPoint:
                cubic(quadratic_control(current, element.points[0]),
                      quadratic_control(element.points[1], element.points[0]), element.points[1]);
                break;
            case kCGPathElementAddCurveToPoint:
                cubic(element.points[0], element.points[1], element.points[2]);
                break;
            case kCGPathElementCloseSubpath:
                emit(ORBPathLineElementCloseSubpath, CGPointZero);
                current = start;
                break;
        }
    }
};

struct LengthAccumulator {
    double tolerance;
    double length = 0;
    CGPoint start = CGPointZero, current = CGPointZero;

    void cubic(CGPoint first, CGPoint second, CGPoint end) {
        Point<double> p[] = {{current.x, current.y}, {first.x, first.y},
                            {second.x, second.y}, {end.x, end.y}};
        auto curvature = [](double p0, double p1, double p2, double p3) {
            return std::fmax(std::abs((p0 - (p1 + p1)) + p2), std::abs((p1 - (p2 + p2)) + p3));
        };
        double x = curvature(p[0].x, p[1].x, p[2].x, p[3].x);
        double y = curvature(p[0].y, p[1].y, p[2].y, p[3].y);
        double estimate = std::ceil(std::sqrt((std::sqrt(x * x + y * y) * 0.75) / tolerance));
        if (std::isfinite(estimate)) {
            int count = estimate < 1 ? (CGPointEqualToPoint(current, end) ? 0 : 1) : int(std::fmin(estimate, 1024));
            if (count) {
                CubicIterator<double> iterator(p, count);
                for (int i = 0; i < count; ++i) {
                    length += std::sqrt(iterator.first.x * iterator.first.x + iterator.first.y * iterator.first.y);
                    iterator.advance();
                }
            }
        }
        current = end;
    }

    void apply(const CGPathElement &element) {
        switch (element.type) {
            case kCGPathElementMoveToPoint:
                current = start = element.points[0];
                break;
            case kCGPathElementAddLineToPoint:
                length += distance(current, element.points[0]);
                current = element.points[0];
                break;
            case kCGPathElementAddQuadCurveToPoint:
                cubic(quadratic_control(current, element.points[0]),
                      quadratic_control(element.points[1], element.points[0]), element.points[1]);
                break;
            case kCGPathElementAddCurveToPoint:
                cubic(element.points[0], element.points[1], element.points[2]);
                break;
            case kCGPathElementCloseSubpath:
                length += distance(current, start);
                current = start;
                break;
        }
    }
};

} // namespace

bool ORBPathApply(ORBPath path, const CGAffineTransform *transform, void *info, ORBPathApplierCallback callback) {
    ApplyContext context{transform, info, callback};
    ORB::Path::apply_mapped_path(path, &context, +[](void *info, const CGPathElement *element) {
        auto &context = *static_cast<ApplyContext *>(info);
        context.apply(*element);
        return context.active;
    });
    return context.active;
}

bool ORBPathApplyLines(ORBPath path, double tolerance, void *info, ORBPathLineApplyCallback callback) {
    Flattener flattener{info, callback, 0.75f * reciprocal(float(tolerance))};
    ORBPathApply(path, nullptr, &flattener, +[](void *info, const CGPathElement *element) {
        static_cast<Flattener *>(info)->apply(*element);
        return true;
    });
    return flattener.active;
}

double ORBPathGetLength(ORBPath path, double tolerance, const CGAffineTransform *transform) {
    LengthAccumulator accumulator{tolerance};
    ORBPathApply(path, transform, &accumulator, +[](void *info, const CGPathElement *element) {
        static_cast<LengthAccumulator *>(info)->apply(*element);
        return true;
    });
    return accumulator.length;
}

#endif
