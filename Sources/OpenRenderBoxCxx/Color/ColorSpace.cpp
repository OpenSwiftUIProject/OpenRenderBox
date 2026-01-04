//
//  ColorSpace.cpp
//  OpenRenderBox

#include <OpenRenderBoxCxx/Color/ColorSpace.hpp>

#if ORB_TARGET_OS_DARWIN

#include <CoreFoundation/CFBase.h>

namespace ORB {

std::optional<ColorSpace> color_space_from_cg_name(CFStringRef name) {
    if (name == nullptr) {
        return std::nullopt;
    } else if (CFEqual(name, kCGColorSpaceSRGB) ||
        CFEqual(name, kCGColorSpaceExtendedSRGB)) {
        return ColorSpace::SRGB;
    } else if (CFEqual(name, kCGColorSpaceLinearSRGB) ||
        CFEqual(name, kCGColorSpaceExtendedLinearSRGB)) {
        return ColorSpace::LinearSRGB;
    } else if (CFEqual(name, kCGColorSpaceDisplayP3) ||
        CFEqual(name, kCGColorSpaceExtendedDisplayP3)) {
        return ColorSpace::DisplayP3;
    } else if (CFEqual(name, kCGColorSpaceLinearDisplayP3) ||
        CFEqual(name, kCGColorSpaceExtendedLinearDisplayP3)) {
        return ColorSpace::LinearDisplayP3;
    } else {
        return std::nullopt;
    }
}

std::optional<ColorSpace> color_space_from_cg(CGColorSpaceRef colorSpace) {
    if (colorSpace == nullptr) {
        return std::nullopt;
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
        case ColorSpace::Unknown:
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

CGColorSpaceRef gray_colorspace() {
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericGrayGamma2_2);
    return colorSpace;
}

} /* namespace ORB */

std::optional<ORB::ColorSpace> orb_color_space(ORBColorSpace orbColorSpace) {
    switch (orbColorSpace) {
        case ORBColorSpaceDefault:
            return std::nullopt;
        case ORBColorSpaceSRGB:
            return ORB::ColorSpace::SRGB;
        case ORBColorSpaceLinearSRGB:
            return ORB::ColorSpace::LinearSRGB;
        case ORBColorSpaceDisplayP3:
            return ORB::ColorSpace::DisplayP3;
        case ORBColorSpaceLinearDisplayP3:
            return ORB::ColorSpace::LinearDisplayP3;
        default:
            return std::nullopt;
    }
}

ORBColorSpace orb_color_space(std::optional<ORB::ColorSpace> colorSpace) {
    if (!colorSpace.has_value()) {
        return ORBColorSpaceDefault;
    }
    switch (colorSpace.value()) {
        case ORB::ColorSpace::LinearSRGB:
            return ORBColorSpaceLinearSRGB;
        case ORB::ColorSpace::SRGB:
            return ORBColorSpaceSRGB;
        case ORB::ColorSpace::LinearDisplayP3:
            return ORBColorSpaceLinearDisplayP3;
        case ORB::ColorSpace::DisplayP3:
            return ORBColorSpaceDisplayP3;
        case ORB::ColorSpace::Unknown:
        case ORB::ColorSpace::PQ:
            return ORBColorSpaceDefault;
        default:
            return ORBColorSpaceLinearSRGB;
    }
}

#endif /* ORB_TARGET_OS_DARWIN */
