//
//  Storage.cpp
//  OpenRenderBox

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBPathCallbacks.h>
#include "NestedCallbacks.hpp"
#include "Storage.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#if ORB_TARGET_OS_DARWIN
#include <malloc/malloc.h>
#endif

const uint8_t __ORBPathElementArgumentCount[25] = {
    2, 2, 4, 6, 0, 4, 4, 4, 6, 6, 8, 8, 5, 5, 5, 5, 0, 0, 2, 3, 0, 0, 2, 1, 1,
};

namespace ORB {
namespace Path {
namespace {
constexpr uint8_t elementFlags[] = {
    0, 0, 2, 4, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 32, 0, 32, 0, 32, 36, 32,
};

enum class InfoKind { none, affine, stroke, boolean, clipStroke };

InfoKind info_kind(ORBPathElement element) {
    switch (static_cast<int>(element)) {
        case 4: case 16: case 18: case 22: case 23: return InfoKind::none;
        case 19: return InfoKind::stroke;
        case 20: return InfoKind::boolean;
        case 24: return InfoKind::clipStroke;
        default: return InfoKind::affine;
    }
}

size_t scalar_size(uint8_t encoding) {
    return encoding & 0x40 ? sizeof(float) : encoding & 0x20 ? sizeof(int16_t) : sizeof(CGFloat);
}

void check_values(uint8_t &encoding, const CGFloat *values, size_t count) {
    for (size_t i = 0; encoding && i < count; ++i) {
        CGFloat magnitude = std::abs(values[i]);
        if (!(magnitude <= 32767)) {
            encoding = 0;
        } else if ((encoding & 0x20) && std::abs(std::floor(magnitude + 0.5) - magnitude) > static_cast<double>(1e-5f)) {
            encoding &= ~0x20;
        }
    }
}

template<typename T>
T read_value(const uint8_t *bytes, size_t &offset) {
    T value;
    std::memcpy(&value, bytes + offset, sizeof(T));
    offset += sizeof(T);
    return value;
}

void read_values(const uint8_t *bytes, size_t &offset, uint8_t encoding, CGFloat *values, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        values[i] = encoding & 0x40 ? read_value<float>(bytes, offset)
            : encoding & 0x20 ? read_value<int16_t>(bytes, offset)
            : read_value<CGFloat>(bytes, offset);
    }
}

#if ORB_TARGET_OS_DARWIN
const ORBPathCallbacks storageCallbacks = {
    .flags = {},
    .retain = nullptr,
    .release = nullptr,
    .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) {
        return static_cast<const Storage *>(object)->apply_elements(info, callback);
    },
    .isEqual = nullptr,
    .isEmpty = nullptr,
    .isSingleElement = +[](const void *object) {
        return static_cast<const Storage *>(object)->flags().isSingleElement();
    },
    .bezierOrder = nullptr,
    .boundingRect = nullptr,
    .cgPath = nullptr,
    .next = nullptr,
};
#endif

} // namespace

std::atomic_uint Storage::_last_identifier;

size_t Storage::argument_count(ORBPathElement element) ORB_NOEXCEPT {
    auto index = static_cast<unsigned>(element);
    return index < sizeof(__ORBPathElementArgumentCount) ? __ORBPathElementArgumentCount[index] : 0;
}

Storage::Storage(size_t capacity) : _bounds(0), _flags(), _identifier(get_identifier()) {
    if (capacity < 64) {
        precondition_failure("invalid capacity");
    }
    _flags = _flags.withCapacity(static_cast<uint32_t>(std::min<size_t>(capacity - 16, 4095)));
}

Storage::Storage(size_t capacity, const Storage &storage) : Storage(capacity) {
    _flags = StorageFlags((_flags.value & ~0x3eu) | (storage._flags.value & 0x3eu));
    push_bytes(storage.actual_storage(), storage.actual_size());
}

Storage::~Storage() {
    invalidate();
    if (_flags.isExternal()) {
        std::free(_external_storage);
    }
}

const uint8_t *Storage::actual_storage() const ORB_NOEXCEPT {
    return _flags.isExternal() ? _external_storage : reinterpret_cast<const uint8_t *>(this) + 16;
}

uint8_t *Storage::bytes() ORB_NOEXCEPT {
    return const_cast<uint8_t *>(actual_storage());
}

size_t Storage::actual_size() const ORB_NOEXCEPT {
    return _flags.isExternal() ? _external_size : _flags.size();
}

