//
//  OBSwiftSupport.h
//  OpenBox

#ifndef OBSwiftSupport_h
#define OBSwiftSupport_h

#if __has_attribute(swift_name)
#define OB_SWIFT_NAME(_name) __attribute__((swift_name(#_name)))
#else
#define OB_SWIFT_NAME
#endif

#if __has_attribute(swift_wrapper)
#define OB_SWIFT_STRUCT __attribute__((swift_wrapper(struct)))
#else
#define OB_SWIFT_STRUCT
#endif

#if __has_attribute(swift_private)
#define OB_REFINED_FOR_SWIFT __attribute__((swift_private))
#else
#define OB_REFINED_FOR_SWIFT
#endif

// MARK: - Call Convension

#define OB_SWIFT_CC(CC) OB_SWIFT_CC_##CC
// OB_SWIFT_CC(c) is the C calling convention.
#define OB_SWIFT_CC_c

// OB_SWIFT_CC(swift) is the Swift calling convention.
#if __has_attribute(swiftcall)
#define OB_SWIFT_CC_swift __attribute__((swiftcall))
#define OB_SWIFT_CONTEXT __attribute__((swift_context))
#define OB_SWIFT_ERROR_RESULT __attribute__((swift_error_result))
#define OB_SWIFT_INDIRECT_RESULT __attribute__((swift_indirect_result))
#else
#define OB_SWIFT_CC_swift
#define OB_SWIFT_CONTEXT
#define OB_SWIFT_ERROR_RESULT
#define OB_SWIFT_INDIRECT_RESULT
#endif

#endif /* OBSwiftSupport_h */
