/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design STD_GFX Header by M.Bender
// Version and date see library.

//------------------------ Graphic Functions --------------------------------

extern void Viewport(int vpx1, int vpy1, int vpx2, int vpy2);
extern void NoViewport(void);
extern void SPixF(int pxx, int pxy, BYTE pxc);
extern BYTE GPixF(int pxx, int pxy);
extern void DLine(int x1, int y1, int x2, int y2, BYTE color);
extern void DFrame(int x1,int y1,int x2,int y2,BYTE color);
extern void DBox(int x1,int y1,int x2,int y2,BYTE color);
extern void DFill(int fx, int fy, BYTE col);
extern void ClPage(void);
extern BYTE COut(BYTE chr, int tx, int ty, BYTE fcol, int bcol);
extern int  CSLen(BYTE *sptr);
extern void SOut(BYTE *sptr, int tx, int ty, BYTE fcol, int bcol=-1, BYTE form=0);
extern void SOutS(BYTE *sptr, int tx, int ty, BYTE fcol, BYTE bcol, BYTE form=0);
extern void TOut(BYTE *tptr, int tx, int ty, BYTE fcol, int bcol=-1, BYTE form=0);
extern void TOutS(BYTE *tptr, int tx, int ty, BYTE fcol, int bcol, BYTE form=0);
extern void TOutSt(BYTE *tptr, int *mwdt, int *mhgt);
extern void FTOut(BYTE *tptr, int tx, int ty, int wdt, int maxhgt, int startl, BYTE fcol, int bcol, BYTE form);
extern void FTOutSt(BYTE *tptr, int wdt, int startl, int *loutp);
extern void FTOutGetHL(BYTE *tptr, int wdt, int maxhgt, int startl, int ax, int ay, BYTE *gethl);

