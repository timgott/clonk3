/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// STD_GFX Library for Standard Graphics Functions

// Viewports, Font, Drawing Functions, String/Text Functions

// by Matthes Bender of RedWolf Design

// Versions: 1.0 ?             (STDVGA)
//           1.1 October  1994
//           1.2 November 1994
//           1.3 December 1994
//           1.4 January  1995
//           1.5 April    1995
//	         1.6 December 1995 (New DACFade capabilities)
//           1.7 January  1996 (FTOut)
//           1.8 June     1996 (STD_GFX, Enhanced FTOut, DAC out)

//------------------------------ Headers -------------------------------------

#include <dos.h>
#include <conio.h>
#include <stdio.h>

#include <standard.h>
#include <vga4.h>

//----------------------- Viewport & Drawing Functions ----------------------------------

int MxVPX1=0,MxVPY1=0,MxVPX2=MaxX,MxVPY2=MaxY;

int VPX1=MxVPX1,VPY1=MxVPY1,VPX2=MxVPX2,VPY2=MxVPY2;

void Viewport(int vpx1, int vpy1, int vpx2, int vpy2)
  {
  VPX1=vpx1; VPX2=vpx2; VPY1=vpy1; VPY2=vpy2;
  }

void NoViewport(void)
  {
  VPX1=MxVPX1; VPX2=MxVPX2; VPY1=MxVPY1; VPY2=MxVPY2;
  }

inline void SPixF(int pxx, int pxy, BYTE pxc)
  {
  if (Inside(pxx,VPX1,VPX2) && Inside(pxy,VPY1,VPY2)) SPixA(pxx,pxy,pxc);
  }

inline BYTE GPixF(int pxx, int pxy)
  {
  if (Inside(pxx,MxVPX1,MxVPX2) && Inside(pxy,MxVPY1,MxVPY2)) return GPixA(pxx,pxy);
  return 0;
  }

void DLine(int x1, int y1, int x2, int y2, BYTE color)
  {
  int d,dx,dy,aincr,bincr,xincr,yincr,x,y;
  if (Abs(x2-x1)<Abs(y2-y1))
    {
    if (y1>y2) { Swap(x1,x2); Swap(y1,y2); }
    xincr=(x2>x1) ? 1 : -1;
    dy=y2-y1; dx=Abs(x2-x1);
    d=2*dx-dy; aincr=2*(dx-dy); bincr=2*dx; x=x1; y=y1;
    SPixF(x,y,color);
    for (y=y1+1; y<=y2; ++y)
      {
      if (d>=0) { x+=xincr; d+=aincr; }
      else d+=bincr;
      SPixF(x,y,color);
      }
    }
  else
    {
    if (x1>x2) { Swap(x1,x2); Swap(y1,y2); }
    yincr=(y2>y1) ? 1 : -1;
    dx=x2-x1; dy=Abs(y2-y1);
    d=2*dy-dx; aincr=2*(dy-dx); bincr=2*dy; x=x1; y=y1;
    SPixF(x,y,color);
    for (x=x1+1; x<=x2; ++x)
      {
      if (d>=0) { y+=yincr; d+=aincr; }
      else d+=bincr;
      SPixF(x,y,color);
      }
    }
  }

void DFrame(int x1,int y1,int x2,int y2,BYTE color)
  {
  register int cnt;
  if (x2<x1) Swap(x1,x2);
  if (y2<y1) Swap(y1,y2);
  for (cnt=x1; cnt<=x2; cnt++)
    { SPixF(cnt,y1,color); SPixF(cnt,y2,color); }
  for (cnt=y1; cnt<=y2; cnt++)
    { SPixF(x1,cnt,color); SPixF(x2,cnt,color); }
  }

void DBox(int x1,int y1,int x2,int y2,BYTE color)
  {
  register int cnt,cnt2;
  if (x2<x1) Swap(x1,x2);
  if (y2<y1) Swap(y1,y2);
  x1=Max(x1,VPX1); x2=Min(x2,VPX2);
  y1=Max(y1,VPY1); y2=Min(y2,VPY2);
  for (cnt=y1; cnt<=y2; cnt++)
    for (cnt2=x1; cnt2<=x2; cnt2++)
      SPixA(cnt2,cnt,color);
  }

