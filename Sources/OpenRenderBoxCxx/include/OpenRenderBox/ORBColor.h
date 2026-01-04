//
//  ORBColor.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef struct ORBColor {
    float red;
    float green;
    float blue;
    float alpha;
} ORBColor;

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

