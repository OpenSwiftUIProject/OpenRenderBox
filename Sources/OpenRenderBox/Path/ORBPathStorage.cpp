//
//  ORBPathStorage.cpp
//  OpenRenderBox

#include <OpenRenderBox/ORBPathStorage.h>
#include <OpenRenderBoxCxx/Path/PathStorage.hpp>
#include <OpenRenderBoxCxx/Util/assert.hpp>

using namespace ORB;

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

void ORBPathStorageAppendPath(ORBPathStorageRef storage, ORBPath path) {
    precondition_failure("TODO");
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
    precondition_failure("TODO");
}
#endif
