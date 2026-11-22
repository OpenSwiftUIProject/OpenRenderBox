//
//  PathStorageObject.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_TARGET_OS_DARWIN

#include <OpenRenderBox/ORBPath.h>

namespace ORB::Path {
class Storage;
ORBPath copy_storage_path(const Storage &storage);
}

#endif /* ORB_TARGET_OS_DARWIN */
