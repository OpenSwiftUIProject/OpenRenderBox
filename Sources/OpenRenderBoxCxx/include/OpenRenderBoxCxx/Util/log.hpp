//
//  log.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>

#if ORB_TARGET_OS_DARWIN

#include <os/log.h>

ORB_ASSUME_NONNULL_BEGIN

namespace ORB {
os_log_t misc_log();
os_log_t error_log();
} /* ORB */

ORB_ASSUME_NONNULL_END

#endif /* ORB_TARGET_OS_DARWIN */
