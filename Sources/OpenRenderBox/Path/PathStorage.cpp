//
//  PathStorage.cpp
//  OpenRenderBox

#include <OpenRenderBoxCxx/Path/PathStorage.hpp>
#include <OpenRenderBoxCxx/Util/assert.hpp>

#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace ORB {
namespace Path {
atomic_long Storage::_last_identifier;

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
        _unknown = nullptr;
        // TODO
    }
}

bool Storage::operator==(const Storage &other) const ORB_NOEXCEPT {
    // TODO
    return false;
}

void Storage::clear() {
    // TODO
}

void * Storage::cgpath() const ORB_NOEXCEPT {
    if (_flags.isInline()) {
        return nullptr;
    }
    if (_cached_cgPath != nullptr) {
        return _cached_cgPath;
    }
    // TODO: Create CGPath via RBPathCopyCGPath
    return nullptr;
}

} /* Path */
} /* ORB */
