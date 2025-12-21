//
//  assert.cpp
//  OpenRenderBox

#include <OpenRenderBoxCxx/Util/assert.hpp>
#include <OpenRenderBoxCxx/Util/log.hpp>

char* error_message = nullptr;

namespace ORB {
void precondition_failure(const char *format, ...) {
    char* s = nullptr;
    va_list va;
    va_start(va, format);
    vasprintf(&s, format, va);
    va_end(va);
    if (s != nullptr) {
        #if ORB_TARGET_OS_DARWIN
        os_log_error(error_log(), "precondition failure: %s", s);
        #endif /* ORB_TARGET_OS_DARWIN */
        if (error_message == nullptr) {
            asprintf(&error_message, "OpenRenderBox precondition failure: %s.\n", s);
        }
        free(s);
    }
    abort();
}

void non_fatal_precondition_failure(const char *format, ...) {
    char* s = nullptr;
    va_list va;
    va_start(va, format);
    vasprintf(&s, format, va);
    va_end(va);
    if (s != nullptr) {
        #if ORB_TARGET_OS_DARWIN
        os_log_fault(error_log(), "precondition failure: %s", s);
        #endif /* ORB_TARGET_OS_DARWIN */
        free(s);
    }
    return;
}
} /* ORB */
