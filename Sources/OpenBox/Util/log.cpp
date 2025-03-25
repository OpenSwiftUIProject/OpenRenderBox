//
//  OGLog.cpp
//  OpenBox

#include "log.hpp"

#if OB_TARGET_OS_DARWIN

namespace OB {
os_log_t misc_log() {
    static os_log_t log = os_log_create("org.openswiftuiproject.openbox", "misc");
    return log;
}
os_log_t error_log() {
    static os_log_t log = os_log_create("org.openswiftuiproject.openbox", "error");
    return log;
}
} /* OB */

#endif /* OB_TARGET_OS_DARWIN */
