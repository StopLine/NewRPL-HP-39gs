/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// SPECIAL FUNCTIONS DEALING WITH NUMERIC CALCULATIONS

#include "newrpl.h"
#include "libraries.h"

// EITHER RETURN THE FACTORIAL AS A BINT64 WHEN THE NUMBER IS SMALL ENOUGH
// OR RETURN THE FACTORIAL ON RReg[0], WHEN THE NUMBER GROWS OUT OF RANGE
// THE RETURN VALUE IS EITHER -1 OR THE ACTUAL FACTORIAL

BINT64 factorialBINT(BINT n)
{
    BINT64 result=1;
    BINT k;

    for(k=2;(k<=n)&&(k<=20);++k) result=result*k;
    if(k>n) return result;

    Context.precdigits+=8;
    newRealFromBINT64(&RReg[0],result);

    for(;k<=n;++k) {
            newRealFromBINT(&RReg[1],k);
            mulReal(&RReg[0],&RReg[0],&RReg[1]);
            if(RReg[0].flags&(F_INFINITY|F_NOTANUMBER|F_OVERFLOW|F_ERROR)) {
                rplError(ERR_NUMBERTOOBIG);
                Context.precdigits-=8;
                return -1;
            }

    }
    Context.precdigits-=8;

    round_real(&RReg[0],Context.precdigits,0);

    return -1;
}







// PRIMALITY TESTS

#define MAX_PRIME 122880
#define PRIMEDATA_SIZE ((MAX_PRIME+29)/30)

