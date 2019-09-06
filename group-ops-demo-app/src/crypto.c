#include "os.h"
#include "cx.h"
#include "crypto.h"
#include <string.h>
#include <stdbool.h>

const fmnt6753 fmnt6753_modulus = {
    0x00, 0x01, 0xc4, 0xc6, 0x2d, 0x92, 0xc4, 0x11, 0x10, 0x22, 0x90, 0x22,
    0xee, 0xe2, 0xcd, 0xad, 0xb7, 0xf9, 0x97, 0x50, 0x5b, 0x8f, 0xaf, 0xed,
    0x5e, 0xb7, 0xe8, 0xf9, 0x6c, 0x97, 0xd8, 0x73, 0x07, 0xfd, 0xb9, 0x25,
    0xe8, 0xa0, 0xed, 0x8d, 0x99, 0xd1, 0x24, 0xd9, 0xa1, 0x5a, 0xf7, 0x9d,
    0xb2, 0x6c, 0x5c, 0x28, 0xc8, 0x59, 0xa9, 0x9b, 0x3e, 0xeb, 0xca, 0x94,
    0x29, 0x21, 0x26, 0x36, 0xb9, 0xdf, 0xf9, 0x76, 0x34, 0x99, 0x3a, 0xa4,
    0xd6, 0xc3, 0x81, 0xbc, 0x3f, 0x00, 0x57, 0x97, 0x4e, 0xa0, 0x99, 0x17,
    0x0f, 0xa1, 0x3a, 0x4f, 0xd9, 0x07, 0x76, 0xe2, 0x40, 0x00, 0x00, 0x01};

const fmnt6753 fmnt6753_zero = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const fmnt6753 fmnt6753_one = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

const fmnt6753 fmnt6753_two = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

const fmnt6753 fmnt6753_three = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};

const fmnt6753 fmnt6753_four = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};

const fmnt6753 fmnt6753_eight = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08};

const fmnt6753 gmnt6753_coeff_a = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b};

const fmnt6753 gmnt6753_coeff_b = {
    0x00, 0x00, 0x7d, 0xa2, 0x85, 0xe7, 0x08, 0x63, 0xc7, 0x9d, 0x56, 0x44,
    0x62, 0x37, 0xce, 0x2e, 0x14, 0x68, 0xd1, 0x4a, 0xe9, 0xbb, 0x64, 0xb2,
    0xbb, 0x01, 0xb1, 0x0e, 0x60, 0xa5, 0xd5, 0xdf, 0xe0, 0xa2, 0x57, 0x14,
    0xb7, 0x98, 0x59, 0x93, 0xf6, 0x2f, 0x03, 0xb2, 0x2a, 0x9a, 0x3c, 0x73,
    0x7a, 0x1a, 0x1e, 0x0f, 0xcf, 0x2c, 0x43, 0xd7, 0xbf, 0x84, 0x79, 0x57,
    0xc3, 0x4c, 0xca, 0x1e, 0x35, 0x85, 0xf9, 0xa8, 0x0a, 0x95, 0xf4, 0x01,
    0x86, 0x7c, 0x4e, 0x80, 0xf4, 0x74, 0x7f, 0xde, 0x5a, 0xba, 0x75, 0x05,
    0xba, 0x6f, 0xcf, 0x24, 0x85, 0x54, 0x0b, 0x13, 0xdf, 0xc8, 0x46, 0x8a};

const gmnt6753 gmnt6753_zero = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

// mnt6753 generator
//  .x =
//  "345842096948423570880626120012885054401707033383394411680148206454072326814
//  9235477762870414664917360605949659630933184751526227993647030875167687492714
//  052872195770088225183259051403087906158701786758441889742618916006546636728",
//  .y =
//  "274605084023319651496266002243821372545029759791683711116409247215891277253
//  7647351483823436111485517548824200743143907422382774281391189981793072811229
//  7763448010814764117701403540298764970469500339646563344680868495474127850569"

