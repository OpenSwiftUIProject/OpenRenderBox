//
//  assert.cpp
//  OpenRenderBox

#include <OpenRenderBoxCxx/Util/assert.hpp>
#include <OpenRenderBoxCxx/Util/log.hpp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#if ORB_TARGET_OS_WASI
static int orb_vasprintf(char **output, const char *format, va_list arguments) {
    va_list sizing_arguments;
    va_copy(sizing_arguments, arguments);
    int length = vsnprintf(nullptr, 0, format, sizing_arguments);
    va_end(sizing_arguments);
    if (length < 0) {
        *output = nullptr;
        return length;
    }

    auto *buffer = static_cast<char *>(malloc(static_cast<size_t>(length) + 1));
    if (buffer == nullptr) {
        *output = nullptr;
        return -1;
    }

    int result = vsnprintf(buffer, static_cast<size_t>(length) + 1, format, arguments);
    if (result < 0) {
        free(buffer);
        *output = nullptr;
        return result;
    }
    *output = buffer;
    return result;
}

static int orb_asprintf(char **output, const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    int result = orb_vasprintf(output, format, arguments);
    va_end(arguments);
    return result;
}
#else
#define orb_vasprintf vasprintf
#define orb_asprintf asprintf
#endif

static char* error_message = nullptr;

namespace ORB {
void precondition_failure(const char *format, ...) {
    char* s = nullptr;
    va_list va;
    va_start(va, format);
    orb_vasprintf(&s, format, va);
    va_end(va);
    if (s != nullptr) {
        #if ORB_TARGET_OS_DARWIN
        os_log_error(error_log(), "precondition failure: %s", s);
        #endif /* ORB_TARGET_OS_DARWIN */
        if (error_message == nullptr) {
            orb_asprintf(&error_message, "OpenRenderBox precondition failure: %s.\n", s);
        }
        free(s);
    }
    abort();
}

void non_fatal_precondition_failure(const char *format, ...) {
    char* s = nullptr;
    va_list va;
    va_start(va, format);
    orb_vasprintf(&s, format, va);
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
