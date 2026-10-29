//
//  Storage.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBPath.h>
#include "../Util/assert.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>
#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

namespace ORB {
namespace Path {

struct StorageFlags {
private:
    /// Bits:
    /// 0 -> isExternal
    /// 1 -> isQuadBezierOrder
    /// 2 -> isCubicBezierOrder
    /// 3 -> isSingleElement
    /// [8, 20) -> size
    /// [20, 32) -> capacity
    friend class Storage;
    uint32_t value;

    ORB_INLINE ORB_CONSTEXPR
    explicit StorageFlags(uint32_t value): value(value) {}
public:
    ORB_INLINE ORB_CONSTEXPR
    StorageFlags(): value(0) {}

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags(bool isExternal,
                 bool isQuadBezierOrder,
                 bool isCubicBezierOrder,
                 bool isSingleElement,
                 uint32_t size,
                 uint32_t capacity)
    : StorageFlags(StorageFlags()
            .withExternal(isExternal)
            .withQuadBezierOrder(isQuadBezierOrder)
            .withCubicBezierOrder(isCubicBezierOrder)
            .withSingleElement(isSingleElement)
            .withSize(size)
            .withCapacity(capacity))
    {}

    ORB_INLINE ORB_CONSTEXPR
    bool isInline() const ORB_NOEXCEPT {
        return (value & 1u) == 0;
    }

    ORB_INLINE ORB_CONSTEXPR
    bool isExternal() const ORB_NOEXCEPT {
        return (value & 1u) != 0;
    }

    ORB_INLINE ORB_CONSTEXPR
    bool isQuadBezierOrder() const ORB_NOEXCEPT {
        return (value & 2u) != 0;
    }

    ORB_INLINE ORB_CONSTEXPR
    bool isCubicBezierOrder() const ORB_NOEXCEPT {
        return (value & 4u) != 0;
    }

    ORB_INLINE ORB_CONSTEXPR
    uint32_t bezierOrder() const ORB_NOEXCEPT {
        bool isQuad = (value & 2u) != 0;
        bool isCubic = (value & 4u) != 0;
        return isCubic ? 3u : (isQuad ? 2u : 1u);
    }

    ORB_INLINE ORB_CONSTEXPR
    bool isSingleElement() const ORB_NOEXCEPT {
        return (value & 8u) != 0;
    }

    ORB_INLINE ORB_CONSTEXPR
    bool requiresMapping() const ORB_NOEXCEPT {
        return (value & 32u) != 0;
    }

    ORB_INLINE ORB_CONSTEXPR
    uint32_t size() const ORB_NOEXCEPT {
        return (value >> 8u) & 0xFFFu;
    }

    ORB_INLINE ORB_CONSTEXPR
    uint32_t capacity() const ORB_NOEXCEPT {
        return (value >> 0x14);
    }

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags withExternal(bool isExternal) const ORB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFFEu) | (isExternal ? 0x1u : 0));
    }

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags withQuadBezierOrder(bool isQuadBezierOrder) const ORB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFFDu) | (isQuadBezierOrder ? 0x2u : 0));
    }

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags withCubicBezierOrder(bool isCubicBezierOrder) const ORB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFFBu) | (isCubicBezierOrder ? 0x4u : 0));
    }

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags withSingleElement(bool isSingleElement) const ORB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFF7u) | (isSingleElement ? 0x8u : 0));
    }

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags withSize(uint32_t size) const ORB_NOEXCEPT {
        return StorageFlags((value & 0xFFF000FFu) | ((size & 0xFFFu) << 8));
    }

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags withCapacity(uint32_t capacity) const ORB_NOEXCEPT {
        return StorageFlags((value & 0x000FFFFFu) | ((capacity & 0xFFFu) << 0x14));
    }

    ORB_INLINE ORB_CONSTEXPR
    uint32_t getIntValue() const ORB_NOEXCEPT {
        return value;
    }
}; /* StorageFlags */

class Storage {
public:
    struct StrokeInfo {
        int32_t cap;
        int32_t join;
        size_t count;
        const void * _Nullable values;
    };

