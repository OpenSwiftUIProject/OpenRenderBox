//
//  PathInterpolation.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include <OpenRenderBox/ORBPath.h>

namespace ORB::Path {
bool can_interpolate(ORBPath from, ORBPath to);
ORBPath copy_interpolated_path(ORBPath from, ORBPath to, float amount);
}

#endif