const gmnt6753 gmnt6753_one = {
    {0x00, 0x00, 0x25, 0x5f, 0x8e, 0x87, 0x6e, 0x83, 0x11, 0x47, 0x41, 0x2c,
     0xfb, 0x10, 0x02, 0x28, 0x4f, 0x30, 0x33, 0x80, 0x88, 0x13, 0x1c, 0x24,
     0x37, 0xe8, 0x84, 0xc4, 0x99, 0x7f, 0xd1, 0xdc, 0xb4, 0x09, 0x36, 0x7d,
     0x0c, 0x0d, 0x5f, 0xc5, 0xe8, 0x18, 0x77, 0x1b, 0x93, 0x1f, 0x1d, 0x5b,
     0xdd, 0x06, 0x9c, 0xe5, 0xe3, 0xc5, 0x7b, 0x6d, 0xf1, 0x20, 0xce, 0xe3,
     0xcd, 0x9d, 0x86, 0x7e, 0x66, 0xd1, 0x1a, 0xcb, 0xf7, 0xda, 0x60, 0x89,
     0x5b, 0x8b, 0x3d, 0x9d, 0x44, 0x2c, 0x4c, 0x41, 0x23, 0x32, 0x9a, 0x6f,
     0xef, 0xa9, 0xa1, 0xf3, 0xf7, 0xa1, 0xfb, 0xd9, 0x3a, 0x7b, 0xff, 0xb8},
    {0x00, 0x01, 0x28, 0xc0, 0x2f, 0xff, 0x6e, 0x2e, 0xb3, 0xfc, 0xa7, 0x0d,
     0xc1, 0x06, 0x3b, 0xac, 0x34, 0x55, 0x18, 0x01, 0x20, 0x2a, 0x35, 0x85,
     0xbd, 0xd6, 0xd7, 0x72, 0x2c, 0x6c, 0x07, 0xd7, 0x87, 0x3b, 0xb0, 0x2d,
     0x4c, 0x7a, 0x18, 0xed, 0x9c, 0x4b, 0xd3, 0xc7, 0xed, 0x0f, 0xfb, 0x31,
     0xc5, 0x7e, 0x61, 0x0d, 0xc7, 0xa5, 0x93, 0xcc, 0xe5, 0xa7, 0x92, 0xe9,
     0x4d, 0x00, 0x20, 0xc3, 0x35, 0xb7, 0x4d, 0x99, 0x92, 0xf5, 0xcb, 0xf4,
     0xb2, 0xcc, 0x4c, 0x42, 0xef, 0xf9, 0xa5, 0xa6, 0xc4, 0x52, 0x1d, 0xf9,
     0x85, 0x56, 0x87, 0x13, 0x9f, 0x0c, 0x51, 0x75, 0x4c, 0x0c, 0xcc, 0x49},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};

const scalar6753 scalar6753_zero = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const scalar6753 gmnt6753_group_order = {
    0x00, 0x01, 0xc4, 0xc6, 0x2d, 0x92, 0xc4, 0x11, 0x10, 0x22, 0x90, 0x22,
    0xee, 0xe2, 0xcd, 0xad, 0xb7, 0xf9, 0x97, 0x50, 0x5b, 0x8f, 0xaf, 0xed,
    0x5e, 0xb7, 0xe8, 0xf9, 0x6c, 0x97, 0xd8, 0x73, 0x07, 0xfd, 0xb9, 0x25,
    0xe8, 0xa0, 0xed, 0x8d, 0x99, 0xd1, 0x24, 0xd9, 0xa1, 0x5a, 0xf7, 0x9d,
    0xb1, 0x17, 0xe7, 0x76, 0xf2, 0x18, 0x05, 0x9d, 0xb8, 0x0f, 0x0d, 0xa5,
    0xcb, 0x53, 0x7e, 0x38, 0x68, 0x5a, 0xcc, 0xe9, 0x76, 0x72, 0x54, 0xa4,
    0x63, 0x88, 0x10, 0x71, 0x9a, 0xc4, 0x25, 0xf0, 0xe3, 0x9d, 0x54, 0x52,
    0x2c, 0xdd, 0x11, 0x9f, 0x5e, 0x90, 0x63, 0xde, 0x24, 0x5e, 0x80, 0x01};

void fmnt6753_add(fmnt6753 c, const fmnt6753 a, const fmnt6753 b) {
  fmnt6753 d;
  cx_math_addm(d, a, b, fmnt6753_modulus, fmnt6753_BYTES);
  c = d;
}

void fmnt6753_sub(fmnt6753 c, const fmnt6753 a, const fmnt6753 b) {
  fmnt6753 d;
  cx_math_subm(d, a, b, fmnt6753_modulus, fmnt6753_BYTES);
  c = d;
}

