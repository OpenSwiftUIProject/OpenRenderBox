//
//  ORBPath.cpp
//  OpenRenderBox
//
//  Created by Kyle on 2025/3/25.
//

#if OPENRENDERBOX_CF_CGTYPES

#include <OpenRenderBox/ORBPath.h>
#include <OpenRenderBox/ORBPathCallbacks.h>
#include "Storage.hpp"
#include "NestedCallbacks.hpp"
#include "PathMapper.hpp"
#include <Util/assert.hpp>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>

// Empty path callbacks (all null) - C++ internal linkage
static const ORBPathCallbacks empty_path_callbacks = {
    {},
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

// Empty path (storage = null)
const ORBPath ORBPathEmpty = {
    nullptr,
    &empty_path_callbacks,
};

// Null path (storage = 0x1)
const ORBPath ORBPathNull = {
    reinterpret_cast<ORBPathStorage *>(0x1),
    &empty_path_callbacks,
};

ORBPath ORBPathRetain(ORBPath path) {
    auto retain = path.callbacks->retain;
    if (retain != nullptr) {
        path.storage = reinterpret_cast<ORBPathStorage *>(
            const_cast<void *>(retain(path.storage))
        );
    }
    return path;
}

void ORBPathRelease(ORBPath path) {
    auto release = path.callbacks->release;
    if (release != nullptr) {
        release(path.storage);
    }
}

#if ORB_TARGET_OS_DARWIN

CG_EXTERN bool CGPathIsPrimitive(CGPathRef path);
CG_EXTERN bool CGPathIsRectWithTransform(
    CGPathRef path,
    CGRect *rect,
    CGAffineTransform *transform
);
CG_EXTERN bool CGPathIsEllipseWithTransform(
    CGPathRef path,
    CGRect *rect,
    CGAffineTransform *transform
);
CG_EXTERN bool CGPathIsRoundedRectWithTransform(
    CGPathRef path,
    CGRect *rect,
    CGFloat *cornerWidth,
    CGFloat *cornerHeight,
    CGAffineTransform *transform
);
CG_EXTERN void CGPathAddContinuousRoundedRect(
    CGMutablePathRef path,
    const CGAffineTransform * _Nullable transform,
    CGRect rect,
    CGFloat cornerWidth,
    CGFloat cornerHeight
);


// MARK: - Path Creation

namespace {
static_assert(sizeof(ORBPathRectShape) == 0x50);
static_assert(offsetof(ORBPathRectShape, transform) == 0x00);
static_assert(offsetof(ORBPathRectShape, rect) == 0x30);
static_assert(sizeof(ORBPathLineShape) == 0x20);
static_assert(sizeof(ORBPathFixedRoundedRectShape) == 0x68);
static_assert(offsetof(ORBPathFixedRoundedRectShape, cornerSize) == 0x50);
static_assert(offsetof(ORBPathFixedRoundedRectShape, style) == 0x60);
static_assert(sizeof(ORBPathStrokeStyle) == 0x30);
static_assert(offsetof(ORBPathStrokeStyle, dashCount) == 0x20);
static_assert(offsetof(ORBPathStrokeStyle, dash) == 0x28);
static_assert(sizeof(ORBPathStrokedShape) == 0x40);
static_assert(offsetof(ORBPathStrokedShape, style) == 0x10);
static_assert(sizeof(ORBPathShape) == 0x78);

struct RectPathArguments {
    CGRect rect;
    uint8_t element;
    uint8_t padding[7] {};
};

struct UnevenRoundedRectPathArguments {
    CGRect rect;
    CGFloat radii[4];
    bool continuous;
    uint8_t padding[7] {};
};

struct EllipsePathArguments {
    CGRect rect;
};

struct RoundedRectPathArguments {
    CGRect rect;
    CGSize cornerSize;
    bool continuous;
    uint8_t padding[7] {};
};

static_assert(sizeof(RectPathArguments) == 0x28);
static_assert(sizeof(EllipsePathArguments) == 0x20);
static_assert(sizeof(RoundedRectPathArguments) == 0x38);

template<typename Arguments>
struct SingleElementPathStorage {
    uint32_t referenceCount { 1 };
    Arguments arguments;
    bool hasTransform;
    CGAffineTransform transform;

    SingleElementPathStorage(
        const Arguments &arguments,
        const CGAffineTransform *transform
    ) : arguments(arguments),
        hasTransform(transform != nullptr && !CGAffineTransformIsIdentity(*transform)),
        transform(hasTransform ? *transform : CGAffineTransformIdentity) {}

    const CGAffineTransform *transformPointer() const {
        return hasTransform ? &transform : nullptr;
    }
};

using UnevenRoundedRectPathStorage = SingleElementPathStorage<UnevenRoundedRectPathArguments>;
using RectPathStorage = SingleElementPathStorage<RectPathArguments>;
using EllipsePathStorage = SingleElementPathStorage<EllipsePathArguments>;
using RoundedRectPathStorage = SingleElementPathStorage<RoundedRectPathArguments>;

template<typename Storage>
const void *retain_single_element_path(const void *object) {
    auto storage = const_cast<Storage *>(static_cast<const Storage *>(object));
    __atomic_fetch_add(&storage->referenceCount, 1, __ATOMIC_RELAXED);
    return object;
}

template<typename Storage>
void release_single_element_path(const void *object) {
    auto storage = const_cast<Storage *>(static_cast<const Storage *>(object));
    if (__atomic_fetch_sub(&storage->referenceCount, 1, __ATOMIC_ACQ_REL) == 1) {
        delete storage;
    }
}

template<typename Storage>
bool single_element_paths_equal(const void *object, const void *otherObject) {
    auto lhs = static_cast<const Storage *>(object);
    auto rhs = static_cast<const Storage *>(otherObject);
    if (lhs->hasTransform != rhs->hasTransform
        || std::memcmp(&lhs->arguments, &rhs->arguments, sizeof(lhs->arguments)) != 0) {
        return false;
    }
    return !lhs->hasTransform
        || CGAffineTransformEqualToTransform(lhs->transform, rhs->transform);
}

static const ORBPathCallbacks rect_path_callbacks = {
    .flags = {},
    .retain = retain_single_element_path<RectPathStorage>,
    .release = release_single_element_path<RectPathStorage>,
    .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) -> bool {
        if (callback == nullptr) {
            return true;
        }
        auto storage = static_cast<const RectPathStorage *>(object);
        return callback(
            info,
            static_cast<ORBPathElement>(storage->arguments.element),
            reinterpret_cast<const CGFloat *>(&storage->arguments.rect),
            storage->transformPointer()
        );
    },
    .isEqual = single_element_paths_equal<RectPathStorage>,
    .isEmpty = +[](const void *) -> bool { return false; },
    .isSingleElement = +[](const void *) -> bool { return true; },
    .bezierOrder = +[](const void *) -> size_t { return 1; },
    .boundingRect = nullptr,
    .cgPath = nullptr,
    .next = nullptr,
};

static const ORBPathCallbacks ellipse_path_callbacks = {
    .flags = {},
    .retain = retain_single_element_path<EllipsePathStorage>,
    .release = release_single_element_path<EllipsePathStorage>,
    .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) -> bool {
        if (callback == nullptr) {
            return true;
        }
        auto storage = static_cast<const EllipsePathStorage *>(object);
        return callback(
            info,
            ORBPathElementEllipse,
            reinterpret_cast<const CGFloat *>(&storage->arguments.rect),
            storage->transformPointer()
        );
    },
    .isEqual = single_element_paths_equal<EllipsePathStorage>,
    .isEmpty = +[](const void *) -> bool { return false; },
    .isSingleElement = +[](const void *) -> bool { return true; },
    .bezierOrder = +[](const void *) -> size_t { return 3; },
    .boundingRect = nullptr,
    .cgPath = nullptr,
    .next = nullptr,
};

