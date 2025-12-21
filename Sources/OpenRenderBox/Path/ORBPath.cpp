//
//  ORBPath.cpp
//  OpenRenderBox
//
//  Created by Kyle on 2025/3/25.
//

#include <OpenRenderBox/ORBPath.h>
#include <OpenRenderBox/ORBPathCallbacks.h>

// Empty path (storage = null)
const ORBPath ORBPathEmpty = {
    nullptr,
    &ORBPathEmptyCallbacks,
};

// Null path (storage = 0x1)
const ORBPath ORBPathNull = {
    reinterpret_cast<ORBPathStorage *>(0x1),
    &ORBPathEmptyCallbacks,
};

void ORBPathRetain(ORBPath path) {
    auto retain = path.callbacks->retain;
    if (retain != nullptr) {
        retain(&path);
    }
}

void ORBPathRelease(ORBPath path) {
    auto release = path.callbacks->release;
    if (release != nullptr) {
        release(&path);
    }
}

#if ORB_TARGET_OS_DARWIN

// MARK: - Path Creation

// TODO: TO be implemented natively

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
    CGPathRef cgPath = CGPathCreateWithRect(rect, transform);
    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

ORBPath ORBPathMakeEllipse(CGRect rect, const CGAffineTransform *transform) {
    CGPathRef cgPath = CGPathCreateWithEllipseInRect(rect, transform);
    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

ORBPath ORBPathMakeRoundedRect(CGRect rect, CGFloat cornerWidth, CGFloat cornerHeight, ORBRoundedCornerStyle style, const CGAffineTransform *transform) {
    // TODO: Handle ORBRoundedCornerStyleContinuous with custom bezier curves
    CGPathRef cgPath = CGPathCreateWithRoundedRect(rect, cornerWidth, cornerHeight, transform);
    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(const_cast<CGPath *>(cgPath)),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

ORBPath ORBPathMakeUnevenRoundedRect(CGRect rect, CGFloat topLeftRadius, CGFloat bottomLeftRadius, CGFloat bottomRightRadius, CGFloat topRightRadius, ORBRoundedCornerStyle style, const CGAffineTransform *transform) {
    // TODO: Handle ORBRoundedCornerStyleContinuous with custom bezier curves
    CGMutablePathRef cgPath = CGPathCreateMutable();

    CGFloat minX = CGRectGetMinX(rect);
    CGFloat minY = CGRectGetMinY(rect);
    CGFloat maxX = CGRectGetMaxX(rect);
    CGFloat maxY = CGRectGetMaxY(rect);

    // Start at top-left corner (after the rounded corner)
    CGPathMoveToPoint(cgPath, transform, minX + topLeftRadius, minY);

    // Top edge and top-right corner
    CGPathAddLineToPoint(cgPath, transform, maxX - topRightRadius, minY);
    CGPathAddArc(cgPath, transform, maxX - topRightRadius, minY + topRightRadius, topRightRadius, -M_PI_2, 0, false);

    // Right edge and bottom-right corner
    CGPathAddLineToPoint(cgPath, transform, maxX, maxY - bottomRightRadius);
    CGPathAddArc(cgPath, transform, maxX - bottomRightRadius, maxY - bottomRightRadius, bottomRightRadius, 0, M_PI_2, false);

    // Bottom edge and bottom-left corner
    CGPathAddLineToPoint(cgPath, transform, minX + bottomLeftRadius, maxY);
    CGPathAddArc(cgPath, transform, minX + bottomLeftRadius, maxY - bottomLeftRadius, bottomLeftRadius, M_PI_2, M_PI, false);

    // Left edge and top-left corner
    CGPathAddLineToPoint(cgPath, transform, minX, minY + topLeftRadius);
    CGPathAddArc(cgPath, transform, minX + topLeftRadius, minY + topLeftRadius, topLeftRadius, M_PI, M_PI + M_PI_2, false);

    CGPathCloseSubpath(cgPath);

    ORBPath path = {
        reinterpret_cast<ORBPathStorage *>(cgPath),
        &ORBPathCGPathCallbacks,
    };
    return path;
}

CGPathRef ORBPathCopyCGPath(ORBPath path) {
    // TODO: Return a retained copy of the CGPath
    return nullptr;
}

bool ORBPathContainsPoint(ORBPath path, CGPoint point, bool eoFill) {
    return ORBPathContainsPoints(path, 1, &point, eoFill, nullptr);
}

bool ORBPathContainsPoints(ORBPath path, uint64_t count, const CGPoint *points, bool eoFill, const CGAffineTransform *transform) {
    // TODO: Implement point containment testing with winding rule
    return false;
}

#endif /* ORB_TARGET_OS_DARWIN */
