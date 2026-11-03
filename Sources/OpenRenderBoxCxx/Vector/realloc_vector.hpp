//
//  realloc_vector.hpp
//  OpenRenderBoxCxx

#ifndef realloc_vector_hpp
#define realloc_vector_hpp

#include <OpenRenderBox/ORBBase.h>

ORB_ASSUME_NONNULL_BEGIN

namespace ORB {
namespace details {
template <typename T = uint32_t, size_t Alignment>
void *_Nullable realloc_vector(void* ptr, T& size, T new_size);

template <typename T = uint32_t, size_t Alignment>
void *_Nullable realloc_vector(void* src, void* dst, T dstSize, T& size, T newSize);
} /* details */
} /* ORB */

ORB_ASSUME_NONNULL_END

#endif /* realloc_vector_hpp */
