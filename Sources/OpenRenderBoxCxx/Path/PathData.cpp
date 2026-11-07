//
//  PathData.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include <OpenRenderBox/ORBPath.h>
#include "PathClipStroke.hpp"
#include "PathStorageObject.hpp"
#include "Storage.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <new>
#include <vector>

namespace {

using Bytes = std::vector<uint8_t>;
using ORB::Path::Storage;
using ORB::Path::ClipStrokeInfo;
using ORB::Path::ClipStrokeKeyframe;

struct Encoder {
    Bytes bytes;

    void varint(uint64_t value) {
        do {
            uint8_t byte = value & 0x7f;
            value >>= 7;
            bytes.push_back(byte | (value ? 0x80 : 0));
        } while (value);
    }

    void integer(unsigned field, uint64_t value) {
        if (!value) return;
        varint(uint64_t(field) << 3);
        varint(value);
    }

    template<typename T> void scalar(T value) {
        const auto *start = reinterpret_cast<const uint8_t *>(&value);
        bytes.insert(bytes.end(), start, start + sizeof(T));
    }

    void real(unsigned field, float value, float defaultValue = 0) {
        if (value == defaultValue) return;
        varint((uint64_t(field) << 3) | 5);
        scalar(value);
    }

    void data(unsigned field, const Bytes &value) {
        varint((uint64_t(field) << 3) | 2);
        varint(value.size());
        bytes.insert(bytes.end(), value.begin(), value.end());
    }
};

struct Field {
    unsigned number = 0;
    unsigned wire = 0;
    uint64_t integer = 0;
    const uint8_t *data = nullptr;
    size_t size = 0;

    template<typename T> T scalar() const {
        T value;
        std::memcpy(&value, data, sizeof(T));
        return value;
    }
};

struct Decoder {
    const uint8_t *bytes;
    size_t size;
    size_t offset = 0;
    bool valid = true;

    bool varint(uint64_t &result) {
        result = 0;
        for (unsigned shift = 0; shift < 70; shift += 7) {
            if (offset == size) return valid = false;
            uint8_t byte = bytes[offset++];
            if (shift == 63 && byte > 1) return valid = false;
            result |= uint64_t(byte & 0x7f) << shift;
            if (!(byte & 0x80)) return true;
        }
        return valid = false;
    }

    bool next(Field &field) {
        if (!valid || offset == size) return false;
        uint64_t key;
        if (!varint(key) || key >> 3 == 0 || key >> 3 > 0x1fffffff) return valid = false;
        field = {};
        field.number = unsigned(key >> 3);
        field.wire = key & 7;
        if (field.wire == 0) return varint(field.integer);
        uint64_t count;
        if (field.wire == 1) count = 8;
        else if (field.wire == 5) count = 4;
        else if (field.wire == 2) {
            if (!varint(count)) return false;
        } else return valid = false;
        if (count > size - offset) return valid = false;
        field.data = bytes + offset;
        field.size = size_t(count);
        offset += field.size;
        return true;
    }
};

struct Metadata {
    unsigned kind = 0;
    CGAffineTransform affine = CGAffineTransformIdentity;
    Storage::StrokeInfo stroke = {};
    uint8_t boolean[2] = {};
    ClipStrokeInfo clip = {0, 1, 0, nullptr};
    std::vector<CGFloat> dash;
    std::vector<ClipStrokeKeyframe> keys;

