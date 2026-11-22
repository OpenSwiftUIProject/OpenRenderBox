#pragma once

#include <OpenRenderBox/ORBTargetConditionals.h>

#if ORB_TARGET_OS_DARWIN
#include <OpenRenderBox/ORBPath.h>

namespace ORB::Path {
void add_rounded_rect(CGMutablePathRef path, ORBPathElement element, const CGFloat *values, const CGAffineTransform *transform);
}
#endif /* ORB_TARGET_OS_DARWIN */