void DFill(int fx, int fy, BYTE col)
  {
  BYTE bcol;
  int hlxcnt,hlx1,hlx2,nhlx;
  bcol=GPixF(fx,fy);
  if (bcol==col) return;
  hlx1=fx; hlx2=fx;
  for (nhlx=fx; (GPixF(nhlx,fy)==bcol) && (nhlx>=VPX1); nhlx--)
    { SPixF(nhlx,fy,col); hlx1=nhlx; }
  if (fx<VPX2)
    for (nhlx=fx+1; (GPixF(nhlx,fy)==bcol) && (nhlx<=VPX2); nhlx++)
      { SPixF(nhlx,fy,col); hlx2=nhlx; }
  if (fy>VPY1)
    for (hlxcnt=hlx1; hlxcnt<=hlx2; hlxcnt++)
      if (GPixF(hlxcnt,fy-1)==bcol)
	DFill(hlxcnt,fy-1,col);
  if (fy<VPY2)
    for (hlxcnt=hlx1; hlxcnt<=hlx2; hlxcnt++)
      if (GPixF(hlxcnt,fy+1)==bcol)
	DFill(hlxcnt,fy+1,col);
  }

void ClPage(void)
  {
  DBox(0,0,319,199,0);
  }

//------------------------------- VGAFONT ------------------------------------

const int MaxSOut=80;

DWRD FnBas[35];
BYTE FntDef=0;

void DefFont(void)
  {
  static char *lines2[35]={"nn    x x x  xx  x x x  x   xx  ", // Chrs 32-41
			   "nn    x x x xxx    x x  x  x  x ",
			   "nn    x    x xx x x  x     x  x ",
			   "nn           xxx x   x     x  x ",
			   "nn    x      xx  x x x      xx  ",
			   "nn                 xxxx x xxxxxx", // Chrs 42-51
			   "nnx x x            xx xxx   x  x",
			   "nn x xxx   xxx    x x x x xxx xx",
			   "nnx x x  x       x  x x x x    x",
			   "nn      x      x x  xxxxxxxxxxxx",
			   "nnx  xxxxxxxxxxxxxxx        x   ", // Chrs 52-61
			   "nnx xx  x    xx xx x x  x  x xxx",
			   "nnxxxxxxxxx x xxxxxx      x     ",
			   "nn  x  xx x x x x  x x  x  x xxx",
			   "nn  xxxxxxx x xxx  x   x    x   ",
			   "nnx  xx xxx x xx xxxxx xxxxxxxxx", // Chrs 62-71
			   "nn x   xx xx xx xx  x xx  x  x  ",
			   "nn  x x xxxx xxx x  x xxx xx x x",
			   "nn x    x  xxxx xx  x xx  x  x x",
			   "nnx   x xxxx xxxxxxxxx xxxx  xxx",
			   "nnx xxxxxxxx xx  x xx xxxxxxxxxx", // Chrs 72-81
			   "nnx x x   xx xx  xxxxxxx xx xx x",
			   "nnxxx x   xxx x  xxxxxxx xxxxx x",
			   "nnx x x x xx xx  x xxxxx xx  xxx",
			   "nnx xxxx x x xxxxx xx xxxxx  xxx",
			   "nnxxxxxxxxxx xx xx xx xx xxxx xx", // Chrs 82-91
			   "nnx xx   x x xx xx xx xx x  x x ",
			   "nnxxxxxx x x xx xxxx x  x  x  x ",
			   "nnxx   x x x xx xxxxx x x x   x ",
			   "nnx xxxx x xxx x x xx x x xxx xx",
			   "nnx  xx xxx    x       x xx xx x", // Chrs 92-101
			   "nnx   x       x x       x       ",
			   "nn x  x        x       x xxxxx x",
			   "nn  x x                xxxx xx x",
			   "nn  xxx    xxx         x xxxxxxx"};

  int lcnt,bcnt;
  for (lcnt=0; lcnt<35; lcnt++)         // not used: 97,98
    {
    FnBas[lcnt]=0;
    for (bcnt=0; bcnt<32; bcnt++)
      {
      FnBas[lcnt]<<=1;
      if (*(lines2[lcnt]+bcnt)!=32)
	FnBas[lcnt]|=1;
      }
    }
  FntDef=1;
  }

BYTE COut(BYTE chr, int tx, int ty, BYTE fcol, int bcol)
  {
  int lcnt,bcnt;
  DWRD lbuf;
  if (!FntDef) DefFont();
  if (Inside(chr,97,122)) chr-=32;
  if (chr==124) chr=38;
  if ((chr==132) || (chr==142)) chr=99;
  if ((chr==148) || (chr==153)) chr=100;
  if ((chr==129) || (chr==154)) chr=101;
  if (chr==248) chr=64;
  if (Inside(chr,32,101))
    {
    chr-=32;
    for (lcnt=0; lcnt<5; lcnt++)
      {
      lbuf=FnBas[(chr/10)*5+lcnt];
      lbuf>>=((9-chr%10)*3);
      for (bcnt=0; bcnt<3; bcnt++)
	{
	if (lbuf & 1) SPixF(tx+2-bcnt,ty+lcnt,fcol);
	if ((!(lbuf & 1)) && (bcol>=0)) SPixF(tx+2-bcnt,ty+lcnt,bcol);
	lbuf>>=1;
	}
      }
    if (bcol>=0) DFrame(tx+3,ty,tx+3,ty+4,bcol);
    if (chr==94-32) SPixF(tx+3,ty,fcol);
    return 1;
    }
  return 0;
  }

