//
//  PathStorageObject.cpp
//  OpenRenderBox

#include "PathStorageObject.hpp"

#if OPENRENDERBOX_CF_CGTYPES

#include "NestedCallbacks.hpp"
#include "Storage.hpp"
#include <atomic>

namespace ORB::Path {
namespace {

struct StorageObject {
    mutable std::atomic_uint references{1};
    Storage storage;

    explicit StorageObject(const Storage &source) : storage(sizeof(Storage), source) {}
};

const ORBPathCallbacks callbacks = {
    .flags = {},
    .retain = +[](const void *object) -> const void * {
        static_cast<const StorageObject *>(object)->references.fetch_add(1, std::memory_order_relaxed);
        return object;
    },
    .release = +[](const void *object) {
        auto *value = static_cast<const StorageObject *>(object);
        if (value->references.fetch_sub(1, std::memory_order_acq_rel) == 1) delete value;
    },
    .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) {
        return static_cast<const StorageObject *>(object)->storage.apply_elements(info, callback);
    },
    .isEqual = +[](const void *a, const void *b) {
        return static_cast<const StorageObject *>(a)->storage == static_cast<const StorageObject *>(b)->storage;
    },
    .isEmpty = +[](const void *object) { return static_cast<const StorageObject *>(object)->storage.isEmpty(); },
    .isSingleElement = +[](const void *object) {
        return static_cast<const StorageObject *>(object)->storage.flags().isSingleElement();
    },
    .bezierOrder = +[](const void *object) -> size_t {
        return static_cast<const StorageObject *>(object)->storage.flags().bezierOrder();
    },
    #if ORB_TARGET_OS_DARWIN
    .boundingRect = +[](const void *object) { return static_cast<const StorageObject *>(object)->storage.bounding_rect(); },
    .cgPath = +[](const void *object) { return static_cast<const StorageObject *>(object)->storage.cgpath(); },
    #else
    .boundingRect = nullptr,
    .cgPath = nullptr,
    #endif
    .next = +[](const void *object) -> ORBPath {
        auto *nested = NestedCallbacks::get(&callbacks, 1);
        return {reinterpret_cast<ORBPathStorageRef>(const_cast<void *>(object)),
                reinterpret_cast<const ORBPathCallbacks *>(&nested->extended)};
    },
};

} // namespace

ORBPath copy_storage_path(const Storage &storage) {
    return {reinterpret_cast<ORBPathStorageRef>(new StorageObject(storage)), &callbacks};
}

} // namespace ORB::Path

#endif
