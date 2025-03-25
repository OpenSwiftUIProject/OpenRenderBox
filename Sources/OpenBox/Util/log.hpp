//
//  log.hpp
//  OpenBox

#ifndef log_hpp
#define log_hpp

#include "OBBase.h"

#if OB_TARGET_OS_DARWIN

#include <os/log.h>

namespace OB {
os_log_t misc_log();
os_log_t error_log();
} /* OB */

#endif /* OB_TARGET_OS_DARWIN */

#endif /* log_hpp */