size_t Storage::capacity() const ORB_NOEXCEPT {
    return _flags.isExternal() ? _external_capacity : _flags.capacity();
}

void Storage::set_size(size_t size) ORB_NOEXCEPT {
    if (_flags.isExternal()) {
        _external_size = size;
    } else {
        _flags = _flags.withSize(static_cast<uint32_t>(size));
    }
}

void Storage::reserve_slow(size_t size) {
    size_t currentCapacity = capacity();
    size_t doubled = currentCapacity <= std::numeric_limits<size_t>::max() / 2 ? currentCapacity * 2 : size;
    size_t newCapacity = std::max(doubled, size);
    #if ORB_TARGET_OS_DARWIN
    newCapacity = malloc_good_size(newCapacity);
    #endif
    if (_flags.isExternal()) {
        auto *newBytes = static_cast<uint8_t *>(std::realloc(_external_storage, newCapacity));
        if (!newBytes) {
            precondition_failure("failed to allocate path storage");
        }
        _external_storage = newBytes;
        _external_capacity = newCapacity;
    } else {
        auto *newBytes = static_cast<uint8_t *>(std::malloc(newCapacity));
        if (!newBytes) {
            precondition_failure("failed to allocate path storage");
        }
        size_t size = actual_size();
        std::memcpy(newBytes, actual_storage(), size);
        _flags = _flags.withExternal(true);
        _external_storage = newBytes;
        _external_size = size;
        _external_capacity = newCapacity;
        #if ORB_TARGET_OS_DARWIN
        _cached_cgPath = nullptr;
        #endif
    }
}

void Storage::push_bytes(const void *values, size_t count) {
    size_t oldSize = actual_size();
    if (count > std::numeric_limits<size_t>::max() - oldSize) {
        precondition_failure("path storage size overflow");
    }
    size_t size = oldSize + count;
    if (size > capacity()) {
        reserve_slow(size);
    }
    if (count) {
        std::memcpy(bytes() + oldSize, values, count);
    }
    set_size(size);
}

void Storage::push_values(uint8_t encoding, const CGFloat *values, size_t count) {
    if (!(encoding & 0x60)) {
        push_bytes(values, count * sizeof(CGFloat));
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        if (encoding & 0x40) {
            float value = static_cast<float>(values[i]);
            push_bytes(&value, sizeof(value));
        } else {
            int16_t value = static_cast<int16_t>(std::floor(values[i] + 0.5));
            push_bytes(&value, sizeof(value));
        }
    }
}

void Storage::invalidate() {
    uintptr_t bounds = __atomic_exchange_n(&_bounds, 0, __ATOMIC_ACQ_REL);
    if (bounds & 1) {
        delete[] reinterpret_cast<float *>(bounds & ~uintptr_t(3));
    }
    #if ORB_TARGET_OS_DARWIN
    if (_flags.isExternal()) {
        CGPathRef path = __atomic_exchange_n(&_cached_cgPath, nullptr, __ATOMIC_ACQ_REL);
        if (path) {
            CGPathRelease(path);
        }
    }
    #endif
}

bool Storage::operator==(const Storage &other) const ORB_NOEXCEPT {
    size_t size = actual_size();
    return size == other.actual_size()
        && ((_flags.value ^ other._flags.value) & 0x3e) == 0
        && std::memcmp(actual_storage(), other.actual_storage(), size) == 0;
}

void Storage::clear() {
    invalidate();
    set_size(0);
    _flags = StorageFlags(_flags.value & ~0x3eu);
}

