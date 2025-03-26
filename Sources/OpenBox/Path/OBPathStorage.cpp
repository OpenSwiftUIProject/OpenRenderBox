//
//  OBPathStorage.cpp
//  OpenBox

#include "OBPathStorage.h"
#include "PathStorage.hpp"
#include "../Util/assert.hpp"

using namespace OB;

void OBPathStorageInit(OBPathStorageRef dst, uint32_t capacity, OBPathStorageRef source) {
    if (source != nullptr) {
        dst->storage = OB::Path::Storage(capacity, source->storage);
    } else {
        dst->storage = OB::Path::Storage(capacity);
    }
}

void OBPathStorageDestroy(OBPathStorageRef storage) {
    storage->storage.~Storage();
}

void OBPathStorageClear(OBPathStorageRef storage) {
    storage->storage.clear();
}

// ...

bool OBPathStorageIsEmpty(OBPathStorageRef storage) {
    return storage->storage.isEmpty();
}

bool OBPathStorageEqualToStorage(OBPathStorageRef lhs, OBPathStorageRef rhs) {
    return lhs->storage == rhs->storage;
}

bool OBPathStorageIsSingleElement(OBPathStorageRef storage) {
    return storage->storage.flags().isSingleElement();
}

uint32_t OBPathStorageGetBezierOrder(OBPathStorageRef storage) {
    return storage->storage.flags().bezierOrder();
}

#if OB_TARGET_OS_DARWIN

CGRect OBPathStorageGetBoundingRect(OBPathStorageRef storage) {
    precondition_failure("TODO");
}

CGPathRef OBPathStorageGetCGPath(OBPathStorageRef storage) {
    precondition_failure("TODO");
}
#endif
