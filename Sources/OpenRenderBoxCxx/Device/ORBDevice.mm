//
//  ORBDevice.mm
//  OpenRenderBox

#include <OpenRenderBoxObjC/Device/ORBDevice.h>

#if ORB_OBJC_FOUNDATION

#include <Foundation/Foundation.h>

// TODO
@implementation ORBDevice

+ (id)sharedDefaultDevice {

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

}

@end

#endif /* ORB_OBJC_FOUNDATION */