void fmnt6753_mul(fmnt6753 c, const fmnt6753 a, const fmnt6753 b) {
  fmnt6753 d;
  cx_math_multm(d, a, b, fmnt6753_modulus, fmnt6753_BYTES);
  c = d;
}

void fmnt6753_sq(fmnt6753 c, const fmnt6753 a) {
  scalar6753 d;
  cx_math_multm(d, a, a, fmnt6753_modulus, fmnt6753_BYTES);
  c = d;
}

bool fmnt6753_is_zero(const fmnt6753 k) {
  return (os_memcmp(k, fmnt6753_zero, fmnt6753_BYTES) == 0);
}

bool fmnt6753_eq(const fmnt6753 a, const fmnt6753 b) {
  return (os_memcmp(a, b, fmnt6753_BYTES) == 0);
}

// scalar arithmetic
bool scalar6753_is_zero(const scalar6753 k) {
  return (os_memcmp(k, scalar6753_zero, scalar6753_BYTES) == 0);
}

void scalar6753_add(scalar6753 c, const scalar6753 a, const scalar6753 b) {
  scalar6753 d;
  cx_math_addm(c, a, b, gmnt6753_group_order, scalar6753_BYTES);
  c = d;
}

void scalar6753_mul(scalar6753 c, const scalar6753 a, const scalar6753 b) {
  scalar6753 d;
  cx_math_multm(c, a, b, gmnt6753_group_order, scalar6753_BYTES);
  c = d;
}

bool is_zero(const gmnt6753 *p) {
  return (os_memcmp(p->X, fmnt6753_zero, fmnt6753_BYTES) == 0 &&
      os_memcmp(p->Y, fmnt6753_zero, fmnt6753_BYTES) == 0);
}

bool is_on_curve(const gmnt6753 *p) {
  if (is_zero(p)) {
    return true;
/*
  } else if (fmnt6753_eq(p->Z, fmnt6753_one)) {
    // we can check y^2 == x^3 + ax + b
    fmnt6753_sq(lhs, p->Y);                       // y^2
    fmnt6753_sq(rhs, p->X);                       // x^2
    fmnt6753_add(rhs, rhs, gmnt6753_coeff_a);     // x^2 + a
    fmnt6753_mul(rhs, rhs, p->X);                 // x^3 + ax
    fmnt6753_add(rhs, rhs, gmnt6753_coeff_b);     // x^3 + ax + b
  } else {
    // we check (y/z)^2 == (x/z)^3 + a(x/z) + b
    // => z(y^2 - bz^2) == x(x^2 + az^2)
    fmnt6753 x2, y2, z2;
    fmnt6753_sq(x2, p->X);
    fmnt6753_sq(y2, p->Y);
    fmnt6753_sq(z2, p->Z);

    fmnt6753_mul(lhs, z2, gmnt6753_coeff_b);      // bz^2
    fmnt6753_sub(lhs, y2, lhs);                   // y^2 - bz^2
    fmnt6753_mul(lhs, p->Z, lhs);                 // z(y^2 - bz^2)
    fmnt6753_mul(rhs, z2, gmnt6753_coeff_a);      // az^2
    fmnt6753_add(rhs, x2, rhs);                   // x^2 + az^2
    fmnt6753_mul(rhs, p->X, rhs);                 // x(x^2 + az^2)

  }
  return (os_memcmp(y2, x3axb, fmnt6753_BYTES) == 0);
*/
}
return true;
}