const BYTE const primesieve_data[PRIMEDATA_SIZE]= {
0xFF,0xDF,0xEF,0x7E,0xB6,0xDB,0x3D,0xF9,0xD5,0x4F,0x1E,0xF3,0xEA,0xA6,0xED,0x9E,0xE6,
0x0C,0xD3,0xD3,0x3B,0xDD,0x59,0xA5,0x6A,0x67,0x92,0xBD,0x78,0x1E,0xA6,0x56,0x56,
0xE3,0xAD,0x2D,0xDE,0x2A,0x4C,0x55,0xD9,0xA3,0xF0,0x9F,0x03,0x54,0xA1,0xF8,0x2E,
0xFD,0x44,0xE9,0x66,0xF6,0x13,0x3A,0xB8,0x4C,0x2B,0x3A,0x45,0x11,0xBF,0x54,0x8C,
0xC1,0x7A,0xB3,0xC8,0xBC,0x8C,0x4F,0x21,0x58,0x71,0x71,0x9B,0xC1,0x17,0xEF,0x54,
0x96,0x1A,0x08,0xE5,0x83,0x8C,0x46,0x72,0xFB,0xAE,0x65,0x92,0x8F,0x58,0x87,0xD2,
0x92,0xD8,0x81,0x65,0x26,0xE3,0xA0,0x11,0x38,0xC7,0x26,0x3C,0x81,0xEB,0x99,0x8D,
0x51,0x88,0x3E,0x24,0xF3,0x33,0x4D,0x5A,0x8B,0x1C,0xA7,0x2A,0xB4,0x58,0x4C,0x4E,
0x26,0xF6,0x19,0x82,0xDC,0x83,0xC3,0x2C,0xF1,0x38,0x02,0xB5,0xCD,0xCD,0x02,0xB2,
0x4A,0x94,0x0C,0x57,0x4C,0x7A,0x30,0x43,0x0B,0xF1,0xCB,0x44,0x6C,0x24,0xF8,0x19,
0x01,0x95,0xA8,0x5C,0x73,0xEA,0x8D,0x24,0x96,0x2B,0x50,0xA6,0x22,0x1E,0xC4,0xD1,
0x48,0x06,0xD4,0x3A,0x2F,0x74,0x9C,0x07,0x6A,0x05,0x88,0xBF,0x68,0x15,0x2E,0x60,
0x55,0xE3,0xB7,0x51,0x98,0x08,0x14,0x86,0x5A,0xAA,0x45,0x4D,0x49,0x70,0x27,0xD2,
0x93,0xD5,0xCA,0xAB,0x02,0x83,0x61,0x05,0x24,0xCE,0x87,0x22,0xC2,0xA9,0xAD,0x18,
0x8C,0x4D,0x78,0xD1,0x89,0x16,0xB0,0x57,0xC7,0x62,0xA2,0xC0,0x34,0x24,0x52,0xAE,
0x5A,0x40,0x32,0x8D,0x21,0x08,0x43,0x34,0xB6,0xD2,0xB6,0xD9,0x19,0xE1,0x60,0x67,
0x1A,0x39,0x60,0xD0,0x44,0x7A,0x94,0x9A,0x09,0x88,0x83,0xA8,0x74,0x55,0x10,0x27,
0xA1,0x5D,0x68,0x1E,0x23,0xC8,0x32,0xE0,0x19,0x03,0x44,0x73,0x48,0xB1,0x38,0xC3,
0xE6,0x2A,0x57,0x61,0x98,0xB5,0x1C,0x0A,0x68,0xC5,0x81,0x8F,0xAC,0x02,0x29,0x1A,
0x47,0xE3,0x94,0x11,0x4E,0x64,0x2E,0x14,0xCB,0x3D,0xDC,0x14,0xC5,0x06,0x10,0xE9,
0x29,0xB1,0x82,0xE9,0x30,0x47,0xE3,0x34,0x19,0xC3,0x25,0x0A,0x30,0x30,0xB4,0x6C,
0xC1,0xE5,0x46,0x44,0xD8,0x8E,0x4C,0x5D,0x22,0x24,0x70,0x78,0x92,0x89,0x81,0x82,
0x56,0x26,0x1B,0x86,0xE9,0x08,0xA5,0x00,0xD3,0xC3,0x29,0xB0,0xC2,0x4A,0x10,0xB2,
0x59,0x38,0xA1,0x1D,0x42,0x60,0xC7,0x22,0x27,0x8C,0xC8,0x44,0x1A,0xC6,0x8B,0x82,
0x81,0x1A,0x46,0x10,0xA6,0x31,0x09,0xF0,0x54,0x2F,0x18,0xD2,0xD8,0xA9,0x15,0x06,
0x2E,0x0C,0xF6,0xC0,0x0E,0x50,0x91,0xCD,0x26,0xC1,0x18,0x38,0x65,0x19,0xC3,0x56,
0x93,0x8B,0x2A,0x2D,0xD6,0x84,0x4A,0x61,0x0A,0xA5,0x2C,0x09,0xE0,0x76,0xC4,0x6A,
0x3C,0xD8,0x08,0xE8,0x14,0x66,0x1B,0xB0,0xA4,0x02,0x63,0x36,0x10,0x31,0x07,0xD5,
0x92,0x48,0x42,0x12,0xC3,0x8A,0xA0,0x9F,0x2D,0x74,0xA4,0x82,0x85,0x78,0x5C,0x0D,
0x18,0xB0,0x61,0x14,0x1D,0x02,0xE8,0x18,0x12,0xC1,0x01,0x49,0x1C,0x83,0x30,0x67,
0x33,0xA1,0x88,0xD8,0x0F,0x0C,0xF4,0x98,0x88,0x58,0xD7,0x66,0x42,0x47,0xB1,0x16,
0xA8,0x96,0x08,0x18,0x41,0x59,0x15,0xB5,0x44,0x2A,0x52,0xE1,0xB3,0xAA,0xA1,0x59,
0x45,0x62,0x55,0x18,0x11,0xA5,0x0C,0xA3,0x3C,0x67,0x00,0xBE,0x54,0xD6,0x0A,0x20,
0x36,0x6B,0x82,0x0C,0x15,0x08,0x7E,0x56,0x91,0x01,0x78,0xD0,0x61,0x0A,0x84,0xA8,
0x2C,0x01,0x57,0x0E,0x56,0xA0,0x50,0x0B,0x98,0x8C,0x47,0x6C,0x20,0x63,0x10,0xC4,
0x09,0xE4,0x0C,0x57,0x88,0x0B,0x75,0x0B,0xC2,0x52,0x82,0xC2,0x39,0x24,0x02,0x2C,
0x56,0x25,0x7A,0x31,0x29,0xD6,0xA3,0x20,0xE1,0xB1,0x18,0xB0,0x0C,0x8A,0x32,0xC1,
0x11,0x32,0x09,0xC5,0xAD,0x30,0x37,0x08,0xBC,0x91,0x82,0xCF,0x20,0x25,0x6B,0x9C,
0x30,0x8F,0x44,0x26,0x46,0x6A,0x07,0x49,0x8E,0x09,0x58,0x10,0x02,0x25,0xC5,0xC4,
0x42,0x5A,0x80,0xA0,0x80,0x3C,0x90,0x28,0x64,0x14,0xE1,0x03,0x84,0x51,0x0C,0x2E,
0xA3,0x8A,0xA4,0x08,0xC0,0x47,0x7E,0xD3,0x2B,0x03,0xCD,0x54,0x2A,0x00,0x04,0xB3,
0x92,0x6C,0x42,0x29,0x4C,0x83,0xC1,0x92,0xCC,0x1C,0x2D,0x46,0x21,0xDB,0x38,0x59,
0x84,0x8C,0x24,0x12,0x58,0xBB,0xE0,0x06,0x0D,0x70,0x30,0xC9,0x09,0x28,0x91,0x41,
0x44,0x32,0xF9,0x8C,0x30,0x80,0xC2,0x72,0xA4,0x62,0x0C,0x7D,0x81,0x83,0x14,0xE1,
0xAA,0x0E,0x15,0x82,0x0A,0x78,0x14,0x70,0x97,0x08,0x10,0x6F,0x2E,0xF0,0xB2,0x1D,
0x30,0x49,0x44,0x32,0x53,0x62,0x86,0x65,0x45,0x84,0x0A,0x11,0x4B,0x36,0xD9,0x8C,
0x69,0x3A,0x61,0x80,0x90,0x7C,0x19,0xC0,0x30,0x95,0x40,0x8B,0x0C,0x05,0x2D,0x0E,
0xC0,0x71,0xA1,0xB4,0x96,0x85,0x1A,0x16,0xC0,0x15,0x14,0x51,0x4C,0x48,0xB7,0x79,
0x95,0x10,0x89,0x8A,0x2E,0x02,0xA1,0x1C,0xD5,0x90,0x81,0x10,0x91,0x08,0x22,0xB4,
0x1E,0xE9,0x78,0xC0,0x33,0x20,0x5C,0x8B,0xC4,0x0E,0xE2,0xAA,0x23,0x10,0x47,0xE3,
0x28,0x55,0x7B,0x19,0xA1,0x51,0xA8,0x06,0x04,0x90,0x82,0x2C,0xD1,0x61,0x60,0xA1,
0x52,0x06,0x41,0x44,0x81,0x54,0x23,0x88,0x18,0xA9,0x04,0x27,0x22,0x72,0x48,0xB6,
0x40,0x1C,0x42,0x12,0x17,0xCA,0x29,0xA0,0x5A,0x01,0x3C,0xE1,0x52,0x84,0x89,0xDA,
0x01,0x40,0x06,0xF1,0x2C,0x69,0xD2,0x48,0x42,0x63,0xC1,0x21,0x34,0x8A,0xC8,0x6C,
0x21,0x71,0x26,0x09,0x88,0x2E,0x44,0xD5,0x28,0x92,0x1D,0x98,0x86,0x12,0xD1,0x52,
0xA3,0xA0,0x46,0x20,0x46,0x14,0x50,0x83,0xDC,0xDD,0x83,0x50,0x44,0xA9,0x8C,0x4D,
0x14,0x2E,0x54,0x14,0x89,0x1B,0x39,0x42,0x80,0x14,0x81,0x29,0x8F,0x80,0x02,0x03,
0x32,0x96,0x82,0x83,0xB0,0x05,0x0C,0x14,0x51,0x88,0x07,0xF8,0xD9,0xEC,0x1A,0x06,
0x48,0x31,0x5C,0x43,0x86,0x06,0x23,0xB3,0x09,0x44,0x0C,0xA5,0x00,0xF0,0xB2,0x80,
0x01,0xCA,0xE1,0x42,0xC2,0x18,0xB6,0x2C,0x9B,0xC7,0x2E,0x52,0x29,0x2C,0x61,0x1A,
0x8A,0x26,0x24,0x8B,0x84,0x80,0x5E,0x60,0x02,0xC1,0x53,0x2A,0x35,0x45,0x21,0x52,
0xD1,0x42,0xB8,0xC5,0x4A,0x48,0x04,0x16,0x70,0x23,0x61,0xC1,0x09,0x50,0xC4,0x81,
0x88,0x18,0xD9,0x01,0x16,0x22,0x28,0x8C,0x90,0x84,0xEB,0x38,0x24,0x53,0x09,0xA9,
0x41,0x6A,0x14,0x82,0x09,0x86,0x30,0x9C,0xA2,0x38,0x42,0x00,0x2B,0x95,0x5A,0x02,
0x4A,0xC1,0xA2,0xAD,0x45,0x04,0x27,0x54,0xC0,0x28,0xA9,0x05,0x8C,0x2E,0x6E,0x60,
0xF3,0x16,0x90,0x1B,0x82,0x1E,0x00,0x80,0x26,0x1C,0x1D,0x4A,0x14,0xC2,0x1A,0x22,
0x3C,0x85,0xC1,0x10,0xB0,0x48,0x11,0x00,0x4A,0xC5,0x30,0xF4,0xAA,0x30,0x8C,0xC8,
0x49,0x18,0x07,0xF0,0x1C,0xE9,0x07,0xCD,0x0C,0x22,0x1B,0x1C,0xEC,0xC2,0x45,0x0A,
0x50,0x3A,0x20,0xE4,0x0E,0x6C,0x20,0x82,0xC3,0x91,0xD9,0xC8,0x62,0x4C,0xD1,0x80,
0x85,0x65,0x09,0x02,0x30,0x95,0xF9,0x10,0x69,0x02,0xA6,0x4A,0x64,0xC2,0xB6,0xF9,
0x16,0x20,0x48,0xA1,0x73,0x94,0x31,0x54,0x61,0x44,0x07,0x03,0x82,0x2C,0x06,0x00,
0x38,0x33,0x09,0x1C,0xC1,0x4B,0xCE,0x12,0x35,0x41,0xA4,0x90,0x99,0x2D,0x2A,0xC0,
0x59,0x84,0xD1,0x4A,0xA4,0x72,0x04,0x22,0x3C,0x54,0xC2,0xA0,0x0C,0x01,0x18,0xAC,
0xAC,0x96,0xE4,0x04,0x04,0x03,0x93,0x05,0x9B,0x48,0x44,0x63,0x32,0x01,0x31,0x5F,
0x60,0x5C,0x02,0x00,0x03,0x80,0xD1,0x04,0x2E,0x34,0xA9,0x30,0x49,0xC4,0xCA,0x1E,
0x02,0x4B,0x32,0x14,0x05,0x86,0x22,0xC5,0xC2,0x2A,0x4C,0xC8,0x00,0x08,0xF3,0x18,
0x38,0x65,0x99,0x82,0x4A,0x54,0x63,0xA1,0x94,0x8F,0x44,0x12,0x93,0xE9,0x19,0x28,
0xCA,0xA1,0x06,0x12,0x8A,0x20,0xA5,0x51,0x48,0x18,0x70,0x31,0xAE,0x90,0x08,0x43,
0x68,0x97,0x32,0xAF,0x91,0xC8,0x0A,0x2C,0x02,0x02,0x8C,0x0C,0x51,0xA5,0x12,0x00,
0x1B,0x1E,0x81,0x8A,0x08,0x28,0x60,0xD2,0x86,0x34,0x44,0x63,0x76,0x04,0x43,0x00,
0xF1,0x01,0x04,0x40,0x36,0xC8,0xA1,0x8C,0xC3,0xCE,0x32,0x42,0x09,0x29,0x55,0x44,
0x4E,0x28,0x43,0x60,0x1C,0xA0,0xDA,0x28,0x0E,0xA5,0xF2,0x05,0x40,0x59,0x8C,0x4E,
0xA2,0x60,0x05,0xE0,0x05,0xC5,0x04,0x62,0x81,0x26,0xA8,0x8A,0xA4,0x24,0xB4,0xD3,
0x1A,0x81,0x42,0xE4,0x60,0x34,0x08,0x2A,0x39,0xCC,0x62,0x08,0x47,0xD0,0x00,0x40,
0x07,0x02,0x78,0x20,0xD9,0xA2,0x89,0x50,0x23,0x32,0x00,0xB8,0x80,0x64,0x11,0xE9,
0x74,0x62,0x0A,0xAA,0x44,0x08,0x8B,0x16,0x06,0x18,0x2D,0xF1,0x4C,0x4F,0x2A,0x21,
0x40,0x15,0xB4,0x0C,0x48,0x22,0xD1,0xE1,0x80,0xD8,0x17,0xA7,0x14,0xA0,0x82,0x15,
0x64,0xC0,0x89,0x5A,0x11,0xBB,0x8C,0x78,0x08,0xCD,0x04,0x24,0xA2,0x9B,0x9C,0x10,
0xCC,0x42,0xA3,0x28,0x3B,0x58,0x5F,0xA6,0x40,0x90,0x23,0x04,0x2C,0xD2,0xAE,0x52,
0x51,0x1A,0xA4,0x69,0x80,0xC1,0x4A,0xA3,0xC8,0x90,0x19,0x48,0x42,0x24,0x22,0x43,
0x02,0x45,0xC1,0x05,0x00,0x74,0x11,0x15,0x94,0x10,0x0C,0x38,0x73,0x8A,0x25,0x04,
0x07,0x28,0x7E,0x01,0x40,0x82,0x41,0x48,0x6D,0x16,0x36,0x29,0x31,0xE5,0x81,0xA4,
0x1C,0x81,0xFA,0x09,0x00,0x14,0x2D,0x10,0x60,0x40,0x97,0xDC,0x88,0x02,0x4E,0x17,
0x98,0x85,0x44,0x9C,0xA2,0x60,0x91,0x8A,0x92,0x68,0x19,0x0A,0x40,0x51,0x80,0x37,
0x84,0x15,0x40,0x62,0x64,0xB2,0x0E,0x91,0x48,0x2A,0x00,0x55,0x48,0x99,0xB0,0x08,
0x85,0x1A,0x13,0x10,0x9B,0x00,0x5E,0xC9,0x50,0x00,0xE8,0xA4,0x00,0x83,0x45,0x6C,
0x40,0x0B,0x9D,0xA0,0x02,0x2D,0x34,0x92,0x01,0x21,0x81,0x17,0xA3,0x46,0xA0,0x8A,
0x85,0xE8,0x88,0x47,0x18,0x96,0x01,0xB6,0x41,0x93,0x84,0x08,0x02,0x18,0x02,0x35,
0x50,0xE4,0x3A,0x81,0x10,0x11,0xA1,0xDA,0x82,0x4C,0x34,0x32,0x85,0x00,0x57,0x4B,
0x28,0xA5,0xA8,0x38,0x84,0x1D,0x0E,0x30,0xB3,0x50,0x22,0x41,0xC3,0xC6,0x06,0xD0,
0x20,0x28,0xA1,0x05,0x8A,0x1A,0x84,0x22,0x85,0x0C,0x46,0x44,0x72,0x90,0xC8,0x17,
0xD8,0x41,0x61,0x64,0x30,0x39,0x00,0x0C,0x97,0xA3,0x0E,0xB2,0x29,0x02,0x50,0x88,
0xA9,0x5C,0x22,0xC1,0x8E,0x90,0xC3,0x47,0x40,0x31,0x12,0x2C,0x38,0x8C,0x63,0x64,
0x86,0xC0,0xB3,0x00,0x4A,0x2A,0x38,0x11,0x91,0x13,0x38,0x47,0x49,0x14,0x81,0x73,
0x02,0x2C,0x4C,0x8E,0x52,0xE5,0xD1,0x10,0x10,0x07,0x4C,0x06,0x65,0x61,0x1A,0x54,
0x84,0x01,0x4A,0xE2,0x01,0x2F,0x10,0x06,0x09,0x42,0x44,0x9A,0x00,0xA9,0x52,0x8C,
0x28,0x94,0x61,0x01,0x99,0x05,0x20,0x1C,0x23,0x78,0xA2,0x51,0x18,0x82,0x3C,0x41,
0xD0,0xB7,0xC4,0x43,0x87,0x44,0xC6,0x0A,0x93,0x68,0xC1,0x81,0x30,0x52,0xB2,0xA8,
0xC1,0x8A,0x24,0x1C,0x21,0x03,0x33,0x41,0x14,0x4A,0x2E,0x41,0xE2,0x82,0x45,0x01,
0xAC,0x06,0x40,0x18,0x20,0x95,0x4C,0x67,0x20,0x26,0x50,0x94,0x55,0x92,0x08,0x1A,
0x82,0xE1,0x8C,0x10,0x5B,0x61,0x02,0x86,0x72,0x99,0x30,0x8D,0x06,0x2C,0xB0,0xA1,
0x18,0x28,0x06,0x27,0x46,0x20,0x18,0x82,0x21,0x9C,0xEB,0x18,0xC4,0x30,0x81,0x58,
0xC1,0x84,0x06,0x37,0x9A,0x24,0x50,0x0D,0xA2,0x40,0x05,0x29,0x19,0x09,0x02,0x84,
0x52,0x64,0x11,0x17,0xEC,0xD1,0x20,0x78,0x86,0x63,0x0E,0xD1,0x11,0x2A,0x60,0x03,
0xD1,0x22,0x60,0x84,0x06,0x32,0xA4,0x68,0x24,0x81,0xC7,0xC0,0x12,0x42,0x01,0xBB,
0xA8,0x00,0x41,0x7C,0x41,0x21,0x81,0x91,0x10,0x85,0x14,0xA0,0xA8,0x2D,0x5C,0x15,
0x47,0x40,0xD0,0xFA,0x2A,0x84,0x0E,0x8F,0x10,0xD2,0x80,0x09,0xD4,0x49,0x21,0x26,
0x32,0xB0,0x38,0x20,0x41,0xA0,0x4A,0x11,0x4B,0xBA,0xAC,0x4E,0x20,0x34,0x61,0x60,
0x15,0x0D,0x13,0x6A,0x70,0x43,0x42,0x9C,0x2D,0x55,0xE8,0x00,0x96,0x98,0x92,0xE4,
0xC8,0x4C,0x0C,0xA5,0x19,0x81,0x31,0xCA,0x42,0x02,0x12,0x4A,0x30,0x30,0x51,0x01,
0x66,0xA2,0xE2,0x16,0x41,0xCF,0xA0,0x2A,0x84,0x51,0x18,0x18,0x41,0xC6,0x56,0xA0,
0x21,0x8E,0xE1,0x96,0x0B,0x04,0x60,0x11,0xB8,0x4C,0x93,0x0D,0x06,0x04,0x2A,0x2A,
0x4C,0x04,0x86,0x2A,0x51,0x51,0x31,0x00,0x01,0x80,0x76,0x51,0x98,0x94,0x2D,0xCC,
0x4A,0x6A,0xA4,0x42,0xB2,0x69,0x48,0x01,0x40,0xB5,0xD2,0x32,0x5C,0x0E,0x62,0x20,
0x16,0x01,0x37,0xE4,0xC1,0xC2,0x58,0x24,0x23,0x08,0x88,0x02,0x24,0x14,0x85,0x98,
0x06,0xD0,0x1D,0xAA,0x34,0x80,0x0A,0x34,0x61,0x10,0x4B,0x48,0x42,0xC2,0x02,0x98,
0x94,0x00,0x20,0x57,0xB3,0xA1,0x88,0x1A,0xCE,0x08,0x05,0xA0,0xB0,0x79,0x4A,0x01,
0x14,0x46,0x03,0x24,0x24,0x8D,0x00,0x44,0x41,0x41,0x03,0x6C,0x8A,0xC1,0x02,0xC1,
0x59,0x26,0xD8,0x41,0x68,0x56,0x53,0x43,0x2A,0x14,0x04,0x28,0x0A,0x25,0x20,0x8A,
0x94,0x47,0x83,0x1A,0x85,0xA0,0x0F,0x84,0xD5,0x08,0x40,0x70,0x19,0x06,0x08,0x51,
0x80,0x5A,0x16,0xA8,0x08,0x55,0xD2,0x28,0x18,0x41,0x49,0x09,0x85,0x02,0x65,0x50,
0xB4,0x80,0x3D,0x80,0x81,0x2A,0x4A,0x86,0x80,0x30,0x01,0x03,0x86,0x1C,0x53,0x93,
0xA3,0x61,0x58,0x2A,0x54,0x21,0xB2,0x97,0xB0,0x86,0xAB,0x52,0x44,0xE9,0x88,0x59,
0x00,0x8B,0x20,0x10,0xD2,0x18,0x18,0x85,0x08,0x1C,0x31,0x50,0x03,0x64,0x8C,0x0A,
0x40,0x61,0xC0,0x0C,0xA8,0x08,0x40,0x32,0x65,0x0B,0x9C,0x88,0x02,0x42,0x4C,0x14,
0x60,0x34,0x85,0x13,0x21,0x50,0x71,0x48,0xA1,0xE5,0x14,0x6E,0x48,0x20,0x32,0x8A,
0x18,0x18,0x45,0x6A,0x32,0x20,0x82,0x71,0x8B,0xA3,0x62,0x16,0x92,0xBB,0x01,0x91,
0x20,0x76,0x11,0x21,0x34,0x54,0x4C,0x80,0x16,0xC6,0x38,0x10,0xE4,0x04,0x84,0x00,
0x41,0x01,0x9A,0x59,0x48,0x84,0x1A,0xA4,0x68,0x89,0x51,0x91,0x48,0x40,0x46,0xA9,
0x09,0x94,0x04,0x4D,0x2A,0x60,0xB9,0x18,0x08,0xC9,0xC6,0x40,0xA4,0x99,0x34,0x3C,
0x11,0x8D,0x10,0xE0,0x20,0xA4,0x75,0x0A,0xE6,0x44,0x93,0xB2,0x94,0x04,0x81,0xA0,
0x48,0x75,0x2A,0xA3,0x48,0x42,0x05,0x6E,0x96,0x0A,0x08,0x29,0x91,0x0C,0x14,0x05,
0xF0,0x22,0x84,0x01,0x21,0x04,0xA1,0x00,0x3A,0xD8,0x15,0x65,0x10,0x11,0xC1,0xA0,
0x98,0x8C,0x2A,0x16,0xC1,0x01,0x88,0xF5,0x12,0x42,0x40,0xA5,0x01,0x0B,0xA0,0xC2,
0x09,0x5C,0xC4,0x72,0x00,0x18,0x06,0xE4,0x0C,0x26,0x5A,0x3D,0x80,0x10,0x40,0x40,
0x40,0x11,0x23,0x30,0x9C,0x80,0x74,0x32,0x82,0x84,0x85,0x00,0xAF,0x20,0x95,0x20,
0x8F,0x51,0x8E,0xC3,0x26,0x84,0x62,0x03,0xC1,0x09,0xA8,0x34,0x34,0x83,0x22,0xCC,
0x90,0x23,0x1A,0x82,0x22,0x88,0x48,0x40,0x6C,0x32,0xC5,0x91,0xA3,0x35,0x89,0xA0,
0x24,0xA0,0x58,0x07,0x21,0x98,0x0E,0x0A,0xF2,0x6B,0xB0,0xAC,0x4A,0x6C,0x08,0x92,
0x29,0x18,0x40,0x42,0xC1,0x2E,0x04,0x91,0x30,0xD1,0x94,0xA3,0x42,0x43,0xD9,0x20,
0x59,0x98,0x2D,0x20,0x74,0x00,0x3D,0x8C,0x13,0x0A,0x46,0x62,0x00,0x05,0x34,0x59,
0x40,0x26,0x02,0x58,0x38,0xAD,0x94,0x2A,0x18,0x10,0x0A,0xA0,0x69,0x47,0xE3,0x18,
0xE2,0x70,0x8C,0x04,0x54,0x01,0x24,0x00,0x8A,0x10,0x29,0x06,0xAD,0x02,0x46,0x28,
0x0B,0xD0,0x50,0xC5,0x72,0x50,0xC1,0xA9,0x14,0x14,0x09,0x60,0xB0,0x52,0x12,0x60,
0x45,0x20,0x16,0x06,0xC3,0x01,0xA9,0x93,0xAE,0x04,0x82,0x1A,0x0B,0x58,0x0E,0x2C,
0x64,0x84,0x30,0x07,0x55,0x92,0x09,0x08,0x90,0xBA,0x91,0x25,0x02,0x47,0x66,0x56,
0x68,0x21,0x00,0x9C,0x06,0x60,0x20,0x88,0x34,0x89,0x9B,0x88,0x22,0xC2,0x52,0x92,
0x1D,0x14,0x80,0x40,0xD5,0x19,0x0B,0xB4,0xC4,0xE0,0x38,0x45,0x50,0x1B,0x44,0x88,
0x08,0x08,0x07,0xE2,0x0F,0x24,0x80,0x65,0x28,0x72,0x00,0x0A,0xE9,0xC1,0x08,0x48,
0xB0,0x42,0x0C,0x41,0x55,0x26,0x0E,0x21,0x1A,0x84,0xD1,0x10,0x42,0x18,0x07,0xA1,
0x13,0x4C,0xD0,0xC0,0x0C,0x44,0x2A,0x83,0x44,0x01,0x46,0x06,0x45,0xF0,0xBC,0x04,
0x8A,0xA6,0x0A,0x30,0x11,0xB4,0x61,0xC3,0x6B,0x06,0x07,0x23,0xB4,0x6C,0x19,0x49,
0x46,0xA0,0x18,0x28,0x60,0x84,0x8A,0x12,0xA5,0x20,0x04,0x31,0xDA,0x4C,0x60,0x21,
0x0A,0x17,0x14,0x02,0x04,0x32,0x90,0x92,0x83,0x89,0x4B,0x42,0x68,0x00,0x53,0x11,
0x80,0xD7,0x88,0x18,0x72,0xA1,0x04,0xB0,0x00,0x64,0x08,0x42,0xC1,0x2E,0x54,0x81,
0x85,0x68,0xA4,0x98,0x92,0x38,0x59,0x04,0x44,0x00,0xA3,0xA1,0x64,0x0F,0x22,0x3E,
0x00,0x90,0x02,0xDD,0x1C,0x8B,0x1C,0x60,0xD0,0x32,0x84,0x04,0x03,0x06,0x74,0x1A,
0x31,0x2D,0xC9,0x82,0x22,0xC3,0x10,0x82,0x30,0x8C,0x41,0x12,0x12,0x60,0x35,0x94,
0x1F,0x09,0x32,0xF1,0xA8,0x8C,0x08,0x90,0x6B,0x48,0x20,0x79,0x3D,0x54,0x86,0x04,
0x4A,0x71,0xC8,0x00,0xCC,0x02,0xE9,0x0C,0x24,0x21,0x0A,0x80,0x52,0xEE,0x00,0xA4,
0x32,0x96,0x1C,0x92,0x64,0x20,0x82,0x8A,0x88,0xA1,0x4B,0x0E,0x78,0x35,0x51,0x00,
0xA0,0x49,0x69,0x72,0x07,0x23,0x14,0xA0,0x45,0x0A,0x04,0xD0,0xD9,0x82,0xA1,0x07,
0xE5,0x08,0x03,0x20,0x3C,0x70,0xD4,0x0C,0x12,0xC0,0x49,0x20,0x08,0xD1,0x08,0x62,
0x63,0x11,0x02,0x10,0x98,0x4F,0x72,0x20,0x81,0x11,0xA8,0x53,0xAB,0x14,0x02,0x10,
0x8E,0x94,0x86,0x49,0x20,0x31,0x02,0x19,0x41,0x48,0x62,0x44,0xC5,0x80,0x18,0x14,
0xD0,0x83,0x00,0x57,0x88,0x25,0xAD,0x42,0x80,0x3A,0x30,0x90,0x15,0xD5,0x1A,0xE1,
0x4C,0x24,0x20,0x80,0xE1,0x08,0xC7,0x14,0x05,0x8A,0x11,0xB0,0x01,0x28,0x10,0x33,
0x21,0x93,0x04,0xCF,0x44,0x22,0xC0,0xC9,0x9A,0x00,0x4A,0x0C,0x2C,0x01,0xB1,0x8E,
0x10,0x58,0x21,0x14,0x96,0x63,0x34,0x44,0x43,0x04,0x44,0xB0,0x5B,0x80,0x28,0x0C,
0x00,0x24,0x33,0x80,0x0A,0x1C,0x0B,0x65,0x26,0xE0,0x2A,0x1E,0x09,0x9D,0xA5,0x48,
0x44,0x13,0x25,0x2C,0xC4,0x22,0x30,0x00,0x62,0xA4,0x48,0x46,0x40,0x50,0x27,0x00,
0xB8,0xA4,0x44,0x0C,0x26,0x44,0x98,0x89,0x81,0x03,0x0D,0x1C,0xC6,0x58,0x82,0x40,
0x18,0x4B,0x40,0x85,0x8B,0x3B,0x38,0xC8,0x2D,0x64,0x87,0x61,0x11,0x58,0x41,0x88,
0x16,0x14,0x89,0x19,0x9D,0x10,0xA0,0x08,0x22,0x80,0x02,0x44,0x86,0x81,0x04,0x33,
0x80,0x10,0x19,0xC0,0xC1,0x48,0x10,0x80,0x31,0x40,0x1F,0x01,0x24,0x03,0xE1,0x3F,
0x48,0x4B,0x8A,0x10,0x21,0xD8,0x06,0x95,0x19,0x6B,0x42,0x80,0x68,0x16,0x64,0x48,
0x22,0x40,0xD5,0x18,0xA9,0x10,0xC5,0x0B,0x1C,0x74,0x51,0xB8,0x25,0x00,0xC0,0x62,
0xA2,0xA9,0x10,0xB4,0x8A,0x60,0x2A,0x77,0xC1,0x8E,0x10,0x83,0x48,0x76,0x92,0x42,
0x25,0x08,0xC0,0x2C,0x1E,0x30,0x60,0x21,0x79,0x02,0x88,0x30,0x95,0x30,0x2A,0x84,
0x00,0x4A,0x4C,0x43,0x10,0x17,0x11,0x94,0xA2,0x08,0x80,0x31,0xB4,0x6D,0xC8,0x80,
0x6A,0x40,0x2B,0x04,0x70,0x1B,0x49,0x78,0xC0,0x88,0x04,0x5C,0x01,0x25,0x40,0x44,
0x41,0x18,0x05,0x4F,0x06,0x0A,0x20,0x61,0x37,0xD0,0x40,0x05,0x0A,0x26,0x48,0xB8,
0x88,0x5B,0x21,0x10,0x43,0xDA,0x9E,0x29,0x81,0xA3,0x18,0x00,0x33,0x09,0x98,0x93,
0xCD,0x52,0x42,0xA3,0x25,0x85,0x1E,0x28,0x14,0x35,0x50,0x01,0xD0,0xD0,0x4A,0x2C,
0xA1,0x22,0xA0,0x5D,0x1C,0xE2,0x02,0x54,0x90,0x02,0x88,0x5F,0x28,0x78,0x04,0x83,
0x02,0x40,0x8C,0x20,0x0C,0x82,0x11,0x04,0x71,0x4D,0x26,0x66,0x32,0x88,0x8A,0xA0,
0x07,0x8C,0x00,0xA3,0x40,0x09,0x31,0xC7,0x81,0x30,0xD4,0x43,0x0A,0x88,0x9D,0x49,
0x4A,0x81,0x01,0x8A,0x18,0x02,0x67,0x64,0x36,0x88,0x18,0x88,0x05,0xE8,0x14,0x40,
0x79,0x07,0x49,0x90,0xC9,0x2C,0xD6,0x02,0x93,0x80,0x48,0xA9,0x0C,0x67,0x10,0xA6,
0x24,0x51,0x66,0x04,0x00,0x5B,0x13,0xC8,0xCD,0x00,0x62,0x05,0x2B,0x32,0x31,0x19,
0xC8,0x20,0x02,0x90,0x92,0x21,0x4D,0xE0,0x64,0xC0,0x43,0x10,0x84,0x44,0xA7,0x1A,
0x12,0x18,0x34,0x01,0x08,0x07,0x0A,0xB4,0xAA,0x32,0x1C,0x16,0x40,0x06,0xC1,0xA8,
0x1E,0x21,0x40,0x41,0x12,0xB4,0x13,0x08,0x85,0x40,0x0C,0x02,0xF0,0x13,0x28,0x69,
0x80,0x45,0x70,0x75,0x70,0x80,0x41,0x02,0x21,0x00,0x33,0x88,0xBC,0x65,0x80,0x46,
0x54,0x82,0x03,0x13,0x04,0xC1,0x08,0x20,0x50,0x03,0x23,0x30,0x1E,0x82,0x32,0xA6,
0xC8,0x2A,0x91,0x48,0x4C,0x10,0x94,0xC1,0x20,0x9C,0x03,0x28,0x34,0x90,0x70,0xBA,
0xB1,0x4A,0xC4,0x58,0x44,0xE0,0xA8,0xBC,0x40,0x05,0x48,0x35,0x22,0x10,0x30,0x16,
0x27,0x24,0x20,0x12,0x38,0x64,0x07,0x46,0x08,0xA3,0xF0,0x09,0x14,0x8A,0xEA,0x20,
0x07,0xC3,0x00,0x85,0x89,0xC0,0x22,0xC0,0x20,0x8C,0x68,0x0A,0x02,0x26,0x43,0x48,
0x8E,0x4C,0x03,0xA2,0x48,0x75,0xB2,0x00,0x61,0x81,0x60,0x68,0x40,0x83,0x90,0xC5,
0x50,0x00,0x50,0x56,0x02,0xA3,0x14,0x44,0x24,0x3E,0xA4,0x49,0x3A,0x01,0xD0,0x0B,
0x48,0x63,0x8A,0xA4,0x20,0xD2,0x4C,0x1E,0x44,0x61,0x11,0x49,0x44,0x61,0x36,0x84,
0x12,0x38,0xE8,0x04,0xA6,0x50,0x92,0x69,0x89,0x35,0x1C,0xA1,0x04,0x75,0x62,0x2D,
0xC0,0x91,0x82,0x02,0x32,0x18,0x05,0x71,0xC8,0x66,0x50,0x06,0x58,0x3A,0x04,0x00,
0x2C,0x18,0x21,0x21,0x07,0x8C,0x41,0x42,0x14,0xC0,0xB0,0x2E,0x08,0x01,0x0B,0x10,
0x03,0x80,0x91,0x88,0x02,0xAE,0x0C,0x14,0x09,0x84,0x60,0x5B,0xC1,0x40,0x23,0x63,
0x08,0x80,0x10,0x0D,0x0E,0x82,0xC8,0x00,0xB9,0xDD,0x44,0x2A,0x30,0x6A,0x30,0x69,
0x10,0x6C,0x00,0x93,0x19,0x82,0xCC,0x10,0x80,0x2C,0x41,0xD2,0x85,0x18,0x88,0x24,
0x4E,0xD7,0x88,0xA9,0x0C,0x86,0xA1,0x04,0x43,0x2A,0x91,0x60,0x03,0x04,0x12,0x50,
0x50,0x20,0xB1,0xCC,0x02,0x60,0x87,0xAB,0x16,0x90,0x10,0xE4,0x72,0x51,0x00,0x2B,
0x60,0x4A,0x05,0x0A,0x35,0x00,0x22,0x40,0x89,0x8A,0x48,0x56,0xC2,0x0C,0x00,0xC2,
0x2C,0x40,0x34,0x3A,0x08,0x39,0x9B,0x85,0x5A,0x14,0x53,0x0D,0x10,0x01,0x0F,0x00,
0x81,0x93,0x2E,0x69,0x94,0x2E,0x1C,0x41,0xC0,0x17,0x18,0x91,0x8E,0x08,0x44,0x98,
0xA3,0x61,0x02,0x44,0x26,0x42,0xE1,0x80,0x88,0x16,0x02,0x00,0x64,0x0A,0x30,0x00,
0x08,0xE1,0x04,0x03,0xD1,0x38,0x14,0x19,0x01,0x72,0x52,0x51,0x18,0x05,0x85,0x4E,
0x06,0xA1,0x30,0x26,0x84,0x4E,0x4B,0x42,0x71,0x01,0x98,0x30,0x44,0x21,0x0E,0x70,
0x62,0x19,0x11,0x00,0x2B,0x10,0x30,0x22,0x25,0x6C,0x14,0xC0,0x5E,0xD2,0xB8,0x07,
0x45,0x90,0x48,0x50,0xC5,0xA3,0x81,0x85,0x40,0x22,0x4A,0x05,0x9A,0x16,0x48,0x8D,
0x05,0x32,0x16,0xE2,0xA2,0x21,0x4C,0x00,0x06,0x05,0x60,0xA2,0xBD,0x48,0xCB,0x50,
0x04,0x02,0x8A,0xE4,0x08,0xA0,0x00,0x41,0x00,0x19,0x65,0x05,0x8D,0x20,0x00,0x20,
0x87,0x30,0x0D,0x4A,0x7A,0xD3,0x28,0x07,0x04,0x84,0x61,0x14,0xD0,0x81,0x88,0xA1,
0xDE,0x86,0x22,0x80,0x69,0x00,0xA9,0xC3,0x00,0x06,0x33,0xC8,0x01,0x51,0x16,0x08,
0x20,0x66,0x10,0x9E,0xC4,0x00,0xC6,0x44,0xC6,0x02,0x02,0x0D,0xD1,0x25,0x74,0xD2,
0x41,0x28,0xE0,0x18,0x82,0x04,0x34,0x52,0xA5,0x48,0x84,0x4A,0x76,0xA1,0x0B,0xB6,
0x0C,0x50,0x20,0x20,0x04,0x03,0x88,0xDC,0x0C,0x41,0x0A,0x06,0x00,0x31,0x91,0x02,
0x89,0x10,0xE4,0x23,0x91,0x10,0x9F,0x45,0x6C,0x54,0xF9,0x9F,0x60,0x0A,0x80,0x02,
0x30,0x71,0x0D,0x10,0x42,0x82,0x46,0x30,0x12,0x04,0x0C,0x12,0x08,0x70,0xA1,0x71,
0xA2,0x50,0x00,0xE4,0x44,0xD2,0x28,0x09,0x0D,0xDA,0x40,0x40,0x16,0x78,0x26,0xA0,
0x86,0xA4,0x08,0x14,0x90,0x00,0xD1,0x0A,0x00,0x0A,0xC3,0x19,0xAC,0xA5,0x41,0xC0,
0x66,0xB1,0x50,0x22,0x28,0x09,0x04,0x0A,0xC6,0x01,0x96,0x20,0x5A,0x4C,0x08,0x45,
0xA2,0x20,0xAC,0x57,0x0C,0x20,0x24,0xA0,0x81,0x48,0x50,0x85,0x4E,0x14,0x8A,0x08,
0x50,0xD5,0x4F,0x46,0xE3,0x79,0x0D,0x0C,0x88,0x83,0x10,0x03,0xA0,0x0B,0x85,0x92,
0x05,0x60,0xE2,0x08,0x20,0x25,0x15,0x44,0x1A,0x45,0x11,0xBC,0x80,0x08,0x80,0x50,
0x34,0x22,0x15,0x80,0x8B,0x8A,0x10,0x41,0x79,0xA8,0x64,0x50,0x4E,0x04,0xA4,0x0A,
0x18,0xCC,0x88,0xA5,0x04,0xA2,0x41,0x8D,0x40,0x56,0x00,0x70,0xA3,0x41,0x03,0x30,
0x88,0x2C,0x2C,0x07,0xCB,0x92,0xC1,0x05,0xAE,0x14,0x82,0xB8,0x01,0x58,0x07,0x09,
0x10,0xC2,0x21,0x2C,0xF0,0x18,0x02,0x40,0x64,0xA2,0x29,0x58,0x41,0x80,0x28,0xC4,
0x88,0x09,0x64,0x87,0x82,0x16,0x90,0x21,0x09,0x09,0x5F,0x06,0x50,0x00,0xD0,0x92,
0xB8,0x83,0x81,0x68,0x01,0x5A,0x10,0x3C,0x05,0x60,0x20,0xB0,0x10,0x80,0x48,0x88,
0x2B,0x04,0x53,0x38,0xB3,0x41,0x92,0x4C,0x62,0xA6,0x21,0x05,0x2C,0x48,0x4D,0x14,
0x34,0x32,0x0B,0x01,0x1A,0xA9,0x48,0x02,0xCA,0x20,0x21,0x44,0x47,0x06,0x40,0x40,
0x81,0xD4,0x12,0x62,0x14,0x95,0x61,0xB1,0x98,0x00,0x6B,0x46,0xA3,0xB2,0x1E,0x10,
0xD6,0x21,0x1C,0x01,0x88,0x94,0x25,0x80,0x80,0x46,0x61,0xCB,0xB3,0x40,0x84,0x8B,
0x50,0x43,0xD1,0x34,0x31,0x1D,0x04,0x12,0x15,0x40,0x30,0x00,0x48,0xC2,0x20,0x95,
0x19,0x18,0xF5,0x8D,0x84,0x10,0x31,0x0A,0xBC,0x04,0xD9,0x20,0x1C,0x42,0xF0,0x01,
0x68,0x94,0x2E,0x0A,0x86,0xB0,0x01,0x4C,0x40,0x21,0x4C,0x91,0x80,0xAA,0x34
};


