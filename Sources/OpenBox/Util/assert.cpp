//
//  assert.cpp
//  OpenBox

#include "assert.hpp"
#include "log.hpp"

char* error_message = nullptr;

namespace OB {
void precondition_failure(const char *format, ...) {
    char* s = nullptr;
    va_list va;
    va_start(va, format);
    vasprintf(&s, format, va);
    va_end(va);
    if (s != nullptr) {
        #if OB_TARGET_OS_DARWIN
        os_log_error(error_log(), "precondition failure: %s", s);
        #endif /* OB_TARGET_OS_DARWIN */
        if (error_message == nullptr) {
            asprintf(&error_message, "OpenGraph precondition failure: %s.\n", s);
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
        #if OB_TARGET_OS_DARWIN
        os_log_fault(error_log(), "precondition failure: %s", s);
        #endif /* OB_TARGET_OS_DARWIN */
        free(s);
    }
    return;
}
} /* OB */
