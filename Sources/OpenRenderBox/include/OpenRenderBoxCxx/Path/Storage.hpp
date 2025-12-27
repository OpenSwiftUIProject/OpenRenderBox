//
//  Storage.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBPath.h>
#include <OpenRenderBoxCxx/Util/assert.hpp>
#include <stdatomic.h>
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
        return StorageFlags((value & 0xFFFFFFu) | ((capacity & 0xFFFu) << 0x14));
    }

    ORB_INLINE ORB_CONSTEXPR
    uint32_t getIntValue() const ORB_NOEXCEPT {
        return value;
    }
}; /* StorageFlags */

class Storage {
public:
    ORB_INLINE ORB_CONSTEXPR
    const static atomic_uint& last_identifier() ORB_NOEXCEPT {
        return _last_identifier;
    }

    ORB_INLINE
    const static uint32_t get_identifier() ORB_NOEXCEPT {
        return atomic_fetch_add_explicit(&_last_identifier, 1, memory_order_relaxed);
    }

private:
    static atomic_uint _last_identifier;

public:
    Storage(uint32_t capacity);
    Storage(uint32_t capacity, const ORB::Path::Storage &storage);
    ~Storage();

    bool operator == (const Storage &other) const ORB_NOEXCEPT;

    void clear();

    bool append_element(ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo);
//    push_values(unsigned char, double const*, unsigned long)
    // update_single_element()

    /// Apply callback to each path element (with fast-path checks)
    bool apply_elements(void *info, ORBPathApplyCallback callback) const ORB_NOEXCEPT;

private:
    /// Core element iteration (no fast-path checks)
    bool apply_elements_(void *info, ORBPathApplyCallback callback) const ORB_NOEXCEPT;

public:

    #if ORB_TARGET_OS_DARWIN
    /// Get cached CGPath, lazily creating if needed (thread-safe with casal)
    CGPathRef _Nullable cgpath() const ORB_NOEXCEPT;
    #endif
public:
    ORB_INLINE ORB_CONSTEXPR
    void * unknown() const ORB_NOEXCEPT {
        return _unknown;
    }

    ORB_INLINE ORB_CONSTEXPR
    StorageFlags flags() const ORB_NOEXCEPT {
        return _flags;
    }

    ORB_INLINE ORB_CONSTEXPR
    uint32_t identifier() const ORB_NOEXCEPT {
        return _identifier;
    }

    ORB_INLINE ORB_CONSTEXPR
    const Storage* external_storage() const ORB_NOEXCEPT {
        return _external_storage;
    }

    ORB_INLINE ORB_CONSTEXPR
    uint64_t external_size() const ORB_NOEXCEPT {
        return _external_size;
    }

    ORB_INLINE ORB_CONSTEXPR
    const Storage* actual_storage() const ORB_NOEXCEPT {
        if (flags().isInline()) {
            return this;
        } else {
            return external_storage();
        }
    }

    ORB_INLINE ORB_CONSTEXPR
    uint64_t actual_size() const ORB_NOEXCEPT {
        if (flags().isInline()) {
            return flags().size();
        } else {
            return external_size();
        }
    }

    ORB_INLINE ORB_CONSTEXPR
    bool isEmpty() const ORB_NOEXCEPT {
        return actual_size() == 0;
    }

    #if ORB_TARGET_OS_DARWIN
    ORB_INLINE
    CGPathRef cachedCGPath() const ORB_NOEXCEPT {
        return _cached_cgPath;
    }
    #endif
private:
    void * _unknown;                    // 0x00
    StorageFlags _flags;                // 0x08
    uint32_t _identifier;               // 0x0C
    Storage* _external_storage;         // 0x10
    uint64_t _external_size;            // 0x18
    void * _reserved1;                  // 0x20
    void * _reserved2;                  // 0x28
    void * _reserved3;                  // 0x30
    #if ORB_TARGET_OS_DARWIN
    mutable CGPathRef _cached_cgPath;      // 0x38 - lazily cached CGPath
    #endif
};
} /* Path */
} /* ORB */

struct ORBPathStorage {
    ORB::Path::Storage storage;
};

ORB_ASSUME_NONNULL_END

