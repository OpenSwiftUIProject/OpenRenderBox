//
//  ORBSwiftSupport.h
//  OpenRenderBox

#pragma once

#if __has_attribute(swift_name)
#define ORB_SWIFT_NAME(_name) __attribute__((swift_name(#_name)))
#else
#define ORB_SWIFT_NAME
#endif

#if __has_attribute(swift_wrapper)
#define ORB_SWIFT_STRUCT __attribute__((swift_wrapper(struct)))
#else
#define ORB_SWIFT_STRUCT
#endif

#if __has_attribute(swift_private)
#define ORB_REFINED_FOR_SWIFT __attribute__((swift_private))
#else
#define ORB_REFINED_FOR_SWIFT
#endif

// MARK: - Call Convension

#define ORB_SWIFT_CC(CC) ORB_SWIFT_CC_##CC
// ORB_SWIFT_CC(c) is the C calling convention.
#define ORB_SWIFT_CC_c

// ORB_SWIFT_CC(swift) is the Swift calling convention.
#if __has_attribute(swiftcall)
#define ORB_SWIFT_CC_swift __attribute__((swiftcall))
#define ORB_SWIFT_CONTEXT __attribute__((swift_context))
#define ORB_SWIFT_ERROR_RESULT __attribute__((swift_error_result))
#define ORB_SWIFT_INDIRECT_RESULT __attribute__((swift_indirect_result))
#else
#define ORB_SWIFT_CC_swift
#define ORB_SWIFT_CONTEXT
#define ORB_SWIFT_ERROR_RESULT
#define ORB_SWIFT_INDIRECT_RESULT
#endif

