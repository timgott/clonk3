/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

//----------------------------- BSEXCSYS Externals -----------------------------

extern int Rnd3(void);
extern BYTE Rnd4(void);
extern BYTE Tick2,Tick3,Tick5,Tick10,Tick20,Tick50;
extern BYTE Sec1,Sec5,Sec10,Sec20,Sec60;

extern BYTE LowMem;

extern DWRD RockCnt,PXSCnt,StructCnt,VehicCnt,LineCnt,ManCnt[3],AnimalCnt;

extern long BackGrSize;

extern int GSTP;

extern void RoundError(char *txt);