static const ORBPathCallbacks rounded_rect_path_callbacks = {
    .flags = {},
    .retain = retain_single_element_path<RoundedRectPathStorage>,
    .release = release_single_element_path<RoundedRectPathStorage>,
    .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) -> bool {
        if (callback == nullptr) {
            return true;
        }
        auto storage = static_cast<const RoundedRectPathStorage *>(object);
        return callback(
            info,
            storage->arguments.continuous
                ? ORBPathElementFixedRoundedRectContinuous
                : ORBPathElementFixedRoundedRectCircular,
            reinterpret_cast<const CGFloat *>(&storage->arguments.rect),
            storage->transformPointer()
        );
    },
    .isEqual = single_element_paths_equal<RoundedRectPathStorage>,
    .isEmpty = +[](const void *) -> bool { return false; },
    .isSingleElement = +[](const void *) -> bool { return true; },
    .bezierOrder = +[](const void *) -> size_t { return 3; },
    .boundingRect = nullptr,
    .cgPath = nullptr,
    .next = nullptr,
};

ORBPath make_rect(CGRect rect, const CGAffineTransform *transform, ORBPathElement element) {
    if (CGRectIsNull(rect)) {
        return ORBPathEmpty;
    }
    RectPathArguments arguments = {};
    arguments.rect = rect;
    arguments.element = static_cast<uint8_t>(element);
    auto storage = new RectPathStorage(arguments, transform);
    return ORBPath {
        reinterpret_cast<ORBPathStorage *>(storage),
        &rect_path_callbacks,
    };
}
} /* anonymous namespace */

