//
//  ColorSpace.cpp
//  OpenRenderBox

#include <OpenRenderBoxCxx/Color/ColorSpace.hpp>

#if ORB_TARGET_OS_DARWIN

#include <CoreFoundation/CFBase.h>

namespace ORB {

ColorSpaceResult color_space_from_cg_name(CFStringRef name) {
    if (name == nullptr) {
        return { ColorSpace::LinearSRGB, false };
    } else if (CFEqual(name, kCGColorSpaceSRGB) ||
        CFEqual(name, kCGColorSpaceExtendedSRGB)) {
        return { ColorSpace::SRGB, true };
    } else if (CFEqual(name, kCGColorSpaceLinearSRGB) ||
        CFEqual(name, kCGColorSpaceExtendedLinearSRGB)) {
        return { ColorSpace::LinearSRGB, true };
    } else if (CFEqual(name, kCGColorSpaceDisplayP3) ||
        CFEqual(name, kCGColorSpaceExtendedDisplayP3)) {
        return { ColorSpace::DisplayP3, true };
    } else if (CFEqual(name, kCGColorSpaceLinearDisplayP3) ||
        CFEqual(name, kCGColorSpaceExtendedLinearDisplayP3)) {
        return { ColorSpace::LinearDisplayP3, true };
    } else {
        return { ColorSpace::LinearSRGB, false };
    }
}

ColorSpaceResult color_space_from_cg(CGColorSpaceRef colorSpace) {
    if (colorSpace == nullptr) {
        return { ColorSpace::LinearSRGB, false };
    }
    CFStringRef name = CGColorSpaceGetName(colorSpace);
    return color_space_from_cg_name(name);
}

CGColorSpaceRef cg_color_space(ColorSpace colorSpace, bool extended) {
    switch (colorSpace) {
        case ColorSpace::LinearSRGB:
            return extended ? extended_linear_srgb_colorspace() : linear_srgb_colorspace();
        case ColorSpace::SRGB:
            return extended ? extended_srgb_colorspace() : srgb_colorspace();
        case ColorSpace::LinearDisplayP3:
            return extended ? extended_linear_display_p3_colorspace() : linear_display_p3_colorspace();
        case ColorSpace::DisplayP3:
            return extended ? extended_display_p3_colorspace() : display_p3_colorspace();
        case ColorSpace::PQ:
            return pq_colorspace();
        case ColorSpace::Invalid:
            abort();
    }
}

CGColorSpaceRef linear_srgb_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceLinearSRGB);
    return colorSpace;
}

CGColorSpaceRef extended_linear_srgb_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceExtendedLinearSRGB);
    return colorSpace;
}

CGColorSpaceRef srgb_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    return colorSpace;
}

CGColorSpaceRef extended_srgb_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceExtendedSRGB);
    return colorSpace;
}

CGColorSpaceRef linear_display_p3_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceLinearDisplayP3);
    return colorSpace;
}

CGColorSpaceRef extended_linear_display_p3_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceExtendedLinearDisplayP3);
    return colorSpace;
}

CGColorSpaceRef display_p3_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceDisplayP3);
    return colorSpace;
}

CGColorSpaceRef extended_display_p3_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceExtendedDisplayP3);
    return colorSpace;
}

CGColorSpaceRef pq_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceITUR_2100_PQ);
    return colorSpace;
}

} /* namespace ORB */

#endif /* ORB_TARGET_OS_DARWIN */