const int const primesieve_pack[30]={-1,0,-1,-1,-1,-1,-1,1,-1,-1,
              -1,2,-1,3,-1,-1,-1,4,-1,5,
              -1,-1,-1,6,-1,-1,-1,-1,-1,7
             };

const int const primesieve_nextpack[30]={0,1,1,1,1,1,1,2,2,
              2,2,3,3,4,4,4,4,5,5,
              6,6,6,6,7,7,7,7,7,7,8
             };

const int const primesieve_unpack[8]={1,7,11,13,17,19,23,29};



// RETURN THE SMALLEST "COULD_BE" PRIME NUMBER > n

BINT64 nextcbprimeBINT(BINT64 n)
{
    BINT idx;
    if(n<MAX_PRIME) {
    idx=primesieve_pack[n%30];
    BINT off=n/30;

    if(!off) {
        if(n<2) return 2;
        if(n<3) return 3;
        if(n<5) return 5;
    }
    if((idx>=0)&&(primesieve_data[off]&(1<<idx))) {
    // IF THE GIVEN NUMBER IS A PRIME ALREADY, SKIP TO THE NEXT
    ++idx;
    } else idx=primesieve_nextpack[n%30];
    off+=idx>>3;
    idx&=7;

    while(!(primesieve_data[off]&(1<<idx))) {
        ++idx;
        off+=idx>>3;
        idx&=7;
        if(off>=PRIMEDATA_SIZE) break;
    }
    if(off<PRIMEDATA_SIZE) return off*30+primesieve_unpack[idx];
    // OTHERWISE FALL THROUGH TO THE GENERIC CASE
    }

    // BASIC PRIMALITY TEST FOR 2,3 AND 5
    idx=primesieve_nextpack[n%30];
    n-=n%30;
    n+=30*(idx>>3);
    n+=primesieve_unpack[idx&7];

    return n;

}