ORBPath ORBPathMakeWithCGPath(CGPathRef cgPath) {
    if (cgPath == nullptr) {
        return ORBPathNull;
    }
    CFRetain(cgPath);
    return ORBPath {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
}

ORBPath ORBPathMakeRect(CGRect rect, const CGAffineTransform *transform) {
    return make_rect(rect, transform, ORBPathElementRect);
}

ORBPath ORBPathMakeEllipse(CGRect rect, const CGAffineTransform *transform) {
    if (CGRectIsNull(rect)) {
        return ORBPathEmpty;
    }
    EllipsePathArguments arguments = { rect };
    auto storage = new EllipsePathStorage(arguments, transform);
    return ORBPath {
        reinterpret_cast<ORBPathStorage *>(storage),
        &ellipse_path_callbacks,
    };
}

ORBPath ORBPathMakeRoundedRect(CGRect rect, CGFloat cornerWidth, CGFloat cornerHeight, ORBPathRoundedCornerStyle style, const CGAffineTransform *transform) {
    if (cornerWidth == 0 && cornerHeight == 0) {
        return make_rect(rect, transform, ORBPathElementRoundedRect);
    }
    if (CGRectIsNull(rect)) {
        return ORBPathEmpty;
    }
    RoundedRectPathArguments arguments = {};
    arguments.rect = rect;
    arguments.cornerSize = CGSizeMake(cornerWidth, cornerHeight);
    arguments.continuous = style == ORBPathRoundedCornerStyleContinuous;
    auto storage = new RoundedRectPathStorage(arguments, transform);
    return ORBPath {
        reinterpret_cast<ORBPathStorage *>(storage),
        &rounded_rect_path_callbacks,
    };
}

ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat topRightRadius, CGFloat bottomRightRadius, CGFloat bottomLeftRadius, ORBPathRoundedCornerStyle style, const CGAffineTransform *transform) {
    if (CGRectIsNull(rect)) {
        return ORBPathEmpty;
    }
    if (topLeftRadius == 0 && topRightRadius == 0 && bottomRightRadius == 0 && bottomLeftRadius == 0) {
        return make_rect(rect, transform, ORBPathElementRoundedRect);
    }
    static const ORBPathCallbacks callbacks = {
        .flags = {},
        .retain = retain_single_element_path<UnevenRoundedRectPathStorage>,
        .release = release_single_element_path<UnevenRoundedRectPathStorage>,
        .apply = +[](const void *object, void *info, ORBPathApplyCallback callback) -> bool {
            auto storage = static_cast<const UnevenRoundedRectPathStorage *>(object);
            return callback(info, storage->arguments.continuous
                ? ORBPathElementUnevenRoundedRectContinuous : ORBPathElementUnevenRoundedRectCircular,
                reinterpret_cast<const CGFloat *>(&storage->arguments.rect), storage->transformPointer());
        },
        .isEqual = single_element_paths_equal<UnevenRoundedRectPathStorage>,
        .isEmpty = +[](const void *) -> bool { return false; },
        .isSingleElement = +[](const void *) -> bool { return true; },
        .bezierOrder = +[](const void *) -> size_t { return 3; },
        .boundingRect = nullptr,
        .cgPath = nullptr,
        .next = nullptr,
    };
    UnevenRoundedRectPathArguments args = {rect, {topLeftRadius, topRightRadius, bottomRightRadius, bottomLeftRadius}, style == ORBPathRoundedCornerStyleContinuous};
    return {reinterpret_cast<ORBPathStorage *>(new UnevenRoundedRectPathStorage(args, transform)), &callbacks};
}

