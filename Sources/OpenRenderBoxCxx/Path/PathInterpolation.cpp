//
//  PathInterpolation.cpp
//  OpenRenderBox

#include "PathInterpolation.hpp"

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include "NestedCallbacks.hpp"
#include "PathStorageObject.hpp"
#include "Storage.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace ORB::Path {
namespace {

bool has_affine_info(ORBPathElement element) {
    auto type = static_cast<unsigned>(element);
    return type <= 24 && ((1u << type) & 0x1dd0010u) == 0;
}

bool identity(const CGAffineTransform &t) {
    return t.a == 1 && t.b == 0 && t.c == 0 && t.d == 1 && t.tx == 0 && t.ty == 0;
}

struct Record {
    ORBPathElement type;
    CGFloat values[8] = {};
    CGAffineTransform transform{1, 0, 0, 1, 0, 0};
    bool hasTransform = false;
    uint8_t boolean[2] = {};
    int32_t flags = 0;
    int32_t cap = 1;
};

std::vector<Record> collect(ORBPath path) {
    std::vector<Record> records;
    ORBPathApplyElements(path, &records, +[](void *context, ORBPathElement type, const CGFloat *values, const void *info) {
        Record record{type};
        std::copy_n(values, Storage::argument_count(type), record.values);
        if (info) {
            if (type == ORBPathElementBoolean) std::memcpy(record.boolean, info, sizeof(record.boolean));
            else if (type == ORBPathElementClipStroke) {
                const auto &properties = *static_cast<const Storage::StrokeInfo *>(info);
                record.flags = properties.cap;
                record.cap = properties.join;
            } else if (has_affine_info(type)) {
                record.transform = *static_cast<const CGAffineTransform *>(info);
                record.hasTransform = !identity(record.transform);
            }
        }
        static_cast<std::vector<Record> *>(context)->push_back(record);
        return true;
    });
    return records;
}

bool compatible_types(unsigned from, unsigned to, bool trimmed) {
    if (from == to) return true;
    bool fromRect = from == 6 || (from == 5 && !trimmed);
    bool toRect = to == 6 || (to == 5 && !trimmed);
    bool fromRounded = from >= 8 && from <= 11;
    bool toRounded = to >= 8 && to <= 11;
    return (fromRect && (toRect || toRounded)) || (fromRounded && toRect);
}

double interpolate(double from, double to, double amount) {
    double result = std::fma(to - from, amount, from);
    return std::isnan(result) ? from : result;
}

CGAffineTransform interpolate_transform(CGAffineTransform from, CGAffineTransform to, float fraction) {
    double amount = fraction;
    auto mix = [amount](double a, double b) { return std::fma(b - a, amount, a); };
    if (from.a == to.a && from.b == to.b && from.c == to.c && from.d == to.d)
        return {from.a, from.b, from.c, from.d, mix(from.tx, to.tx), mix(from.ty, to.ty)};
    if ((from.b == 0 && from.c == 0 && to.b == 0 && to.c == 0) ||
        (from.a == 0 && from.d == 0 && to.a == 0 && to.d == 0))
        return {mix(from.a, to.a), mix(from.b, to.b), mix(from.c, to.c), mix(from.d, to.d),
                mix(from.tx, to.tx), mix(from.ty, to.ty)};

    auto a = CGAffineTransformDecompose(from), b = CGAffineTransformDecompose(to);
    if ((a.scale.width < 0 && b.scale.height < 0) || (a.scale.height < 0 && b.scale.width < 0)) {
        a.scale.width = -a.scale.width;
        a.scale.height = -a.scale.height;
        a.rotation += a.rotation < 0 ? M_PI : -M_PI;
    }
    if (a.rotation == 0) a.rotation = 2 * M_PI;
    if (b.rotation == 0) b.rotation = 2 * M_PI;
    if (std::abs(a.rotation - b.rotation) > M_PI) {
        if (a.rotation > b.rotation) a.rotation -= 2 * M_PI;
        else b.rotation -= 2 * M_PI;
    }
    CGAffineTransformComponents result{
        {mix(a.scale.width, b.scale.width), mix(a.scale.height, b.scale.height)},
        mix(a.horizontalShear, b.horizontalShear), mix(a.rotation, b.rotation),
        {mix(a.translation.dx, b.translation.dx), mix(a.translation.dy, b.translation.dy)},
    };
    return CGAffineTransformMakeWithComponents(result);
}

} // namespace