// RETURN SMALLEST CONFIRMED PRIME NUMBER > n
// OR -1 IF NEXT PRIME >2^63 (IN SUCH CASE USER NEEDS TO USE REAL NUMBERS)

BINT64 nextprimeBINT(BINT64 n)
{
    BINT idx;
    if(n<MAX_PRIME) {
    idx=primesieve_pack[n%30];
    BINT off=n/30;

    if(!off) {
        if(n<2) return 2;
        if(n<3) return 3;
        if(n<5) return 5;
    }
    if((idx>=0)&&(primesieve_data[off]&(1<<idx))) {
    // IF THE GIVEN NUMBER IS A PRIME ALREADY, SKIP TO THE NEXT
    ++idx;
    } else idx=primesieve_nextpack[n%30];
    off+=idx>>3;
    idx&=7;

    while(!(primesieve_data[off]&(1<<idx))) {
        ++idx;
        off+=idx>>3;
        idx&=7;
        if(off>=PRIMEDATA_SIZE) break;
    }
    if(off<PRIMEDATA_SIZE) return off*30+primesieve_unpack[idx];
    // OTHERWISE FALL THROUGH TO THE GENERIC CASE
    }

    // FULL PRIMALITY TEST
    do {
    idx=primesieve_nextpack[n%30];
    n-=n%30;
    n+=30*(idx>>3);
    n+=primesieve_unpack[idx&7];

    if(n<0) return -1;  // OVERFLOW!

    } while(!isprimeBINT(n));

    return n;
}