bool Storage::append_element(ORBPathElement element, const CGFloat *points, const void *userInfo) {
    auto type = static_cast<unsigned>(element);
    if (type >= sizeof(__ORBPathElementArgumentCount)) {
        precondition_failure("invalid path element: %d", element);
    }
    if ((type == 8 || type == 9) && points[4] == 0 && points[5] == 0) {
        type = 6;
    } else if ((type == 10 || type == 11) && points[4] == 0 && points[5] == 0 && points[6] == 0 && points[7] == 0) {
        type = 6;
    }
    element = static_cast<ORBPathElement>(type);
    InfoKind kind = userInfo ? info_kind(element) : InfoKind::none;
    if (kind == InfoKind::affine) {
        auto *transform = static_cast<const CGFloat *>(userInfo);
        if (transform[0] == 1 && transform[1] == 0 && transform[2] == 0 && transform[3] == 1 && transform[4] == 0 && transform[5] == 0) {
            kind = InfoKind::none;
        }
    }
    uint8_t encoding = 0x60;
    check_values(encoding, points, __ORBPathElementArgumentCount[type]);
    if (kind == InfoKind::affine) {
        check_values(encoding, static_cast<const CGFloat *>(userInfo), 6);
    } else if (kind == InfoKind::stroke) {
        auto &stroke = *static_cast<const StrokeInfo *>(userInfo);
        check_values(encoding, static_cast<const CGFloat *>(stroke.values), std::min<size_t>(stroke.count, 4096));
    }
    encoding = encoding & 0x20 ? 0x20 : encoding & 0x40;
    encoding |= static_cast<uint8_t>(type) | (kind == InfoKind::none ? 0 : 0x80);

    invalidate();
    _flags = _flags.withSingleElement(isEmpty());
    _flags = StorageFlags(_flags.value | elementFlags[type]);
    if (kind == InfoKind::stroke || kind == InfoKind::clipStroke) {
        auto &stroke = *static_cast<const StrokeInfo *>(userInfo);
        if (stroke.join == 1 || (kind == InfoKind::stroke && stroke.cap == 1)) {
            _flags = _flags.withCubicBezierOrder(true);
        }
    }
    push_bytes(&encoding, 1);
    push_values(encoding, points, __ORBPathElementArgumentCount[type]);
    switch (kind) {
        case InfoKind::none:
            break;
        case InfoKind::affine:
            push_values(encoding, static_cast<const CGFloat *>(userInfo), 6);
            break;
        case InfoKind::boolean:
            push_bytes(userInfo, 2);
            break;
        case InfoKind::stroke:
        case InfoKind::clipStroke: {
            auto &stroke = *static_cast<const StrokeInfo *>(userInfo);
            uint8_t cap = static_cast<uint8_t>(stroke.cap);
            uint8_t join = static_cast<uint8_t>(stroke.join);
            uint16_t count = static_cast<uint16_t>(std::min<size_t>(stroke.count, kind == InfoKind::stroke ? 4096 : 2048));
            push_bytes(&cap, 1);
            push_bytes(&join, 1);
            push_bytes(&count, 2);
            if (kind == InfoKind::stroke) {
                push_values(encoding, static_cast<const CGFloat *>(stroke.values), count);
            } else {
                auto *records = static_cast<const StrokeRecord *>(stroke.values);
                for (size_t i = 0; i < count; ++i) {
                    uint8_t type = static_cast<uint8_t>(records[i].type);
                    push_bytes(&type, 1);
                    push_bytes(records[i].values, sizeof(records[i].values));
                }
            }
            break;
        }
    }
    if (type == 4 || type == 16) {
        update_single_element();
    }
    return true;
}

void Storage::append_storage(const Storage &storage) {
    if (storage.isEmpty()) {
        return;
    }
    StorageFlags sourceFlags = storage._flags;
    std::vector<uint8_t> copy;
    const uint8_t *source = storage.actual_storage();
    size_t count = storage.actual_size();
    if (this == &storage) {
        copy.assign(source, source + count);
        source = copy.data();
    }
    invalidate();
    _flags = _flags.withSingleElement(isEmpty() && sourceFlags.isSingleElement());
    _flags = StorageFlags(_flags.value | (sourceFlags.value & 0x26));
    push_bytes(source, count);
}

void Storage::update_single_element() {
    if (_flags.value & 0x10) {
        return;
    }
    NestedCallbacks::SingleInfo info{};
    NestedCallbacks::single_element_fast(*this, &info);
    _flags = StorageFlags(_flags.value & ~0x18u);
    if (info.currentDepth == 0 && info.count != 0) {
        _flags = StorageFlags(_flags.value | (info.count == 1 ? 8 : 16));
    }
}