int CSLen(BYTE *sptr, int maxlen=MaxSOut)
  {
  int rval=0;
  if (!sptr) return 0;
  while (*sptr && (rval<maxlen))
    {
    if (*sptr==128) rval-=2; // Color definition '€c'
    if (*sptr==226) rval-=1; // HLink start/stop 'â'
    //if (*sptr==9) rval+=7;   // Tab character 9
    sptr++; rval++;
    }
  return rval;
  }

void SReplaceChar(BYTE *str, BYTE ochar, BYTE nchar)
  {
  while (*str) { if (*str==ochar) *str=nchar; str++; }
  }

void SOut(BYTE *sptr, int tx, int ty, BYTE fcol, int bcol=-1, BYTE form=0)
  {
  int lenchk=0;
  if (form) tx+=(form==2)-CSLen(sptr)*(2+2*(form==2));
  while (*sptr && (lenchk<MaxSOut))
    {
    if (*sptr==128) { sptr++; fcol=*sptr; } // Change Color
    else { COut(*sptr,tx,ty,fcol,bcol); tx+=4; }
    sptr++; lenchk++;
    }
  }

void SOutS(BYTE *sptr, int tx, int ty, BYTE fcol, BYTE bcol, BYTE form=0)
  {
  int lenchk=0;
  if (form) tx+=(form==2)-CSLen(sptr)*(2+2*(form==2));
  while (*sptr && (lenchk<MaxSOut))
    {
    if (*sptr==128) { sptr++; fcol=*sptr; } // Change Color
    else { COut(*sptr,tx+1,ty+1,bcol,-1); COut(*sptr,tx,ty,fcol,-1); tx+=4; }
    sptr++; lenchk++;
    }
  }

void TOut(BYTE *tptr, int tx, int ty, BYTE fcol, int bcol=-1, BYTE form=0)
  {
  BYTE *cptr=tptr;
  while (*cptr) { if (*cptr==124) { *cptr=0; SOut(tptr,tx,ty,fcol,bcol,form); *cptr=124; ty+=6; tptr=cptr+1; } cptr++; }
  SOut(tptr,tx,ty,fcol,bcol,form);
  }

void TOutS(BYTE *tptr, int tx, int ty, BYTE fcol, int bcol, BYTE form=0)
  {
  BYTE *cptr=tptr;
  while (*cptr) { if (*cptr==124) { *cptr=0; SOutS(tptr,tx,ty,fcol,bcol,form); *cptr=124; ty+=6; tptr=cptr+1; } cptr++; }
  SOutS(tptr,tx,ty,fcol,bcol,form);
  }

void TOutSt(BYTE *tptr, int *mwdt, int *mhgt)
  {
  int cwdt=0;
  *mwdt=0; *mhgt=1;
  while (*tptr)
    {
    cwdt++; if (cwdt>*mwdt) *mwdt=cwdt;
    if (*tptr==124) { cwdt=0; (*mhgt)++; }
    tptr++;
    }
  }



const BYTE HLinkCol=140;



