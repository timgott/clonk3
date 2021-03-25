/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design STANDARD Header by M.Bender
// Version and date see Library.

#pragma once

#include <stdint.h>

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

extern BYTE Inside(long ival, long lbound, long rbound);
extern long BoundBy(long bval, long lbound, long rbound);
extern void Swap(int &int1, int &int2);
extern long Max(long val1, long val2);
extern long Min(long val1, long val2);
extern long Max3(long val1, long val2, long val3);
extern long Min3(long val1, long val2, long val3);
extern long Max4(long val1, long val2, long val3, long val4);
extern long Min4(long val1, long val2, long val3, long val4);
extern long Abs(long val);
extern int  Sign(int val);
extern void Toggle(BYTE &tbyte);
//extern void SetBit(BYTE &tbyte, BYTE bit);
//extern void UnSetBit(BYTE &tbyte, BYTE bit);
//extern BYTE GetBit(BYTE tbyte, BYTE bit);
//extern void ToggleBit(BYTE &tbyte, BYTE bit);
//extern void SetBBit(BYTE *buf, WORD bitn);
//extern BYTE GetBBit(BYTE *buf, WORD bitn);
//extern void ToggleBBit(BYTE *buf, WORD bitn);
extern void FillMem(BYTE *bptr, DWORD cnt, BYTE val);
extern void ZeroMem(BYTE *bptr, DWORD cnt);
extern BYTE MemEqual(BYTE *ptr1, BYTE *ptr2, DWORD cnt);
extern void MemCopy(const BYTE *fptr, BYTE *tptr, DWORD cnt);
extern float PathC(int fc, int tc, float pc);

extern int  SLen(const char *sptr);
extern void SCopy(const char *src, char *trg, int maxn = 500);
extern BYTE SEqual(const char *str1, const char *str2);
extern BYTE SEqualL(const char *str1, const char *str2);
extern void Capitalize(char *str);
extern int SCharPos(char tchar, const char *istr);

//extern BYTE RTCRead(BYTE address);
//extern void RTCWrite(BYTE address, BYTE contents);
//extern BYTE RTCTime(BYTE address);