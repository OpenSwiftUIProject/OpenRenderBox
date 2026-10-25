//
//  ORBSymbolRenderingMode.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

typedef ORB_ENUM(uint32_t, ORBSymbolRenderingMode) {
    ORBSymbolRenderingModePreferred = 0,
    ORBSymbolRenderingModeMonochrome = 1,
    ORBSymbolRenderingModeMulticolor = 2,
    ORBSymbolRenderingModeHierarchical = 3,
    ORBSymbolRenderingModePalette = 4,
    ORBSymbolRenderingModeHierarchicalUnlessSlashed = 128,
    ORBSymbolRenderingModeHierarchicalSlashBadge = 129,
    ORBSymbolRenderingModePaletteSlashBadge = 130,
    // No explicit mode. A symbol update keeps the animator's current mode.
    ORBSymbolRenderingModeUnspecified = 255,
};
