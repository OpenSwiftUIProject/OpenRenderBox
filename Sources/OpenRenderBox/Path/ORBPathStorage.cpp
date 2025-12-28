//
//  ORBPathStorage.cpp
//  OpenRenderBox

#if OPENRENDERBOX_CF_CGTYPE

#include <OpenRenderBox/ORBPathStorage.h>
#include <OpenRenderBoxCxx/Path/Storage.hpp>
#include <OpenRenderBoxCxx/Util/assert.hpp>

using namespace ORB;

namespace ORB {
namespace Path {
namespace {
bool append_element_callback(void * info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo) {
    reinterpret_cast<Storage *>(info)->append_element(element, points, userInfo);
    return true;
}
} /* anonymous namespace */
} /* Path */
} /* ORB */

void ORBPathStorageInit(ORBPathStorageRef dst, uint32_t capacity, ORBPathStorageRef source) {
    if (source != nullptr) {
        dst->storage = ORB::Path::Storage(capacity, source->storage);
    } else {
        dst->storage = ORB::Path::Storage(capacity);
    }
}

void ORBPathStorageDestroy(ORBPathStorageRef storage) {
    storage->storage.~Storage();
}

void ORBPathStorageClear(ORBPathStorageRef storage) {
    storage->storage.clear();
}

bool ORBPathStorageAppendElement(ORBPathStorageRef storage, ORBPathElement element, const CGFloat * points, const void * userInfo) {
    return storage->storage.append_element(element, points, userInfo);
}

bool ORBPathStorageAppendPath(ORBPathStorageRef storage, ORBPath path) {
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

uint32_t ORBPathStorageGetBezierOrder(ORBPathStorageRef storage) {
    return storage->storage.flags().bezierOrder();
}

#if ORB_TARGET_OS_DARWIN

CGRect ORBPathStorageGetBoundingRect(ORBPathStorageRef storage) {
    precondition_failure("TODO");
}

CGPathRef ORBPathStorageGetCGPath(ORBPathStorageRef storage) {
    // FIXME
    return storage->storage.cgpath();
}
#endif

#endif /* OPENRENDERBOX_CF_CGTYPE */
