//
//  OBPathStorage.h
//  OpenBox

#ifndef OBPathStorage_h
#define OBPathStorage_h

#include "OBBase.h"
#include "OBPath.h"

#if OB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

OB_ASSUME_NONNULL_BEGIN

OB_EXTERN_C_BEGIN

OB_EXPORT
OB_REFINED_FOR_SWIFT
void OBPathStorageInit(OBPathStorageRef dst, uint32_t capacity, OBPathStorageRef _Nullable source);

OB_EXPORT
OB_REFINED_FOR_SWIFT
void OBPathStorageDestroy(OBPathStorageRef storage) OB_SWIFT_NAME(OBPathStorage.destroy(self:));

OB_EXPORT
OB_REFINED_FOR_SWIFT
void OBPathStorageClear(OBPathStorageRef storage) OB_SWIFT_NAME(OBPathStorage.clear(self:));

//OB_EXPORT
//OB_REFINED_FOR_SWIFT
//void OBPathStorageAppendPath(OBPathStorage, OBPath);

OB_EXPORT
OB_REFINED_FOR_SWIFT
bool OBPathStorageIsEmpty(OBPathStorageRef storage) OB_SWIFT_NAME(getter:OBPathStorage.isEmpty(self:));

OB_EXPORT
OB_REFINED_FOR_SWIFT
bool OBPathStorageEqualToStorage(OBPathStorageRef lhs, OBPathStorageRef rhs) OB_SWIFT_NAME(OBPathStorage.isEqualTo(self:_:));

OB_EXPORT
OB_REFINED_FOR_SWIFT
bool OBPathStorageIsSingleElement(OBPathStorageRef storage) OB_SWIFT_NAME(getter:OBPathStorage.isSingleElement(self:));

OB_EXPORT
OB_REFINED_FOR_SWIFT
uint32_t OBPathStorageGetBezierOrder(OBPathStorageRef storage) OB_SWIFT_NAME(getter:OBPathStorage.bezierOrder(self:));

#if OB_TARGET_OS_DARWIN
OB_EXPORT
OB_REFINED_FOR_SWIFT
CGRect OBPathStorageGetBoundingRect(OBPathStorageRef storage) OB_SWIFT_NAME(getter:OBPathStorage.boundingRect(self:));

OB_EXPORT
OB_REFINED_FOR_SWIFT
CGPathRef RBPathStorageGetCGPath(OBPathStorageRef storage) OB_SWIFT_NAME(getter:OBPathStorage.cgPath(self:));
#endif

OB_EXTERN_C_END

OB_ASSUME_NONNULL_END

#endif /* OBPathStorage_h */
