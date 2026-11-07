//
//  ClipStrokeKeyframes.hpp
//  OpenRenderBox

#pragma once

#include "PathClipStroke.hpp"

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
#if defined(__aarch64__)
#include <arm_neon.h>
#endif

namespace ORB::Path::ClipStroke {

using Keyframe = ClipStrokeKeyframe;

struct SingleKeyframe {
    double distance;
    float value;
    int32_t sourceIndex;
    int32_t tag;
};
static_assert(sizeof(Keyframe) == 16);
static_assert(sizeof(SingleKeyframe) == 24);

inline float normalizeProgress(uint32_t flags, float progress) {
    if (flags & 2) {
        if (!(flags & 4)) progress = std::fmax(progress, 0.0f);
        progress = std::fmin(progress, 1.0f);
    }
    if (flags & 1) progress = 1.0f - progress;
    return progress;
}

inline bool valueClipped(uint32_t flags, float progress, float value) {
    if (flags & 1) {
        if ((flags & 4) && value >= 1.0f) return false;
        return std::fmax(progress, 0.0f) >= value;
    }
    if ((flags & 4) && value <= 0.0f) return false;
    return std::fmin(progress, 1.0f) <= value;
}

inline float reciprocal(float value) {
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

inline bool makeKeyframes(const Keyframe *input, size_t count,
                          double subpathLength, float progress,
                          std::vector<SingleKeyframe> &output, double &rate) {
    if (count == 0) return false;
    output.clear();
    output.reserve(count * 2);
    float minimum = std::numeric_limits<float>::infinity();
    float maximum = -minimum;
    for (size_t i = 0; i != count; ++i) {
        const auto &key = input[i];
        for (float position : {key.start, key.end}) {
            const float fraction = position - std::floor(position);
            output.push_back({subpathLength * double(fraction), key.value,
                              int32_t(i), key.tag});
        }
        if (key.value < minimum) minimum = key.value;
        if (maximum < key.value) maximum = key.value;
    }
    // Equal distances have no secondary key; the target uses an unstable sort.
    std::sort(output.begin(), output.end(), [](const auto &a, const auto &b) {
        return a.distance < b.distance;
    });
    double totalDistance = 0;
    float totalValue = 0;
    for (size_t i = 1; i != output.size(); ++i) {
        const float difference = std::fabs(output[i].value - output[i - 1].value);
        if (!(difference < 0.0001f)) {
            totalDistance += output[i].distance;
            totalDistance -= output[i - 1].distance;
            totalValue += difference;
        }
    }
    rate = totalValue > 0 ? totalDistance * double(reciprocal(totalValue)) : 0;
    float previous = output[0].value;
    float current = output[1].value;
    for (size_t i = 1; i + 1 < output.size(); ++i) {
        const float next = output[i + 1].value;
        if (minimum < current && current < maximum) {
            float low = current < previous ? current : previous;
            if (next < low) low = next;
            float high = previous < current ? current : previous;
            if (high < next) high = next;
            if (low > progress || high < progress)
                output[i].value = std::numeric_limits<float>::infinity();
        }
        // The simplifier compares original neighboring values.
        previous = current;
        current = next;
    }
    return !output.empty();
}

struct Cursor {
    const std::vector<SingleKeyframe> &keys;
    double subpathLength;
    size_t index = 1;
    SingleKeyframe previous;
    SingleKeyframe next;

    Cursor(const std::vector<SingleKeyframe> &values, double length)
        : keys(values), subpathLength(length), previous(values.back()),
          next(values.front()) {
        previous.distance -= length;
    }

    float initialValue() const {
        const float t = float(previous.distance / (next.distance - previous.distance));
        return std::fma(previous.value - next.value, t, previous.value);
    }

    float valueAt(double distance) const {
        const float t = float((distance - previous.distance) /
                              (next.distance - previous.distance));
        return std::fma(next.value - previous.value, t, previous.value);
    }

    void advance(double distance) {
        while (next.distance <= distance && index < keys.size()) {
            const SingleKeyframe candidate = keys[index];
            if (candidate.distance < next.distance) break;
            ++index;
            if (std::isfinite(candidate.value)) {
                previous = next;
                next = candidate;
            }
        }
        if (next.distance <= distance && index == keys.size()) {
            previous = next;
            if (index != 0) {
                next = keys.front();
                next.distance += subpathLength;
                ++index;
            }
        }
    }
};

struct DistanceMap {
    struct Entry {
        double sourceStart;
        double destinationStart;
        double sourceLength;
    };
    std::vector<Entry> entries;

    void append(double source, double destination, double length) {
        if (!entries.empty()) {
            auto &last = entries.back();
            if (std::fabs((last.sourceStart - source) + last.sourceLength) < 1 &&
                std::fabs((last.sourceLength - destination) + last.destinationStart) < 1) {
                last.sourceLength = (length + source) - last.sourceStart;
                return;
            }
        }
        entries.push_back({source, destination, length});
    }

    double operator()(double source) const {
        const auto found = std::lower_bound(entries.begin(), entries.end(), source,
            [](const Entry &entry, double value) {
                return (entry.sourceStart + 0.0001) + entry.sourceLength < value;
            });
        if (found != entries.end()) {
            const double offset = source - found->sourceStart;
            if (!(offset < -0.0001)) return found->destinationStart + offset;
        }
        return std::numeric_limits<double>::infinity();
    }
};

inline double signedCircularDifference(float difference) {
    if (difference > 0.5f) return double(difference) - 1;
    if (difference < -0.5f) return double(difference) + 1;
    return double(difference);
}

struct TaggedSplit {
    double firstStart;
    float firstEnd;
    float secondStart;
    double secondEnd;
};

inline bool selectTaggedSplit(const std::vector<Keyframe> &keys, size_t index,
                              uint32_t flags, float progress, TaggedSplit &result) {
    if (index == 0 || index + 1 >= keys.size() || !(keys[index].tag & 1)) return false;
    if (valueClipped(flags, progress, keys[index - 1].value) ||
        !valueClipped(flags, progress, keys[index + 1].value)) return false;
    const auto &key = keys[index];
    const bool reverse = signedCircularDifference(key.end - key.start) < 0;
    const float start = reverse ? key.end : key.start;
    const float end = reverse ? key.start : key.end;
    double nearestBefore = std::numeric_limits<double>::infinity();
    double nearestAfter = nearestBefore;
    double pairedBefore = 0;
    double pairedAfter = 0;
    for (size_t i = 0; i != keys.size(); ++i) {
        if (i == index) continue;
        const auto &other = keys[i];
        for (int endpoint = 0; endpoint != 2; ++endpoint) {
            const float position = endpoint == 0 ? other.start : other.end;
            const float pair = endpoint == 0 ? other.end : other.start;
            const double before = signedCircularDifference(start - position);
            const double after = signedCircularDifference(position - end);
            if (before >= 0 && before < nearestBefore) {
                nearestBefore = before;
                pairedBefore = pair;
            }
            if (after >= 0 && after < nearestAfter) {
                nearestAfter = after;
                pairedAfter = pair;
            }
        }
    }
    if (!std::isfinite(nearestBefore) || !std::isfinite(nearestAfter)) return false;
    result = {pairedBefore, end, start, pairedAfter};
    return true;
}

inline double positionDistance(double position, double pathLength) {
    return (position - std::floor(position)) * pathLength;
}

inline std::vector<Keyframe> remapKeyframes(const std::vector<Keyframe> &keys,
                                          size_t splitIndex, double sourceLength,
                                          double closedDestinationLength,
                                          const DistanceMap &map) {
    std::vector<Keyframe> result;
    const double inverseLength = 1 / closedDestinationLength;
    for (size_t i = 0; i != keys.size(); ++i) {
        Keyframe key = keys[i];
        key.start = float(map(positionDistance(key.start, sourceLength)) * inverseLength);
        key.end = float(map(positionDistance(key.end, sourceLength)) * inverseLength);
        if (i == splitIndex) key.tag &= ~1;
        if (std::isfinite(key.start) && std::isfinite(key.end)) result.push_back(key);
    }
    return result;
}

} // namespace ORB::Path::ClipStroke
#endif
