#pragma once

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN
#include <OpenRenderBox/ORBPath.h>

namespace ORB::Path {
void add_rounded_rect(CGMutablePathRef path, ORBPathElement element, const CGFloat *values, const CGAffineTransform *transform);
}
#endif