ORBPath ORBPathCopyTransformedPath(ORBPath path, const CGAffineTransform *transform) {
    if (ORBPathIsNull(path)) {
        return ORBPathNull;
    }
    CGPathRef source = ORBPathCopyCGPath(path);
    CGPathRef transformed = CGPathCreateCopyByTransformingPath(source, transform);
    ORBPath result = ORBPathMakeWithCGPath(transformed);
    CGPathRelease(transformed);
    CGPathRelease(source);
    return result;
}

#endif /* ORB_TARGET_OS_DARWIN */

bool ORBPathIsEmpty(ORBPath path) {
    if (path.callbacks == &empty_path_callbacks) {
        return true;
    }
    if (path.callbacks->flags.isExtended) {
        auto extended = reinterpret_cast<const ORBPathCallbacksExtended*>(path.callbacks);
        auto isEmptyCallback = extended->isEmpty;
        if (isEmptyCallback) {
            return isEmptyCallback(path.storage, extended);
        } else {
            bool isEmpty = true;
            auto applyCallback = extended->apply;
            if (applyCallback) {
                applyCallback(path.storage, &isEmpty, +[](void * info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo) -> bool {
                    *((bool *)info) = false;
                    return false;
                }, extended);
            }
            return isEmpty;
        }
    } else {
        auto isEmptyCallback = path.callbacks->isEmpty;
        if (isEmptyCallback) {
            return isEmptyCallback(path.storage);
        } else {
            bool isEmpty = true;
            auto applyCallback = path.callbacks->apply;
            if (applyCallback) {
                applyCallback(path.storage, &isEmpty, +[](void * info, ORBPathElement element, const CGFloat *points, const void * _Nullable userInfo) -> bool {
                    *((bool *)info) = false;
                    return false;
                });
            }
            return isEmpty;
        }
    }
}

bool ORBPathIsSingleElement(ORBPath path) {
    if (path.callbacks == &empty_path_callbacks) {
        return false;
    }
    if (path.callbacks->flags.isExtended) {
        auto extended = reinterpret_cast<const ORBPathCallbacksExtended *>(path.callbacks);
        if (extended->isSingleElement != nullptr) {
            return extended->isSingleElement(path.storage, extended);
        }
    } else if (path.callbacks->isSingleElement != nullptr) {
        return path.callbacks->isSingleElement(path.storage);
    }

    ORB::Path::NestedCallbacks::SingleInfo count;
    ORBPathApplyElements(path, &count, ORB::Path::NestedCallbacks::single_element_callback);
    return count.count == 1;
}

size_t ORBPathGetBezierOrder(ORBPath path) {
    if (path.callbacks->flags.isExtended) {
        auto *extended = reinterpret_cast<const ORBPathCallbacksExtended *>(path.callbacks);
        return extended->bezierOrder ? extended->bezierOrder(path.storage, extended) : 3;
    }
    return path.callbacks->bezierOrder ? path.callbacks->bezierOrder(path.storage) : 3;
}

bool ORBPathApplyElements(ORBPath path, void *info, ORBPathApplyCallback callback) {
    if (callback == nullptr) {
        return true;
    }
    if (path.callbacks->flags.isExtended) {
        auto extended = reinterpret_cast<const ORBPathCallbacksExtended*>(path.callbacks);
        auto applyCallback = extended->apply;
        if (applyCallback) {
            return applyCallback(path.storage, info, callback, extended);
        }
        return true;
    } else {
        auto applyCallback = path.callbacks->apply;
        if (applyCallback) {
            return applyCallback(path.storage, info, callback);
        }
        return true;
    }
}

