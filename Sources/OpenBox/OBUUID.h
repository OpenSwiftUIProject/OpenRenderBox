//
//  OBUUID.hpp
//  OpenBox

#ifndef OBUUID_hpp
#define OBUUID_hpp

#include "OBBase.h"
#include <Foundation/Foundation.h>

typedef struct OBUUID {
    uint8_t bytes[16];
} OBUUID;

OB_EXTERN_C_BEGIN

OB_EXPORT
OB_REFINED_FOR_SWIFT
OBUUID OBUUIDInitFromNSUUID(NSUUID *uuid) OB_SWIFT_NAME(OBUUID.init(uuid:));

OB_EXPORT
OB_REFINED_FOR_SWIFT
OBUUID OBUUIDInitFromHash(uint64_t words0, uint64_t words1, uint32_t words2) OB_SWIFT_NAME(OBUUID.init(hash:_:_:));

OB_EXTERN_C_END

#endif /* OBUUID_hpp */
