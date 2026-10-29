//
//  PathMapper.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include <OpenRenderBox/ORBPath.h>

namespace ORB {
namespace Path {
class Storage;
CGPathRef copy_cg_path(ORBPath path);
bool apply_mapping_element(void *context, ORBPathElement element, const CGFloat *values, const void *info);
bool apply_mapped_storage(const Storage &storage, void *context, size_t depth);
void invalidate_mapped_storage(uint32_t identifier);
} /* Path */
} /* ORB */

#endif
