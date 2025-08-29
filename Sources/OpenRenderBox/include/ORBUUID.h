//
//  ORBUUID.hpp
//  OpenRenderBox

#ifndef ORBUUID_hpp
#define ORBUUID_hpp

#include "ORBBase.h"

#if ORB_TARGET_OS_DARWIN
#include <Foundation/Foundation.h>
#endif

typedef struct ORBUUID {
    uint8_t bytes[16];
} ORBUUID;

ORB_EXTERN_C_BEGIN

#if ORB_TARGET_OS_DARWIN
ORB_EXPORT
ORB_REFINED_FOR_SWIFT
ORBUUID ORBUUIDInitFromNSUUID(NSUUID *uuid) ORB_SWIFT_NAME(ORBUUID.init(uuid:));
#endif

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
ORBUUID ORBUUIDInitFromHash(uint64_t words0, uint64_t words1, uint32_t words2) ORB_SWIFT_NAME(ORBUUID.init(_:_:_:));

ORB_EXTERN_C_END

#endif /* ORBUUID_hpp */
