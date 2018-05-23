/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  VGA4 Module  by M.Bender

#include <dos.h>

typedef unsigned char BYTE;

int MaxX=319,MaxY=199;

int IsVGA(void)
  {
  union REGS Regs;
  Regs.x.ax=0x1a00;
  int86(0x10, &Regs, &Regs);
  if (Regs.h.al==0x1a) return 1;
  return 0;
  }

void CloseVGA(void)
  {
  union REGS Regs;
  Regs.x.ax=0x0003;
  int86(0x10,&Regs,&Regs);
  }

extern void B4Move(BYTE fpge,int fx,int fy,BYTE tpge,int tx,int ty,BYTE wdt,BYTE hgt);

void CPageY(BYTE fpge, BYTE fy, BYTE tpge, BYTE ty, BYTE hgt)
  {
  B4Move(fpge,0,fy,tpge,0,ty,40,hgt);
  B4Move(fpge,40,fy,tpge,40,ty,40,hgt);
  }

//--------------------------------- DAC --------------------------------------

typedef union { struct { BYTE red, green, blue; } b; BYTE RGB[3]; } COLOR;

void GetDAC(int first, int num, void far *bufptr)
  {
  union REGS Regs;
  struct SREGS SRegs;
  Regs.x.ax=0x1017;
  Regs.x.bx=first;
  Regs.x.cx=num;
  Regs.x.dx=FP_OFF(bufptr);
  SRegs.es=FP_SEG(bufptr);
  int86x(0x10,&Regs,&Regs,&SRegs);
  }

void SetDAC(int first, int num, void far *bufptr)
  {
  union REGS Regs;
  struct SREGS SRegs;
  Regs.x.ax=0x1012;
  Regs.x.bx=first;
  Regs.x.cx=num;
  Regs.x.dx=FP_OFF(bufptr);
  SRegs.es=FP_SEG(bufptr);
  int86x(0x10,&Regs,&Regs,&SRegs);
  }

void SetColor(COLOR *pal, BYTE col, BYTE red, BYTE green, BYTE blue)
  {
  pal[col].b.red=red; pal[col].b.green=green; pal[col].b.blue=blue;
  }

void FadeColor(COLOR *pal, BYTE fcol, BYTE tcol)
  {
  long cnt,ccnt,flen;
  long cdis[3];

  if (!pal || (fcol>=tcol)) return;

  flen=tcol-fcol;
  for (ccnt=0; ccnt<3; ccnt++)
    cdis[ccnt]=pal[tcol].RGB[ccnt]-pal[fcol].RGB[ccnt];

  for (cnt=0; cnt<=flen; cnt++)
    for (ccnt=0; ccnt<3; ccnt++)
      pal[fcol+cnt].RGB[ccnt]=pal[fcol].RGB[ccnt]+cdis[ccnt]*cnt/flen;

  }

COLOR FadeFPal[256];
COLOR *FadeTPal;
int FadeSteps,FadeCStep;

void InitFadeDAC(COLOR *tpal, int steps)
  {
  GetDAC(0,256,FadeFPal);
  FadeTPal=tpal;
  FadeSteps=steps; FadeCStep=0;
  }

BYTE FadeDAC(void)
  {
  COLOR cpal[256];
  int cnt2,cnt3;
  FadeCStep++; if (FadeCStep>FadeSteps) return 0;
  for (cnt2=0; cnt2<256; cnt2++)
    for (cnt3=0; cnt3<3; cnt3++)
      cpal[cnt2].RGB[cnt3]=FadeFPal[cnt2].RGB[cnt3]+FadeCStep*(FadeTPal[cnt2].RGB[cnt3]-FadeFPal[cnt2].RGB[cnt3])/FadeSteps;
  SetDAC(0,256,cpal);
  return 1;
  }
