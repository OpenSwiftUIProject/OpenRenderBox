//
//  ORBDevice.mm
//  OpenRenderBox

#include <OpenRenderBoxObjC/Device/ORBDevice.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>

@implementation ORBDevice

+ (id)sharedDefaultDevice {
    // TODO
    return nil;
}

+ (id)allDevices {
    // TODO
    return nil;
}

+ (BOOL)isSupported {
    id device = [self sharedDefaultDevice];
    return device != nil;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device {
    self = [super init];
    if (self) {
        // TODO
    }
    return self;
}

@end

#endif /* ORB_OBJC_FOUNDATION */
