//
//  ORBBase.h
//  OpenRenderBox

#pragma once

#if DEBUG
#define ORB_ASSERTION
#else
#undef ORB_ASSERTION
#endif

#if __has_attribute(cold)
#define __cold          __attribute__((__cold__))
#else
#define __cold
#endif

#if __has_attribute(noreturn)
#define __dead2         __attribute__((__noreturn__))
#else
#define __dead2
#endif

#if defined(__cplusplus)
#define ORB_NOEXCEPT noexcept
#else
#define ORB_NOEXCEPT
#endif

#if defined(__GNUC__)
#define ORB_INLINE __inline__ __attribute__((always_inline))
#elif defined(__cplusplus)
#define ORB_INLINE inline
#else
#define ORB_INLINE
#endif

#if defined(__cplusplus)
#define ORB_CONSTEXPR constexpr
#else
#define ORB_CONSTEXPR
#endif

#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>
#include <stdbool.h>
#include "ORBSwiftSupport.h"
#include "ORBTargetConditionals.h"

#define ORB_ENUM CF_ENUM
#define ORB_CLOSED_ENUM CF_CLOSED_ENUM
#define ORB_OPTIONS CF_OPTIONS
#define ORB_EXTERN_C_BEGIN CF_EXTERN_C_BEGIN
#define ORB_EXTERN_C_END CF_EXTERN_C_END
#define ORB_ASSUME_NONNULL_BEGIN CF_ASSUME_NONNULL_BEGIN
#define ORB_ASSUME_NONNULL_END CF_ASSUME_NONNULL_END
#define ORB_IMPLICIT_BRIDGING_ENABLED CF_IMPLICIT_BRIDGING_ENABLED
#define ORB_IMPLICIT_BRIDGING_DISABLED CF_IMPLICIT_BRIDGING_DISABLED
#define ORB_EXPORT CF_EXPORT
#define ORB_BRIDGED_TYPE CF_BRIDGED_TYPE

#if ORB_TARGET_OS_DARWIN && __ORBJC__
#define ORB_ORBJC_FOUNDATION 1
#else
#define ORB_ORBJC_FOUNDATION 0
#endif /* TARGET_OS_DARWIN && __ORBJC__ */

#if !ORB_TRRET_OS_DARWIN
#include "CFCGTypes.h"
#endif