bool ORBPathApplyShape(
    ORBPath path,
    bool allowLine,
    void *info,
    ORBPathApplyShapeCallback callback
) {
    if (callback == nullptr) {
        return false;
    }

#if ORB_TARGET_OS_DARWIN
    if (path.callbacks == &ORBPathCGPathCallbacks) {
        auto cgPath = reinterpret_cast<CGPathRef>(path.storage);
        if (!CGPathIsPrimitive(cgPath)) {
            return false;
        }

        CGRect rect = CGRectZero;
        CGAffineTransform transform = CGAffineTransformIdentity;
        ORBPathShape shape = {};
        if (CGPathIsRectWithTransform(cgPath, &rect, &transform)) {
            shape.rect = { transform, rect };
            callback(info, ORBPathShapeTypeRect, &shape);
            return true;
        }
        if (CGPathIsEllipseWithTransform(cgPath, &rect, &transform)) {
            shape.ellipse = { transform, rect };
            callback(info, ORBPathShapeTypeEllipse, &shape);
            return true;
        }

        CGFloat cornerWidth = 0;
        CGFloat cornerHeight = 0;
        if (CGPathIsRoundedRectWithTransform(
            cgPath,
            &rect,
            &cornerWidth,
            &cornerHeight,
            &transform
        )) {
            if (cornerWidth == 0 && cornerHeight == 0) {
                shape.rect = { transform, rect };
                callback(info, ORBPathShapeTypeRect, &shape);
                return true;
            }
            shape.fixedRoundedRect = {
                transform,
                rect,
                CGSizeMake(cornerWidth, cornerHeight),
                ORBPathRoundedCornerStyleCircular,
            };
            callback(info, ORBPathShapeTypeFixedRoundedRect, &shape);
            return true;
        }
        return false;
    }
#endif

    if (ORBPathIsSingleElement(path)) {
        struct ShapeInfo {
            void *info;
            ORBPathApplyShapeCallback callback;
            ORBPath path;
            bool matched;
        } shapeInfo = { info, callback, path, false };

        ORBPathApplyElements(path, &shapeInfo, +[](
            void *rawInfo,
            ORBPathElement element,
            const CGFloat *values,
            const void *userInfo
        ) -> bool {
            auto info = static_cast<ShapeInfo *>(rawInfo);
            switch (element) {
            case ORBPathElementRect:
            case ORBPathElementRoundedRect:
            case ORBPathElementEllipse:
            case ORBPathElementFixedRoundedRectCircular:
            case ORBPathElementFixedRoundedRectContinuous:
            case ORBPathElementUnevenRoundedRectCircular:
            case ORBPathElementUnevenRoundedRectContinuous:
                break;
            default:
                return false;
            }
            CGAffineTransform transform = userInfo == nullptr
                ? CGAffineTransformIdentity
                : *static_cast<const CGAffineTransform *>(userInfo);
            CGRect rect = CGRectMake(values[0], values[1], values[2], values[3]);
            ORBPathShape shape = {};
            ORBPathShapeType type;
            switch (element) {
            case ORBPathElementRect:
            case ORBPathElementRoundedRect:
                shape.rect = { transform, rect };
                type = ORBPathShapeTypeRect;
                break;
            case ORBPathElementEllipse:
                shape.ellipse = { transform, rect };
                type = ORBPathShapeTypeEllipse;
                break;
            case ORBPathElementFixedRoundedRectCircular:
            case ORBPathElementFixedRoundedRectContinuous:
                shape.fixedRoundedRect = {
                    transform,
                    rect,
                    CGSizeMake(values[4], values[5]),
                    element == ORBPathElementFixedRoundedRectContinuous
                        ? ORBPathRoundedCornerStyleContinuous
                        : ORBPathRoundedCornerStyleCircular,
                };
                type = ORBPathShapeTypeFixedRoundedRect;
                break;
            case ORBPathElementUnevenRoundedRectCircular:
            case ORBPathElementUnevenRoundedRectContinuous:
                shape.unevenRoundedRect = {
                    transform,
                    rect,
                    values[4], values[5], values[6], values[7],
                    element == ORBPathElementUnevenRoundedRectContinuous
                        ? ORBPathRoundedCornerStyleContinuous
                        : ORBPathRoundedCornerStyleCircular,
                };
                type = ORBPathShapeTypeUnevenRoundedRect;
                break;
            default:
                return false;
            }
            info->callback(info->info, type, &shape);
            info->matched = true;
            return false;
        });
        return shapeInfo.matched;
    }

    // RenderBox's 6.5.4 implementation takes this branch when the ABI
    // parameter named `allowLine` is false.
    if (!allowLine) {
        struct LineInfo {
            CGPoint points[2];
            uint64_t count;
        } lineInfo = {};

        bool applied = ORBPathApplyElements(path, &lineInfo, +[](
            void *rawInfo,
            ORBPathElement element,
            const CGFloat *values,
            const void *userInfo
        ) -> bool {
            auto info = static_cast<LineInfo *>(rawInfo);
            if (element == ORBPathElementMoveToPoint) {
                if (info->count > 1) {
                    return false;
                }
                info->count = 0;
            } else if (element != ORBPathElementAddLineToPoint) {
                return false;
            }
            if (info->count != static_cast<uint64_t>(element)) {
                return false;
            }
            CGPoint point = reinterpret_cast<const CGPoint *>(values)[0];
            if (userInfo != nullptr) {
                point = CGPointApplyAffineTransform(
                    point,
                    *static_cast<const CGAffineTransform *>(userInfo)
                );
            }
            info->points[info->count++] = point;
            return true;
        });
        if (applied && lineInfo.count == 2) {
            ORBPathShape shape = {};
            shape.line = { lineInfo.points[0], lineInfo.points[1] };
            callback(info, ORBPathShapeTypeLine, &shape);
            return true;
        }
    }

    return false;
}

