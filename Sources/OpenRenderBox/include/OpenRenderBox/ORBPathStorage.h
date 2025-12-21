//
//  ORBPathStorage.h
//  OpenRenderBox

#pragma once

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBPath.h>

#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathStorageInit(ORBPathStorageRef dst, uint32_t capacity, ORBPathStorageRef _Nullable source);

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathStorageDestroy(ORBPathStorageRef storage) ORB_SWIFT_NAME(ORBPathStorageRef.destroy(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathStorageClear(ORBPathStorageRef storage) ORB_SWIFT_NAME(ORBPathStorageRef.clear(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathStorageAppendPath(ORBPathStorageRef, ORBPath) ORB_SWIFT_NAME(ORBPathStorageRef.append(self:path:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
bool ORBPathStorageIsEmpty(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.isEmpty(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
bool ORBPathStorageEqualToStorage(ORBPathStorageRef lhs, ORBPathStorageRef rhs) ORB_SWIFT_NAME(ORBPathStorageRef.isEqualTo(self:_:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
bool ORBPathStorageIsSingleElement(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.isSingleElement(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
uint32_t ORBPathStorageGetBezierOrder(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.bezierOrder(self:));

#if ORB_TARGET_OS_DARWIN
ORB_EXPORT
ORB_REFINED_FOR_SWIFT
CGRect ORBPathStorageGetBoundingRect(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.boundingRect(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
CGPathRef RBPathStorageGetCGPath(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.cgPath(self:));
#endif

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

