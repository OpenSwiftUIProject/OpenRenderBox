//
//  ORBPathStorage.cpp
//  OpenRenderBox

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBPathStorage.h>
#include "Storage.hpp"
#include <Util/assert.hpp>
#include <new>

using namespace ORB;

namespace ORB {
namespace Path {
bool append_element_callback(void * info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo) {
    reinterpret_cast<Storage *>(info)->append_element(element, points, userInfo);
    return true;
}
} /* Path */
} /* ORB */

void ORBPathStorageInit(ORBPathStorageRef dst, size_t capacity, ORBPathStorageRef source) {
    if (source != nullptr) {
        new (dst) ORBPathStorage{ORB::Path::Storage(capacity, source->storage)};
    } else {
        new (dst) ORBPathStorage{ORB::Path::Storage(capacity)};
    }
}

void ORBPathStorageDestroy(ORBPathStorageRef storage) {
    storage->storage.~Storage();
}

void ORBPathStorageClear(ORBPathStorageRef storage) {
    storage->storage.clear();
}

void ORBPathStorageAppendElement(ORBPathStorageRef storage, ORBPathElement element, const CGFloat * points, const void * userInfo) {
    storage->storage.append_element(element, points, userInfo);
}

bool ORBPathStorageAppendPath(ORBPathStorageRef storage, ORBPath path) {
    if (path.storage == storage) {
        ORB::Path::Storage snapshot(sizeof(ORB::Path::Storage));
        if (!ORBPathApplyElements(path, &snapshot, ORB::Path::append_element_callback)) {
            return false;
        }
        storage->storage.append_storage(snapshot);
        return true;
    }
    return ORBPathApplyElements(path, storage, ORB::Path::append_element_callback);
}

bool ORBPathStorageApplyElements(ORBPathStorageRef storage, void *info, ORBPathApplyCallback callback) {
    return storage->storage.apply_elements(info, callback);
}

bool ORBPathStorageIsEmpty(ORBPathStorageRef storage) {
    return storage->storage.isEmpty();
}

bool ORBPathStorageEqualToStorage(ORBPathStorageRef lhs, ORBPathStorageRef rhs) {
    return lhs->storage == rhs->storage;
}

bool ORBPathStorageIsSingleElement(ORBPathStorageRef storage) {
    return storage->storage.flags().isSingleElement();
}

size_t ORBPathStorageGetBezierOrder(ORBPathStorageRef storage) {
    return storage->storage.flags().bezierOrder();
}

#if ORB_TARGET_OS_DARWIN

CGRect ORBPathStorageGetBoundingRect(ORBPathStorageRef storage) {
    precondition_failure("TODO");
}

CGPathRef ORBPathStorageGetCGPath(ORBPathStorageRef storage) {
    return storage->storage.cgpath();
}
#endif

#endif /* OPENRENDERBOX_CF_CGTYPES */
