//
//  NestedCallbacks.cpp
//  OpenRenderBox

// TODO: Implement NestedCallbacks

#if OPENRENDERBOX_CF_CGTYPE

#include <OpenRenderBoxCxx/Path/NestedCallbacks.hpp>
#include <mutex>
#if ORB_TARGET_OS_DARWIN
#include <os/lock.h>
#endif
#include <OpenRenderBoxCxx/Vector/vector.hpp>

namespace ORB {
namespace Path {

#if ORB_TARGET_OS_DARWIN

namespace {

struct CacheEntry {
    const ORBPathCallbacks* callbacks;
    size_t index;
    const NestedCallbacks* result;
};

struct Cache {
    ORB::vector<CacheEntry, 0, size_t> entries;
    os_unfair_lock lock;

    Cache() : entries(), lock(OS_UNFAIR_LOCK_INIT) {}
};

Cache* gCache = nullptr;

// Helper to call bezierOrder on callbacks
inline uint32_t callBezierOrder(const void* object, const ORBPathCallbacks* callbacks) {
    if (callbacks->bezierOrder) {
        return callbacks->bezierOrder(object);
    }
    return 3; // Default: cubic bezier
}

} // anonymous namespace

NestedCallbacks::NestedCallbacks(const ORBPathCallbacks* callbacks, size_t index)
    : extended{}
    , originalCallbacks(callbacks)
    , nestedIndex(index)
{
    // Set isExtended flag (byte at offset 0x2)
    extended.flags.isExtended = 1;

    // Copy retain/release from original callbacks
    extended.retain = reinterpret_cast<decltype(extended.retain)>(callbacks->retain);
    extended.release = reinterpret_cast<decltype(extended.release)>(callbacks->release);

    extended.apply = +[](const void *object, void *info, ORBPathApplyCallback callback, const ORBPathCallbacksExtended *ext) -> bool {
        auto self = reinterpret_cast<const NestedCallbacks*>(ext);
        auto original = self->originalCallbacks;
        if (original->apply) {
            return original->apply(object, info, callback);
        }
        return true;
    };

    extended.isEqual = nullptr;

    extended.isEmpty = +[](const void *object, const ORBPathCallbacksExtended *ext) -> bool {
        auto self = reinterpret_cast<const NestedCallbacks*>(ext);
        auto original = self->originalCallbacks;
        if (original->apply) {
            // Use single_element_callback to detect emptiness
            struct Info {
                size_t count;
                size_t depth;
                bool sawClose;
            } info = {0, 0, false};
            info.depth = self->nestedIndex;
            original->apply(object, &info, NestedCallbacks::single_element_callback);
            return info.count == 1;
        }
        return true;
    };

    extended.bezierOrder = +[](const void *object, const ORBPathCallbacksExtended *ext) -> uint32_t {
        auto self = reinterpret_cast<const NestedCallbacks*>(ext);
        return callBezierOrder(object, self->originalCallbacks);
    };

    extended.isSingleElement = nullptr;
    extended.boundingRect = nullptr;
    extended.cgPath = nullptr;

    extended.next = +[](const void *object, const ORBPathCallbacksExtended *ext) -> const ORBPathCallbacksExtended* {
        auto self = reinterpret_cast<const NestedCallbacks*>(ext);
        auto nextCallbacks = NestedCallbacks::get(self->originalCallbacks, self->nestedIndex + 1);
        return &nextCallbacks->extended;
    };
}

const NestedCallbacks* NestedCallbacks::get(const ORBPathCallbacks* callbacks, size_t index) {
    // Lazy initialization of global cache
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        gCache = new Cache();
    });

    os_unfair_lock_lock(&gCache->lock);

    // Binary search for existing entry
    auto& entries = gCache->entries;
    size_t lo = 0;
    size_t hi = entries.size();

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        const auto& entry = entries[mid];

        if (entry.callbacks < callbacks) {
            lo = mid + 1;
        } else if (entry.callbacks > callbacks) {
            hi = mid;
        } else if (entry.index < index) {
            lo = mid + 1;
        } else if (entry.index > index) {
            hi = mid;
        } else {
            // Found exact match
            const NestedCallbacks* result = entry.result;
            os_unfair_lock_unlock(&gCache->lock);
            return result;
        }
    }

    // Not found, create new NestedCallbacks
    auto* nested = new NestedCallbacks(callbacks, index);

    // Insert at position 'lo' to maintain sorted order
    entries.reserve(entries.size() + 1);

    // Shift elements to make room
    if (lo < entries.size()) {
        // Use memmove for efficiency
        size_t count = entries.size() - lo;
        memmove(&entries[lo + 1], &entries[lo], count * sizeof(CacheEntry));
    }

    // Insert new entry
    entries[lo] = CacheEntry{callbacks, index, nested};

    os_unfair_lock_unlock(&gCache->lock);
    return nested;
}

