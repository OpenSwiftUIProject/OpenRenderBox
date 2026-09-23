//
//  ColorSpace.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBColorSpace.h>

#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CGColorSpace.h>
#include <CoreFoundation/CFString.h>
#endif /* ORB_TARGET_OS_DARWIN */
#if __has_include(<optional>)
#include <optional>
namespace ORB {
template <typename T> using Optional = std::optional<T>;
constexpr auto nullopt = std::nullopt;
}
#else
#include <experimental/optional>
namespace ORB {
template <typename T> using Optional = std::experimental::optional<T>;
constexpr auto nullopt = std::experimental::nullopt;
}
#endif

namespace ORB {

/// Internal color space representation used by ORB::cg_color_space
enum class ColorSpace : uint32_t {
    LinearSRGB = 0,
    SRGB = 1,
    LinearDisplayP3 = 2,
    DisplayP3 = 3,
    Unknown = 4,
    PQ = 5,
};

#if ORB_TARGET_OS_DARWIN
/// Converts a CGColorSpace name (CFStringRef) to internal ColorSpace enum.
/// @param name The color space name from CGColorSpaceGetName.
/// @return The ColorSpace if recognized, or ORB::nullopt if not.
Optional<ColorSpace> color_space_from_cg_name(CFStringRef name);

/// Converts a CGColorSpaceRef to internal ColorSpace enum.
/// @param colorSpace The CGColorSpace to convert.
/// @return The ColorSpace if recognized, or ORB::nullopt if not.
Optional<ColorSpace> color_space_from_cg(CGColorSpaceRef colorSpace);

/// Returns a CGColorSpaceRef for the given internal color space.
/// @param colorSpace The internal color space enum value.
/// @param extended If true, returns the extended range variant of the color space.
/// @return A CGColorSpaceRef. The caller does not own this reference.
CGColorSpaceRef cg_color_space(ColorSpace colorSpace, bool extended);

/// Returns the linear sRGB color space.
CGColorSpaceRef linear_srgb_colorspace();

/// Returns the extended linear sRGB color space.
CGColorSpaceRef extended_linear_srgb_colorspace();

/// Returns the sRGB color space.
CGColorSpaceRef srgb_colorspace();

/// Returns the extended sRGB color space.
CGColorSpaceRef extended_srgb_colorspace();

/// Returns the linear Display P3 color space.
CGColorSpaceRef linear_display_p3_colorspace();

/// Returns the extended linear Display P3 color space.
CGColorSpaceRef extended_linear_display_p3_colorspace();

/// Returns the Display P3 color space.
CGColorSpaceRef display_p3_colorspace();

/// Returns the extended Display P3 color space.
CGColorSpaceRef extended_display_p3_colorspace();

/// Returns the PQ (HDR) color space.
CGColorSpaceRef pq_colorspace();

/// Returns the gray color space.
CGColorSpaceRef gray_colorspace();
#endif /* ORB_TARGET_OS_DARWIN */

} /* namespace ORB */

ORB::Optional<ORB::ColorSpace> orb_color_space(ORBColorSpace orbColorSpace);

ORBColorSpace orb_color_space(ORB::Optional<ORB::ColorSpace> colorSpace);