void IFTOut(BYTE *tptr, int tx, int ty, int wdt, int maxhgt, int startl, int *loutp, BYTE fcol, int bcol, BYTE form, BYTE output, int ax, int ay, BYTE *gethl)
  {
  int cox,coy,wlen,cwlen,cnt,hlridx;
  BYTE ccol,srun,cchar,hlrun,eotext,lfull,hlgot;

  if (form==1) tx+=wdt*2; if (form==2) tx+=wdt*4;
  wdt=BoundBy(wdt,1,80); maxhgt=Max(maxhgt,0);

  coy=0; ccol=fcol; srun=0; hlrun=0; eotext=0; hlgot=0;
  if (gethl) gethl[0]=0;

  do
    {
    cox=0; lfull=0;

    // BackCol bar
    if (output) if (bcol>-1) if (Inside(coy-startl,0,maxhgt-1))
      DBox(tx,ty+6*(coy-startl),tx+4*wdt-1,ty+6*(coy-startl)+5,bcol);

    do
      {
      // Get next word
      for (wlen=0; *(tptr+wlen) && (*(tptr+wlen)!=' ') && (*(tptr+wlen)!='|') && (*(tptr+wlen)!=0x0A); wlen++);

      // If too long, truncate word till it fits in line
      do
	{
	cwlen=CSLen(tptr,wlen);
	if (cwlen>wdt) wlen--;
	}
      while ((cwlen>wdt) && (wlen>0));

      cchar=*(tptr+wlen);
      // Text paragraph/EOL?
      if ((cchar=='|') || (cchar==0x0A)) lfull=1;
      // Include pipe,space,EOL character
      if (cchar) wlen++;
      // End of text?
      if (!cchar) eotext=1;

      if (cox+cwlen<=wdt) // Word fits in line
	{
	// Output word
	for (cnt=0; cnt<wlen; cnt++)
	  {
	  cchar=*(tptr+cnt);
	  switch (srun)
	    {
	    case 0: // No special run
	      switch (cchar)
		{
		case 128: srun=1; break; // Color def run '€'
		case 226: // HLink run toggle 'â'
		  if (!hlrun)
		    { hlrun=1; hlridx=0; }
		  else
		    {
		    hlrun=0; if (gethl) gethl[hlridx]=0;
		    if (hlgot==1) hlgot=2;
		    }
		  break;
		case 9: cox=8*((cox/8)+1); break; // Tab 8
		default: // Output character
		  if (cchar=='|') cchar=' '; // Don't show pipes
		  if (output && Inside(coy-startl,0,maxhgt-1))
		    COut(cchar,tx+4*cox,ty+6*(coy-startl),hlrun ? HLinkCol : ccol,-1);
		  if (hlrun) if (gethl)
		    {
		    gethl[hlridx]=cchar; hlridx++;
		    if ((ax==cox) && (ay==coy-startl)) hlgot=1;
		    }
		  cox++;
		  break;
		}
	      break;
	    case 1: // Color def run
	      ccol=cchar; srun=0; break;
	    }
	  }
	// Advance pointers
	tptr+=wlen;
	}
      else // Word doesn't fit, line full
	lfull=1;
      }
    while (!lfull && !eotext && (hlgot!=2));

    // Advance y-position
    coy++;
    }
  while (!(eotext && !lfull) && (coy-startl<maxhgt) && (hlgot!=2));

  // Top More-Mark
  if (startl>0) if (output)
    {
    DLine(tx+4*wdt-8,ty-1,tx+4*wdt-1,ty+7-1,76);
    DLine(tx+4*wdt-1,ty-1,tx+4*wdt-1,ty+7-1,76);
    DLine(tx+4*wdt-8,ty-1,tx+4*wdt-1,ty-1,76);
    }
  if (startl==0) if (output) if (bcol>-1)
    DLine(tx+4*wdt-8,ty-1,tx+4*wdt-1,ty-1,bcol);
  // Bottom More-Mark
  if ((coy-startl>=maxhgt) && *tptr) if (output)
    {
    DLine(tx+4*wdt-8,ty+6*maxhgt-1,tx+4*wdt-1,ty+6*maxhgt-8,76);
    DLine(tx+4*wdt-1,ty+6*maxhgt-1,tx+4*wdt-1,ty+6*maxhgt-8,76);
    DLine(tx+4*wdt-8,ty+6*maxhgt-1,tx+4*wdt-1,ty+6*maxhgt-1,76);
    }

  // Returns number of output lines
  // -1 assumes last line contains NULL character only
  if (loutp) *loutp=coy-startl-1;

  // HL not found
  if (gethl) if (hlgot!=2) gethl[0]=0;

  // Remaining BackCol bars
  if (output) if (bcol>-1)
    while (Inside(coy-startl,0,maxhgt-1))
      {
      DBox(tx,ty+6*(coy-startl),tx+4*wdt-1,ty+6*(coy-startl)+5,bcol);
      coy++;
      }
  }

void FTOut(BYTE *tptr, int tx, int ty, int wdt, int maxhgt, int startl, BYTE fcol, int bcol, BYTE form)
  {
  IFTOut(tptr,tx,ty,wdt,maxhgt,startl,NULL,fcol,bcol,form,1,0,0,NULL);
  }

void FTOutSt(BYTE *tptr, int wdt, int startl, int *loutp)
  {
  IFTOut(tptr,0,0,wdt,30000,startl,loutp,0,0,0,0,0,0,NULL);
  }

void FTOutGetHL(BYTE *tptr, int wdt, int maxhgt, int startl, int ax, int ay, BYTE *gethl)
  {
  IFTOut(tptr,0,0,wdt,maxhgt,startl,NULL,0,0,0,0,ax,ay,gethl);
  }
