//
//  NestedCallbacks.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBPathCallbacks.h>
#include "Storage.hpp"

ORB_ASSUME_NONNULL_BEGIN

namespace ORB {
namespace Path {

struct NestedCallbacks {
    struct ApplyInfo {
        void *context;
        ORBPathApplyCallback callback;
        size_t targetDepth;
        size_t currentDepth;
        bool reachedEnd;
    };

    struct SingleInfo {
        size_t targetDepth = 0;
        size_t currentDepth = 0;
        size_t count = 0;
        bool pendingClose = false;
    };

    ORBPathCallbacksExtended extended;
    const ORBPathCallbacks *originalCallbacks;
    size_t nestedIndex;

    static const NestedCallbacks *get(const ORBPathCallbacks *callbacks, size_t index);
    static bool is_nesting_element(ORBPathElement element) ORB_NOEXCEPT;
    static bool apply_elements_callback(void *info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo);
    static bool single_element_callback(void *info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo);
    static bool apply_elements_fast(const Storage &storage, void *info);
    static bool single_element_fast(const Storage &storage, void *info);
    static bool first_element(const Storage &storage, Storage::Iterator &iterator, size_t depth);

private:
    NestedCallbacks(const ORBPathCallbacks *callbacks, size_t index);
};

static_assert(sizeof(NestedCallbacks) == 0x68, "NestedCallbacks size mismatch");
static_assert(offsetof(NestedCallbacks, originalCallbacks) == 0x58, "originalCallbacks offset mismatch");
static_assert(offsetof(NestedCallbacks, nestedIndex) == 0x60, "nestedIndex offset mismatch");

} /* Path */
} /* ORB */

ORB_ASSUME_NONNULL_END

#endif /* OPENRENDERBOX_CF_CGTYPES */
