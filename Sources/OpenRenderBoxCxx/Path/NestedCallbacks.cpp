//
//  NestedCallbacks.cpp
//  OpenRenderBox

#if OPENRENDERBOX_CF_CGTYPES

#include "NestedCallbacks.hpp"
#include <Vector/vector.hpp>
#include <cstring>
#include <functional>
#include <mutex>

namespace ORB {
namespace Path {
namespace {

struct CacheEntry {
    const ORBPathCallbacks *callbacks;
    size_t index;
    const NestedCallbacks *result;
};

struct Cache {
    ORB::vector<CacheEntry> entries;
    std::mutex lock;
};

} // namespace

NestedCallbacks::NestedCallbacks(const ORBPathCallbacks *callbacks, size_t index)
    : extended{}, originalCallbacks(callbacks), nestedIndex(index) {
    extended.flags.isExtended = true;
    extended.retain = callbacks->retain;
    extended.release = callbacks->release;
    extended.apply = +[](const void *object, void *info, ORBPathApplyCallback callback, const ORBPathCallbacksExtended *extended) {
        auto &self = *reinterpret_cast<const NestedCallbacks *>(extended);
        ApplyInfo context{info, callback, self.nestedIndex, 0, false};
        ORBPath path{reinterpret_cast<ORBPathStorageRef>(const_cast<void *>(object)), self.originalCallbacks};
        bool result = ORBPathApplyElements(path, &context, apply_elements_callback);
        return result || context.reachedEnd;
    };
    extended.isSingleElement = +[](const void *object, const ORBPathCallbacksExtended *extended) {
        auto &self = *reinterpret_cast<const NestedCallbacks *>(extended);
        SingleInfo context{self.nestedIndex};
        ORBPath path{reinterpret_cast<ORBPathStorageRef>(const_cast<void *>(object)), self.originalCallbacks};
        ORBPathApplyElements(path, &context, single_element_callback);
        return context.count == 1;
    };
    extended.bezierOrder = +[](const void *object, const ORBPathCallbacksExtended *extended) -> size_t {
        auto &self = *reinterpret_cast<const NestedCallbacks *>(extended);
        ORBPath path{reinterpret_cast<ORBPathStorageRef>(const_cast<void *>(object)), self.originalCallbacks};
        return ORBPathGetBezierOrder(path);
    };
    extended.next = +[](const void *object, const ORBPathCallbacksExtended *extended) -> ORBPath {
        auto &self = *reinterpret_cast<const NestedCallbacks *>(extended);
        auto *next = NestedCallbacks::get(self.originalCallbacks, self.nestedIndex + 1);
        return {reinterpret_cast<ORBPathStorageRef>(const_cast<void *>(object)), reinterpret_cast<const ORBPathCallbacks *>(&next->extended)};
    };
}

const NestedCallbacks *NestedCallbacks::get(const ORBPathCallbacks *callbacks, size_t index) {
    // Callback tables are process-wide identities and must remain valid for paths.
    static Cache *cache = new Cache;
    std::lock_guard<std::mutex> guard(cache->lock);
    auto &entries = cache->entries;
    std::less<const ORBPathCallbacks *> less;
    size_t lo = 0;
    size_t hi = entries.size();
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        const auto &entry = entries[mid];
        if (less(entry.callbacks, callbacks) || (entry.callbacks == callbacks && entry.index < index)) {
            lo = mid + 1;
        } else if (less(callbacks, entry.callbacks) || (entry.callbacks == callbacks && entry.index > index)) {
            hi = mid;
        } else {
            return entry.result;
        }
    }
    auto *result = new NestedCallbacks(callbacks, index);
    size_t count = entries.size();
    entries.push_back(CacheEntry{});
    if (lo < count) {
        std::memmove(&entries[lo + 1], &entries[lo], (count - lo) * sizeof(CacheEntry));
    }
    entries[lo] = {callbacks, index, result};
    return result;
}

bool NestedCallbacks::is_nesting_element(ORBPathElement element) ORB_NOEXCEPT {
    auto type = static_cast<unsigned>(element);
    return type <= 24 && ((1u << type) & 0x1de0000) != 0;
}

bool NestedCallbacks::apply_elements_callback(void *info, ORBPathElement element, const CGFloat *points, const void *userInfo) {
    auto &context = *static_cast<ApplyInfo *>(info);
    size_t depth = context.currentDepth;
    if (is_nesting_element(element)) {
        ++context.currentDepth;
    } else if (static_cast<unsigned>(element) == 16) {
        --context.currentDepth;
        if (depth <= context.targetDepth) {
            context.reachedEnd = depth == context.targetDepth;
            return false;
        }
    }
    return depth < context.targetDepth || context.callback(context.context, element, points, userInfo);
}

bool NestedCallbacks::single_element_callback(void *info, ORBPathElement element, const CGFloat *, const void *) {
    auto &context = *static_cast<SingleInfo *>(info);
    size_t depth = context.currentDepth;
    auto type = static_cast<unsigned>(element);
    if (is_nesting_element(element)) {
        ++context.currentDepth;
        if (depth == context.targetDepth) {
            ++context.count;
            context.pendingClose = type != 19;
        }
    } else if (type == 16) {
        --context.currentDepth;
        if (depth == context.targetDepth) {
            return false;
        }
    } else if (depth == context.targetDepth) {
        if (type < 4 || (type >= 12 && type <= 15)) {
            context.pendingClose = true;
            ++context.count;
        } else if (type == 4) {
            if (context.pendingClose) {
                context.pendingClose = false;
                ++context.count;
            }
        } else {
            ++context.count;
        }
    }
    return context.count < 2;
}

bool NestedCallbacks::apply_elements_fast(const Storage &storage, void *info) {
    auto &context = *static_cast<ApplyInfo *>(info);
    return storage.apply_elements_at_depth(context.context, context.callback, context.targetDepth);
}

bool NestedCallbacks::single_element_fast(const Storage &storage, void *info) {
    auto &context = *static_cast<SingleInfo *>(info);
    Storage::Iterator iterator;
    if (context.targetDepth != 0) {
        if (!first_element(storage, iterator, context.targetDepth)) {
            return false;
        }
        context.currentDepth = context.targetDepth;
    }
    ORBPathElement element;
    while (context.count < 2 && iterator.next_type(storage, element)) {
        size_t depth = context.currentDepth;
        const CGFloat unused = 0;
        single_element_callback(info, element, &unused, nullptr);
        if (static_cast<unsigned>(element) == 16 && depth == context.targetDepth) {
            return false;
        }
    }
    return true;
}

bool NestedCallbacks::first_element(const Storage &storage, Storage::Iterator &iterator, size_t depth) {
    size_t currentDepth = 0;
    ORBPathElement element;
    while (iterator.next_type(storage, element)) {
        if (is_nesting_element(element)) {
            if (++currentDepth == depth) {
                return true;
            }
        } else if (static_cast<unsigned>(element) == 16) {
            return false;
        }
    }
    return false;
}

} /* Path */
} /* ORB */

#endif /* OPENRENDERBOX_CF_CGTYPES */