    struct StrokeRecord {
        int32_t type;
        uint8_t values[12];
    };

    struct Element {
        CGFloat points[8] = {};
        ORBPathElement type = ORBPathElementMoveToPoint;
        bool hasInfo = false;
        union {
            CGFloat transform[6];
            StrokeInfo stroke;
            uint8_t boolean[2];
        } metadata = {};
        std::vector<CGFloat> values;
        std::vector<StrokeRecord> records;

        const void * _Nullable info() const ORB_NOEXCEPT {
            return hasInfo ? &metadata : nullptr;
        }
    };

    struct Iterator {
        size_t offset = 0;
        bool next(const Storage &storage, Element &element);
        bool next_type(const Storage &storage, ORBPathElement &element);
    };

    static size_t argument_count(ORBPathElement element) ORB_NOEXCEPT;

    ORB_INLINE ORB_CONSTEXPR
    static const std::atomic_uint &last_identifier() ORB_NOEXCEPT {
        return _last_identifier;
    }

    ORB_INLINE
    static uint32_t get_identifier() ORB_NOEXCEPT {
        return _last_identifier.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    Storage(size_t capacity);
    Storage(size_t capacity, const Storage &storage);
    Storage(const Storage &) = delete;
    Storage &operator=(const Storage &) = delete;
    ~Storage();

    bool operator==(const Storage &other) const ORB_NOEXCEPT;
    void clear();
    bool append_element(ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo);
    void append_storage(const Storage &storage);
    bool apply_elements(void *info, ORBPathApplyCallback _Nullable callback) const ORB_NOEXCEPT;
    bool apply_elements_at_depth(void *info, ORBPathApplyCallback callback, size_t depth) const ORB_NOEXCEPT;

    #if ORB_TARGET_OS_DARWIN
    CGRect bounding_rect() const ORB_NOEXCEPT;
    CGPathRef _Nullable cgpath() const ORB_NOEXCEPT;
    #endif

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags flags() const ORB_NOEXCEPT { return _flags; }

    ORB_INLINE ORB_CONSTEXPR
    uint32_t identifier() const ORB_NOEXCEPT { return _identifier; }

    const uint8_t *actual_storage() const ORB_NOEXCEPT;
    size_t actual_size() const ORB_NOEXCEPT;

    ORB_INLINE
    bool isEmpty() const ORB_NOEXCEPT { return actual_size() == 0; }

private:
    static std::atomic_uint _last_identifier;
    uint8_t *bytes() ORB_NOEXCEPT;
    size_t capacity() const ORB_NOEXCEPT;
    void set_size(size_t size) ORB_NOEXCEPT;
    void reserve_slow(size_t size);
    void push_bytes(const void *values, size_t count);
    void push_values(uint8_t encoding, const CGFloat *values, size_t count);
    void invalidate();
    void update_single_element();
    bool apply_elements_(void *info, ORBPathApplyCallback callback) const ORB_NOEXCEPT;

    mutable uintptr_t _bounds;          // 0x00: tagged bounds cache
    StorageFlags _flags;                // 0x08
    uint32_t _identifier;               // 0x0c
    // Inline bytes start at 0x10 and can extend to the caller's capacity.
    // These fields are live only when flags bit 0 selects external storage.
    uint8_t *_external_storage;         // 0x10
    size_t _external_size;              // 0x18
    size_t _external_capacity;          // 0x20
    mutable float _external_bounds[4];  // 0x28
    #if ORB_TARGET_OS_DARWIN
    mutable CGPathRef _Nullable _cached_cgPath; // 0x38
    #else
    void *_reserved;                    // 0x38
    #endif
};

static_assert(sizeof(Storage) == 64, "Storage header and external overlay must fit 64 bytes");

bool append_element_callback(void *info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo);

} /* Path */
} /* ORB */

struct ORBPathStorage {
    ORB::Path::Storage storage;
};

ORB_ASSUME_NONNULL_END

#endif /* OPENRENDERBOX_CF_CGTYPES */