bool ORBPathEqualToPath(ORBPath lhs, ORBPath rhs) {
    if (lhs.callbacks == rhs.callbacks) {
        if (lhs.storage == rhs.storage) { return true; }
        if (lhs.callbacks->flags.isExtended) {
            auto *extended = reinterpret_cast<const ORBPathCallbacksExtended *>(lhs.callbacks);
            if (extended->isEqual) { return extended->isEqual(lhs.storage, rhs.storage, extended); }
        } else if (lhs.callbacks->isEqual) {
            return lhs.callbacks->isEqual(lhs.storage, rhs.storage);
        }
    } else {
        if (lhs.callbacks == &empty_path_callbacks) {
            return lhs.storage == ORBPathNull.storage ? false : ORBPathIsEmpty(rhs);
        }
        if (rhs.callbacks == &empty_path_callbacks) {
            return rhs.storage == ORBPathNull.storage ? false : ORBPathIsEmpty(lhs);
        }
    }
    using Storage = ORB::Path::Storage;
    // Only the left stream is normalized by Storage, matching the callback ABI.
    Storage storage(sizeof(Storage));
    ORBPathApplyElements(lhs, &storage, ORB::Path::append_element_callback);
    struct Comparison {
        const Storage &storage;
        Storage::Iterator iterator;
    } comparison{storage, {}};
    bool equal = ORBPathApplyElements(rhs, &comparison, +[](void *context, ORBPathElement type, const CGFloat *points, const void *info) -> bool {
        auto &comparison = *static_cast<Comparison *>(context);
        Storage::Element element;
        if (!comparison.iterator.next(comparison.storage, element) || type != element.type) { return false; }
        for (size_t index = 0; index < Storage::argument_count(type); ++index) {
            if (element.points[index] != points[index]) { return false; }
        }
        if (!info && !element.hasInfo) { return true; }
        switch (type) {
        case ORBPathElementCloseSubpath:
        case ORBPathElementEnd:
        case ORBPathElementTrimmed:
        case ORBPathElementDilateGlyph:
        case ORBPathElementOffset:
            return true;
        case ORBPathElementStroked:
        case ORBPathElementClipStroke: {
            Storage::StrokeInfo defaults{0, type == ORBPathElementClipStroke ? 1 : 0, 0, nullptr};
            auto &a = element.hasInfo ? element.metadata.stroke : defaults;
            auto &b = info ? *static_cast<const Storage::StrokeInfo *>(info) : defaults;
            if (a.cap != b.cap || a.join != b.join || a.count != b.count) { return false; }
            if (type == ORBPathElementStroked) {
                auto *av = static_cast<const CGFloat *>(a.values), *bv = static_cast<const CGFloat *>(b.values);
                for (size_t index = 0; index < a.count; ++index) { if (av[index] != bv[index]) { return false; } }
            } else {
                auto *av = static_cast<const Storage::StrokeRecord *>(a.values), *bv = static_cast<const Storage::StrokeRecord *>(b.values);
                for (size_t index = 0; index < a.count; ++index) {
                    if (av[index].type != bv[index].type) { return false; }
                    float ap[3], bp[3];
                    std::memcpy(ap, av[index].values, sizeof(ap));
                    std::memcpy(bp, bv[index].values, sizeof(bp));
                    if (!std::equal(ap, ap + 3, bp)) { return false; }
                }
            }
            return true;
        }
        case ORBPathElementBoolean: {
            const uint8_t defaults[2] = {};
            auto *a = element.hasInfo ? element.metadata.boolean : defaults;
            auto *b = info ? static_cast<const uint8_t *>(info) : defaults;
            return a[0] == b[0] && a[1] == b[1];
        }
        default: {
            CGAffineTransform a = CGAffineTransformIdentity;
            if (element.hasInfo) { std::memcpy(&a, element.metadata.transform, sizeof(a)); }
            auto &b = info ? *static_cast<const CGAffineTransform *>(info) : CGAffineTransformIdentity;
            return CGAffineTransformEqualToTransform(a, b);
        }
        }
    });
    return equal && comparison.iterator.offset >= storage.actual_size();
}

