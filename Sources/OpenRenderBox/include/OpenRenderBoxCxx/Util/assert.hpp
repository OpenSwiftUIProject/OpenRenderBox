//
//  assert.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

namespace ORB {
void precondition_failure(const char *format, ...) __cold __dead2;
void non_fatal_precondition_failure(const char *format, ...);
} /* ORB */

