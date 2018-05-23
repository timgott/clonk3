/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RW-D DSP-DTSS Header

typedef struct SAMPLE { BYTE *buf; WORD len, freq; };

//extern BYTE LoadSample(char *fname, SAMPLE *smp);
//extern void DestroySample(SAMPLE *smp);
//extern BYTE PlaySound(SAMPLE *smp);

extern BYTE DSPPlaySound(BYTE *vocbuf);
extern BYTE DSPSoundCheck(void);
extern BYTE InitDSPSound(unsigned *basea);
extern void DeInitDSPSound(void);