#if ORB_TARGET_OS_DARWIN
CGPathRef ORBPathCopyCGPath(ORBPath path) {
    if (path.callbacks == &empty_path_callbacks) {
        return CGPathCreateMutable();
    }

    CGPathRef result = nullptr;
    if (path.callbacks->flags.isExtended) {
        auto extended = reinterpret_cast<const ORBPathCallbacksExtended *>(path.callbacks);
        if (extended->cgPath != nullptr) {
            result = extended->cgPath(path.storage, extended);
        }
    } else if (path.callbacks->cgPath != nullptr) {
        result = path.callbacks->cgPath(path.storage);
    }
    if (result != nullptr) {
        return CGPathRetain(result);
    }

    return ORB::Path::copy_cg_path(path);
}

CGRect ORBPathGetBoundingRect(ORBPath path) {
    if (path.callbacks->flags.isExtended) {
        auto extended = reinterpret_cast<const ORBPathCallbacksExtended *>(path.callbacks);
        if (extended->boundingRect != nullptr) {
            return extended->boundingRect(path.storage, extended);
        }
    } else if (path.callbacks->boundingRect != nullptr) {
        return path.callbacks->boundingRect(path.storage);
    }

    struct PrimitiveBounds { CGRect rect; bool matched; } bounds{CGRectNull, false};
    if (ORBPathIsSingleElement(path)) {
        ORBPathApplyShape(path, true, &bounds, +[](void *context, ORBPathShapeType type, const ORBPathShape *shape) {
            auto &bounds = *static_cast<PrimitiveBounds *>(context);
            switch (type) {
            case ORBPathShapeTypeRect:
            case ORBPathShapeTypeEllipse:
            case ORBPathShapeTypeFixedRoundedRect:
            case ORBPathShapeTypeUnevenRoundedRect:
                bounds.rect = CGRectApplyAffineTransform(shape->rect.rect, shape->rect.transform);
                bounds.matched = true;
                break;
            case ORBPathShapeTypeNested:
                bounds.rect = CGRectApplyAffineTransform(ORBPathGetBoundingRect(shape->nested.path), shape->nested.transform);
                bounds.matched = true;
                break;
            default: break;
            }
        });
        if (bounds.matched) { return bounds.rect; }
    }

    CGPathRef cgPath = ORBPathCopyCGPath(path);
    CGRect result = CGPathGetPathBoundingBox(cgPath);
    CGPathRelease(cgPath);
    return result;
}

bool ORBPathContainsPoint(ORBPath path, CGPoint point, bool eoFill) {
    return ORBPathContainsPoints(path, 1, &point, eoFill, nullptr);
}

namespace {
using FloatPoint = float __attribute__((ext_vector_type(2)));

struct PathWinding {
    FloatPoint queries[64];
    int winding[64] = {};
    size_t count;
    FloatPoint minimum, maximum;
    FloatPoint start = {}, current = {};

    static FloatPoint narrow(CGPoint p) { return {static_cast<float>(p.x), static_cast<float>(p.y)}; }

    void line(FloatPoint end) {
        auto from = current;
        current = end;
        auto delta = end - from;
        if (std::abs(delta.y) < 0.0001f) { return; }
        for (size_t i = 0; i < count; ++i) {
            float t = (queries[i].y - from.y) / delta.y;
            if (t >= 0 && t <= 1 && std::fma(delta.x, t, from.x) > queries[i].x) {
                winding[i] += delta.y > 0 ? 1 : -1;
            }
        }
    }

    void close() {
        if (current.x != start.x || current.y != start.y) { line(start); }
    }

