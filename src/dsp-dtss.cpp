/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design DSP-Direct Timer Sound System

// Version 1.0 April 1995
//         1.1 May   1996

// Original Source: Arthur Burda/DATA BECKER

//-------------------------- Include Headers ---------------------------------

#include "standard.h"

//----------------------- DSP Command Definitions ----------------------------

#define DSPWriteDirectUnpacked 0x10
#define DSPSpeakerOn           0xD1
#define DSPSpeakerOff          0xD3

//--------------------------- Type Definitions -------------------------------

typedef struct SAMPLE { BYTE *buf; WORD len, freq; };

//--------------------------- Global Variables -------------------------------

int SBBaseA;

//-------------------- DTSTimerInt Global Variables --------------------------

static BYTE *DTSSndPtr = 0;
static WORD DTSSndCnt = 0;
static BYTE DTSRunning = 0;
static WORD DTSSndSize = 0;

static WORD DTSOldTCnt = 0;
static WORD DTSOldTCall = 0;

//static void interrupt (*TimerOld)(...);
static BYTE TimerOldSet = 1;

//-------------------------- DSP Functions -----------------------------------

void SetTimerFreq(WORD freq) // and DTSTimerInt-CallOld-Freq
{
	//WORD nfreq;
	//if (freq < 19) freq = 19; // Check range
	//if (freq > 50000) freq = 50000;
	//outportb(0x43, 0x36);                                    /* Steuerbefehl */
	//nfreq = 1193180 / freq;      /* internen Wert fr Timer-Frequenz berechnen */
	//outportb(0x40, nfreq & 0xFF);      /* Low-BYTE fr den Timer-Wert setzen */
	//outportb(0x40, (nfreq >> 8) & 0xFF);                 /* High-BYTE setzen */

	//DTSOldTCall = 65536 / nfreq; // OldT-Call-Freq
}

void SetOrigFreq()
{
	//WORD nfreq;                          /* interner Wert fr Timer-Frequenz */
	//outportb(0x43, 0x36);                                     /* Steuerbefehl */
	//nfreq = 0xFFFF;               /* Timer-Frequenz auf Originalwert (18,2 Hz) */
	//outportb(0x40, nfreq & 0xFF);          /* Low-BYTE fr die Timer-Frequenz */
	//outportb(0x40, (nfreq >> 8) & 0xFF);  /* High-BYTE fr die Timer-Frequenz */
}

int ReadDSP()
{
	//WORD chkcnt;                                                /* ein Z„hler */
	//int  Reading;                 /* TRUE, wenn der DSP zum Lesen bereit ist */

	//chkcnt = 0;
	//Reading = 0;
	//while (!Reading)            /* wiederholen, bis DSP zum Lesen bereit ist */
	//{
	//	chkcnt++;                                           /* Z„hler erh”hen */
	//	Reading = ((inportb(SBBaseA + 0x0E) & 0x80) == 0x80);
	//	if (chkcnt == 10000) // took too long?
	//		return -1; // Error: Cannot read from DSP / maybe missing sound card
	//}
	//return inportb(SBBaseA + 0x0A);
	return -1;
}

BYTE WriteDSP(BYTE val)
{
	/*WORD chkcnt = 0;
	int  writeok = 0;
	while (!writeok)
	{
		chkcnt++;
		writeok = ((inportb(SBBaseA + 0x0C) & 0x80) == 0x00);
		if (chkcnt == 10000) return 0; // Error: Cannot write to DSP (took too long)
	}
	outportb(SBBaseA + 0x0C, val); // write to DSP
	return 1;
	*/
	return 0;
}

inline void WriteDirect(BYTE Value)
{
	WriteDSP(DSPWriteDirectUnpacked); // No WriteDSP error checking!
	WriteDSP(Value);
}

BYTE SpeakerOn()
{
	return WriteDSP(DSPSpeakerOn);
}

BYTE SpeakerOff()
{
	return WriteDSP(DSPSpeakerOff);
}

BYTE InitDSP()
{
	//WORD chkcnt;                                                /* ein Z„hler */
	//int  Reading;       /* TRUE, wenn der BYTEwert AAh gelesen werden konnte */

	//chkcnt = 0;
	//Reading = 0;
	//outportb(SBBaseA + 0x06, 0x01);    /* Port 2x6h mit 01h beschreiben */
	//delay(1);                                    /* eine Millisekunde warten */
	//outportb(SBBaseA + 0x06, 0x00);    /* Port 2x6h mit 00h beschreiben */
	//while (Reading != 0xAA)
	//{
	//	chkcnt++;                                           /* Z„hler erh”hen */
	//	Reading = ReadDSP();
	//	if (Reading == -1) return 0;
	//	if (chkcnt == 1000) return 0; // Took too long -> error
	//}           	 // Error: Reset error / incorrect Base-I/O-Address
	//return 1;
	return 0;
}