bool can_interpolate(ORBPath from, ORBPath to) {
    if (ORBPathEqualToPath(from, to)) return true;
    auto records = collect(from);
    struct Comparison {
        const std::vector<Record> &records;
        size_t position = 0;
        std::vector<bool> trimmed{false};
    } comparison{records};
    bool result = ORBPathApplyElements(to, &comparison, +[](void *context, ORBPathElement type, const CGFloat *values, const void *info) {
        auto &state = *static_cast<Comparison *>(context);
        if (NestedCallbacks::is_nesting_element(type))
            state.trimmed.push_back(state.trimmed.back() || type == ORBPathElementTrimmed);
        else if (type == ORBPathElementEnd && state.trimmed.size() > 1) state.trimmed.pop_back();
        if (state.position == state.records.size()) return false;
        const auto &record = state.records[state.position++];
        if (!compatible_types(record.type, type, state.trimmed.back())) return false;
        if (type == ORBPathElementBoolean) {
            uint8_t properties[2] = {};
            if (info) std::memcpy(properties, info, sizeof(properties));
            return std::equal(std::begin(record.boolean), std::end(record.boolean), properties);
        }
        if (type == ORBPathElementClipStroke) {
            Storage::StrokeInfo defaults{0, 1, 0, nullptr};
            const auto &properties = info ? *static_cast<const Storage::StrokeInfo *>(info) : defaults;
            int32_t a = record.flags, b = properties.cap;
            if (((a ^ b) & 1) && (record.values[0] < 0.0001 || record.values[0] > 0.9999) &&
                std::abs(record.values[0] + (values[0] - 1)) < 0.0001) {
                a &= ~1;
                b &= ~1;
            }
            return a == b && record.cap == properties.join;
        }
        return true;
    });
    return result && comparison.position == records.size();
}

ORBPath copy_interpolated_path(ORBPath from, ORBPath to, float amount) {
    if (amount == 0 || (from.storage == to.storage && from.callbacks == to.callbacks)) return ORBPathRetain(from);
    if (amount == 1) return ORBPathRetain(to);
    // The result takes discrete metadata from the second path, including
    // metadata for which the compatibility query returns false.
    auto records = collect(from);
    Storage storage(sizeof(Storage));
    struct Mixing {
        const std::vector<Record> &records;
        Storage &storage;
        float amount;
        size_t position = 0;
    } mixing{records, storage, amount};
    bool result = ORBPathApplyElements(to, &mixing, +[](void *context, ORBPathElement type, const CGFloat *values, const void *info) {
        auto &state = *static_cast<Mixing *>(context);
        // The callback ABI permits arbitrary streams. Do not read beyond the
        // first stream when a caller skips the compatibility query.
        if (state.position == state.records.size()) return false;
        const auto &record = state.records[state.position++];
        unsigned outputType = record.type;
        size_t count = Storage::argument_count(record.type);
        CGFloat left[8], right[8] = {}, output[8] = {};
        std::copy_n(record.values, count, left);
        std::copy_n(values, Storage::argument_count(type), right);
        if (record.type != type) {
            bool fromRect = record.type == 5 || record.type == 6;
            bool toRect = type == 5 || type == 6;
            bool fromRounded = record.type >= 8 && record.type <= 11;
            bool toRounded = type >= 8 && type <= 11;
            if (fromRect && toRounded) {
                outputType = type;
                count = Storage::argument_count(type);
                std::fill(left + 4, left + count, 0);
            } else if (!(fromRounded && toRect) && !(fromRect && toRect)) return false;
        }
        for (size_t i = 0; i < count; ++i) output[i] = interpolate(left[i], right[i], state.amount);

        CGAffineTransform transform;
        if (has_affine_info(type)) {
            CGAffineTransform target = info ? *static_cast<const CGAffineTransform *>(info) : CGAffineTransformIdentity;
            if (record.hasTransform || !identity(target)) {
                transform = interpolate_transform(record.transform, target, state.amount);
                info = &transform;
            }
        }
        return state.storage.append_element(static_cast<ORBPathElement>(outputType), output, info);
    });
    return result ? copy_storage_path(storage) : ORBPathNull;
}

} // namespace ORB::Path

extern "C" bool ORBPathCanInterpolate(ORBPath from, ORBPath to) {
    return ORB::Path::can_interpolate(from, to);
}

extern "C" ORBPath ORBPathCopyInterpolatedPath(ORBPath from, ORBPath to, float amount) {
    return ORB::Path::copy_interpolated_path(from, to, amount);
}

#endif