BINT isprimeBINT(BINT64 n)
{
    if(n<0) n=-n;
    if(n<=1) return 0;
    if(n<MAX_PRIME) {
    int idx=primesieve_pack[n%30];
    int off=n/30;

    if(!off) {
        if(n==2) return 1;
        if(n==3) return 1;
        if(n==5) return 1;
    }
    if(idx>=0) return primesieve_data[off]&(1<<idx);
    return 0;
    }

    // GENERIC "SLOW" PRIMALITY TEST FOR LARGER NUMBERS

    if(primesieve_pack[n%30]<0) return 0;

    // NOW TEST FOR ALL PRIMES, 7 AND UP TO SQRT OF N
    BINT64 i=7;

    while(i*i<=n) {
        if(n%i == 0) return 0;
        i=nextcbprimeBINT(i);
    }

    return 1;
}


BINT isprimeReal(REAL *n)
{
    if(inBINT64Range(n)) {
        BINT64 nbint=getBINT64Real(n);
        return isprimeBINT(nbint);
    }

    //  BIG INTEGERS HERE

    // MAKE POSITIVE
    n->flags&=~F_NEGATIVE;
    newRealFromBINT(&RReg[2],30);

    divmodReal(&RReg[0],&RReg[1],n,&RReg[2]);

    BINT rem=getBINTReal(&RReg[1]); // EXTRACT THE INTEGER REMAINDER

    if(primesieve_pack[rem]<0) return 0;

    // NOW TEST FOR ALL PRIMES, 7 AND UP TO SQRT OF N
    BINT64 i=7;

    while(i<3037000500LL) {
        newRealFromBINT64(&RReg[2],i);
        divmodReal(&RReg[0],&RReg[1],n,&RReg[2]);
        if(iszeroReal(&RReg[1])) return 0;

        i=nextcbprimeBINT(i);
    }

    // HERE WE TESTED ALL PRIMES UP TO 2^63 AND IT'S STILL PRIME!

    // TODO: MORE TESTING (FERMAT?)



    return 1;


}