    const void *pointer() {
        switch (kind) {
            case 1: return &affine;
            case 2:
                stroke.count = dash.size();
                stroke.values = dash.data();
                return &stroke;
            case 3: return boolean;
            case 4:
                clip.count = keys.size();
                clip.keys = keys.data();
                return &clip;
            default: return nullptr;
        }
    }
};

template<typename T, typename F> bool read_reals(const Field &field, F append) {
    if (field.wire != 2 && field.wire != (sizeof(T) == 4 ? 5u : 1u)) return false;
    if (field.size % sizeof(T)) return false;
    for (size_t i = 0; i < field.size; i += sizeof(T)) {
        T value;
        std::memcpy(&value, field.data + i, sizeof(T));
        append(value);
    }
    return true;
}

bool decode_key(const Field &message, ClipStrokeKeyframe &key) {
    if (message.wire != 2) return false;
    Decoder decoder{message.data, message.size};
    Field field;
    while (decoder.next(field)) {
        if (field.number <= 3) {
            float *value = field.number == 1 ? &key.value : field.number == 2 ? &key.start : &key.end;
            if (!read_reals<float>(field, [&](float v) { *value = v; })) return false;
        } else if (field.number <= 5) {
            if (field.wire != 0) return false;
            if (field.integer) key.tag |= field.number == 4 ? 1 : 2;
        }
    }
    return decoder.valid;
}

bool decode_metadata(const Field &message, Metadata &metadata) {
    if (message.wire != 2) return false;
    Decoder outer{message.data, message.size};
    Field variant;
    while (outer.next(variant)) {
        if (variant.number > 4 || metadata.kind) continue;
        if (variant.wire != 2) return false;
        metadata.kind = variant.number;
        Decoder decoder{variant.data, variant.size};
        Field field;
        while (decoder.next(field)) {
            if (metadata.kind == 1) {
                CGFloat *values[] = {&metadata.affine.a, &metadata.affine.b, &metadata.affine.c,
                                     &metadata.affine.d, &metadata.affine.tx, &metadata.affine.ty};
                if (field.number <= 6 && !read_reals<float>(field, [&](float v) { *values[field.number - 1] = v; })) return false;
            } else if (metadata.kind == 2) {
                if (field.number <= 2) {
                    if (field.wire != 0) return false;
                    if (field.number == 1 && field.integer <= 6) metadata.stroke.cap = int(field.integer);
                    if (field.number == 2 && field.integer <= 2) metadata.stroke.join = int(field.integer);
                } else if (field.number == 3 && !read_reals<float>(field, [&](float v) { metadata.dash.push_back(v); })) return false;
            } else if (metadata.kind == 3) {
                if (field.number <= 2) {
                    if (field.wire != 0) return false;
                    if (field.number == 1 && field.integer <= 6) metadata.boolean[0] = uint8_t(field.integer);
                    if (field.number == 2) metadata.boolean[1] = field.integer != 0;
                }
            } else if (field.number == 5) {
                ClipStrokeKeyframe key{};
                if (!decode_key(field, key)) return false;
                metadata.keys.push_back(key);
            } else if (field.number <= 6) {
                if (field.wire != 0) return false;
                if (field.number == 1) {
                    if (field.integer <= 6) metadata.clip.cap = int(field.integer);
                } else if (field.integer) {
                    constexpr uint32_t flags[] = {0, 0, 4, 1, 2, 0, 8};
                    metadata.clip.flags |= flags[field.number];
                }
            }
        }
        if (!decoder.valid) return false;
    }
    return outer.valid;
}

unsigned metadata_kind(unsigned element) {
    switch (element) {
        case 4: case 16: case 18: case 22: case 23: return 0;
        case 19: return 2;
        case 20: return 3;
        case 24: return 4;
        default: return 1;
    }
}

Encoder encode_metadata(unsigned element, const void *info) {
    Encoder result, value;
    if (!info || element == 4 || element == 16 || element == 18 || element == 22 || element == 23) return result;
    if (element == 19) {
        const auto &stroke = *static_cast<const Storage::StrokeInfo *>(info);
        if (stroke.cap == 0 && stroke.join == 0 && stroke.count == 0) return result;
        value.integer(1, uint64_t(int64_t(stroke.cap)));
        value.integer(2, uint64_t(int64_t(stroke.join)));
        if (stroke.count && stroke.values) {
            Encoder dash;
            auto *values = static_cast<const CGFloat *>(stroke.values);
            for (size_t i = 0; i < stroke.count; ++i) dash.scalar(float(values[i]));
            value.data(3, dash.bytes);
        }
        result.data(2, value.bytes);
    } else if (element == 20) {
        const auto *flags = static_cast<const uint8_t *>(info);
        if (flags[0] == 0 && flags[1] != 1) return result;
        value.integer(1, flags[0]);
        value.integer(2, flags[1] == 1);
        result.data(3, value.bytes);
    } else if (element == 24) {
        const auto &clip = *static_cast<const ClipStrokeInfo *>(info);
        if (clip.cap == 1 && clip.count == 0) return result;
        if (clip.cap != 1) {
            value.varint(8);
            value.varint(uint64_t(int64_t(clip.cap)));
        }
        value.integer(2, (clip.flags & 4) != 0);
        value.integer(3, (clip.flags & 1) != 0);
        value.integer(4, (clip.flags & 2) != 0);
        if (clip.keys) for (size_t i = 0; i < clip.count; ++i) {
            const auto &key = clip.keys[i];
            Encoder item;
            item.real(1, key.value);
            item.real(2, key.start);
            item.real(3, key.end);
            item.integer(4, (key.tag & 1) != 0);
            item.integer(5, (key.tag & 2) != 0);
            value.data(5, item.bytes);
        }
        value.integer(6, (clip.flags & 8) != 0);
        result.data(4, value.bytes);
    } else {
        const auto &affine = *static_cast<const CGAffineTransform *>(info);
        if (CGAffineTransformIsIdentity(affine)) return result;
        value.real(1, float(affine.a), 1);
        value.real(2, float(affine.b));
        value.real(3, float(affine.c));
        value.real(4, float(affine.d), 1);
        value.real(5, float(affine.tx));
        value.real(6, float(affine.ty));
        result.data(1, value.bytes);
    }
    return result;
}

} // namespace

