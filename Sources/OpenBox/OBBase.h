//
//  OBBase.h
//  OpenBox

#ifndef OBBase_h
#define OBBase_h

#include "OBVersion.h"

#if DEBUG
#define OB_ASSERTION
#else
#undef OB_ASSERTION
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
#define OB_NOEXCEPT noexcept
#else
#define OB_NOEXCEPT
#endif

#if defined(__GNUC__)
#define OB_INLINE __inline__ __attribute__((always_inline))
#elif defined(__cplusplus)
#define OB_INLINE inline
#else
#define OB_INLINE
#endif

#if defined(__cplusplus)
#define OB_CONSTEXPR constexpr
#else
#define OB_CONSTEXPR
#endif

#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>
#include <stdbool.h>
#include "OBTargetConditionals.h"
#include "OBSwiftSupport.h"

#define OB_ENUM CF_ENUM
#define OB_OPTIONS CF_OPTIONS
#define OB_EXTERN_C_BEGIN CF_EXTERN_C_BEGIN
#define OB_EXTERN_C_END CF_EXTERN_C_END
#define OB_ASSUME_NONNULL_BEGIN CF_ASSUME_NONNULL_BEGIN
#define OB_ASSUME_NONNULL_END CF_ASSUME_NONNULL_END
#define OB_EXPORT CF_EXPORT
#define OB_BRIDGED_TYPE CF_BRIDGED_TYPE

#if TARGET_OS_DARWIN && __OBJC__
#define OB_OBJC_FOUNDATION 1
#else
#define OB_OBJC_FOUNDATION 0
#endif /* TARGET_OS_DARWIN && __OBJC__ */

#endif /* OBBase_h */