// RETURN THE NEXT PRIME NUMBER IN RReg[regnum]
void nextprimeReal(BINT regnum,REAL *n)
{
    // MAKE POSITIVE
    n->flags&=~F_NEGATIVE;

    if(inBINT64Range(n)) {
        BINT64 nbint=getBINT64Real(n);
        BINT64 next=nextprimeBINT(nbint);
        if(next>0) {
            newRealFromBINT64(&RReg[regnum],next);
            return;
        }
        // TESTED ALL INTEGERS UP TO 2^63, CONTINUE WITH LARGER ONES???
        RReg[0].exp=0;
        RReg[0].flags=0;
        RReg[0].len=3;
        RReg[0].data[0]=54775809;
        RReg[0].data[1]=33720368;
        RReg[0].data[2]=922;

    }
    else copyReal(&RReg[0],n);

    newRealFromBINT(&RReg[2],30);

    BINT64 i;

    do {

    divmodReal(&RReg[3],&RReg[1],&RReg[0],&RReg[2]);

    BINT rem=getBINTReal(&RReg[1]); // EXTRACT THE INTEGER REMAINDER

    if(primesieve_pack[rem]<0) {
        // NOT PRIME, NEXT
        BINT idx=primesieve_nextpack[rem];
        // THIS IS LOW-LEVEL ACCESS TO DECIMAL LIBRARY FOR SPEED
        RReg[0].data[0]-=rem;
        RReg[0].data[0]+=30*(idx>>3);
        RReg[0].data[0]+=primesieve_unpack[idx&7];
        normalize(&RReg[0]);
        continue;
    }

    // NOW TEST FOR ALL PRIMES, 7 AND UP TO SQRT OF N
    i=7;

    while(i<3037000500LL) {
        newRealFromBINT64(&RReg[4],i);
        divmodReal(&RReg[3],&RReg[1],&RReg[0],&RReg[4]);
        if(iszeroReal(&RReg[1])) {
            // NOT PRIME, NEXT
            rem=getBINTReal(&RReg[1]); // EXTRACT THE INTEGER REMAINDER
            BINT idx=primesieve_nextpack[rem];
            // THIS IS LOW-LEVEL ACCESS TO DECIMAL LIBRARY FOR SPEED
            RReg[0].data[0]-=rem;
            RReg[0].data[0]+=30*(idx>>3);
            RReg[0].data[0]+=primesieve_unpack[idx&7];
            normalize(&RReg[0]);
            break;
        }

        i=nextcbprimeBINT(i);
    }

    // HERE WE TESTED ALL PRIMES UP TO 2^63 AND IT'S STILL PRIME!

    // TODO: MORE TESTING (FERMAT?)
    } while(i<3037000500LL);

    if(regnum!=0) swapReal(&RReg[regnum],&RReg[0]);
}