// Mask for nesting elements (bits 17-20,21-24): used to detect depth changes
// From assembly: 0x1de0000 = bits for elements 17, 18, 19, 20, 22, 23, 24
static constexpr uint32_t kNestingElementMask = 0x1de0000;

// Helper to check if an element affects nesting depth
static inline bool isNestingElement(ORBPathElement element) {
    if (element > 0x18) return false;
    return ((1u << element) & kNestingElementMask) != 0;
}

// Helper to check if element is in range 0x0-0x3 (move/line/quad/curve)
static inline bool isDrawingElement(ORBPathElement element) {
    return element < 4;
}

// apply_elements_callback: tracks nesting depth during path enumeration
// Info structure: { context, callback, depth, elementCount, sawClose }
bool NestedCallbacks::apply_elements_callback(void* info, ORBPathElement element, const double* points, const void* userInfo) {
    struct ApplyInfo {
        void* context;
        ORBPathApplyCallback callback;
        size_t depth;
    };
    auto* applyInfo = static_cast<ApplyInfo*>(info);

    if (element > 0x18) {
        // Forward to user callback if we're at depth 0
        if (applyInfo->depth == 0) {
            return applyInfo->callback(applyInfo->context, element, points, userInfo);
        }
        return false;
    }

    uint32_t mask = 1u << element;
    if (mask & kNestingElementMask) {
        // Nesting element: increment depth
        applyInfo->depth++;
        return false;
    }

    if (element == 0x10) { // Close subpath
        if (applyInfo->depth > 0) {
            applyInfo->depth--;
        }
        return false;
    }

    // Forward non-nesting elements at depth 0
    if (applyInfo->depth == 0) {
        return applyInfo->callback(applyInfo->context, element, points, userInfo);
    }
    return false;
}

// single_element_callback: checks if path is a single element
// Info structure: { depth, elementCount, sawClose }
bool NestedCallbacks::single_element_callback(void* info, ORBPathElement element, const double* points, const void* userInfo) {
    struct SingleInfo {
        size_t depth;
        size_t count;
        bool sawClose;
    };
    auto* singleInfo = static_cast<SingleInfo*>(info);

    if (element > 0x18) {
        // Elements > 0x18: count as elements at depth 0
        if (singleInfo->depth == singleInfo->count) {
            singleInfo->count++;
            if (element == 0x13) { // Specific element marker
                singleInfo->sawClose = false;
            } else {
                singleInfo->sawClose = true;
            }
        }
        return false;
    }

    uint32_t mask = 1u << element;

    // Handle nesting elements
    if ((mask & 0xf000) != 0) {
        // Elements 12-15
        if (singleInfo->depth == singleInfo->count) {
            singleInfo->sawClose = true;
            singleInfo->count++;
        }
        return false;
    }

    if (mask & kNestingElementMask) {
        // Other nesting elements: increment depth
        if (singleInfo->depth == singleInfo->count) {
            singleInfo->count++;
        }
        return false;
    }

    if (element == 0x10) { // Close subpath
        if (singleInfo->depth > 0) {
            singleInfo->depth--;
        }
        if (singleInfo->depth == singleInfo->count && singleInfo->sawClose) {
            singleInfo->sawClose = false;
            singleInfo->count++;
        }
        return false;
    }

    // Drawing elements (0-3)
    if (isDrawingElement(element)) {
        if (singleInfo->depth == singleInfo->count) {
            singleInfo->sawClose = true;
            singleInfo->count++;
        }
    }

    // Return true if count >= 2 (stop enumeration)
    return singleInfo->count >= 2;
}

// apply_elements_fast: fast path for Storage
bool NestedCallbacks::apply_elements_fast(const Storage& storage, void* info) {
    // TODO: Implement fast path using Storage iterator
    (void)storage;
    (void)info;
    return true;
}

// single_element_fast: fast path for single element detection on Storage
bool NestedCallbacks::single_element_fast(const Storage& storage, void* info) {
    // TODO: Implement fast path using Storage iterator
    (void)storage;
    (void)info;
    return true;
}

// first_element: find first element at given nesting depth
bool NestedCallbacks::first_element(const Storage& storage, void* iterator, size_t depth) {
    // TODO: Implement using Storage iterator
    (void)storage;
    (void)iterator;
    (void)depth;
    return false;
}

#endif /* ORB_TARGET_OS_DARWIN */

} /* namespace Path */
} /* namespace ORB */

#endif /* OPENRENDERBOX_CF_CGTYPE */
