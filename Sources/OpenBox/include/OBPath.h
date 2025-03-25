//
//  OBPath.h
//  OpenBox

#ifndef OBPath_h
#define OBPath_h

#include "OBBase.h"

OB_ASSUME_NONNULL_BEGIN

OB_EXTERN_C_BEGIN

typedef struct OB_BRIDGED_TYPE(id) OBPathStorage * OBPathStorageRef OB_SWIFT_NAME(OBPath.Storage);

struct OBPathStorage;

typedef struct OBPath {
    OBPathStorageRef storage;
    void *callbacks;
} OBPath;

OB_EXPORT
OB_REFINED_FOR_SWIFT
void OBPathRetain(OBPath path) OB_SWIFT_NAME(OBPath.retain(self:));

OB_EXPORT
OB_REFINED_FOR_SWIFT
void OBPathRelease(OBPath path) OB_SWIFT_NAME(OBPath.release(self:));

OB_EXTERN_C_END

OB_ASSUME_NONNULL_END

#endif /* OBPath_h */