bool Storage::Iterator::next(const Storage &storage, Element &element) {
    if (offset >= storage.actual_size()) {
        return false;
    }
    const uint8_t *bytes = storage.actual_storage();
    uint8_t encoding = read_value<uint8_t>(bytes, offset);
    element.type = static_cast<ORBPathElement>(encoding & 0x1f);
    read_values(bytes, offset, encoding, element.points, argument_count(element.type));
    InfoKind kind = encoding & 0x80 ? info_kind(element.type) : InfoKind::none;
    element.hasInfo = kind != InfoKind::none;
    switch (kind) {
        case InfoKind::none:
            break;
        case InfoKind::affine:
            read_values(bytes, offset, encoding, element.metadata.transform, 6);
            break;
        case InfoKind::boolean:
            std::memcpy(element.metadata.boolean, bytes + offset, 2);
            offset += 2;
            break;
        case InfoKind::stroke:
        case InfoKind::clipStroke: {
            auto &stroke = element.metadata.stroke;
            stroke.cap = read_value<uint8_t>(bytes, offset);
            stroke.join = read_value<uint8_t>(bytes, offset);
            stroke.count = read_value<uint16_t>(bytes, offset);
            if (kind == InfoKind::stroke) {
                element.values.resize(stroke.count);
                read_values(bytes, offset, encoding, element.values.data(), stroke.count);
                stroke.values = element.values.data();
            } else {
                element.records.resize(stroke.count);
                for (auto &record : element.records) {
                    record.type = read_value<uint8_t>(bytes, offset);
                    std::memcpy(record.values, bytes + offset, sizeof(record.values));
                    offset += sizeof(record.values);
                }
                stroke.values = element.records.data();
            }
            break;
        }
    }
    return true;
}

bool Storage::Iterator::next_type(const Storage &storage, ORBPathElement &element) {
    if (offset >= storage.actual_size()) {
        return false;
    }
    const uint8_t *bytes = storage.actual_storage();
    uint8_t encoding = read_value<uint8_t>(bytes, offset);
    element = static_cast<ORBPathElement>(encoding & 0x1f);
    size_t width = scalar_size(encoding);
    offset += argument_count(element) * width;
    if (encoding & 0x80) {
        switch (info_kind(element)) {
            case InfoKind::none: break;
            case InfoKind::affine: offset += 6 * width; break;
            case InfoKind::boolean: offset += 2; break;
            case InfoKind::stroke:
            case InfoKind::clipStroke: {
                offset += 2;
                size_t count = read_value<uint16_t>(bytes, offset);
                offset += count * (static_cast<unsigned>(element) == 19 ? width : 13);
                break;
            }
        }
    }
    return true;
}

bool Storage::apply_elements(void *info, ORBPathApplyCallback callback) const ORB_NOEXCEPT {
    if (!callback) {
        return true;
    }
    if (callback == append_element_callback) {
        static_cast<Storage *>(info)->append_storage(*this);
        return true;
    }
    if (callback == NestedCallbacks::apply_elements_callback) {
        return NestedCallbacks::apply_elements_fast(*this, info);
    }
    if (callback == NestedCallbacks::single_element_callback) {
        return NestedCallbacks::single_element_fast(*this, info);
    }
    return apply_elements_(info, callback);
}

bool Storage::apply_elements_(void *info, ORBPathApplyCallback callback) const ORB_NOEXCEPT {
    Iterator iterator;
    Element element;
    while (iterator.next(*this, element)) {
        if (!callback(info, element.type, element.points, element.info())) {
            return false;
        }
    }
    return true;
}

bool Storage::apply_elements_at_depth(void *info, ORBPathApplyCallback callback, size_t depth) const ORB_NOEXCEPT {
    if (depth == 0) {
        return apply_elements_(info, callback);
    }
    Iterator iterator;
    if (!NestedCallbacks::first_element(*this, iterator, depth)) {
        return false;
    }
    Element element;
    size_t currentDepth = depth;
    while (iterator.next(*this, element)) {
        if (NestedCallbacks::is_nesting_element(element.type)) {
            ++currentDepth;
        } else if (static_cast<unsigned>(element.type) == 16) {
            if (currentDepth == depth) {
                return true;
            }
            --currentDepth;
        }
        if (!callback(info, element.type, element.points, element.info())) {
            return false;
        }
    }
    return true;
}

#if ORB_TARGET_OS_DARWIN


CGPathRef Storage::cgpath() const ORB_NOEXCEPT {
    if (_flags.isInline()) {
        return nullptr;
    }
    CGPathRef cached = __atomic_load_n(&_cached_cgPath, __ATOMIC_ACQUIRE);
    if (cached) {
        return cached;
    }
    ORBPath path{reinterpret_cast<ORBPathStorageRef>(const_cast<Storage *>(this)), &storageCallbacks};
    CGPathRef newPath = ORBPathCopyCGPath(path);
    CGPathRef expected = nullptr;
    if (__atomic_compare_exchange_n(&_cached_cgPath, &expected, newPath, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        return newPath;
    }
    CGPathRelease(newPath);
    return expected;
}
#endif

} /* Path */
} /* ORB */

#endif /* OPENRENDERBOX_CF_CGTYPES */
