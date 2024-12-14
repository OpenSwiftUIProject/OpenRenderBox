//
//  UUIDTests.swift
//  OpenBoxCompatibilityTests

import Testing
import Foundation

struct UUIDTests {
    @Test(arguments: [
        (
            UUID(uuidString: "00000000-0000-0000-0000-000000000000")!,
            [
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
            ]
        ),
        (
            UUID(uuidString: "E621E1F8-C36C-495A-93FC-0C247A3E6E5F")!,
            [
                0xE6, 0x21, 0xE1, 0xF8,
                0xC3, 0x6C, 0x49, 0x5A,
                0x93, 0xFC, 0x0C, 0x24,
                0x7A, 0x3E, 0x6E, 0x5F,
            ]
        ),
    ])
    func foundationUUID(uuid: UUID, expectedBytes: [UInt8]) {
        #if canImport(Darwin)
        let id = OBUUID(uuid: uuid)
        let bytes = id.bytes
        #expect(bytes.0 == expectedBytes[0])
        #expect(bytes.1 == expectedBytes[1])
        #expect(bytes.2 == expectedBytes[2])
        #expect(bytes.3 == expectedBytes[3])
        #expect(bytes.4 == expectedBytes[4])
        #expect(bytes.5 == expectedBytes[5])
        #expect(bytes.6 == expectedBytes[6])
        #expect(bytes.7 == expectedBytes[7])
        #expect(bytes.8 == expectedBytes[8])
        #expect(bytes.9 == expectedBytes[9])
        #expect(bytes.10 == expectedBytes[10])
        #expect(bytes.11 == expectedBytes[11])
        #expect(bytes.12 == expectedBytes[12])
        #expect(bytes.13 == expectedBytes[13])
        #expect(bytes.14 == expectedBytes[14])
        #expect(bytes.15 == expectedBytes[15])
        #else
        withKnownIssue {
            Issue.record("NSUUID supported on non-Darwin platforms")
        }
        #endif
    }
    
    @Test(arguments: [
        (
            (0,0,0,0,0),
            [
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x80,
            ]
        ),
        (
            (0xF8E1_21E6, 0x5A49_6CC3, 0xFC93_5A49, 0x5F6E_7A24, 0x0000_0000),
            [
                0xE6, 0x01, 0xE1, 0xF8,
                0xC3, 0x6C, 0x49, 0x5A,
                0x49, 0x5A, 0x93, 0xFC,
                0x24, 0x7A, 0x6E, 0x9F,
            ]
        ),
        (
            (0x0123_4567, 0x89AB_CDEF, 0x4567_89AB, 0xCDEF_0123, 0xFFFF_FFF7),
            [
                0x67, 0x75, 0x23, 0x01,
                0xEF, 0xCD, 0xAB, 0x89,
                0xAB, 0x89, 0x67, 0x45,
                0x23, 0x01, 0xEF, 0x8D,
            ]
        ),
    ])
    func strongHashUUID(words: (UInt32, UInt32, UInt32, UInt32, UInt32), expectedBytes: [UInt8]) {
        let id: OBUUID = OBUUID(hash: UInt64(words.0) | UInt64(words.1) &<< 32, UInt64(words.2) | UInt64(words.3) &<< 32, words.4)
        let bytes = id.bytes
        #expect(bytes.0 == expectedBytes[0])
        #expect(bytes.1 == expectedBytes[1])
        #expect(bytes.2 == expectedBytes[2])
        #expect(bytes.3 == expectedBytes[3])
        #expect(bytes.4 == expectedBytes[4])
        #expect(bytes.5 == expectedBytes[5])
        #expect(bytes.6 == expectedBytes[6])
        #expect(bytes.7 == expectedBytes[7])
        #expect(bytes.8 == expectedBytes[8])
        #expect(bytes.9 == expectedBytes[9])
        #expect(bytes.10 == expectedBytes[10])
        #expect(bytes.11 == expectedBytes[11])
        #expect(bytes.12 == expectedBytes[12])
        #expect(bytes.13 == expectedBytes[13])
        #expect(bytes.14 == expectedBytes[14])
        #expect(bytes.15 == expectedBytes[15])
    }
}