//------------------- DTSTimerInt Interrupt Function -------------------------

/*void DTSTimerInt(...)
{
	WriteDirect(*DTSSndPtr);                 // Output sound byte
	if (DTSSndCnt == DTSSndSize) DTSRunning = 0; // End if done
	else { DTSSndCnt++; DTSSndPtr++; }       // Else advance
	DTSOldTCnt++;
	if (DTSOldTCnt == DTSOldTCall)
	{
		DTSOldTCnt = 0; TimerOld();
	} // Call OldT
	outportb(0x20, 0x20); 			   // End interrupt routine
}*/

//-------------------------- Public Functions -------------------------------

/*BYTE PlaySound(SAMPLE *smp) // For RAW SAMPLES
  {
  if (!smp->buf || !smp->len) return 0;
  if (TimerOldSet)
	{
	TimerOld=getvect(0x08);      // Save old timer vector
	TimerOldSet=0;
	setvect(0x08,DTSTimerInt);   // Set timer vector to DTSTimerInt
	DTSOldTCnt=0;
	}
  SetTimerFreq(smp->freq);
  DTSSndPtr=smp->buf;
  DTSSndCnt=0;
  DTSRunning=1;
  DTSSndSize=smp->len;
  return 1;
  }*/

BYTE DSPPlaySound(BYTE *vocbuf) // For VOC Files in memory
{
	//WORD smplen, freq;

	//if (!vocbuf) return 0;

	//vocbuf += *((WORD*)(vocbuf + 20)); // Header offset -> 1st Block

	//if (*vocbuf != 1) return 0; // Not a new sample data block

	//smplen = vocbuf[1] + 256 * vocbuf[2] + 65536 * vocbuf[3] - 6; // get block length-head
	//freq = (long)1000000 / (long)(256 - vocbuf[4]);       // get sample rate

	//if (vocbuf[5] != 0) return 0; // Sample is packed!

	//if (TimerOldSet)
	//{
	//	TimerOld = getvect(0x08);      // Save old timer vector
	//	TimerOldSet = 0;
	//	setvect(0x08, DTSTimerInt);   // Set timer vector to DTSTimerInt
	//	DTSOldTCnt = 0;
	//}
	//SetTimerFreq(freq);
	//DTSSndPtr = vocbuf + 6; // jump block type 1 header
	//DTSSndCnt = 0;
	//DTSRunning = 1;
	//DTSSndSize = smplen;
	//return 1;
	return 0;
}

BYTE DSPSoundCheck(void)
{
	//if (!DTSRunning)
	//{
	//	if (!TimerOldSet)
	//	{
	//		setvect(0x08, TimerOld); TimerOldSet = 1;
	//		SetOrigFreq();
	//	}
	//	return 0;
	//}
	//return 1; // 1:Sound running 0:Sound off,timer reset
	return 0;
}

BYTE InitDSPSound(unsigned *basea)
{
	SBBaseA = *basea;
	if (!InitDSP())
	{
		SBBaseA = 0x210; // Pref-Base incorrect, try all
		while (!InitDSP() && SBBaseA < 0x260) SBBaseA += 0x10;
		if (SBBaseA == 0x270) return 0;
	}
	if (!SpeakerOn()) return 0;
	*basea = SBBaseA;
	return 1;
}

void DeInitDSPSound(void)
{
	SpeakerOff();
}

//--------------------------- Sample loader ----------------------------------

/*BYTE LoadSample(char *fname, SAMPLE *smp) // For RAW SAMPLES
  {
  int fhnd,flen;
  smp->buf=0;
  if (!DOSOpen(fname,&fhnd)) return 1;
  flen=filelength(fhnd);
  if (flen==-1) { DOSClose(fhnd); return 2; }
  smp->len=flen;
  if (!(smp->buf=new BYTE [smp->len])) { DOSClose(fhnd); smp->buf=0; return 3; }
  smp->freq=10000;
  if (!DOSRead(fhnd,smp->buf,smp->len)) { DOSClose(fhnd); delete [] smp->buf; smp->buf=0; return 2; }
  DOSClose(fhnd);
  return 0;
  } // 0:NoError 1:NotFound 2:FileError 3:InsufMem

void DestroySample(SAMPLE *smp)
  {
  if (smp->buf) delete [] smp->buf;
  smp->buf=0; smp->len=0;
  }*/