CFDataRef ORBPathCopyData(ORBPath path) {
    struct Encoding {
        Encoder output, elements, floats, doubles;
    } encoding;
    ORBPathApplyElements(path, &encoding, +[](void *context, ORBPathElement element, const CGFloat *points, const void *info) {
        auto &encoding = *static_cast<Encoding *>(context);
        unsigned type = unsigned(element);
        Encoder metadata = encode_metadata(type, info);
        uint8_t code = uint8_t(type);
        if (!metadata.bytes.empty()) {
            code |= 0x80;
            encoding.output.data(4, metadata.bytes);
        }
        size_t count = Storage::argument_count(element);
        for (size_t i = 0; i < count; ++i) if (!(std::abs(points[i]) <= 32767)) code |= 0x40;
        encoding.elements.bytes.push_back(code);
        for (size_t i = 0; i < count; ++i) {
            if (code & 0x40) encoding.doubles.scalar(points[i]);
            else encoding.floats.scalar(float(points[i]));
        }
        return true;
    });
    if (!encoding.elements.bytes.empty()) encoding.output.data(3, encoding.elements.bytes);
    if (!encoding.floats.bytes.empty()) encoding.output.data(2, encoding.floats.bytes);
    if (!encoding.doubles.bytes.empty()) encoding.output.data(5, encoding.doubles.bytes);
    return CFDataCreate(nullptr, encoding.output.bytes.data(), CFIndex(encoding.output.bytes.size()));
}

ORBPath ORBPathMakeWithData(CFDataRef data) {
    if (!data) return ORBPathNull;
    Decoder decoder{CFDataGetBytePtr(data), size_t(CFDataGetLength(data))};
    Bytes elements;
    std::vector<CGFloat> floats, doubles;
    std::vector<Metadata> metadata;
    Field field;
    while (decoder.next(field)) {
        switch (field.number) {
            case 1: case 3:
                if (field.wire != 2) { decoder.valid = false; break; }
                elements.assign(field.data, field.data + field.size);
                if (field.number == 1) for (auto &element : elements) element &= 0x1f;
                break;
            case 2:
                if (!read_reals<float>(field, [&](float v) { floats.push_back(v); })) return ORBPathNull;
                break;
            case 4:
                metadata.emplace_back();
                if (!decode_metadata(field, metadata.back())) return ORBPathNull;
                break;
            case 5:
                if (!read_reals<double>(field, [&](double v) { doubles.push_back(v); })) return ORBPathNull;
                break;
            default: break;
        }
    }
    // The format accepts a valid decoded prefix after a truncated field.
    size_t floatIndex = 0, doubleIndex = 0, metadataIndex = 0, depth = 0;
    // Validate the complete stream before Storage reads any payload.
    for (uint8_t code : elements) {
        unsigned type = code & 0x3f;
        if (type > 24) return ORBPathNull;
        if (code & 0x80) {
            if (metadataIndex == metadata.size() || metadata[metadataIndex++].kind != metadata_kind(type)) return ORBPathNull;
        }
        size_t count = Storage::argument_count(ORBPathElement(type));
        auto &index = code & 0x40 ? doubleIndex : floatIndex;
        const auto &values = code & 0x40 ? doubles : floats;
        if (count > values.size() - index) return ORBPathNull;
        index += count;
        if ((uint32_t(1) << type) & 0x1de0000) ++depth;
        else if (type == 16) {
            if (!depth) return ORBPathNull;
            --depth;
        }
    }
    if (depth) return ORBPathNull;

    alignas(Storage) uint8_t buffer[144];
    auto &storage = *new (buffer) Storage(sizeof(buffer));
    floatIndex = doubleIndex = metadataIndex = 0;
    for (uint8_t code : elements) {
        auto type = ORBPathElement(code & 0x3f);
        const void *info = code & 0x80 ? metadata[metadataIndex++].pointer() : nullptr;
        auto &index = code & 0x40 ? doubleIndex : floatIndex;
        const auto &values = code & 0x40 ? doubles : floats;
        size_t count = Storage::argument_count(type);
        storage.append_element(type, count ? values.data() + index : nullptr, info);
        index += count;
    }
    ORBPath result = ORB::Path::copy_storage_path(storage);
    storage.~Storage();
    return result;
}

