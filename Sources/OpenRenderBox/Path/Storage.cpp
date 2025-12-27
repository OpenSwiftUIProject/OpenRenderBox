//
//  Storage.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBPath.h>
#include <OpenRenderBox/ORBPathCallbacks.h>
#include <OpenRenderBoxCxx/Path/Storage.hpp>
#include <OpenRenderBoxCxx/Util/assert.hpp>

#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace ORB {
namespace Path {
atomic_uint Storage::_last_identifier;

Storage::Storage(uint32_t capacity) {
    _unknown = nullptr;
    if (capacity <= 63) {
        precondition_failure("invalid capacity");
    }
    uint32_t cappedCapacity = fmin(capacity - 16, 4095);
    _flags = StorageFlags().withCapacity(cappedCapacity);
    _identifier = get_identifier();
}

Storage::Storage(uint32_t capacity, const Storage &storage): Storage(capacity) {
    uint32_t originalCapity = flags().capacity();
    StorageFlags sourceFlags = storage.flags();

    _flags = _flags
        .withExternal(false)
        .withQuadBezierOrder(sourceFlags.isQuadBezierOrder())
        .withCubicBezierOrder(sourceFlags.isCubicBezierOrder())
        .withSingleElement(sourceFlags.isSingleElement())
        // bit 4 and 5 is from sourceFlags
        // bit 6 and bit 7 is zero
        .withCapacity(sourceFlags.capacity());

    const Storage* sourceStorage = storage.actual_storage();
    uint64_t sourceSize = storage.actual_size();

    if (originalCapity < sourceSize) {
        // reserve_slow(sourceSize);
        sourceStorage = external_storage();
        if (flags().isExternal()) {
            // sourceStorage = external_storage()->0;
        }
    }
    memcpy((void *)external_storage(), (const void *)sourceStorage, sourceSize);
    //    // Update our capacity info with the size
    //    if (!(flags() & 0x1)) {
    //        // Inline storage - update size bits
    //        _flags = (flags() & ~(0xFFF << 8)) | (sourceSize << 8);
    //    } else {
    //        // External storage - store size externally
    //        *reinterpret_cast<uint64_t*>(this + 0x18) = sourceSize;
    //    }
}

Storage::~Storage() {
    if (_unknown != nullptr) {
        auto oldValue = _unknown;
        _unknown = nullptr;
        // TODO
    }
    // TODO: MapCache
    if (flags().isExternal()) {
        #if ORB_TARGET_OS_DARWIN
        if (cachedCGPath() != nullptr) {
            auto oldCache = cachedCGPath();
            _cached_cgPath = nullptr;
            CFRelease(oldCache);
        }
        #endif
        free((void *)external_storage());
    }
}

bool Storage::operator==(const Storage &other) const ORB_NOEXCEPT {
    // TODO
    return false;
}

void Storage::clear() {
    // TODO
}

bool Storage::append_element(ORBPathElement element, const CGFloat *points, const void *info) {
    if (element >= ORBPathElementInvalid) {
        precondition_failure("invalid path element: %d", element);
    }
    // TODO: Implement element appending
    precondition_failure("TODO");
}

bool Storage::apply_elements(void *info, ORBPathApplyCallback callback) const ORB_NOEXCEPT {
    // TODO: Add fast-path checks for special callbacks:
    // - append_element_callback → append_storage(info, this)
    // - NestedCallbacks::apply_elements_callback → apply_elements_fast
    // - NestedCallbacks::single_element_callback → single_element_fast
    // - Mapper::apply_callback with flags check → MapCache::apply
    return apply_elements_(info, callback);
}

bool Storage::apply_elements_(void *info, ORBPathApplyCallback callback) const ORB_NOEXCEPT {
    // TODO: Implement actual element iteration over storage
    return true;
}

#if ORB_TARGET_OS_DARWIN
CGPathRef Storage::cgpath() const ORB_NOEXCEPT {
    if (flags().isInline()) {
        return nullptr;
    }
    CGPathRef cached = __atomic_load_n(&_cached_cgPath, __ATOMIC_SEQ_CST);
    if (cached != nullptr) {
        return cached;
    }
    static const ORBPathCallbacks callbacks = {
        {},
        nullptr,
        nullptr,
        .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) -> bool {
            auto storage = reinterpret_cast<const ORB::Path::Storage *>(object);
            return storage->apply_elements(info, callback);
        },
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
    };
    ORBPath path = {
        const_cast<ORBPathStorage *>(reinterpret_cast<const ORBPathStorage *>(this)),
        &callbacks
    };
    CGPathRef new_path = ORBPathCopyCGPath(path);
    CGPathRef expected = nullptr;
    if (__atomic_compare_exchange_n(&_cached_cgPath, &expected, new_path,
                                     false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        return new_path;
    } else {
        CGPathRelease(new_path);
        return expected;
    }
}
#endif

} /* Path */
} /* ORB */
