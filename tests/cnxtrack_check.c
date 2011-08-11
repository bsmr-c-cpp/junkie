// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
#include <stdlib.h>
#include <stdio.h>
#undef NDEBUG
#include <assert.h>
#include <string.h>
#include <junkie/tools/miscmacs.h>
#include <junkie/tools/mallocer.h>
#include <junkie/cpp.h>
#include <junkie/proto/pkt_wait_list.h>
#include <junkie/proto/cap.h>
#include <junkie/proto/eth.h>
#include <junkie/proto/ip.h>
#include <junkie/proto/tcp.h>
#include <junkie/proto/ftp.h>
#include <junkie/proto/http.h>

static struct test {
    uint8_t const *packet;
    size_t packet_len;
    char const *last_proto_name;
} tests[] = {
    {   // The first packet sets FTP passive mode
        .packet = (uint8_t const []){
            0x00, 0x12, 0x3f, 0x71, 0x57, 0xa1, 0x00, 0x10, 0xf3, 0x0a, 0x9a, 0xdf, 0x08, 0x00, 0x45, 0x00,
            0x00, 0x66, 0xdd, 0x90, 0x40, 0x00, 0x40, 0x06, 0x59, 0x00, 0xc3, 0x53, 0x76, 0x01, 0xc0, 0xa8,
            0x0a, 0x04, 0x00, 0x15, 0xac, 0x17, 0x5c, 0xd1, 0x3f, 0x3d, 0x32, 0x89, 0xbf, 0x55, 0x80, 0x18,
            0x43, 0xe0, 0xc3, 0x60, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x70, 0xeb, 0x09, 0xc4, 0x0f, 0xa7,
            0x0d, 0x5e, 0x32, 0x32, 0x37, 0x20, 0x45, 0x6e, 0x74, 0x65, 0x72, 0x69, 0x6e, 0x67, 0x20, 0x50,
            0x61, 0x73, 0x73, 0x69, 0x76, 0x65, 0x20, 0x4d, 0x6f, 0x64, 0x65, 0x20, 0x28, 0x31, 0x39, 0x35,
            0x2c, 0x38, 0x33, 0x2c, 0x31, 0x31, 0x38, 0x2c, 0x31, 0x2c, 0x32, 0x31, 0x35, 0x2c, 0x31, 0x32,
            0x34, 0x29, 0x0d, 0x0a,
        },
        .packet_len = 16*7 + 4,
        .last_proto_name = "FTP",
    }, {    // Magic happens here : Although on unknown ports, this CNX is recognized as the FTP continuation of previous packet
        .packet = (uint8_t const []){
            0x00, 0x12, 0x3f, 0x71, 0x57, 0xa1, 0x00, 0x10, 0xf3, 0x0a, 0x9a, 0xdf, 0x08, 0x00, 0x45, 0x00,
            0x02, 0x02, 0x29, 0x6c, 0x40, 0x00, 0x37, 0x06, 0x14, 0x89, 0xc3, 0x53, 0x76, 0x01, 0xc0, 0xa8,
            0x0a, 0x04, 0xd7, 0x7c, 0xa7, 0x23, 0xcd, 0x6a, 0x22, 0xb0, 0x33, 0xfc, 0x35, 0x94, 0x80, 0x18,
            0x80, 0x52, 0xda, 0x7f, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x74, 0x6f, 0x74, 0x61, 0x6c, 0x20, 0x31, 0x30, 0x33, 0x31, 0x39, 0x36, 0x0d, 0x0a,
            0x64, 0x72, 0x2d, 0x78, 0x72, 0x2d, 0x78, 0x72, 0x2d, 0x78, 0x20, 0x20, 0x20, 0x32, 0x20, 0x30,
            0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
            0x35, 0x31, 0x32, 0x20, 0x4e, 0x6f, 0x76, 0x20, 0x33, 0x30, 0x20, 0x31, 0x34, 0x3a, 0x35, 0x32,
            0x20, 0x65, 0x74, 0x63, 0x0d, 0x0a, 0x64, 0x72, 0x77, 0x78, 0x72, 0x2d, 0x78, 0x72, 0x2d, 0x78,
            0x20, 0x20, 0x20, 0x39, 0x20, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x35, 0x31, 0x32, 0x20, 0x46, 0x65, 0x62, 0x20, 0x31, 0x36,
            0x20, 0x20, 0x32, 0x30, 0x30, 0x31, 0x20, 0x69, 0x62, 0x70, 0x0d, 0x0a, 0x64, 0x72, 0x77, 0x78,
            0x72, 0x2d, 0x78, 0x72, 0x2d, 0x78, 0x20, 0x20, 0x31, 0x30, 0x20, 0x36, 0x30, 0x36, 0x20, 0x20,
            0x20, 0x33, 0x30, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x35, 0x31, 0x32, 0x20,
            0x4a, 0x75, 0x6e, 0x20, 0x32, 0x32, 0x20, 0x20, 0x32, 0x30, 0x30, 0x30, 0x20, 0x6a, 0x75, 0x73,
            0x73, 0x69, 0x65, 0x75, 0x0d, 0x0a, 0x64, 0x72, 0x77, 0x78, 0x72, 0x2d, 0x78, 0x72, 0x2d, 0x78,
            0x20, 0x20, 0x20, 0x32, 0x20, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x35, 0x31, 0x32, 0x20, 0x46, 0x65, 0x62, 0x20, 0x31, 0x36,
            0x20, 0x20, 0x32, 0x30, 0x30, 0x31, 0x20, 0x6c, 0x69, 0x61, 0x66, 0x61, 0x0d, 0x0a, 0x64, 0x72,
            0x77, 0x78, 0x72, 0x2d, 0x78, 0x72, 0x2d, 0x78, 0x20, 0x20, 0x20, 0x35, 0x20, 0x31, 0x30, 0x30,
            0x33, 0x20, 0x20, 0x36, 0x30, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x35, 0x31,
            0x32, 0x20, 0x4a, 0x75, 0x6c, 0x20, 0x32, 0x33, 0x20, 0x20, 0x32, 0x30, 0x30, 0x33, 0x20, 0x6c,
            0x69, 0x70, 0x36, 0x0d, 0x0a, 0x2d, 0x72, 0x77, 0x2d, 0x72, 0x2d, 0x2d, 0x72, 0x2d, 0x2d, 0x20,
            0x20, 0x20, 0x31, 0x20, 0x31, 0x30, 0x30, 0x33, 0x20, 0x20, 0x31, 0x30, 0x30, 0x33, 0x20, 0x20,
            0x35, 0x32, 0x38, 0x30, 0x32, 0x32, 0x31, 0x38, 0x20, 0x46, 0x65, 0x62, 0x20, 0x20, 0x39, 0x20,
            0x30, 0x39, 0x3a, 0x31, 0x33, 0x20, 0x6c, 0x73, 0x2d, 0x6c, 0x52, 0x2e, 0x67, 0x7a, 0x0d, 0x0a,
            0x64, 0x72, 0x77, 0x78, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x20, 0x20, 0x20, 0x33, 0x20, 0x31,
            0x30, 0x30, 0x33, 0x20, 0x20, 0x31, 0x30, 0x30, 0x33, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
            0x35, 0x31, 0x32, 0x20, 0x46, 0x65, 0x62, 0x20, 0x31, 0x36, 0x20, 0x20, 0x32, 0x30, 0x30, 0x31,
            0x20, 0x70, 0x72, 0x69, 0x76, 0x61, 0x74, 0x65, 0x0d, 0x0a, 0x64, 0x72, 0x77, 0x78, 0x72, 0x2d,
            0x78, 0x72, 0x2d, 0x78, 0x20, 0x20, 0x34, 0x33, 0x20, 0x31, 0x30, 0x30, 0x33, 0x20, 0x20, 0x31,
            0x30, 0x30, 0x33, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x31, 0x30, 0x32, 0x34, 0x20, 0x4a, 0x61,
            0x6e, 0x20, 0x32, 0x32, 0x20, 0x20, 0x32, 0x30, 0x30, 0x38, 0x20, 0x70, 0x75, 0x62, 0x0d, 0x0a,
        },
        .packet_len = 33 * 16,
        .last_proto_name = "FTP",
    }, {    // Once again, on another site
        .packet = (uint8_t const []) {
            0xa4, 0xba, 0xdb, 0xe6, 0x15, 0xfa, 0x00, 0x10, 0xf3, 0x0c, 0xf3, 0xac, 0x08, 0x00, 0x45, 0x00,
            0x00, 0x66, 0x61, 0xe7, 0x40, 0x00, 0x40, 0x06, 0xf9, 0x5c, 0x52, 0x43, 0xc2, 0x59, 0xc0, 0xa8,
            0x0a, 0x09, 0x00, 0x15, 0xd3, 0x8d, 0xb2, 0x8c, 0x11, 0xf2, 0x07, 0xf3, 0xd5, 0x2b, 0x80, 0x18,
            0x43, 0xe0, 0x47, 0x9b, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x80, 0x94, 0x1b, 0x43, 0x02, 0x64,
            0x5e, 0xb2, 0x32, 0x32, 0x37, 0x20, 0x45, 0x6e, 0x74, 0x65, 0x72, 0x69, 0x6e, 0x67, 0x20, 0x50,
            0x61, 0x73, 0x73, 0x69, 0x76, 0x65, 0x20, 0x4d, 0x6f, 0x64, 0x65, 0x20, 0x28, 0x38, 0x32, 0x2c,
            0x36, 0x37, 0x2c, 0x31, 0x39, 0x34, 0x2c, 0x38, 0x39, 0x2c, 0x32, 0x32, 0x37, 0x2c, 0x31, 0x39,
            0x32, 0x29, 0x0d, 0x0a,
        },
        .packet_len = 7*16 + 4,
        .last_proto_name = "FTP",
    }, {    // From client to server (RETR command)
        .packet = (uint8_t const []) {
            0x00, 0x10, 0xf3, 0x0c, 0xf3, 0xac, 0xa4, 0xba, 0xdb, 0xe6, 0x15, 0xfa, 0x08, 0x00, 0x45, 0x10,
            0x00, 0x43, 0x80, 0x63, 0x40, 0x00, 0x40, 0x06, 0xda, 0xf3, 0xc0, 0xa8, 0x0a, 0x09, 0x52, 0x43,
            0xc2, 0x59, 0xd3, 0x8d, 0x00, 0x15, 0x07, 0xf3, 0xd5, 0x2b, 0xb2, 0x8c, 0x12, 0x24, 0x80, 0x18,
            0x00, 0x2e, 0x97, 0xae, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x02, 0x64, 0x5e, 0xbe, 0x80, 0x94,
            0x1b, 0x43, 0x52, 0x45, 0x54, 0x52, 0x20, 0x6d, 0x61, 0x69, 0x6e, 0x2e, 0x74, 0x65, 0x78, 0x0d,
            0x0a,
        },
        .packet_len = 5*16 + 1,
        .last_proto_name = "FTP",
    }, {    // From server to client
        .packet = (uint8_t const []) {
            0xa4, 0xba, 0xdb, 0xe6, 0x15, 0xfa, 0x00, 0x10, 0xf3, 0x0c, 0xf3, 0xac, 0x08, 0x00, 0x45, 0x00,
            0x00, 0x7b, 0xe0, 0xec, 0x40, 0x00, 0x40, 0x06, 0x7a, 0x42, 0x52, 0x43, 0xc2, 0x59, 0xc0, 0xa8,
            0x0a, 0x09, 0x00, 0x15, 0xd3, 0x8d, 0xb2, 0x8c, 0x12, 0x24, 0x07, 0xf3, 0xd5, 0x3a, 0x80, 0x18,
            0x43, 0xe0, 0x17, 0x44, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x80, 0x94, 0x1b, 0x43, 0x02, 0x64,
            0x5e, 0xbe, 0x31, 0x35, 0x30, 0x20, 0x4f, 0x70, 0x65, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x42, 0x49,
            0x4e, 0x41, 0x52, 0x59, 0x20, 0x6d, 0x6f, 0x64, 0x65, 0x20, 0x64, 0x61, 0x74, 0x61, 0x20, 0x63,
            0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x27, 0x6d,
            0x61, 0x69, 0x6e, 0x2e, 0x74, 0x65, 0x78, 0x27, 0x20, 0x28, 0x31, 0x39, 0x34, 0x31, 0x35, 0x20,
            0x62, 0x79, 0x74, 0x65, 0x73, 0x29, 0x2e, 0x0d, 0x0a,
        },
        .packet_len = 8*16 + 9,
        .last_proto_name = "FTP",
    }, {    // Now an unrelated packet, same client/server but different ports
        .packet = (uint8_t const []) {
            0x00, 0x10, 0xf3, 0x0c, 0xf3, 0xac, 0xa4, 0xba, 0xdb, 0xe6, 0x15, 0xfa, 0x08, 0x00, 0x45, 0x00,
            0x01, 0xc4, 0x2e, 0xab, 0x40, 0x00, 0x40, 0x06, 0x2b, 0x3b, 0xc0, 0xa8, 0x0a, 0x09, 0x52, 0x43,
            0xc2, 0x59, 0xae, 0xf3, 0x00, 0x50, 0xa2, 0x79, 0xf8, 0xe6, 0xcd, 0x6e, 0xc9, 0x88, 0x80, 0x18,
            0x00, 0x2e, 0x2c, 0x82, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x02, 0x65, 0xd6, 0x55, 0x4b, 0x83,
            0x92, 0x05, 0x47, 0x45, 0x54, 0x20, 0x2f, 0x7e, 0x72, 0x69, 0x78, 0x65, 0x64, 0x2f, 0x6c, 0x69,
            0x6e, 0x6b, 0x73, 0x2e, 0x70, 0x68, 0x70, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31,
            0x0d, 0x0a, 0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, 0x68, 0x61, 0x70, 0x70, 0x79, 0x6c, 0x65, 0x70,
            0x74, 0x69, 0x63, 0x2e, 0x6f, 0x72, 0x67, 0x0d, 0x0a, 0x55, 0x73, 0x65, 0x72, 0x2d, 0x41, 0x67,
            0x65, 0x6e, 0x74, 0x3a, 0x20, 0x4d, 0x6f, 0x7a, 0x69, 0x6c, 0x6c, 0x61, 0x2f, 0x35, 0x2e, 0x30,
            0x20, 0x28, 0x58, 0x31, 0x31, 0x3b, 0x20, 0x55, 0x3b, 0x20, 0x4c, 0x69, 0x6e, 0x75, 0x78, 0x20,
            0x78, 0x38, 0x36, 0x5f, 0x36, 0x34, 0x3b, 0x20, 0x65, 0x6e, 0x2d, 0x55, 0x53, 0x3b, 0x20, 0x72,
            0x76, 0x3a, 0x31, 0x2e, 0x39, 0x2e, 0x31, 0x2e, 0x39, 0x29, 0x20, 0x47, 0x65, 0x63, 0x6b, 0x6f,
            0x2f, 0x32, 0x30, 0x31, 0x30, 0x30, 0x34, 0x30, 0x32, 0x20, 0x55, 0x62, 0x75, 0x6e, 0x74, 0x75,
            0x2f, 0x39, 0x2e, 0x31, 0x30, 0x20, 0x28, 0x6b, 0x61, 0x72, 0x6d, 0x69, 0x63, 0x29, 0x20, 0x46,
            0x69, 0x72, 0x65, 0x66, 0x6f, 0x78, 0x2f, 0x33, 0x2e, 0x35, 0x2e, 0x39, 0x0d, 0x0a, 0x41, 0x63,
            0x63, 0x65, 0x70, 0x74, 0x3a, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x2c,
            0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2f, 0x78, 0x68, 0x74, 0x6d,
            0x6c, 0x2b, 0x78, 0x6d, 0x6c, 0x2c, 0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f,
            0x6e, 0x2f, 0x78, 0x6d, 0x6c, 0x3b, 0x71, 0x3d, 0x30, 0x2e, 0x39, 0x2c, 0x2a, 0x2f, 0x2a, 0x3b,
            0x71, 0x3d, 0x30, 0x2e, 0x38, 0x0d, 0x0a, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x4c, 0x61,
            0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x3a, 0x20, 0x65, 0x6e, 0x2d, 0x75, 0x73, 0x2c, 0x65, 0x6e,
            0x3b, 0x71, 0x3d, 0x30, 0x2e, 0x35, 0x0d, 0x0a, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x45,
            0x6e, 0x63, 0x6f, 0x64, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x67, 0x7a, 0x69, 0x70, 0x2c, 0x64, 0x65,
            0x66, 0x6c, 0x61, 0x74, 0x65, 0x0d, 0x0a, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x43, 0x68,
            0x61, 0x72, 0x73, 0x65, 0x74, 0x3a, 0x20, 0x49, 0x53, 0x4f, 0x2d, 0x38, 0x38, 0x35, 0x39, 0x2d,
            0x31, 0x2c, 0x75, 0x74, 0x66, 0x2d, 0x38, 0x3b, 0x71, 0x3d, 0x30, 0x2e, 0x37, 0x2c, 0x2a, 0x3b,
            0x71, 0x3d, 0x30, 0x2e, 0x37, 0x0d, 0x0a, 0x4b, 0x65, 0x65, 0x70, 0x2d, 0x41, 0x6c, 0x69, 0x76,
            0x65, 0x3a, 0x20, 0x33, 0x30, 0x30, 0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69,
            0x6f, 0x6e, 0x3a, 0x20, 0x6b, 0x65, 0x65, 0x70, 0x2d, 0x61, 0x6c, 0x69, 0x76, 0x65, 0x0d, 0x0a,
            0x0d, 0x0a,
        },
        .packet_len = 29*16+2,
        .last_proto_name = "HTTP",
    }
};

