//
//  NestedCallbacks.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBPathCallbacks.h>

ORB_ASSUME_NONNULL_BEGIN

namespace ORB {
namespace Path {

class Storage;

/// Extended callbacks wrapper for accessing nested paths.
/// Maintains a global cache to avoid creating duplicate wrappers.
struct NestedCallbacks {
    ORBPathCallbacksExtended extended;               // 0x00-0x57
    const ORBPathCallbacks * _Nonnull originalCallbacks; // 0x58
    size_t nestedIndex;                              // 0x60

    /// Get or create a NestedCallbacks for the given callbacks and nesting index
    static const NestedCallbacks* get(const ORBPathCallbacks* callbacks, size_t index);

    /// Callback for apply_elements that tracks nesting depth
    static bool apply_elements_callback(void* info, ORBPathElement element, const double* points, const void* userInfo);

    /// Callback for single_element detection
    static bool single_element_callback(void* info, ORBPathElement element, const double* points, const void* userInfo);

    /// Fast path for applying elements on Storage
    static bool apply_elements_fast(const Storage& storage, void* info);

    /// Fast path for single element detection on Storage
    static bool single_element_fast(const Storage& storage, void* info);

    /// Find the first element at a given nesting depth
    static bool first_element(const Storage& storage, void* iterator, size_t depth);

private:
    NestedCallbacks(const ORBPathCallbacks* callbacks, size_t index);
};

static_assert(sizeof(NestedCallbacks) == 0x68, "NestedCallbacks size mismatch");
static_assert(offsetof(NestedCallbacks, originalCallbacks) == 0x58, "originalCallbacks offset mismatch");
static_assert(offsetof(NestedCallbacks, nestedIndex) == 0x60, "nestedIndex offset mismatch");

} /* namespace Path */
} /* namespace ORB */

ORB_ASSUME_NONNULL_END