    void cubic(CGPoint cp1, CGPoint cp2, CGPoint endpoint) {
        auto a = current, b = narrow(cp1), c = narrow(cp2), d = narrow(endpoint);
        float minX = std::min({a.x, b.x, c.x, d.x}), minY = std::min({a.y, b.y, c.y, d.y});
        float maxX = std::max({a.x, b.x, c.x, d.x}), maxY = std::max({a.y, b.y, c.y, d.y});
        if (minY > maximum.y || maxX < minimum.x || maxY < minimum.y) { current = d; return; }
        if (minX > maximum.x) { line(d); return; }
        auto e = c - (b + b) + a, f = b - (c + c) + d;
        float x = std::max(std::abs(e.x), std::abs(f.x));
        float y = std::max(std::abs(e.y), std::abs(f.y));
        float estimate = std::ceil(std::sqrt(std::sqrt(x * x + y * y)));
        if (!std::isfinite(estimate)) { current = d; return; }
        int segments = static_cast<int>(std::min(estimate, 1024.0f));
        if (segments >= 2) {
            auto first = (b - a) * 3.0f;
            FloatPoint second, third;
            for (size_t axis = 0; axis != 2; ++axis) {
                second[axis] = std::fma(3.0f, c[axis] - b[axis], -first[axis]);
                third[axis] = d[axis] - (std::fma(3.0f, b[axis] - a[axis], a[axis]) + second[axis]);
            }
            float dt = 1.0f / segments, dt2 = dt * dt, dt3 = dt2 * dt;
            FloatPoint delta1, delta2, delta3 = (third * 6.0f) * dt3;
            second *= dt2;
            for (size_t axis = 0; axis != 2; ++axis) {
                delta2[axis] = std::fma(2.0f, second[axis], delta3[axis]);
                delta1[axis] = std::fma(third[axis], dt3, std::fma(first[axis], dt, second[axis]));
            }
            for (int step = 1; step < segments; ++step) {
                a += delta1;
                delta1 += delta2;
                delta2 += delta3;
                line(a);
            }
        }
        line(d);
    }
};
} // namespace

uint64_t ORBPathContainsPoints(ORBPath path, uint64_t count, const CGPoint *points, bool eoFill, const CGAffineTransform *transform) {
    if (count == 0) { return 0; }
    PathWinding state{};
    state.count = std::min<uint64_t>(count, 64);
    state.minimum = {INFINITY, INFINITY};
    state.maximum = {-INFINITY, -INFINITY};
    for (size_t i = 0; i < state.count; ++i) {
        CGPoint point = transform ? CGPointApplyAffineTransform(points[i], *transform) : points[i];
        state.queries[i] = PathWinding::narrow(point);
        for (size_t axis = 0; axis != 2; ++axis) {
            state.minimum[axis] = std::fmin(state.minimum[axis], state.queries[i][axis]);
            state.maximum[axis] = std::fmax(state.maximum[axis], state.queries[i][axis]);
        }
    }
    auto bounds = ORBPathGetBoundingRect(path);
    bool pointQuery = state.minimum.x == state.maximum.x && state.minimum.y == state.maximum.y;
    CGRect queryBounds = CGRectMake(state.minimum.x, state.minimum.y, state.maximum.x - state.minimum.x, state.maximum.y - state.minimum.y);
    if (pointQuery ? !CGRectContainsPoint(bounds, CGPointMake(state.minimum.x, state.minimum.y)) : !CGRectIntersectsRect(bounds, queryBounds)) { return 0; }
    auto cgPath = ORBPathCopyCGPath(path);
    CGPathApply(cgPath, &state, +[](void *info, const CGPathElement *element) {
        auto &state = *static_cast<PathWinding *>(info);
        auto *p = element->points;
        switch (element->type) {
        case kCGPathElementMoveToPoint:
            state.close();
            state.current = state.start = PathWinding::narrow(p[0]);
            break;
        case kCGPathElementAddLineToPoint: state.line(PathWinding::narrow(p[0])); break;
        case kCGPathElementAddQuadCurveToPoint: {
            CGPoint c1 = {std::fma(2.0, p[0].x, state.current.x) / 3, std::fma(2.0, p[0].y, state.current.y) / 3};
            CGPoint c2 = {std::fma(2.0, p[0].x, p[1].x) / 3, std::fma(2.0, p[0].y, p[1].y) / 3};
            state.cubic(c1, c2, p[1]);
            break;
        }
        case kCGPathElementAddCurveToPoint: state.cubic(p[0], p[1], p[2]); break;
        case kCGPathElementCloseSubpath: state.close(); break;
        }
    });
    state.close();
    CGPathRelease(cgPath);
    uint64_t result = 0;
    for (size_t i = 0; i < state.count; ++i) {
        if (eoFill ? (state.winding[i] & 1) != 0 : state.winding[i] != 0) { result |= uint64_t(1) << i; }
    }
    return result;
}

#endif /* ORB_TARGET_OS_DARWIN */

#endif /* OPENRENDERBOX_CF_CGTYPES */
