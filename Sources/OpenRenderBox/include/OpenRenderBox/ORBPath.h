//
//  ORBPath.h
//  OpenRenderBox

#pragma once

#include "ORBBase.h"

ORB_ASSUME_NONNULL_BEGIN

ORB_EXTERN_C_BEGIN

typedef struct ORB_BRIDGED_TYPE(id) ORBPathStorage * ORBPathStorageRef ORB_SWIFT_NAME(ORBPath.Storage);

struct ORBPathStorage;

typedef struct ORBPath {
    ORBPathStorageRef storage;
    void *callbacks;
} ORBPath;

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathRetain(ORBPath path) ORB_SWIFT_NAME(ORBPath.retain(self:));

ORB_EXPORT
ORB_REFINED_FOR_SWIFT
void ORBPathRelease(ORBPath path) ORB_SWIFT_NAME(ORBPath.release(self:));

ORB_EXTERN_C_END

ORB_ASSUME_NONNULL_END

