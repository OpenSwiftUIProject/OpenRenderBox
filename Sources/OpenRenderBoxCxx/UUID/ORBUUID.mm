//
//  ORBUUID.mm
//  OpenRenderBox

#include <OpenRenderBox/ORBUUID.h>

#if ORB_TARGET_OS_DARWIN && __OBJC__
ORBUUID ORBUUIDInitFromNSUUID(NSUUID *uuid) {
    ORBUUID ob_uuid;
    [uuid getUUIDBytes:ob_uuid.bytes];
    return ob_uuid;
}
#endif
