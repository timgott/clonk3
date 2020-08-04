/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design "Creative Forces" Intro V1.0 Oct 1995 for CLONK 3 Radikal
// Module plus VGA/SVGA Titles

#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <SDL_timer.h>

#include "standard.h"
#include "vga4.h"
#include "std_gfx.h"
#include <stdio.h>

#include "stdfile.h"

#include "RandomWrapper.h"
#include "UserInput.h"

const BYTE CBackPage = 2, GFXPage = 3;

extern BYTE Mousebut(void);

COLOR cpal[256], BlackPal[256], WhitePal[256];

//---------------------------- Extern AGC Loader -----------------------------

extern BYTE LoadAGC2PageV1(char *fname, BYTE tpge, int tx, int ty, WORD *iwdt, WORD *ihgt, BYTE *palptr, BYTE *scram);

//---------------------------- Star System -----------------------------------

const int StarNum = 1000, ShootingNum = 10;

typedef struct STARTYPE { int ang, rad; BYTE col; };

STARTYPE *Stars = NULL, *Shooting = NULL;

float *FDegSin, *FDegCos;

BYTE InitStars(void)
{
	int cnt;
	if (!(Stars = new STARTYPE[StarNum])) return 0;
	for (cnt = 0; cnt < StarNum; cnt++)
	{
		Stars[cnt].ang = random(360);
		Stars[cnt].rad = random(190);
		Stars[cnt].col = 192 + random(33) + random(32)*(!random(7));
	}
	if (!(Shooting = new STARTYPE[ShootingNum])) return 0;
	for (cnt = 0; cnt < ShootingNum; cnt++)
	{
		Shooting[cnt].ang = 360 * cnt / ShootingNum;
		Shooting[cnt].rad = 10 + 70 * (cnt % 3);
		Shooting[cnt].col = 192;
	}
	if (!(FDegSin = new float[360])) return 0;
	if (!(FDegCos = new float[360])) return 0;
	for (cnt = 0; cnt < 360; cnt++)
	{
		FDegSin[cnt] = sin(M_PI*((float)cnt) / 180.0);
		FDegCos[cnt] = cos(M_PI*((float)cnt) / 180.0);
	}

	return 1;
}

void DeInitStars(void)
{
	delete[] Stars;
	delete[] Shooting;
	delete[] FDegSin;
	delete[] FDegCos;
}

void DrawStars(BYTE tpge, int ctx, int cty, float ang, float zm)
{
	int cnt, tx, ty, angidx;
	LPage(tpge);
	for (cnt = 0; cnt < StarNum; cnt++)
	{
		angidx = (Stars[cnt].ang + (int)ang) % 360;

		tx = ctx + FDegSin[angidx] * Stars[cnt].rad*zm;
		ty = cty - FDegCos[angidx] * Stars[cnt].rad*zm;
		if (Inside(tx, 0, 319) && Inside(ty, 0, 199))
			if (!GPixA(tx, ty))
				SPixA(tx, ty, Stars[cnt].col);
	}
}

void CopyMoonDisc(int tx, int ty, int rad)
{
	int xcnt, ycnt, lwdt, dpx, dpy;
	BYTE cpmbuf;

	for (ycnt = -rad; ycnt <= rad; ycnt++)
	{
		lwdt = sqrt(rad*rad - ycnt*ycnt);
		dpy = ty + ycnt;
		for (xcnt = -lwdt; xcnt < lwdt; xcnt++)
		{
			dpx = tx + xcnt;

			if (Inside(70 + dpx, 0, 139) && Inside(65 - dpy, 0, 129))
			{
				LPage(GFXPage); cpmbuf = GPixA(70 + dpx, 65 - dpy);
				LPage(CBackPage); SPixA(158 + dpx, 100 - dpy, cpmbuf);
			}
		}
	}
}