void gmnt6753_affine_add(gmnt6753 *r, const gmnt6753 *p, const gmnt6753 *q) {

  if (is_zero(p)) {
    *r = *q;
    return;
  }
  if (is_zero(q)) {
    *r = *p;
    return;
  }

  fmnt6753 lambda, xqxp, yqyp, ixqxp;
  cx_math_subm(xqxp, q->X, p->X, fmnt6753_modulus, fmnt6753_BYTES);     // xq - xp
  cx_math_subm(yqyp, q->Y, p->Y, fmnt6753_modulus, fmnt6753_BYTES);     // yq - yp
  cx_math_invprimem(ixqxp, xqxp, fmnt6753_modulus, fmnt6753_BYTES);     // 1 / (xq - xp)
  cx_math_multm(lambda, yqyp, ixqxp, fmnt6753_modulus, fmnt6753_BYTES); // (yq - yp)/(xq - xp)

  fmnt6753 l2, lxp;
  cx_math_multm(l2, lambda, lambda, fmnt6753_modulus, fmnt6753_BYTES);  // lambda^2
  cx_math_subm(lxp, l2, p->X, fmnt6753_modulus, fmnt6753_BYTES);        // lambda^2 - xp
  cx_math_subm(r->X, lxp, q->X, fmnt6753_modulus, fmnt6753_BYTES);      // lambda^2 - xp - xq

  fmnt6753 xpxr, lxpxr;
  cx_math_subm(xpxr, p->X, r->X, fmnt6753_modulus, fmnt6753_BYTES);     // xp - xr
  cx_math_multm(lxpxr, lambda, xpxr, fmnt6753_modulus, fmnt6753_BYTES); // lambda(xp - xr)
  cx_math_subm(r->Y, lxpxr, p->Y, fmnt6753_modulus, fmnt6753_BYTES);    // lambda(xp - xr) - yp
}

void gmnt6753_affine_double(gmnt6753 *r, const gmnt6753 *p) {

  if (is_zero(p)) {
    *r = gmnt6753_zero;
    return;
  }

  fmnt6753 lambda, xp2, xp22, xp23, xp23a, yp2, iyp2;
  cx_math_multm(xp2, p->X, p->X, fmnt6753_modulus, fmnt6753_BYTES);     // xp^2
  cx_math_addm(xp22, xp2, xp2, fmnt6753_modulus, fmnt6753_BYTES);       // 2xp^2
  cx_math_addm(xp23, xp22, xp2, fmnt6753_modulus, fmnt6753_BYTES);      // 3xp^2
  cx_math_addm(xp23a, xp23, gmnt6753_coeff_a, fmnt6753_modulus, fmnt6753_BYTES);  // 3xp^2 + a
  cx_math_addm(yp2, p->Y, p->Y, fmnt6753_modulus, fmnt6753_BYTES);      // 2yp
  cx_math_invprimem(iyp2, yp2, fmnt6753_modulus, fmnt6753_BYTES);       // 1/2yp
  cx_math_multm(lambda, xp23a, iyp2, fmnt6753_modulus, fmnt6753_BYTES); // (3xp^2 + a)/2yp

  fmnt6753 l2, lxp;
  cx_math_multm(l2, lambda, lambda, fmnt6753_modulus, fmnt6753_BYTES);  // lambda^2
  cx_math_subm(lxp, l2, p->X, fmnt6753_modulus, fmnt6753_BYTES);        // lambda^2 - xp
  cx_math_subm(r->X, lxp, p->X, fmnt6753_modulus, fmnt6753_BYTES);      // lambda^2 - xp - xp

  fmnt6753 xpxr, lxpxr;
  cx_math_subm(xpxr, p->X, r->X, fmnt6753_modulus, fmnt6753_BYTES);     // xp - xr
  cx_math_multm(lxpxr, lambda, xpxr, fmnt6753_modulus, fmnt6753_BYTES); // lambda(xp - xr)
  cx_math_subm(r->Y, lxpxr, p->Y, fmnt6753_modulus, fmnt6753_BYTES);    // lambda(xp - xr) - yp
}

void gmnt6753_affine_scalar_mul(gmnt6753 *r, const scalar6753 k, const gmnt6753 *p) {
//void gmnt6753_affine_scalar_mul(gmnt6753 *r, const scalar6753 k, const gmnt6753 *p) {

  *r = gmnt6753_zero;
  if (is_zero(p)) {
    return;
  }
  if (scalar6753_is_zero(k)) {
    return;
  }

  gmnt6753 q = gmnt6753_zero;
  // 96 bytes = 8 * 96 = 768. we want 753, 768 - 753 = 15 bits
  // which means we have an offset of 15 bits
   for (int i = 15; i < (scalar6753_BITS + 15); i++) {
    int di = k[i/8] & (1 << ((7-i) % 8));
    gmnt6753 q0 = gmnt6753_zero;
    gmnt6753_affine_double(&q0, &q);
    // gmnt6753 temp = q;
    q = q0;
    // q0 = temp;
    if (di != 0) {
      gmnt6753 q1 = gmnt6753_zero;
      gmnt6753_affine_add(&q1, &q, p);
      // gmnt6753 temp = q;
      q = q1;
      // q1 = temp;
    }
  }
  *r = q;
}