CFStringRef ORBPathCopyDebugDescription(ORBPath path) {
    auto result = CFStringCreateMutable(nullptr, 0);
    CFStringAppend(result, CFSTR("RBPath:"));
    ORBPathApplyElements(path, result, +[](void *context, ORBPathElement element, const CGFloat *points, const void *info) {
        auto result = static_cast<CFMutableStringRef>(context);
        constexpr const char *names[] = {
            "moveto", "lineto", "quadto", "cubeto", "close", "rect", "rect*", "ellipse",
            "rounded-rect", "continuous-rect", "uneven-rounded-rect", "uneven-continuous-rect",
            "relative-arc", "clockwise-arc", "counter-clockwise-arc", "arc-to-point", "pop",
            "push affine", "push cut", "push stroke", "push bool", "end-path", "push-dilate-glyph",
            "push offset", "push clip-stroke"
        };
        constexpr const char *caps[] = {"butt", "round", "square", "outwards-triangle", "inwards-triangle", "forwards-triangle", "backwards-triangle"};
        constexpr const char *joins[] = {"miter", "round", "bevel"};
        constexpr const char *operations[] = {"normalize", "intersect", "union", "subtract", "symmetric-difference", "line-intersect", "line-subtract"};
        unsigned type = unsigned(element);
        CFStringAppendFormat(result, nullptr, CFSTR("\n  %s"), type < 25 ? names[type] : "unknown");
        for (size_t i = 0; i < Storage::argument_count(element); ++i) CFStringAppendFormat(result, nullptr, CFSTR(" %g"), points[i]);
        if (!info) return true;
        switch (metadata_kind(type)) {
            case 1: {
                const auto &t = *static_cast<const CGAffineTransform *>(info);
                CFStringAppendFormat(result, nullptr, CFSTR(" affine(%g %g; %g %g; %g %g)"), t.a, t.b, t.c, t.d, t.tx, t.ty);
                break;
            }
            case 2: {
                const auto &stroke = *static_cast<const Storage::StrokeInfo *>(info);
                CFStringAppendFormat(result, nullptr, CFSTR(" %s %s ["), unsigned(stroke.cap) < 7 ? caps[stroke.cap] : "unknown", unsigned(stroke.join) < 3 ? joins[stroke.join] : "unknown");
                const auto *values = static_cast<const CGFloat *>(stroke.values);
                if (values) for (size_t i = 0; i < stroke.count; ++i) CFStringAppendFormat(result, nullptr, CFSTR("%s%g"), i ? ", " : "", values[i]);
                CFStringAppend(result, CFSTR("]"));
                break;
            }
            case 3: {
                const auto *flags = static_cast<const uint8_t *>(info);
                CFStringAppendFormat(result, nullptr, CFSTR(" %s, %s"), flags[0] < 7 ? operations[flags[0]] : "unknown", flags[1] == 1 ? "even-odd" : "non-zero");
                break;
            }
            case 4: {
                const auto &clip = *static_cast<const ClipStrokeInfo *>(info);
                CFStringAppendFormat(result, nullptr, CFSTR(" %s ["), unsigned(clip.cap) < 7 ? caps[clip.cap] : "unknown");
                if (clip.keys) for (size_t i = 0; i < clip.count; ++i) {
                    const auto &key = clip.keys[i];
                    CFStringAppendFormat(result, nullptr, CFSTR("%s%s%s%g:%g,%g"), i ? ", " : "", key.tag & 1 ? "^" : "", key.tag & 2 ? "*" : "", double(key.value), double(key.start), double(key.end));
                }
                CFStringAppend(result, CFSTR("]"));
                break;
            }
        }
        return true;
    });
    CFStringAppend(result, CFSTR("\n"));
    return result;
}

#endif