void DrawShooting(BYTE tpge, float ang, BYTE cpymoon)
{
	int cnt, cnt2, tx, ty, ltx, lty, angidx;

	for (cnt = 0; cnt < ShootingNum; cnt++)
		for (cnt2 = 0; (cnt2 < 5) && (Shooting[cnt].col - 10 * cnt2 >= 192); cnt2++)
		{
			angidx = (Shooting[cnt].ang + (int)ang - 10 * cnt2) % 360;
			tx = FDegSin[angidx] * Shooting[cnt].rad;
			ty = FDegCos[angidx] * Shooting[cnt].rad*0.97;

			if (cpymoon && ((cnt == 1) || (Shooting[cnt].rad < 66)))
			{
				if (cnt2 == 0)
					CopyMoonDisc(tx, ty, 6 + cnt);
			}
			else
				if (cnt2 > 0) // Draw Tail
				{
					LPage(tpge);
					DLine(158 + tx, 100 - ty, 158 + ltx, 100 - lty, Shooting[cnt].col - 10 * cnt2);
				}

			ltx = tx; lty = ty;
		}
}

//---------------------------- Intro Process ----------------------------------

void MoveShooting(int prc) // Process 0-200
{
	int cnt;
	if (prc < 100)
		for (cnt = 0; cnt < ShootingNum; cnt++)
		{
			Shooting[cnt].rad += Sign(66 - Shooting[cnt].rad);
			if (Shooting[cnt].col < 235) Shooting[cnt].col++;
		}
	else
		for (cnt = 0; cnt < ShootingNum; cnt++)
			if (prc - 100 > cnt * 5)
				if (Shooting[cnt].col >= 192)
				{
					Shooting[cnt].rad -= 1 + cnt % 2;
					if (Shooting[cnt].rad < 0) Shooting[cnt].col = 0;
				}
}

void RedWolfIn(void)
{
	B4Move(GFXPage, 40, 0, CBackPage, 20, 50, 40, 100);
}

void MoveRedWolfFlash(int prc) // Process 0-40
{
	const BYTE rwbcol = 148;
	int cnt, cnt2;
	LPage(CBackPage);
	// FlashUp new 5 columns
	for (cnt = 80 + prc * 5; cnt < 80 + prc * 5 + 5; cnt++)
		for (cnt2 = 50; cnt2 < 150; cnt2++)
			if (GPixA(cnt, cnt2) == rwbcol)
				SPixA(cnt, cnt2, 245);
	// DeFlash old 5 columns
	for (cnt = 65 + prc * 5; cnt < 65 + prc * 5 + 5; cnt++)
		for (cnt2 = 50; cnt2 < 150; cnt2++)
			if (GPixA(cnt, cnt2) == 245)
				SPixA(cnt, cnt2, rwbcol);
}

void MoveRedWTextFlash(int prc) // Process 0-50
{
	const BYTE rwbcol = 161;
	int cnt, cnt2;
	LPage(CBackPage);
	// FlashUp new 8 columns
	for (cnt = 316 - prc * 8; cnt < 316 - prc * 8 + 8; cnt++) if (cnt < 320)
		for (cnt2 = 170; cnt2 < 200; cnt2++)
			if (GPixA(cnt, cnt2) == rwbcol)
				SPixA(cnt, cnt2, 245);
	// DeFlash old 8 columns
	for (cnt = 340 - prc * 8; cnt < 340 - prc * 8 + 8; cnt++)
		for (cnt2 = 170; cnt2 < 200; cnt2++)
			if (GPixA(cnt, cnt2) == 245)
				SPixA(cnt, cnt2, rwbcol);
}

void MoveInRedWText(int prc) // Process 0-30
{
	B4Move(GFXPage, 0, 130, CBackPage, 0, 199 - prc, 80, Min(1 + prc, 30));
}

