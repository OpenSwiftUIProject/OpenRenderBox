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

#include <stdint.h>
#include <stdbool.h>
#include "ORBSwiftSupport.h"
#include "ORBTargetConditionals.h"

#if ORB_TARGET_OS_DARWIN

#include <CoreFoundation/CoreFoundation.h>

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

#else

#if __has_attribute(enum_extensibility)
#define ORB_ENUM_ATTRIBUTES __attribute__((enum_extensibility(open)))
#define ORB_CLOSED_ENUM_ATTRIBUTES __attribute__((enum_extensibility(closed)))
#define ORB_OPTIONS_ATTRIBUTES __attribute__((flag_enum, enum_extensibility(open)))
#else
#define ORB_ENUM_ATTRIBUTES
#define ORB_CLOSED_ENUM_ATTRIBUTES
#define ORB_OPTIONS_ATTRIBUTES
#endif

#if defined(__OBJC__)
#define ORB_ENUM(_type, _name) enum ORB_ENUM_ATTRIBUTES _name : _type _name; enum _name : _type
#define ORB_CLOSED_ENUM(_type, _name) enum ORB_CLOSED_ENUM_ATTRIBUTES _name : _type _name; enum _name : _type
#define ORB_OPTIONS(_type, _name) _type _name; enum ORB_OPTIONS_ATTRIBUTES : _name
#elif defined(__cplusplus)
#define ORB_ENUM(_type, _name) _type _name; enum ORB_ENUM_ATTRIBUTES
#define ORB_CLOSED_ENUM(_type, _name) _type _name; enum ORB_CLOSED_ENUM_ATTRIBUTES
#define ORB_OPTIONS(_type, _name) _type _name; enum ORB_OPTIONS_ATTRIBUTES
#define ORB_EXTERN_C_BEGIN extern "C" {
#define ORB_EXTERN_C_END }
#else
#define ORB_ENUM(_type, _name) _type _name; enum ORB_ENUM_ATTRIBUTES
#define ORB_CLOSED_ENUM(_type, _name) _type _name; enum ORB_CLOSED_ENUM_ATTRIBUTES
#define ORB_OPTIONS(_type, _name) _type _name; enum ORB_OPTIONS_ATTRIBUTES
#define ORB_EXTERN_C_BEGIN
#define ORB_EXTERN_C_END
#endif

#if __has_feature(assume_nonnull)
#define ORB_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define ORB_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
#else
#define ORB_ASSUME_NONNULL_BEGIN
#define ORB_ASSUME_NONNULL_END
#endif

#define ORB_IMPLICIT_BRIDGING_ENABLED
#define ORB_IMPLICIT_BRIDGING_DISABLED
#define ORB_EXPORT extern __attribute__((visibility("default")))
#define ORB_BRIDGED_TYPE(_type)

#endif /* ORB_TARGET_OS_DARWIN */

#if ORB_TARGET_OS_DARWIN && __OBJC__
#define ORB_OBJC_FOUNDATION 1
#else
#define ORB_OBJC_FOUNDATION 0
#endif /* TARGET_OS_DARWIN && __OBJC__ */

#if !ORB_TARGET_OS_DARWIN
#include "CFCGTypes.h"
#endif
