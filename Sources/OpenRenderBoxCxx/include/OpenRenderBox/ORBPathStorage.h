//
//  ORBPathStorage.h
//  OpenRenderBox
//
//  Audited for 6.5.1
//  Status: Complete

#pragma once

#if OPENRENDERBOX_CF_CGTYPE

#include <OpenRenderBox/ORBBase.h>
#include <OpenRenderBox/ORBPath.h>
#if ORB_TARGET_OS_DARWIN
#include <CoreGraphics/CoreGraphics.h>
#endif

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

ORB_EXPORT
void ORBPathStorageInit(ORBPathStorageRef dst, uint32_t capacity, ORBPathStorageRef _Nullable source) ORB_SWIFT_NAME(ORBPathStorageRef.initialize(self:capacity:source:));

ORB_EXPORT
void ORBPathStorageDestroy(ORBPathStorageRef storage) ORB_SWIFT_NAME(ORBPathStorageRef.destroy(self:));

ORB_EXPORT
void ORBPathStorageClear(ORBPathStorageRef storage) ORB_SWIFT_NAME(ORBPathStorageRef.clear(self:));

ORB_EXPORT
bool ORBPathStorageAppendElement(ORBPathStorageRef storage, ORBPathElement element, CGFloat const * points, const void * _Nullable userInfo) ORB_SWIFT_NAME(ORBPathStorageRef.append(self:element:points:userInfo:));

ORB_EXPORT
bool ORBPathStorageAppendPath(ORBPathStorageRef, ORBPath) ORB_SWIFT_NAME(ORBPathStorageRef.append(self:path:));

ORB_EXPORT
bool ORBPathStorageApplyElements(ORBPathStorageRef, void *info, ORBPathApplyCallback _Nullable callback) ORB_SWIFT_NAME(ORBPathStorageRef.apply(self:info:callback:));

ORB_EXPORT
bool ORBPathStorageIsEmpty(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.isEmpty(self:));

ORB_EXPORT
bool ORBPathStorageEqualToStorage(ORBPathStorageRef lhs, ORBPathStorageRef rhs) ORB_SWIFT_NAME(ORBPathStorageRef.isEqual(self:to:));

ORB_EXPORT
bool ORBPathStorageIsSingleElement(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.isSingleElement(self:));

ORB_EXPORT
uint32_t ORBPathStorageGetBezierOrder(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.bezierOrder(self:));

#if ORB_TARGET_OS_DARWIN
ORB_EXPORT
CGRect ORBPathStorageGetBoundingRect(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.boundingRect(self:));

ORB_EXPORT
CF_RETURNS_NOT_RETAINED
__nullable CGPathRef ORBPathStorageGetCGPath(ORBPathStorageRef storage) ORB_SWIFT_NAME(getter:ORBPathStorageRef.cgPath(self:));
#endif

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

#endif /* OPENRENDERBOX_CF_CGTYPE */