BYTE RedWolfD(void) // Return 1 for full abort
{
	long cnt;
	float cang = 0.0, czm = 1.0, rspd = 0.0, zspd = 0.0, csang = 0.0, srspd;
	BYTE cpge = 0;
	char incom = 0;

	// CBackPage must be clear
	if (LoadAGC2PageV1("C3RGRAFX.GRP|C3REDWLF.AGC", GFXPage, 0, 0, 0, 0, (BYTE*)cpal, 0)) return 0;

	SetDAC(0, 256, BlackPal);

	if (InitStars())
	{
		LPage(2); DBox(0, 0, 319, 199, 0);
		for (cnt = 0; (cnt < 1000) && !CheckKeyOrMouse(); cnt++)
		{
			/*Toggle(cpge);*/
			CPage(CBackPage, cpge);
			// Fade in
			if (cnt == 0) InitFadeDAC(cpal, 30);
			if (Inside(cnt, 0, 30)) FadeDAC();
			// Background star movement
			if (Inside(cnt, 0, 120)) rspd += 0.03; // Speed up
			if (Inside(cnt, 180, 250)) if (rspd > 1.0) rspd -= 0.07; // Slow down
			if (Inside(cnt, 90, 130)) zspd += 0.001; // Zoom in
			if (Inside(cnt, 200, 300)) if (zspd > -0.01) zspd -= 0.0015; // Zoom out
			cang += rspd; czm += zspd;
			DrawStars(cpge, 160, 100, cang, czm);
			// Shooting star movement
			if (cnt == 100) srspd = rspd;
			if (Inside(cnt, 100, 269))
			{
				csang += srspd; if (cnt < 200) srspd += 0.03;
				MoveShooting(cnt - 100);
				DrawShooting(cpge, csang, (cnt >= 200));
			}
			// RedWolf in
			if (cnt == 270)
			{
				RedWolfIn();
				SetDAC(0, 256, WhitePal);
				InitFadeDAC(cpal, 10);
			}
			if (Inside(cnt, 270, 280)) FadeDAC();
			if (Inside(cnt, 275, 315)) MoveRedWolfFlash(cnt - 275);
			// RedWolf Text in
			if (Inside(cnt, 290, 320)) MoveInRedWText(cnt - 290);
			if (Inside(cnt, 320, 370)) MoveRedWTextFlash(cnt - 320);
			// Fade out
			if (cnt == 480) InitFadeDAC(BlackPal, 20);
			if (Inside(cnt, 480, 500)) FadeDAC();
			// End
			if (cnt == 500) break;

			PPage(cpge);
			SDL_Delay(14);
		}
	}
	DeInitStars();

	//while (kbhit()) { incom = getch(); if (!incom) incom = getch(); }

	if (incom == 27) return 1;
	return 0;
}

void ColorRunThrough(COLOR *pal, int first, int last)
{
	COLOR buf;
	int cnt;
	if (first > last) return;
	buf = pal[last];
	for (cnt = last; cnt > first; cnt--) pal[cnt] = pal[cnt - 1];
	pal[first] = buf;
}

