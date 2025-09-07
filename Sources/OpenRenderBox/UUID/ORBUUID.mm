//
//  ORBUUID.mm
//  OpenRenderBox

#include <OpenRenderBox/ORBUUID.h>

#if ORB_TARGET_OS_DARWIN
ORBUUID ORBUUIDInitFromNSUUID(NSUUID *uuid) {
    ORBUUID ob_uuid;
    [uuid getUUIDBytes:ob_uuid.bytes];
    return ob_uuid;
}
#endif

ORBUUID ORBUUIDInitFromHash(uint64_t words0, uint64_t words1, uint32_t words2) {
    ORBUUID ob_uuid;
    ob_uuid.bytes[0] = words0 & 0xFF;
    ob_uuid.bytes[1] = ((words0 >> 8) & 0x0F) | ((words2 & 0xF) << 0x4);
    ob_uuid.bytes[2] = (words0 >> 16) & 0xFF;
    ob_uuid.bytes[3] = (words0 >> 24) & 0xFF;
    ob_uuid.bytes[4] = (words0 >> 32) & 0xFF;
    ob_uuid.bytes[5] = (words0 >> 40) & 0xFF;
    ob_uuid.bytes[6] = (words0 >> 48) & 0xFF;
    ob_uuid.bytes[7] = (words0 >> 56) & 0xFF;
    
    ob_uuid.bytes[8] = words1 & 0xFF;
    ob_uuid.bytes[9] = (words1 >> 8) & 0xFF;
    ob_uuid.bytes[10] = (words1 >> 16) & 0xFF;
    ob_uuid.bytes[11] = (words1 >> 24) & 0xFF;
    ob_uuid.bytes[12] = (words1 >> 32) & 0xFF;
    ob_uuid.bytes[13] = (words1 >> 40) & 0xFF;
    ob_uuid.bytes[14] = (words1 >> 48) & 0xFF;
    ob_uuid.bytes[15] = ((words1 >> 56) & 0x3F) | 0x80;
    return ob_uuid;
}
