//
//  PathClipStroke.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if OPENRENDERBOX_CF_CGTYPES && ORB_TARGET_OS_DARWIN

#include <CoreGraphics/CoreGraphics.h>
#include <cstddef>
#include <cstdint>

namespace ORB::Path {

struct ClipStrokeKeyframe {
    int32_t tag;
    float value;
    float start;
    float end;
};

struct ClipStrokeInfo {
    uint32_t flags;
    int32_t cap;
    size_t count;
    const ClipStrokeKeyframe *keys;
};

CGPathRef copy_clip_stroke_path(CGPathRef path, float progress, const ClipStrokeInfo &info);

} // namespace ORB::Path

#endif