//----------------------- VESA SVGA Operation ---------------------------------
/*
BYTE *VESAVideoRam,*VESAOutputPtr;
WORD VESAFramePos,VESAGranularity;
DWORD VESAOutputCnt;

BYTE *VESAStatus=NULL;

BYTE GetVESAModeInfo(WORD mnum, BYTE *mbuf)
  {
  union REGS Regs;
  struct SREGS SRegs;
  Regs.h.ah=0x4F; // VESA function number
  Regs.h.al=0x01; // Request mode data
  Regs.x.cx=mnum; // Video mode number
  Regs.x.di=FP_OFF(mbuf); // Buf ptr to ES:DI
  SRegs.es=FP_SEG(mbuf);
  int86x(0x10,&Regs,&Regs,&SRegs); // Call BIOS-Video-Int 10h
  if ((Regs.h.al!=0x4F) || (Regs.h.ah!=0x00)) return 0;
  return 1;
  }

BYTE CheckVESAMode(WORD mnum)
  {
  union REGS Regs;
  struct SREGS SRegs;
  BYTE ibuf[256];
  WORD *mptr;

  // Check for VESA availability
  Regs.h.ah=0x4F; // VESA function number
  Regs.h.al=0x00; // Request SVGA-card capabilities
  Regs.x.di=FP_OFF(ibuf); // Buf ptr to ES:DI
  SRegs.es=FP_SEG(ibuf);
  int86x(0x10,&Regs,&Regs,&SRegs); // Call BIOS-Video-Int 10h
  if ((Regs.h.al!=0x4F) || (Regs.h.ah!=0x00)) return 0;
  // Check for mode availability
  mptr=(WORD*)(*((DWORD*)(ibuf+0x0E))); // Pointer to mode number list
  if (!mptr) return 0; // safety
  while (*mptr!=0xFFFF)
	{
	if (*mptr==mnum) return 1;
	mptr++;
	}

  return 0;
  }

BYTE CheckModeUsage(BYTE *minfo)
  {
  // Can mode be used with installed monitor?
  if (!(*(minfo+0x00) & 1)) return 0;
  // Has optional information been returned?
  if (!(*(minfo+0x00) & 2)) return 0;

  // Mode 101h should be 640x480x256
  if (*((WORD*)(minfo+0x12)) != 640) return 0;
  if (*((WORD*)(minfo+0x14)) != 480) return 0;
  if (*(minfo+0x19) != 8) return 0;

  // Is the first access window available for writing?
  if (!(*(minfo+0x02) & 1) || !(*(minfo+0x02) & 4)) return 0;

  return 1;
  }

BYTE InitVESAMode(WORD mnum)
  {
  union REGS Regs;

  Regs.h.ah=0x4F; // VESA function number
  Regs.h.al=0x02; // Init VESA mode
  Regs.x.bx=mnum; // Mode number
  int86(0x10,&Regs,&Regs); // Call BIOS-Video-Int 10h
  if ((Regs.h.al!=0x4F) || (Regs.h.ah!=0x00)) return 0;

  return 1;
  }

BYTE SetVESAFrame(BYTE frame, WORD offs) // Offset in KByte
  {
  union REGS Regs;
  if (offs%VESAGranularity!=0) return 0;
  Regs.h.ah=0x4F; // VESA function number
  Regs.h.al=0x05; // Set frame
  Regs.h.bh=0x00;
  Regs.h.bl=frame; // Frame number (0 or 1)
  Regs.x.dx=offs/VESAGranularity; // Offset factor
  int86(0x10,&Regs,&Regs); // Call BIOS-Video-Int 10h
  if ((Regs.h.al!=0x4F) || (Regs.h.ah!=0x00)) return 0;
  return 1;
  }

BYTE ResetVESAOutput(void)
  {
  VESAOutputPtr=VESAVideoRam;
  VESAOutputCnt=0L;
  if (!SetVESAFrame(0,VESAFramePos)) return 0;
  return 1;
  }

void VESAOutputPixel(BYTE col)
  {
  *VESAOutputPtr=col; VESAOutputPtr++; VESAOutputCnt++;
  if (VESAOutputCnt==65536L)
	{
	VESAFramePos+=64; // Advance 64 KB
	ResetVESAOutput();
	}
  }

BYTE StoreVESAStatus(void)
  {
  WORD stsize;
  union REGS Regs;
  struct SREGS SRegs;

  // Request status storage size

  Regs.h.ah=0x4F; // VESA function number
  Regs.h.al=0x04; // Store/restore card status
  Regs.h.dl=0x00; // SubSub request storage size
  Regs.x.cx=11; // Store hardware status, BIOS data, SVGA status
  int86(0x10,&Regs,&Regs); // Call BIOS-Video-Int 10h
  if ((Regs.h.al!=0x4F) || (Regs.h.ah!=0x00)) return 0;
  stsize=64*Regs.x.bx;

  // Allocate memory
  if (!(VESAStatus=new BYTE[stsize])) return 0;

  // Store status
  Regs.h.ah=0x4F; // VESA function number
  Regs.h.al=0x04; // Store/restore card status
  Regs.h.dl=0x01; // SubSub store status
  Regs.x.cx=11; // Store hardware status, BIOS data, SVGA status
  Regs.x.bx=FP_OFF(VESAStatus); // Buf ptr to ES:BX
  SRegs.es=FP_SEG(VESAStatus);
  int86x(0x10,&Regs,&Regs,&SRegs); // Call BIOS-Video-Int 10h
  if ((Regs.h.al!=0x4F) || (Regs.h.ah!=0x00))
	{ delete [] VESAStatus; VESAStatus=NULL; return 0; }

  return 1;
  }

BYTE RestoreVESAStatus(void)
  {
  union REGS Regs;
  struct SREGS SRegs;

  if (!VESAStatus) return 0;

  // Restore status

  Regs.h.ah=0x4F; // VESA function number
  Regs.h.al=0x04; // Store/restore card status
  Regs.h.dl=0x02; // SubSub store status
  Regs.x.cx=11; // Restore hardware status, BIOS data, SVGA status
  Regs.x.bx=FP_OFF(VESAStatus); // Buf ptr to ES:BX
  SRegs.es=FP_SEG(VESAStatus);
  int86x(0x10,&Regs,&Regs,&SRegs); // Call BIOS-Video-Int 10h
  delete [] VESAStatus; VESAStatus=NULL;
  if ((Regs.h.al!=0x4F) || (Regs.h.ah!=0x00)) return 0;

  return 1;
  }

BYTE InitSVGA(void)
  {
  BYTE minfo[256];

  if (CheckVESAMode(0x101))
	if (GetVESAModeInfo(0x101,minfo))
	  if (CheckModeUsage(minfo))
	if (StoreVESAStatus())
	  if (InitVESAMode(0x101))
		{
		VESAVideoRam=(BYTE*)MK_FP(0xA000,0x0000);
		VESAGranularity=*((WORD*)(minfo+0x04));
		VESAFramePos=0;
		ResetVESAOutput();
		return 1;
		}
  return 0;
  }

void DeInitSVGA(void)
  {
  RestoreVESAStatus();
  }

typedef struct PCXHEADTYPE {
	BYTE manf, vers, encd, bppx;
	WORD imx, imy, wdt, hgt, hres, vres;
	BYTE egapal[48];
	BYTE resv, plns;
	WORD bpln, palt;
	WORD scrw, scrh;
	BYTE rest[54];
};

BYTE LoadPCX640x480(char *fname) // Returns 1 if okay
{
	PCXHEADTYPE head;
	COLOR cpal[256];
	DWORD cnt, iwdt, ihgt;
	WORD rlen, itx, ity;
	BYTE fbuf;

	VESAFramePos = 0;
	ResetVESAOutput();

	if (InitBFI(fname)) return 0;

	if (GetBFI(&head, sizeof(PCXHEADTYPE))) return 0;

	iwdt = head.wdt + 1; ihgt = head.hgt + 1;
	if ((head.manf != 10) || (iwdt != 640) || (ihgt != 480))
	{
		DeInitBFI(); return 0;
	}

	cnt = iwdt*ihgt;

	SetDAC(0, 256, BlackPal);

	while (cnt > 0)
	{
		if (GetBFI(&fbuf)) return 0;
		if (fbuf > 191)
		{
			rlen = fbuf - 192;
			if (GetBFI(&fbuf)) return 0;
			while (rlen > 0) { VESAOutputPixel(fbuf); rlen--; cnt--; }
		}
		else
		{
			VESAOutputPixel(fbuf); cnt--;
		}
	}

	if (GetBFI(&fbuf)) return 0; // 0CH CPAL ID
	if (GetBFI(&cpal, 768)) return 0;
	for (cnt = 0; cnt < 768; cnt++) cpal[cnt / 3].RGB[cnt % 3] >>= 2;
	SetDAC(0, 256, cpal);

	DeInitBFI();

	return 1;
}*/

