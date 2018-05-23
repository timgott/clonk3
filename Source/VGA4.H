/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design VGA4_ASM  Header by M.Bender
// Version 1.0 January 1995 1.1 March 1995 1.2 June 1996

// Includes references to assembler/C VGA functions
// in modules VGA4_ASM.OBJ and VGA4.C

//------------------------------ VGA4 Functions ------------------------------

extern int MaxX,MaxY;
extern BYTE *VGARam;

extern "C" int IsVGA(void);
extern "C" void InitVGA(void);
extern "C" void CloseVGA(void);
extern "C" void SPixA(int x, int y, BYTE col);
extern "C" BYTE GPixA(int x, int y);
extern "C" void LPage(BYTE page);
extern "C" void PPage(BYTE page);
extern "C" void BMove(BYTE fpge,int fx,int fy,BYTE tpge,int tx,int ty,BYTE wdt,BYTE hgt,WORD bmskps,WORD bmskpo);
extern "C" void B4Move(BYTE fpge,int fx,int fy,BYTE tpge,int tx,int ty,BYTE wdt,BYTE hgt);
extern "C" void CPage(BYTE fpge, BYTE tpge);
extern "C" void CPageTM(BYTE fpge, WORD memseg, WORD memoff);
extern "C" void CPageY(BYTE fpge, BYTE fy, BYTE tpge, BYTE ty, BYTE hgt);

//------------------------------- DAC ----------------------------------------

typedef union { struct { BYTE red, green, blue; } b; BYTE RGB[3]; } COLOR;

extern "C" void GetDAC(int first, int num, void far *bufptr);
extern "C" void SetDAC(int first, int num, void far *bufptr);
extern "C" void SetColor(COLOR *pal, BYTE col, BYTE red, BYTE green, BYTE blue);
extern "C" void FadeColor(COLOR *pal, BYTE fcol, BYTE tcol);
extern "C" void InitFadeDAC(COLOR *tpal, int steps);
extern "C" BYTE FadeDAC(void);

