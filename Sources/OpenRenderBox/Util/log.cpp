//
//  log.cpp
//  OpenRenderBox

#include <OpenRenderBoxCxx/Util/log.hpp>

#if ORB_TARGET_OS_DARWIN

namespace ORB {
os_log_t misc_log() {
    static os_log_t log = os_log_create("org.openswiftuiproject.openrenderbox", "misc");
    return log;
}
os_log_t error_log() {
    static os_log_t log = os_log_create("org.openswiftuiproject.openrenderbox", "error");
    return log;
}
} /* ORB */

#endif /* ORB_TARGET_OS_DARWIN */