/*BYTE Wait4User(long len) // Ersetzung in SDLmain
{
	char incom = 0;
	len *= 100L;
	while (!kbhit() && !Mousebut() && (len > 0)) { len--; SDL_Delay(10); PPage(0); }
	while (kbhit()) { incom = getch(); if (!incom) incom = getch(); }
	if (incom == 27) return 1;
}*/

BYTE TitleGraphic(BYTE hires)
{
	BYTE svgaok = 0, rval;
	/*if (hires && FileExists("C3RTITLE.GRP")) // Nur für das hochauflösende Bild am Anfang mach ich mir jetzt nicht so eine Mühe.
		if (InitSVGA())
		{
			if (LoadPCX640x480("C3RTITLE.GRP|C3HTITLE.PCX"))
			{
				svgaok = 1;
				rval = Wait4User(20);
				SetDAC(0, 256, BlackPal);
			}
			DeInitSVGA();
		}*/
	if (!svgaok)
	{
		LPage(0); ClPage(); PPage(0); LPage(GFXPage);
		if (LoadAGC2PageV1("C3RGRAFX.GRP|C3LTITLE.AGC", GFXPage, 0, 0, 0, 0, NULL, 0)) return 0;
		B4Move(GFXPage, 0, 0, 0, 0, 0, 80, 200);
		//CPage(GFXPage, 0);
		rval = WaitForInputTimeout(15);
	}
	return rval;
}

