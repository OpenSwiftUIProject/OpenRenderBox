//
//  PathStorageObject.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBPath.h>

namespace ORB::Path {
class Storage;
ORBPath copy_storage_path(const Storage &storage);
}

#endif