// COMPUTE a^b MOD mod
// WHERE a AND b, AND mod ARE INTEGERS
// mod HAS TO BE < 2^31 OTHERWISE USE THE REAL VERSION

BINT64 powmodBINT(BINT64 a, BINT64 b, BINT64 mod)
{
    if(b==0) return 1;  // SPECIAL CASE: 0^0 IS NOT CONSIDERED HERE, USER MUST CHECK ARGUMENTS AND RAISE ERROR
    if(b==1) return a%mod;
    if(b==-1) {
        if(a>1) return 0;
        if(a<-1) return 0;
        return a;
    }
    BINT64 result=1;

    a=a%mod;
    while(b) {
        if(b&1) result=(result*a)%mod;
        b>>=1;
        a=(a*a)%mod;
    }


    return result;


}

// COMPUTE a^b MOD mod
// WHERE a AND b, AND mod ARE INTEGERS
// SYSTEM PRECISION MUST REPRESENT AT LEAST mod^2 EXACTLY

// NOTE: result  OR ARGUMENTS CANNOT BE RReg[0],[1],[2],[3],[4] or [5]

void powmodReal(REAL *result,REAL *a,REAL *b,REAL *mod)
{
    if(iszeroReal(b)) return newRealFromBINT(result,1);  // SPECIAL CASE: 0^0 IS NOT CONSIDERED HERE, USER MUST CHECK ARGUMENTS AND RAISE ERROR


    // STORE NUMBER 1
    RReg[0].exp=0;
    RReg[0].len=1;
    RReg[0].data[0]=1;
    RReg[0].flags=0;

    if(eqReal(b,&RReg[0])) {
        divmodReal(&RReg[0],result,a,mod);
        return;
    }
    RReg[0].flags^=F_NEGATIVE;
    if(eqReal(b,&RReg[0])) {    // b==-1

        if(ltReal(a,&RReg[0])) {    // a<-1
            // RETURN ZERO
            result->flags=0;
            result->data[0]=0;
            result->exp=0;
            result->len=1;
            return;
        }
        RReg[0].flags^=F_NEGATIVE;
        if(gtReal(a,&RReg[0])) {    // a>1
            // RETURN ZERO
            result->flags=0;
            result->data[0]=0;
            result->exp=0;
            result->len=1;
            return;
        }
        copyReal(result,a);
        return;
    }

    // STORE NUMBER 1
    result->exp=0;
    result->len=1;
    result->data[0]=1;
    result->flags=0;

    // STORE NUMBER -1
    RReg[1].exp=0;
    RReg[1].len=1;
    RReg[1].data[0]=1;
    RReg[1].flags=F_NEGATIVE;

    // STORE NUMBER 0.5
    RReg[2].exp=-1;
    RReg[2].len=1;
    RReg[2].data[0]=5;
    RReg[2].flags=0;

    divmodReal(&RReg[3],&RReg[4],a,mod);
    copyReal(&RReg[5],b);

    while(!iszeroReal(&RReg[5])) {
        if(isoddReal(&RReg[5])) {
            mulReal(&RReg[0],result,&RReg[4]);
            divmodReal(&RReg[3],result,&RReg[0],mod);
            addReal(&RReg[5],&RReg[5],&RReg[1]);
        }
        // b IS GUARANTEED TO BE EVEN
        mulReal(&RReg[5],&RReg[5],&RReg[2]);

        mulReal(&RReg[0],&RReg[4],&RReg[4]);
        divmodReal(&RReg[3],&RReg[4],&RReg[0],mod);
    }

    // RESULT IS IN result
}
