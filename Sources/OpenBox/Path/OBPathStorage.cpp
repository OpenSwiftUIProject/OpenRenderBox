//
//  OBPathStorage.cpp
//  OpenBox

#include "OBPathStorage.h"
#include "PathStorage.hpp"
#include "../Util/assert.hpp"

using namespace OB;

void OBPathStorageInit(OBPathStorage* dst, uint32_t capacity, OBPathStorage* source) {
    if (source != nullptr) {
        dst->storage = OB::Path::Storage(capacity, source->storage);
    } else {
        dst->storage = OB::Path::Storage(capacity);
    }
}

void OBPathStorageDestroy(OBPathStorage storage) {
    storage.storage.~Storage();
}

void OBPathStorageClear(OBPathStorage storage) {
    storage.storage.clear();
}

// ...

bool OBPathStorageIsEmpty(OBPathStorage storage) {
    return storage.storage.isEmpty();
}

bool OBPathStorageEqualToStorage(OBPathStorage lhs, OBPathStorage rhs) {
    return lhs.storage == rhs.storage;
}

bool OBPathStorageIsSingleElement(OBPathStorage storage) {
    return storage.storage.flags().isSingleElement();
}

uint32_t OBPathStorageGetBezierOrder(OBPathStorage storage) {
    return storage.storage.flags().bezierOrder();
}

#if OB_TARGET_OS_DARWIN

CGRect OBPathStorageGetBoundingRect(OBPathStorage storage) {
    precondition_failure("TODO");
}

CGPathRef OBPathStorageGetCGPath(OBPathStorage storage) {
    precondition_failure("TODO");
}
#endif