static unsigned current_test = 0;

static int check_last_proto(struct proto_info const *last, size_t unused_ cap_len, uint8_t const unused_ *packet)
{
    assert(0 == strcmp(last->parser->proto->name, tests[current_test].last_proto_name));
    return 0;
}

static void cnxtrack_check(void)
{
    struct timeval now;
    timeval_set_now(&now);
    struct parser *eth_parser = proto_eth->ops->parser_new(proto_eth);

    for (current_test = 0; current_test < NB_ELEMS(tests); current_test ++) {
        struct test const *test = tests + current_test;
        printf("Testing packet %u...", current_test);
        int ret = proto_parse(eth_parser, NULL, 0, test->packet, test->packet_len, test->packet_len, &now, check_last_proto, test->packet_len, test->packet);
        assert(ret == 0);
        printf("Ok\n");
    }

    parser_unref(eth_parser);
}

int main(void)
{
    log_init();
    mallocer_init();
    proto_init();
    pkt_wait_list_init();
    ref_init();
    cap_init();
    eth_init();
    ip_init();
    ip6_init();
    tcp_init();
    ftp_init();
    http_init();
    log_set_level(LOG_DEBUG, NULL);
    log_set_file("cnxtrack_check.log");

    cnxtrack_check();

    doomer_stop();
    http_fini();
    ftp_fini();
    tcp_fini();
    ip6_fini();
    ip_fini();
    eth_fini();
    cap_fini();
    ref_fini();
    pkt_wait_list_fini();
    proto_fini();
    mallocer_fini();
    log_fini();
    return EXIT_SUCCESS;
}

