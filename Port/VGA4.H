/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design VGA4_ASM  Header by M.Bender
// Version 1.0 January 1995 1.1 March 1995 1.2 June 1996

// Includes references to assembler/C VGA functions T:... assembler? D: nope nope nope nope nope nope
// in modules VGA4_ASM.OBJ and VGA4.C T:... nopedy-nope nope nope nope nope nope

// T: Emulates VGA with SDL in C(++)! 8:]

typedef unsigned char BYTE;

//------------------------------ VGA4 Functions ------------------------------

extern int MaxX, MaxY;
extern BYTE *VGARam;

extern int IsVGA(void);
extern int InitVGA(void);
extern void CloseVGA(void);
extern void SPixA(int x, int y, BYTE col);
extern BYTE GPixA(int x, int y);
extern void LPage(BYTE page);
extern void PPage(BYTE page);
extern void UpdateScreen(); // Neue Funktion
//extern void BMove(BYTE fpge,int fx,int fy,BYTE tpge,int tx,int ty,BYTE wdt,BYTE hgt,WORD bmskps,WORD bmskpo);
extern void BMove(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE* bmskp); // keine Segmente und Offsets
extern void BMoveSwapColor(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, BYTE wdt, BYTE hgt, BYTE oldcol, BYTE newcol); // Neue Funktion
extern void B4Move(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt);
extern void CPage(BYTE fpge, BYTE tpge);
//extern void CPageTM(BYTE fpge, WORD memseg, WORD memoff);
extern void CPageY(BYTE fpge, BYTE fy, BYTE tpge, BYTE ty, BYTE hgt);

//------------------------------- DAC ----------------------------------------

typedef union { struct { BYTE red, green, blue; } b; BYTE RGB[3]; } COLOR;

extern void GetDAC(int first, int num, void *bufptr);
extern void SetDAC(int first, int num, void *bufptr);
extern void SetColor(COLOR *pal, BYTE col, BYTE red, BYTE green, BYTE blue);
extern void FadeColor(COLOR *pal, BYTE fcol, BYTE tcol);
extern void InitFadeDAC(COLOR *tpal, int steps);
extern BYTE FadeDAC(void);