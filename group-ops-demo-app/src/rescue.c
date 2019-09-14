/*
#include "os.h"
#include "cx.h"
#include "rescue.h"
#include "crypto.h"

// alpha = smallest prime st gcd(p, alpha) = 1
// m = number of field elements in the state
// N = number of rounds
// For m = rq + cq, rescue absorbs (via field addition) and squeezes rq field
// elements per iteration, and offers log2(cq) bits of security.

// here alpha = 11, m = 3, N = 11, r = 1, s = 2 ?

#define RESCUE_ROUNDS 11
#define RESCUE_SPONGE_SIZE 3

const fmnt6753 alpha = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const fmnt6753 alpha_inv = {
    0x00, 0x00, 0x00, 0x00, 0x01, 0x9b, 0x9c, 0xe3, 0x9c, 0xb2, 0x3e,
    0x0e, 0xab, 0x0e, 0xab, 0x64, 0xce, 0x2f, 0x58, 0x1b, 0x9d, 0x15,
    0x31, 0xc7, 0x99, 0xe5, 0xc0, 0x84, 0xa7, 0x30, 0xe2, 0xbf, 0xcf,
    0xdc, 0x0b, 0x7b, 0xa0, 0xd6, 0xdc, 0xa4, 0xef, 0x63, 0x98, 0x00,
    0x32, 0x7e, 0x97, 0x4c, 0xde, 0x55, 0x78, 0x16, 0x91, 0x0d, 0xf6,
    0x87, 0x97, 0x54, 0x5e, 0x96, 0x4a, 0xb8, 0x29, 0x99, 0xc1, 0x0b,
    0x77, 0x91, 0xb4, 0x57, 0x25, 0xa4, 0x2e, 0x35, 0x50, 0x09, 0x0e,
    0xd3, 0x08, 0x39, 0x46, 0x21, 0x15, 0x30, 0x34, 0xe8, 0x43, 0x82,
    0x92, 0x92, 0x1a, 0x0b, 0x1e, 0x0e, 0xfc, 0x3a, 0x2e, 0x8b, 0xa3};

const fmnt6753 rescue_keys[RESCUE_ROUNDS * 2 + 1][RESCUE_SPONGE_SIZE] = {
    {
        {0xac, 0xb6, 0x46, 0x32, 0x16, 0x9b, 0x32, 0xea, 0xac, 0xa0, 0x21,
         0xd8, 0x5a, 0x37, 0xe6, 0x2b, 0x12, 0x74, 0x4b, 0xd1, 0x2a, 0x71,
         0xc3, 0x79, 0xa9, 0x40, 0x45, 0x3a, 0xf4, 0x3f, 0xb6, 0x2d},
        {0x5c, 0xac, 0xb0, 0xc7, 0x34, 0x9e, 0x78, 0x28, 0x1b, 0xb7, 0x87,
         0x05, 0x51, 0xe5, 0x52, 0xe6, 0x99, 0xa3, 0xb7, 0x37, 0x3f, 0x85,
         0xac, 0xd3, 0xc7, 0x90, 0x44, 0x9c, 0x2a, 0x8a, 0x67, 0xa7},
        {0xa4, 0xeb, 0x12, 0x2d, 0xcd, 0xaf, 0x65, 0x17, 0xb7, 0x07, 0xed,
         0x42, 0xd6, 0xcb, 0x7a, 0x07, 0x7d, 0xa7, 0x01, 0x68, 0xa7, 0x5c,
         0x78, 0xa9, 0x41, 0xc1, 0x1d, 0xc9, 0x80, 0x6f, 0xa1, 0x69},
    },
    {
        {0xd4, 0xb8, 0x30, 0xa0, 0xcb, 0x1a, 0x4d, 0x53, 0xa3, 0x1b, 0x9b,
         0x11, 0xd0, 0xda, 0x41, 0x52, 0x65, 0x71, 0x2e, 0x3d, 0xd0, 0x2e,
         0xba, 0x24, 0x67, 0xe7, 0x39, 0x5d, 0xee, 0x4d, 0x4c, 0x54},
        {0x8d, 0xa6, 0xac, 0xe6, 0xa2, 0x9e, 0x6c, 0xc4, 0x49, 0x3a, 0x58,
         0x47, 0x36, 0x2d, 0x00, 0x5b, 0x63, 0xf6, 0x1d, 0xe8, 0x1e, 0xc2,
         0xd2, 0x86, 0x84, 0x40, 0x6c, 0x7c, 0xdd, 0x65, 0xbb, 0xf7},
        {0xf7, 0xe3, 0x7f, 0x99, 0x13, 0x4e, 0xbc, 0x0e, 0x76, 0x54, 0x86,
         0x9b, 0xed, 0x1e, 0x44, 0x8c, 0xb2, 0x91, 0xf9, 0xcd, 0x55, 0x96,
         0x73, 0x43, 0xbe, 0x04, 0xe7, 0xa9, 0x4d, 0x59, 0xd2, 0xe5},
    },
    {
        {0xba, 0xfa, 0x31, 0x9e, 0xe4, 0xcf, 0x95, 0xab, 0x44, 0x48, 0x5a,
         0x9e, 0xb0, 0x12, 0xc3, 0x7c, 0x04, 0xc3, 0xa5, 0x2d, 0x13, 0x4a,
         0x79, 0x2d, 0x08, 0x74, 0xcd, 0x92, 0x21, 0x1c, 0x4e, 0x06},
        {0x63, 0xee, 0xe9, 0xdb, 0x53, 0xe6, 0x4c, 0xcb, 0xf4, 0x17, 0x8e,
         0x92, 0xce, 0x48, 0x09, 0xdd, 0xf5, 0x8f, 0x33, 0x8c, 0xf3, 0x0b,
         0x4e, 0xb9, 0xb0, 0xea, 0xc9, 0x7b, 0x6e, 0xa7, 0x02, 0xc5},
        {0x33, 0xc6, 0xda, 0xfd, 0x93, 0x09, 0x07, 0x51, 0xa7, 0x50, 0x84,
         0x7a, 0x68, 0x53, 0xca, 0x9d, 0x50, 0xaa, 0x55, 0xf3, 0xaf, 0x8c,
         0x55, 0x8d, 0xc5, 0x65, 0xab, 0xac, 0x48, 0x4b, 0x6b, 0x08},
    },
    {
        {0x8b, 0xc5, 0xa9, 0x6f, 0x35, 0x38, 0x8c, 0x69, 0x62, 0x64, 0x23,
         0x74, 0x15, 0x24, 0x8c, 0x1c, 0xdc, 0x12, 0x1c, 0x43, 0x62, 0xfe,
         0xed, 0x9c, 0x44, 0x9b, 0x38, 0x57, 0xfe, 0x5d, 0x7d, 0xfd},
        {0x96, 0x1e, 0x95, 0xcc, 0x0c, 0xea, 0x64, 0x42, 0x7b, 0x33, 0x4b,
         0x69, 0xc3, 0x04, 0xdf, 0xc4, 0xe2, 0xac, 0xa6, 0xff, 0xa2, 0x28,
         0xcb, 0x33, 0x06, 0x56, 0x5b, 0x41, 0x0b, 0x5a, 0x69, 0x63},
        {0x7c, 0xa7, 0x27, 0xf1, 0xd2, 0x23, 0xf7, 0xab, 0xcd, 0xe6, 0x98,
         0x98, 0x25, 0x8b, 0x03, 0x03, 0xd4, 0x89, 0x99, 0x2f, 0xcd, 0xbd,
         0xed, 0x70, 0x08, 0xdf, 0xba, 0x26, 0x42, 0xd8, 0x99, 0x53},
    },
    {
        {0xb1, 0x92, 0x4b, 0x2a, 0xd4, 0x3a, 0xc7, 0xee, 0xa5, 0xe9, 0x43,
         0x00, 0xde, 0x7e, 0x21, 0x9e, 0x7d, 0x5d, 0xbb, 0xc4, 0xc4, 0xec,
         0xe3, 0x6a, 0xa3, 0xe4, 0x1d, 0xf7, 0x21, 0x06, 0x76, 0xfd},
        {0xf5, 0x5a, 0xdc, 0x1b, 0x3e, 0x82, 0xfe, 0x72, 0x6c, 0x16, 0xe8,
         0x11, 0xdd, 0x5d, 0x7e, 0x52, 0xcc, 0xa3, 0x58, 0x3f, 0xef, 0x56,
         0x3e, 0x58, 0x6f, 0x11, 0x0c, 0x07, 0x69, 0xaa, 0xef, 0xcf},
        {0x37, 0xb4, 0x4e, 0x88, 0xbd, 0xde, 0xab, 0x5d, 0x93, 0x0f, 0x5d,
         0xea, 0x63, 0x22, 0x10, 0x85, 0x1e, 0xbf, 0x6a, 0x50, 0x2d, 0x46,
         0xfc, 0x75, 0x94, 0x93, 0x33, 0x48, 0xbd, 0x19, 0xcd, 0x70},
    },
    {
        {0xf0, 0x3d, 0xe4, 0x86, 0x02, 0x0f, 0x0f, 0x7d, 0x47, 0xf9, 0x3f,
         0xed, 0x5a, 0x6a, 0x72, 0xc0, 0xad, 0x26, 0xea, 0xc8, 0x2a, 0x2f,
         0x78, 0x9f, 0xca, 0x49, 0xb1, 0x7a, 0x57, 0x14, 0xb3, 0xb8},
        {0x27, 0x05, 0x25, 0x88, 0x91, 0x7d, 0x2a, 0x0b, 0x2b, 0x38, 0xa7,
         0x76, 0xd7, 0xa4, 0x40, 0xee, 0x78, 0xa7, 0xf9, 0x0a, 0x61, 0xab,
         0xe3, 0xc0, 0x3d, 0x8d, 0xa2, 0x88, 0xf7, 0x28, 0x8d, 0x96},
        {0xd2, 0x8d, 0xe9, 0x78, 0xd2, 0xec, 0x79, 0x3c, 0x2d, 0xb1, 0xbe,
         0xf3, 0xd8, 0x11, 0x83, 0x0a, 0xab, 0x4b, 0xf9, 0x87, 0xa6, 0x8e,
         0xb7, 0x4b, 0x1f, 0xa7, 0x98, 0x4d, 0x16, 0x96, 0xf5, 0x34},
    },
    {
        {0x5f, 0x66, 0x4d, 0x54, 0x4e, 0xd1, 0xd3, 0x27, 0xef, 0x1c, 0x1e,
         0xdb, 0xf7, 0xb2, 0x9c, 0x1d, 0xa6, 0x55, 0x0c, 0x4e, 0x03, 0xfd,
         0xbc, 0x13, 0x6d, 0x56, 0x9e, 0xc4, 0xe3, 0xde, 0xf3, 0x6c},
        {0xab, 0x05, 0xe4, 0xe5, 0xb7, 0xb1, 0x0e, 0xd9, 0xa1, 0xae, 0x2b,
         0xf7, 0x9f, 0x30, 0xb8, 0xcd, 0xdc, 0x5d, 0xe6, 0xd8, 0x46, 0x86,
         0x0c, 0x7a, 0xbe, 0x2f, 0x47, 0xe5, 0x7d, 0x9f, 0x0e, 0xb6},
        {0x13, 0x36, 0xc7, 0x2a, 0x14, 0xa1, 0x5e, 0x65, 0x28, 0xc3, 0x2f,
         0x36, 0x2a, 0xd5, 0xbe, 0x64, 0xb7, 0xc0, 0x18, 0xd7, 0x31, 0x77,
         0xbb, 0xe4, 0xef, 0x73, 0x98, 0xf3, 0x9c, 0x46, 0xed, 0x40},
    },
    {
        {0xf1, 0x5e, 0xfd, 0xd3, 0xda, 0x58, 0x1d, 0x6a, 0x9e, 0x93, 0x6c,
         0x8d, 0xc1, 0xc5, 0x98, 0x91, 0x8c, 0xb5, 0x07, 0x5a, 0x16, 0xc7,
         0x3f, 0xed, 0xa9, 0x5c, 0x1d, 0x20, 0xcd, 0x82, 0xe4, 0x9f},
        {0x8f, 0x5c, 0x8c, 0x02, 0x8e, 0x04, 0xa4, 0xe5, 0xe6, 0xf1, 0xd1,
         0xa0, 0x03, 0x17, 0x65, 0x74, 0xef, 0x61, 0x04, 0xe5, 0xd3, 0x54,
         0x1d, 0x7c, 0x95, 0xf7, 0xf4, 0x2c, 0xdc, 0x10, 0x6d, 0x13},
        {0x5f, 0x97, 0x50, 0xab, 0x5b, 0x96, 0x18, 0x1e, 0xe7, 0xb6, 0xd1,
         0x16, 0xe9, 0xf3, 0x85, 0x95, 0x85, 0x62, 0x37, 0x0d, 0xbb, 0xb5,
         0x27, 0x98, 0xdf, 0xdc, 0x15, 0xc9, 0x98, 0x21, 0x51, 0xb7},
    },
    {
        {0x05, 0xfa, 0xa8, 0xcb, 0xc1, 0x51, 0xd9, 0xc8, 0x4e, 0x81, 0xd9,
         0x75, 0x30, 0x2e, 0x7e, 0x64, 0xf6, 0x7a, 0x0e, 0xef, 0xaf, 0x17,
         0xb7, 0x13, 0x1f, 0x43, 0x97, 0xcd, 0x91, 0xc2, 0xde, 0xe8},
        {0x93, 0xb3, 0x4c, 0x07, 0xd6, 0xb4, 0x70, 0xd9, 0x25, 0x5d, 0x67,
         0x73, 0x85, 0x6c, 0x1e, 0xe8, 0x10, 0x67, 0x3c, 0xba, 0x0e, 0x16,
         0x28, 0x52, 0x45, 0xca, 0x71, 0xc0, 0xec, 0xcc, 0xb0, 0x15},
        {0x38, 0x77, 0xcc, 0xc5, 0xe2, 0xd5, 0x35, 0x0c, 0xd0, 0x1b, 0xc1,
         0x70, 0xa6, 0x76, 0xd9, 0x73, 0x16, 0xf1, 0x26, 0x05, 0xf3, 0xff,
         0x74, 0xda, 0x72, 0x47, 0x2f, 0xf8, 0xaf, 0x5f, 0x80, 0xf0},
    },
    {
        {0xc5, 0x6e, 0x5b, 0x18, 0xc5, 0xf2, 0xa9, 0x93, 0x78, 0x0f, 0xf4,
         0xc7, 0x65, 0x52, 0xac, 0xf1, 0xb4, 0x45, 0x8d, 0x12, 0x12, 0xd3,
         0xa2, 0xf9, 0xd1, 0x71, 0x79, 0x92, 0x80, 0x74, 0x07, 0x2a},
        {0x51, 0x69, 0xb0, 0x60, 0x58, 0x62, 0xa2, 0xd3, 0x41, 0x3f, 0x90,
         0xf4, 0x7f, 0xe3, 0x38, 0xed, 0xb5, 0xb6, 0x58, 0x72, 0xa1, 0x0b,
         0xd4, 0x1b, 0x42, 0xb0, 0xb0, 0x6c, 0x3d, 0xb9, 0xb1, 0x7c},
        {0x98, 0x48, 0xf3, 0xe7, 0x9d, 0x6a, 0xea, 0xb4, 0xc5, 0x54, 0xb1,
         0x12, 0xd1, 0x7d, 0xc2, 0x90, 0xb5, 0x20, 0x0f, 0x60, 0x12, 0x1f,
         0x23, 0xdc, 0x81, 0x7b, 0x54, 0x45, 0x75, 0x4b, 0xdc, 0x75},
    },
    {
        {0x97, 0x91, 0xd2, 0xe6, 0xb0, 0xb6, 0x26, 0x72, 0x6c, 0xfd, 0x07,
         0x96, 0x09, 0xbf, 0x57, 0x64, 0x29, 0x89, 0x6e, 0x76, 0xfc, 0xc0,
         0xfe, 0x87, 0x68, 0x58, 0x3e, 0xfd, 0x89, 0x70, 0x25, 0xfa},
        {0xf6, 0x3e, 0xb4, 0xd2, 0x2d, 0x57, 0xf7, 0xfd, 0x77, 0x2d, 0x1b,
         0x94, 0x00, 0x10, 0x04, 0x84, 0x23, 0x63, 0x5b, 0x98, 0x29, 0xde,
         0x36, 0x85, 0x7b, 0x30, 0x28, 0x57, 0x01, 0x7d, 0xc8, 0x11},
        {0xe0, 0x55, 0x86, 0x31, 0x65, 0x8a, 0xdc, 0xc4, 0xb7, 0x1c, 0xb4,
         0x5b, 0xde, 0x38, 0xf4, 0xe8, 0xd0, 0x2d, 0x42, 0xc9, 0x6e, 0x54,
         0x66, 0x89, 0xe1, 0x38, 0x15, 0x67, 0x92, 0xe7, 0x2e, 0xe1},
    },
    {
        {0xc2, 0x28, 0x3d, 0xd5, 0x3c, 0xa1, 0xf5, 0xe3, 0xc3, 0xe1, 0xd7,
         0x2a, 0x1f, 0x10, 0x0a, 0x70, 0x2a, 0xf3, 0x27, 0x44, 0xfe, 0x6d,
         0x1a, 0x53, 0x48, 0xea, 0x9d, 0x17, 0xa8, 0xa8, 0x1d, 0x12},
        {0x18, 0x71, 0x20, 0x93, 0xa9, 0xb1, 0xf6, 0x4d, 0x5b, 0x64, 0xdc,
         0x0a, 0xe7, 0x20, 0x11, 0x60, 0x27, 0xa4, 0xdb, 0x31, 0xaf, 0xe7,
         0x20, 0xe9, 0xd0, 0x69, 0xfb, 0x44, 0xbc, 0xea, 0xd0, 0xcf},
        {0xf7, 0x7f, 0x18, 0x79, 0x4b, 0x5b, 0x4b, 0xef, 0xf2, 0x00, 0x9d,
         0x9d, 0x03, 0x7d, 0x56, 0x48, 0x82, 0x26, 0xe4, 0x7e, 0x4a, 0xf4,
         0x2d, 0xff, 0xd3, 0xc9, 0x45, 0xf7, 0x73, 0x8e, 0x90, 0x31},
    },
    {
        {0xd3, 0x6c, 0xac, 0x1b, 0x7e, 0xe6, 0x10, 0x41, 0x30, 0xba, 0xb1,
         0xc3, 0xa9, 0xa1, 0x28, 0x00, 0xb6, 0x34, 0x62, 0xd9, 0x56, 0xea,
         0xa7, 0x56, 0x5c, 0xc2, 0x17, 0x83, 0x8b, 0x69, 0x10, 0x5a},
        {0xd1, 0xaa, 0x22, 0x0a, 0x13, 0xf2, 0x96, 0x21, 0x2f, 0x1e, 0xdc,
         0xaa, 0xf9, 0xb1, 0x8a, 0xe3, 0xea, 0xa9, 0x17, 0x8f, 0xcc, 0x42,
         0x0f, 0x18, 0x71, 0xc9, 0xc8, 0xa4, 0xe6, 0x58, 0x35, 0x2c},
        {0xab, 0x84, 0x13, 0x4f, 0x82, 0x3d, 0x95, 0x4c, 0xaa, 0x59, 0x5e,
         0x39, 0x0c, 0x6a, 0xe0, 0xf7, 0x04, 0xd3, 0x1d, 0x6e, 0xf0, 0x11,
         0xbc, 0xa1, 0x79, 0x1b, 0x03, 0x82, 0x13, 0x9f, 0xac, 0x13},
    },
    {
        {0x3d, 0x76, 0x06, 0x85, 0x96, 0x7f, 0x3c, 0x1e, 0xf6, 0x21, 0x02,
         0x6c, 0xaa, 0xc2, 0xe0, 0xe9, 0xf1, 0xc6, 0x75, 0x94, 0xc8, 0xf3,
         0xca, 0xd4, 0x1e, 0x28, 0x28, 0xec, 0x72, 0x49, 0x29, 0x36},
        {0x5e, 0x38, 0x15, 0xdf, 0xb5, 0x13, 0xbd, 0x46, 0x94, 0x4a, 0x3b,
         0xff, 0x8b, 0x0e, 0xc1, 0xec, 0x6f, 0xed, 0x5a, 0xad, 0x56, 0x82,
         0x1e, 0x27, 0xc2, 0x6e, 0x44, 0xd8, 0x60, 0x39, 0xd0, 0x3e},
        {0xd3, 0xfc, 0x79, 0x6a, 0xe9, 0xd3, 0xbf, 0xae, 0x59, 0xc5, 0xe9,
         0x55, 0xe5, 0x8a, 0x01, 0x94, 0x14, 0xcc, 0x70, 0x41, 0xd3, 0x09,
         0xd1, 0x7f, 0xc6, 0x4f, 0x21, 0xd1, 0x71, 0x0d, 0x71, 0x12},
    },
    {
        {0xed, 0xbc, 0xac, 0x34, 0xfa, 0x1b, 0xe8, 0x4d, 0xcd, 0xa3, 0x41,
         0xa8, 0x5b, 0x78, 0xc7, 0x9c, 0xf3, 0xd5, 0xef, 0x77, 0x05, 0x1b,
         0x63, 0x1c, 0xec, 0x92, 0xc3, 0x89, 0xb7, 0xc9, 0x28, 0xba},
        {0x26, 0x48, 0x99, 0x27, 0x97, 0xef, 0xb6, 0xe6, 0xd6, 0x00, 0x73,
         0x99, 0x9a, 0x1a, 0x19, 0x6a, 0xd0, 0x94, 0x12, 0x28, 0x0e, 0xc2,
         0x21, 0x8d, 0x5a, 0x0c, 0x1b, 0xaf, 0x27, 0x9f, 0x6c, 0xda},
        {0x72, 0x22, 0x88, 0x93, 0x30, 0xd3, 0xd8, 0x41, 0xa1, 0x8e, 0x7a,
         0x42, 0x8a, 0xd7, 0x0d, 0x50, 0x19, 0x6c, 0x88, 0xe3, 0xdc, 0x8f,
         0x9d, 0xa1, 0xa0, 0x25, 0x08, 0x5d, 0x46, 0xe7, 0x72, 0xe4},
    },
    {
        {0x27, 0x20, 0xc3, 0x3e, 0xa9, 0x5a, 0x5d, 0xe0, 0xa4, 0xb5, 0xea,
         0x6a, 0x75, 0x76, 0x24, 0xec, 0x35, 0x6e, 0x5e, 0x4e, 0x1a, 0x7b,
         0xdd, 0x45, 0x6b, 0x1c, 0xbb, 0xd6, 0x8c, 0xbd, 0xb5, 0xe2},
        {0xf7, 0x73, 0xba, 0x73, 0xdf, 0x51, 0x3f, 0xd2, 0xbb, 0xc2, 0x7d,
         0x4b, 0x8f, 0xf5, 0x6d, 0xaf, 0xf1, 0xe2, 0xf7, 0x81, 0xc3, 0xcb,
         0xed, 0x04, 0xb1, 0x5b, 0x93, 0xff, 0xb9, 0xa1, 0xd0, 0xbb},
        {0x7e, 0x58, 0xef, 0x87, 0x2b, 0x62, 0x28, 0x9d, 0xa9, 0xfe, 0xc1,
         0x5e, 0x6e, 0x68, 0x6e, 0x6b, 0x6e, 0xe0, 0x7c, 0xdf, 0x6d, 0x2b,
         0xb5, 0x6c, 0xab, 0x54, 0xbd, 0x59, 0xbf, 0xfe, 0x24, 0xcf},
    },
    {
        {0xcb, 0x67, 0xd6, 0xee, 0x83, 0xb4, 0x42, 0xb4, 0x2c, 0x34, 0x78,
         0x60, 0x32, 0x2d, 0xe8, 0xcc, 0x00, 0xe8, 0x73, 0x3b, 0xb4, 0xea,
         0xa9, 0x4f, 0xc0, 0x94, 0x57, 0x86, 0x66, 0xfe, 0xa2, 0xcb},
        {0x82, 0x04, 0x1e, 0xec, 0x89, 0x1b, 0xe8, 0xe9, 0x2c, 0xae, 0x8e,
         0x7d, 0xf4, 0x8a, 0xbc, 0xe3, 0x5d, 0xb4, 0x45, 0xb8, 0x89, 0xeb,
         0xba, 0xfa, 0x9a, 0xfa, 0xd0, 0xf3, 0x50, 0x44, 0x2a, 0x6d},
        {0xce, 0xe5, 0x76, 0xa3, 0x63, 0x72, 0x5e, 0x5e, 0x2a, 0x49, 0x69,
         0xe0, 0x8d, 0xde, 0x67, 0xdd, 0x8b, 0xbd, 0xdd, 0x52, 0xca, 0x81,
         0x27, 0xba, 0x55, 0x56, 0xd8, 0x5e, 0x09, 0x5f, 0x2a, 0xea},
    },
    {
        {0x67, 0x52, 0xa1, 0x54, 0xba, 0x67, 0xe9, 0x23, 0x5b, 0xd4, 0x1b,
         0x07, 0x07, 0x37, 0x9f, 0x4b, 0xff, 0x35, 0x67, 0x6f, 0x15, 0xaf,
         0xf2, 0x67, 0xbe, 0xa9, 0x01, 0xba, 0x18, 0x59, 0x7f, 0x09},
        {0x1b, 0xc7, 0xce, 0xc4, 0x64, 0x6c, 0xfb, 0x0e, 0x9e, 0xea, 0x8c,
         0xa6, 0x47, 0x6f, 0x2d, 0x8c, 0xb2, 0x20, 0xb4, 0x17, 0x6c, 0x9a,
         0x2c, 0x04, 0xa2, 0x65, 0xb3, 0xab, 0xd7, 0x8f, 0xbc, 0x75},
        {0x78, 0x04, 0x1d, 0x95, 0x2a, 0x10, 0x47, 0xcc, 0x9d, 0xd3, 0x6d,
         0x0c, 0x67, 0x54, 0x23, 0xce, 0x36, 0xa8, 0x00, 0xd2, 0xb1, 0x53,
         0x8b, 0x80, 0x0a, 0x75, 0xca, 0x8f, 0xda, 0x74, 0xfe, 0xa1},
    },
    {
        {0xcb, 0x80, 0x6d, 0xe8, 0xcb, 0xab, 0x16, 0x48, 0x49, 0x1d, 0x86,
         0x3f, 0xee, 0x79, 0x9d, 0xc6, 0x89, 0x55, 0x7a, 0x55, 0x18, 0xde,
         0xd8, 0xed, 0x7c, 0x8c, 0xf7, 0x37, 0xab, 0x8e, 0x01, 0x90},
        {0x21, 0x5f, 0xb2, 0xc5, 0x7d, 0x96, 0xa6, 0x89, 0x09, 0xa4, 0x3b,
         0x43, 0x41, 0x40, 0xb3, 0xb9, 0x20, 0x71, 0xd6, 0xd2, 0x41, 0x82,
         0x55, 0xfa, 0xf9, 0x04, 0x7d, 0xe9, 0x2a, 0xce, 0x0a, 0xed},
        {0xa9, 0x18, 0x54, 0x75, 0xd8, 0xa9, 0x52, 0x20, 0xc4, 0x29, 0x67,
         0xfc, 0x67, 0x32, 0x5c, 0x6a, 0x25, 0x8e, 0x7b, 0xf6, 0x00, 0xf7,
         0x4b, 0xd4, 0x71, 0x45, 0xd9, 0x1a, 0x4a, 0xd8, 0x9e, 0xbd},
    },
    {
        {0x53, 0x8e, 0x5e, 0x76, 0x2d, 0xb8, 0x9e, 0xee, 0xcf, 0x54, 0x48,
         0x15, 0x48, 0x26, 0x8d, 0xc8, 0x70, 0xa9, 0xbe, 0xd6, 0x58, 0x81,
         0xc1, 0xc2, 0xb1, 0xcc, 0x2a, 0x1b, 0x58, 0xc3, 0xdf, 0x3a},
        {0xbb, 0xd2, 0xc9, 0xf8, 0xa7, 0x0c, 0xed, 0x2c, 0xa2, 0x14, 0x68,
         0x98, 0x74, 0xaa, 0xf4, 0x1b, 0xe4, 0x1b, 0x36, 0x8c, 0x3c, 0x6b,
         0xa6, 0xaa, 0xf1, 0x3e, 0xd6, 0xd4, 0xd8, 0xc1, 0x59, 0xd5},
        {0xb3, 0x56, 0x65, 0x76, 0x25, 0x76, 0xb3, 0xd4, 0x86, 0xf3, 0x06,
         0x83, 0x4b, 0x08, 0xab, 0xe7, 0x70, 0xa7, 0xf2, 0xda, 0x80, 0xca,
         0x95, 0x1a, 0xc2, 0xa8, 0xa8, 0xf9, 0x73, 0x3b, 0xa6, 0xe0},
    },
    {
        {0x3e, 0x99, 0x11, 0x07, 0x83, 0xd9, 0x24, 0x40, 0x6d, 0x5b, 0xd0,
         0xf9, 0x7e, 0xe3, 0x44, 0xb1, 0xf6, 0xc5, 0x16, 0xdc, 0x7c, 0x7a,
         0x93, 0x04, 0x84, 0xc7, 0x6e, 0xa3, 0x1e, 0x75, 0x14, 0xbd},
        {0x26, 0x43, 0xd2, 0x05, 0x73, 0x54, 0xa4, 0x7c, 0xb1, 0xca, 0x7a,
         0x0a, 0x02, 0x7d, 0xf2, 0x84, 0x70, 0x91, 0xea, 0x74, 0x64, 0x17,
         0xdd, 0x18, 0x2b, 0xf2, 0x61, 0xb5, 0xbd, 0xb9, 0xd8, 0x59},
        {0x8b, 0x9c, 0xf0, 0x18, 0x5a, 0x5e, 0xc7, 0xdc, 0x68, 0x92, 0xbd,
         0xbc, 0x30, 0x06, 0xd5, 0xfb, 0xac, 0xa6, 0xcc, 0xa7, 0x49, 0xce,
         0xac, 0xec, 0xde, 0xcc, 0xe8, 0x52, 0xc0, 0x41, 0x63, 0x01},
    },
    {
        {0x20, 0x8e, 0x05, 0x3d, 0x93, 0x1e, 0xef, 0x69, 0x7c, 0x30, 0x40,
         0x5c, 0x3d, 0xc1, 0xe8, 0xb7, 0xe6, 0x9e, 0xef, 0x48, 0x62, 0x03,
         0x34, 0x71, 0xf7, 0xd9, 0xb8, 0x7f, 0x0f, 0x03, 0xf7, 0xb6},
        {0x0a, 0x5b, 0x9d, 0x29, 0x74, 0x36, 0x43, 0x9c, 0x6c, 0xda, 0xbf,
         0xbc, 0x1f, 0x83, 0x33, 0x96, 0xc0, 0x83, 0x49, 0x57, 0xb4, 0x84,
         0x09, 0x09, 0xb0, 0x2c, 0xdd, 0x5f, 0xcb, 0x10, 0xbd, 0xf8},
        {0x8c, 0xcb, 0x2a, 0xc2, 0xba, 0x61, 0x93, 0xee, 0x9a, 0x5e, 0x43,
         0x28, 0x99, 0x5e, 0x24, 0x55, 0xcf, 0x4a, 0x2a, 0x20, 0xb3, 0x77,
         0x40, 0x13, 0xed, 0xe8, 0x97, 0xcd, 0x9c, 0x84, 0x2b, 0x6b},
    },
    {
        {0x69, 0xa0, 0x5c, 0x02, 0x5e, 0xeb, 0x25, 0xcc, 0x57, 0x23, 0x5c,
         0xc5, 0xf3, 0xd3, 0x70, 0xcc, 0x60, 0xef, 0xc3, 0x3b, 0x28, 0xfa,
         0x36, 0x01, 0xac, 0xb8, 0x94, 0x8b, 0xa0, 0x5d, 0x7b, 0x1b},
        {0x5c, 0xe5, 0x21, 0x0d, 0xbd, 0x3e, 0xb5, 0x65, 0x0d, 0x9d, 0x31,
         0x6c, 0x9d, 0xa2, 0x14, 0xab, 0xb7, 0x2a, 0x67, 0xba, 0xa5, 0xa4,
         0x4d, 0xc8, 0xf9, 0xef, 0x81, 0xd0, 0xef, 0x99, 0xa9, 0x35},
        {0x56, 0x99, 0x49, 0x70, 0xd6, 0x67, 0x29, 0x99, 0x62, 0xa9, 0xa4,
         0xb5, 0x21, 0x20, 0xa4, 0xd6, 0x9e, 0x86, 0x26, 0x35, 0x7f, 0xaf,
         0xc6, 0xb8, 0x8c, 0x38, 0xfa, 0x28, 0x9b, 0x07, 0x1c, 0xfe},
    }};

const fmnt6753 rescue_MDS[RESCUE_SPONGE_SIZE][RESCUE_SPONGE_SIZE] = {
    {
        {0x00, 0xc2, 0x9b, 0x4a, 0xf8, 0x2e, 0x80, 0xfd, 0xfb, 0xae, 0x68, 0x0d,
         0x1e, 0xd1, 0x71, 0xba, 0xa0, 0xd5, 0xc8, 0xa1, 0x4e, 0xda, 0xcf, 0x0c,
         0xdd, 0xdd, 0xe2, 0xb3, 0xc7, 0xab, 0x08, 0xcf, 0x3c, 0x95, 0x35, 0x87,
         0x22, 0xcc, 0x08, 0x77, 0x5b, 0x10, 0x74, 0x7c, 0x29, 0xa8, 0x05, 0x5c,
         0x26, 0xea, 0xe9, 0x51, 0x15, 0x94, 0x4e, 0x37, 0x25, 0x50, 0xad, 0x9d,
         0x44, 0x2c, 0x51, 0xc6, 0x33, 0x8c, 0xad, 0x26, 0xda, 0x2a, 0x6c, 0x0e,
         0xcf, 0xb0, 0xa8, 0xde, 0x52, 0x84, 0x2d, 0x0e, 0x81, 0xbf, 0xb1, 0xe2,
         0x12, 0x92, 0x8a, 0x76, 0x97, 0x90, 0x1a, 0x8c, 0x1b, 0xd8, 0x49},
        {0x01, 0x64, 0x34, 0x0a, 0xe6, 0x88, 0x2c, 0x5e, 0xa7, 0xb1, 0xf7, 0x1d,
         0xd5, 0x1e, 0x24, 0x63, 0xb5, 0x15, 0x89, 0x5f, 0xb2, 0xbe, 0x25, 0xb0,
         0xf9, 0x7f, 0x65, 0x29, 0x17, 0xda, 0x39, 0x2a, 0x66, 0xe4, 0x21, 0x27,
         0x3c, 0x09, 0xe1, 0xcc, 0x25, 0x65, 0xd4, 0x4f, 0x11, 0x58, 0x64, 0x59,
         0x47, 0x01, 0x84, 0x76, 0xf7, 0x15, 0xbd, 0x2e, 0x54, 0xc3, 0xea, 0xef,
         0x39, 0x6e, 0x58, 0x5f, 0x7f, 0x80, 0x2b, 0x74, 0x23, 0x91, 0x48, 0x6f,
         0xbe, 0xa2, 0xc1, 0xe2, 0xb5, 0xc5, 0x55, 0x31, 0xb0, 0x68, 0xb1, 0x46,
         0x06, 0x4e, 0xeb, 0xc2, 0x9d, 0x39, 0x00, 0xaf, 0x2f, 0x52, 0xc7},
        {0x00, 0x3b, 0x42, 0x3e, 0x05, 0xbd, 0xf4, 0xcc, 0x39, 0x81, 0x7b, 0x31,
         0x73, 0xe1, 0x34, 0xef, 0x93, 0x2b, 0x20, 0xdf, 0x16, 0x22, 0x3e, 0xcd,
         0x4a, 0x9b, 0xe3, 0xff, 0xc5, 0x2c, 0xcf, 0xd5, 0x17, 0xc6, 0x40, 0xa6,
         0xe7, 0x67, 0x5a, 0x11, 0x1b, 0x37, 0x48, 0x53, 0x68, 0x44, 0xc1, 0x20,
         0x5c, 0x18, 0x34, 0xce, 0xed, 0x09, 0x6f, 0x7c, 0xa8, 0x8c, 0x61, 0x40,
         0xba, 0x05, 0xca, 0x01, 0xb7, 0x80, 0xe1, 0x77, 0x1a, 0xec, 0x73, 0x94,
         0xac, 0x13, 0x6a, 0xc6, 0xfc, 0x7e, 0xff, 0x34, 0x5c, 0x6c, 0x06, 0x27,
         0xff, 0x93, 0xa2, 0x70, 0xee, 0xed, 0xb1, 0xea, 0x93, 0xf0, 0x87},
    },
    {
        {0x00, 0xf8, 0x51, 0x56, 0x2a, 0xe5, 0x83, 0xaf, 0x26, 0x9f, 0x41, 0x09,
         0x01, 0x49, 0xa3, 0x94, 0x1f, 0xe9, 0x99, 0x70, 0x9c, 0xcc, 0x3c, 0x2e,
         0x5b, 0x48, 0xc3, 0x27, 0x6b, 0x84, 0x21, 0x51, 0x86, 0x4c, 0x09, 0x8b,
         0x8c, 0x7d, 0xfe, 0x81, 0xc5, 0x5d, 0x11, 0xbb, 0xc9, 0x81, 0x93, 0x6e,
         0x29, 0xba, 0xd7, 0x26, 0x5f, 0xcf, 0xd2, 0xdd, 0xae, 0x9c, 0xe9, 0x43,
         0x17, 0x43, 0xd5, 0x72, 0xdf, 0x59, 0xd6, 0x77, 0x20, 0x24, 0xbc, 0xed,
         0xc0, 0xbe, 0x1c, 0x1b, 0x27, 0x24, 0x84, 0xf0, 0x6c, 0x01, 0x07, 0xdf,
         0x85, 0x9d, 0x95, 0x87, 0x6a, 0x25, 0xd8, 0x2e, 0x9a, 0x14, 0x6d},
        {0x01, 0x4e, 0xf1, 0x30, 0x01, 0xf0, 0xbe, 0x83, 0x5f, 0x93, 0xf7, 0x70,
         0xe4, 0x82, 0x23, 0xf0, 0x43, 0x46, 0x0b, 0x70, 0x10, 0x50, 0xc3, 0x9e,
         0xef, 0xbb, 0x12, 0x24, 0xed, 0xef, 0x91, 0xb3, 0xc6, 0x03, 0x9d, 0x19,
         0x2b, 0xf9, 0x36, 0x6e, 0x13, 0x45, 0x24, 0xb5, 0x68, 0xe6, 0x52, 0x45,
         0x75, 0x8b, 0xab, 0xcd, 0x33, 0x49, 0x76, 0x3f, 0xa9, 0x6c, 0xbf, 0xa6,
         0xfc, 0x57, 0x32, 0x98, 0x73, 0x16, 0x1b, 0x1f, 0x5a, 0x88, 0x7f, 0xd5,
         0x6e, 0x9c, 0x6b, 0xd2, 0x4c, 0x8e, 0x55, 0xc0, 0x97, 0x39, 0xa7, 0x32,
         0x23, 0x11, 0x28, 0x09, 0xa3, 0xe4, 0x83, 0xc6, 0x61, 0xa5, 0x27},
        {0x00, 0x4c, 0xb0, 0x42, 0x39, 0x9a, 0xe4, 0x94, 0xe5, 0x59, 0xb2, 0x92,
         0xb7, 0x60, 0xd8, 0x99, 0x07, 0x3b, 0xf8, 0xaf, 0x8b, 0x28, 0x27, 0x22,
         0x85, 0xdb, 0xf0, 0x8d, 0xca, 0x7a, 0xee, 0x72, 0xe5, 0x82, 0x9f, 0xd8,
         0x39, 0xaa, 0xac, 0x53, 0xeb, 0xf9, 0xa8, 0xa6, 0x71, 0x2b, 0xa6, 0x25,
         0xcf, 0x58, 0x8d, 0x66, 0xc6, 0xab, 0xfc, 0x3d, 0x4d, 0x1c, 0x50, 0xc5,
         0x90, 0x0a, 0xab, 0x59, 0x04, 0x9e, 0xd2, 0x37, 0xd4, 0xf7, 0xb2, 0x5a,
         0x0a, 0x49, 0x7a, 0x0c, 0xf0, 0xd9, 0x79, 0x60, 0xbf, 0xcd, 0x2e, 0x99,
         0x27, 0xff, 0x5d, 0x49, 0x4c, 0xe6, 0x30, 0x27, 0xbb, 0x01, 0xe8},
    },
    {
        {0x01, 0x8f, 0x8a, 0x3e, 0xf5, 0x2c, 0x49, 0x8f, 0xe7, 0x64, 0x64, 0x8f,
         0x0d, 0x13, 0x23, 0x04, 0x55, 0xe1, 0x0e, 0x51, 0x2d, 0x89, 0xab, 0x85,
         0x7a, 0xd7, 0xa2, 0xc7, 0x09, 0x5a, 0x74, 0xd4, 0x32, 0x28, 0xb0, 0x65,
         0x6b, 0x6f, 0x39, 0xdc, 0x7b, 0xa2, 0x54, 0xc0, 0xbe, 0x9b, 0x27, 0x80,
         0x22, 0x75, 0x8b, 0x76, 0xa2, 0xf4, 0x08, 0x43, 0x21, 0x3f, 0xe4, 0x14,
         0x9c, 0x2f, 0xad, 0xbd, 0xce, 0x9d, 0xc0, 0x7b, 0xc8, 0x2a, 0xab, 0x6f,
         0xdb, 0x6e, 0xda, 0x43, 0xe3, 0x05, 0xd6, 0x40, 0x23, 0xae, 0x48, 0xb9,
         0xc7, 0xca, 0xfe, 0x53, 0xf8, 0xe9, 0x6e, 0xe0, 0x75, 0x29, 0x33},
        {0x01, 0xc3, 0x08, 0x18, 0x3b, 0x36, 0x7a, 0x99, 0x59, 0x30, 0xb3, 0x51,
         0xa5, 0xd8, 0xed, 0x75, 0xa1, 0x9a, 0x6f, 0x49, 0x99, 0x30, 0x0f, 0xe4,
         0x13, 0x77, 0x16, 0x3d, 0x7c, 0xd8, 0xd8, 0x94, 0x27, 0xec, 0x86, 0xd1,
         0x3e, 0x9f, 0x8c, 0x93, 0xf7, 0xc5, 0x63, 0x7b, 0x60, 0x4a, 0x1b, 0x8f,
         0x4e, 0xf1, 0xb3, 0xe0, 0xb2, 0x7f, 0x5d, 0xa0, 0x56, 0x8c, 0x3f, 0x6e,
         0xfb, 0x8e, 0x91, 0xef, 0x9b, 0x6a, 0x4a, 0x51, 0x2e, 0x63, 0x74, 0x85,
         0x3c, 0x82, 0xce, 0xc4, 0x6c, 0xbc, 0x3d, 0x40, 0x17, 0x7b, 0xcf, 0x77,
         0x85, 0x4f, 0x35, 0x47, 0xd0, 0x42, 0xcc, 0x27, 0x05, 0xdc, 0xfc},
        {0x00, 0xb9, 0x96, 0x31, 0xbc, 0xad, 0xb6, 0x84, 0x0f, 0x26, 0xc2, 0x22,
         0xe4, 0xc8, 0x6f, 0xaf, 0x87, 0x2a, 0xb8, 0xe8, 0x6d, 0x7b, 0xf2, 0x4a,
         0xd3, 0x13, 0x37, 0x4c, 0x2a, 0x6b, 0x29, 0x18, 0x55, 0x42, 0x5a, 0xc6,
         0xed, 0x7b, 0xad, 0xb4, 0x6d, 0x83, 0x03, 0xcb, 0xf6, 0x5c, 0x7d, 0x5b,
         0x9d, 0x92, 0x11, 0x7d, 0x0a, 0x27, 0xa9, 0x3d, 0x39, 0xb6, 0xd6, 0x28,
         0x04, 0xf9, 0x2e, 0x73, 0x1e, 0x15, 0x26, 0xd6, 0x03, 0x0b, 0xe9, 0xf7,
         0x83, 0xb4, 0x3a, 0x38, 0xcb, 0xce, 0xfb, 0x30, 0xbe, 0x94, 0xdd, 0x56,
         0xd1, 0x86, 0x6d, 0xf6, 0xe3, 0x5f, 0x17, 0x7e, 0xb2, 0x1a, 0xb5},
    }};

// s = x^(1/alpha)
void alphath_root(fmnt6753 c, fmnt6753 x) {
  // calculates a^e mod m, with
  // cx_math_powm(result_pointer, a, e, len_e, m, len(result, a, m)
  cx_math_powm(c, x, alpha_inv, fmnt6753_BYTES, fmnt6753_modulus,
               fmnt6753_BYTES);
}

void to_the_alpha(fmnt6753 c, fmnt6753 x) {
  cx_math_powm(c, x, alpha, fmnt6753_BYTES, fmnt6753_modulus, fmnt6753_BYTES);
}

void rescue(fmnt6753 state, const fmnt6753 input) {
  // Evaluates the block cipher with key=0 in forward direction.
  fmnt6753_add(state, input, rescue_keys[0]);
  for (int r = 1; r < (2 * RESCUE_ROUNDS); r++) {
    for (int i = 1; i < RESCUE_SPONGE_SIZE; i++) {
      if (r % 2 == 0) {
        alphath_root(state[i]);
      } else {
        to_the_alpha(state[i]);
      }
      fmnt6753_add(state, state, rescue_keys[r][i]);
      fmnt6753_mul(state, state, rescue_MDS[i]);
    }
  }
}
*/
