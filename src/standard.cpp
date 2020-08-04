/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  STANDARD Library  Matthes Bender

// Version: 1.0 ?
//          1.1 October  1994
//          1.2 November 1994
//          1.3 December 1994
//          1.4 January  1995 (STDVGA split up)
//          1.5 February 1995
//	        1.6 May      1995
//          1.7 March    1996 (BFI added)
//          1.8 May      1996 (File functions removed)

//------------------------------ Headers -------------------------------------

#include "standard.h"

//----------------------- Basic Type Definitions -----------------------------

typedef unsigned char     BYTE;
typedef unsigned int      WORD;
typedef unsigned long int DWORD;

//------------------------- Standard Functions -------------------------------

BYTE Inside(long ival, long lbound, long rbound)
{
	if ((ival >= lbound) && (ival <= rbound)) return 1;
	return 0;
}

long BoundBy(long bval, long lbound, long rbound)
{
	if (Inside(bval, lbound, rbound)) return bval;
	if (bval < lbound) return lbound;
	return rbound;
}

void Swap(int &int1, int &int2)
{
	int temp;
	temp = int2; int2 = int1; int1 = temp;
}

long Max(long val1, long val2)
{
	if (val1 > val2) return val1;
	return val2;
}

long Min(long val1, long val2)
{
	if (val1 < val2) return val1;
	return val2;
}

long Max3(long val1, long val2, long val3)
{
	return Max(Max(val1, val2), val3);
}

long Min3(long val1, long val2, long val3)
{
	return Min(Min(val1, val2), val3);
}

long Abs(long val)
{
	if (val < 0) return -val;
	return val;
}

int Sign(int val)
{
	if (val < 0) return -1;
	if (val > 0) return +1;
	return 0;
}

void Toggle(BYTE &tbyte)
{
	if (tbyte) tbyte = 0; else tbyte = 1;
}

/*void SetBit(BYTE &tbyte, BYTE bit)
  {
  BYTE sbyte=1;
  sbyte<<=bit;
  tbyte|=sbyte;
  }

void UnSetBit(BYTE &tbyte, BYTE bit)
  {
  BYTE sbyte=1;
  sbyte<<=bit;
  sbyte^=255;
  tbyte&=sbyte;
  }

BYTE GetBit(BYTE tbyte, BYTE bit)
  {
  tbyte>>=bit;
  return tbyte & 1;
  }

void ToggleBit(BYTE &tbyte, BYTE bit)
  {
  if (GetBit(tbyte,bit)) UnSetBit(tbyte,bit); else SetBit(tbyte,bit);
  }

void SetBBit(BYTE *buf, WORD bitn)
  {
  SetBit(buf[bitn/8],bitn%8);
  }

BYTE GetBBit(BYTE *buf, WORD bitn)
  {
  return GetBit(buf[bitn/8],bitn%8);
  }

void ToggleBBit(BYTE *buf, WORD bitn)
  {
  ToggleBit(buf[bitn/8],bitn%8);
  }*/

void FillMem(BYTE *bptr, DWORD cnt, BYTE val)
{
	while (cnt > 0) { *bptr = val; cnt--; bptr++; }
}

void ZeroMem(BYTE *bptr, DWORD cnt)
{
	while (cnt > 0) { *bptr = 0; cnt--; bptr++; }
}

BYTE MemEqual(BYTE *ptr1, BYTE *ptr2, DWORD cnt)
{
	while (cnt > 0) { if (*ptr1 != *ptr2) return 0; cnt--; ptr1++; ptr2++; }
	return 1;
}

void MemCopy(BYTE *fptr, BYTE *tptr, DWORD cnt)
{
	if (tptr < fptr) // Downwards in mem
		while (cnt > 0) { *tptr = *fptr; tptr++; fptr++; cnt--; }
	else // Upwards in mem
	{
		tptr += cnt - 1; fptr += cnt - 1;
		while (cnt > 0) { *tptr = *fptr; tptr--; fptr--; cnt--; }
	}
}

float PathC(int fc, int tc, float pc)
{
	return fc + (tc - fc)*pc;
}

//---------------------------- String Functions ------------------------------

const int MaxSLen = 500;

int SLen(char *sptr)
{
	int slen = 0;
	if (!sptr) return 0;
	while ((slen < MaxSLen) && *(sptr + slen)) slen++;
	return slen;
}

void SCopy(char *src, char *trg, int maxn)
{
	int cnt;
	for (cnt = 0; *src && (cnt < maxn); cnt++, src++, trg++) *trg = *src;
	*trg = 0;
}

BYTE SEqual(char *str1, char *str2)
{
	int cnt;
	for (cnt = 0; *str1 && *str2 && (cnt < MaxSLen); cnt++, str1++, str2++)
		if (*str1 != *str2) return 0;
	return 1;
}

BYTE SEqualL(char *str1, char *str2)
{
	if (SEqual(str1, str2) && (SLen(str1) == SLen(str2))) return 1;
	return 0;
}

int SCharPos(char tchar, char *istr)
{
	int cpos;
	if (!istr) return -1;
	for (cpos = 0; istr[cpos] && (istr[cpos] != tchar) && (cpos < 512); cpos++);
	if (istr[cpos] != tchar) cpos = -1;
	return cpos;
}

void Capitalize(char *str)
{
	while (str && *str)
	{
		if (Inside(*str, 97, 122)) *str -= 32;
		if (*str == '„') *str = 'Ž';
		if (*str == '”') *str = '™';
		if (*str == '') *str = 'š';
		str++;
	}
}

//---------------------------------- RTC -------------------------------------
// Real-time-clock. Wird durch time(NULL) ersetzt.
/*
BYTE RTCRead(BYTE address)
  {
  outp(0x70,address);
  return(inp(0x71));
  }

void RTCWrite(BYTE address, BYTE contents)
  {
  outp(0x70,address);
  outp(0x71,contents);
  }

BYTE RTCTime(BYTE address)
  {
  if (RTCRead(0x0B) & 2)
	return((RTCRead(address) >> 4)*10+(RTCRead(address) & 15));
  else return(RTCRead(address));
  }*/