#define ANSI_COLOR_LIGHTGRAY "\033[22;37m"
#define ANSI_COLOR_DARKGRAY "\033[1;30m"
#define ANSI_COLOR_WHITE "\033[01;37m"
#define ANSI_RESET "\033[0m"

void EndTextMessage(void)
{
	//textcolor(LIGHTGRAY);
	printf(ANSI_COLOR_LIGHTGRAY "              All ");
	//textcolor(WHITE);
	printf(ANSI_COLOR_WHITE "Clonks ");
	//textcolor(LIGHTGRAY);
	printf(ANSI_COLOR_LIGHTGRAY "will be happy to see you again soon for\n\r");
	printf("                     an all new round of ");
	//textcolor(WHITE);
	printf(ANSI_COLOR_WHITE "CLONK 3 Radikal");
	//textcolor(LIGHTGRAY);
	printf(ANSI_COLOR_LIGHTGRAY "!\n\r");
	//textcolor(DARKGRAY);
	printf(ANSI_COLOR_DARKGRAY "     ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n\r");
	//textcolor(LIGHTGRAY);
	printf(ANSI_COLOR_LIGHTGRAY "           Information and contact / ");
	//textcolor(WHITE);
	printf(ANSI_COLOR_WHITE "RedWolf Design ");
	//textcolor(LIGHTGRAY);
	printf(ANSI_COLOR_LIGHTGRAY "on the Internet:\n\r");
	printf("     Homepage: ");
	//textcolor(WHITE);
	printf(ANSI_COLOR_WHITE "http://members.aol.com/RedWolfD");
	//textcolor(LIGHTGRAY);
	printf(ANSI_COLOR_LIGHTGRAY "   EMail: ");
	//textcolor(WHITE);
	printf(ANSI_COLOR_WHITE "RedWolfD@aol.com\n\r" ANSI_RESET);
	//textcolor(LIGHTGRAY);
}




void ClearVGAPages(void)
{
	int cnt;
	for (cnt = 0; cnt < 4; cnt++)
	{
		LPage(cnt); DBox(0, 0, 319, 199, 0);
	}
	LPage(0); PPage(0);
}

extern BYTE InitSVIMouse(void);

void Intro(BYTE hires)
{
	ZeroMem((BYTE*)BlackPal, 256 * 3);
	FillMem((BYTE*)WhitePal, 256 * 3, 63); ZeroMem((BYTE*)WhitePal, 3);

	if (!RedWolfD())
		TitleGraphic(hires);

	ClearVGAPages();
	InitSVIMouse();
}

//------------------------------- FInteg -------------------------------------

int TextFileCheckSum(char *fname) // Checksums division remainder 12345
{
	FILE *fhnd;
	int checksum = 0;
	int fbuf;
	if (!(fhnd = fopen(fname, "rt"))) return -1;
	do
	{
		fbuf = fgetc(fhnd);
		if (fbuf != EOF) { checksum += fbuf; checksum %= 12345; }
	} while (fbuf != EOF);
	fclose(fhnd);
	return checksum;
}

BYTE FIntegCheck(void)
{
	//if (TextFileCheckSum("CLONK3.HLP") != 12315) return 0; // 
	//if (TextFileCheckSum("MISSIONS.SCR") != 453) return 0; // 
	return 1;
}
