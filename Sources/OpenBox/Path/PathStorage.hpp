//
//  PathStorage.hpp
//  OpenBox

#ifndef PathStorage_hpp
#define PathStorage_hpp

#include "OBBase.h"
#include "../Util/assert.hpp"
#include <stdatomic.h>

OB_ASSUME_NONNULL_BEGIN

namespace OB {
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

    OB_INLINE OB_CONSTEXPR
    explicit StorageFlags(uint32_t value): value(value) {}
public:
    OB_INLINE OB_CONSTEXPR
    StorageFlags(): value(0) {}

    OB_INLINE OB_CONSTEXPR
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

    OB_INLINE OB_CONSTEXPR
    bool isInline() const OB_NOEXCEPT {
        return (value & 1u) == 0;
    }

    OB_INLINE OB_CONSTEXPR
    bool isExternal() const OB_NOEXCEPT {
        return (value & 1u) != 0;
    }

    OB_INLINE OB_CONSTEXPR
    bool isQuadBezierOrder() const OB_NOEXCEPT {
        return (value & 2u) != 0;
    }

    OB_INLINE OB_CONSTEXPR
    bool isCubicBezierOrder() const OB_NOEXCEPT {
        return (value & 4u) != 0;
    }

    OB_INLINE OB_CONSTEXPR
    uint32_t bezierOrder() const OB_NOEXCEPT {
        bool isQuad = (value & 2u) != 0;
        bool isCubic = (value & 4u) != 0;
        return isCubic ? 3u : (isQuad ? 2u : 1u);
    }

    OB_INLINE OB_CONSTEXPR
    bool isSingleElement() const OB_NOEXCEPT {
        return (value & 8u) != 0;
    }

    OB_INLINE OB_CONSTEXPR
    uint32_t size() const OB_NOEXCEPT {
        return (value >> 8u) & 0xFFFu;
    }

    OB_INLINE OB_CONSTEXPR
    uint32_t capacity() const OB_NOEXCEPT {
        return (value >> 0x14);
    }

    OB_INLINE OB_CONSTEXPR
    StorageFlags withExternal(bool isExternal) const OB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFFEu) | (isExternal ? 0x1u : 0));
    }

    OB_INLINE OB_CONSTEXPR
    StorageFlags withQuadBezierOrder(bool isQuadBezierOrder) const OB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFFDu) | (isQuadBezierOrder ? 0x2u : 0));
    }

    OB_INLINE OB_CONSTEXPR
    StorageFlags withCubicBezierOrder(bool isCubicBezierOrder) const OB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFFBu) | (isCubicBezierOrder ? 0x4u : 0));
    }

    OB_INLINE OB_CONSTEXPR
    StorageFlags withSingleElement(bool isSingleElement) const OB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFF7u) | (isSingleElement ? 0x8u : 0));
    }

    OB_INLINE OB_CONSTEXPR
    StorageFlags withSize(uint32_t size) const OB_NOEXCEPT {
        return StorageFlags((value & 0xFFF000FFu) | ((size & 0xFFFu) << 8));
    }

    OB_INLINE OB_CONSTEXPR
    StorageFlags withCapacity(uint32_t capacity) const OB_NOEXCEPT {
        return StorageFlags((value & 0xFFFFFFu) | ((capacity & 0xFFFu) << 0x14));
    }

    OB_INLINE OB_CONSTEXPR
    uint32_t getIntValue() const OB_NOEXCEPT {
        return value;
    }
}; /* StorageFlags */

class Storage {
public:
    OB_INLINE OB_CONSTEXPR
    const static atomic_long& last_identifier() OB_NOEXCEPT {
        return _last_identifier;
    }

    OB_INLINE
    const static uint32_t get_identifier() OB_NOEXCEPT {
        _last_identifier++;
        return _last_identifier;
    }

private:
    static atomic_long _last_identifier;

public:
    Storage(uint32_t capacity) {
        _unknonw = nullptr;
        if (capacity <= 63) {
            precondition_failure("invalid capacity");
        }
        _flags = StorageFlags().withCapacity(capacity - 10);
        _identifier = get_identifier();
    }

    
    Storage(uint32_t capacity, const OB::Path::Storage &storage);
    ~Storage();

    bool operator == (const Storage &other) const OB_NOEXCEPT;

    void clear();

//    void append_element(RBPathElement, double const*, void const*);
//    push_values(unsigned char, double const*, unsigned long)
    // update_single_element()
public:
    OB_INLINE OB_CONSTEXPR
    void * unknown() const OB_NOEXCEPT {
        return _unknonw;
    }

    OB_INLINE OB_CONSTEXPR
    StorageFlags flags() const OB_NOEXCEPT {
        return _flags;
    }

    OB_INLINE OB_CONSTEXPR
    uint32_t identifier() const OB_NOEXCEPT {
        return _identifier;
    }

    OB_INLINE OB_CONSTEXPR
    const Storage* external_storage() const OB_NOEXCEPT {
        return _external_storage;
    }

    OB_INLINE OB_CONSTEXPR
    uint64_t external_size() const OB_NOEXCEPT {
        return _external_size;
    }

    OB_INLINE OB_CONSTEXPR
    const Storage* actual_storage() const OB_NOEXCEPT {
        if (flags().isInline()) {
            return this;
        } else {
            return external_storage();
        }
    }

    OB_INLINE OB_CONSTEXPR
    uint64_t actual_size() const OB_NOEXCEPT {
        if (flags().isInline()) {
            return flags().size();
        } else {
            return external_size();
        }
    }

    OB_INLINE OB_CONSTEXPR
    bool isEmpty() const OB_NOEXCEPT {
        return actual_size() == 0;
    }
private:
    void * _unknonw;
    StorageFlags _flags;
    uint32_t _identifier;
    Storage* _external_storage;
    uint64_t _external_size;
};
} /* Path */
} /* OB */

struct OBPathStorage {
    OB::Path::Storage storage;
};

OB_ASSUME_NONNULL_END

#endif /* PathStorage_hpp */
