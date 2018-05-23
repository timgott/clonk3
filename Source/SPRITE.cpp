/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  SPRITE 1.4  Library  by Matthes Bender

// Versions: 1.0 Oct 1994  1.1 Dec 1994  1.2 Jul 1995  1.3 Sep 1995
//           1.4 Jun 1996

//--------------------------- Include Headers --------------------------------

#include <dos.h>

#include "standard.h"
#include "vga4.h"
#include "std_gfx.h"

//--------------------------- Type Defintions --------------------------------

typedef struct BSPRITE {
	BYTE spge, alias;
	int sx, sy, sw, sh;
	BYTE *smask, *cmask;
};

typedef struct ASPRITE { BSPRITE shift[4]; };

//-------------------------- Global Variables --------------------------------

BYTE CBaseCol = 0;
int CBaseX = 0, CBaseY = 0;

BYTE SpriteInitError = 0;

//----------------------------- RWCopy ---------------------------------------

void RWCopy(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE trns)
{
	BYTE linebuf[320];
	BYTE *bufptr;
	register int xcnt, ycnt;
	for (ycnt = 0; ycnt < hgt; ycnt++)
	{
		LPage(fpge); bufptr = linebuf;
		for (xcnt = 0; xcnt < wdt; xcnt++, bufptr++) *bufptr = GPixF(fx + xcnt, fy);
		LPage(tpge); bufptr = linebuf;
		for (xcnt = 0; xcnt < wdt; xcnt++, bufptr++)
			if (!(trns && !(*bufptr)))
				SPixF(tx + xcnt, ty, *bufptr);
		fy++; ty++;
	}
	LPage(tpge);
}

void RWCopyE(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE trns, float scalex, float scaley)
{
	BYTE linebuf[320];
	int xcnt, ycnt;
	if ((scalex == 0.0) || (scaley == 0.0)) return;
	for (ycnt = 0; ycnt < hgt*scaley; ycnt++)
	{
		LPage(fpge);
		for (xcnt = 0; xcnt < wdt*scalex; xcnt++) linebuf[xcnt] = GPixF(fx + xcnt / scalex, fy + ycnt / scaley);
		LPage(tpge);
		for (xcnt = 0; xcnt < wdt*scalex; xcnt++)
			if (!(trns && !linebuf[xcnt]))
				SPixF(tx + xcnt, ty + ycnt, linebuf[xcnt]);
	}
	LPage(tpge);
}

//----------------------------- ABC Sprites ----------------------------------

void SetColorBase(BYTE newsbc, int cbx, int cby)
{
	CBaseCol = newsbc;
	CBaseX = cbx; CBaseY = cby;
}

WORD MaskSize(int wdt, int hgt)
{
	return (wdt*hgt + 7) / 8;
}

void CreateMask(BYTE *mbuf, BYTE spge, int sx, int sy, int wdt, int hgt)
{
	WORD bmsize, bcnt, btcnt, pxn;
	bmsize = MaskSize(wdt, hgt);
	pxn = 0; LPage(spge);
	for (bcnt = 0; bcnt < bmsize; bcnt++)
	{
		*(mbuf + bcnt) = 0;
		for (btcnt = 0; btcnt < 8; btcnt++)
		{
			*(mbuf + bcnt) >>= 1;
			if (GPixF(sx + pxn%wdt, sy + pxn / wdt) && (pxn / wdt <= hgt))
				*(mbuf + bcnt) |= 128;
			pxn++;
		}
	}
}

void CreateColorMask(BYTE *mbuf, BYTE spge, int sx, int sy, int wdt, int hgt, BYTE bcol)
{
	WORD bmsize, bcnt, btcnt, pxn;
	bmsize = MaskSize(wdt, hgt);
	pxn = 0; LPage(spge);
	for (bcnt = 0; bcnt < bmsize; bcnt++)
	{
		*(mbuf + bcnt) = 0;
		for (btcnt = 0; btcnt < 8; btcnt++)
		{
			*(mbuf + bcnt) >>= 1;
			if ((GPixF(sx + pxn%wdt, sy + pxn / wdt) == bcol) && (pxn / wdt <= hgt))
				*(mbuf + bcnt) |= 128;
			pxn++;
		}
	}
}

BYTE TBMove(BYTE fpge, int frx, int fry, BYTE tpge, int tox, int toy, BYTE wdt, BYTE hgt)
{
	WORD bmsize, bcnt, btcnt, pxn;
	BYTE *bmskp;
	bmsize = MaskSize(wdt, hgt);
	if (!(bmskp = new BYTE[bmsize])) bmskp = 0;
	else CreateMask(bmskp, fpge, frx, fry, wdt, hgt);
	BMove(fpge, frx, fry, tpge, tox, toy, wdt, hgt, bmskp);
	if (bmskp) { delete[] bmskp; return 1; }
	return 0;
}

void CreateCSprite(BYTE pge, int sx, int sy, int sw, int sh)
{
	int cnt;
	LPage(pge); DBox(sx + sw, sy, sx + 4 * (sw + 4) - 1, sy + sh - 1, 0);
	for (cnt = 0; cnt < 3; cnt++)
		RWCopy(pge, sx, sy, pge, sx + sw + 4 + (sw + 4)*cnt + 1 + cnt, sy, sw, sh, 0);
}

inline void CSprite(BYTE fpge, int sx, int sy, BYTE tpge, int tx, int ty, BYTE sw, BYTE sh)
{
	TBMove(fpge, sx + (sw + 4)*(tx % 4), sy, tpge, (tx / 4) * 4, ty, sw + 4, sh);
}

BYTE CreateBSprite(BSPRITE *bsptr, BYTE spge, int sx, int sy, int sw, int sh, BYTE clrd = 0)
{
	WORD bmsize;
	bsptr->spge = spge;
	bsptr->sx = sx; bsptr->sy = sy;
	bsptr->sw = sw; bsptr->sh = sh;
	bsptr->alias = 0;
	bmsize = MaskSize(sw, sh);
	if (!(bsptr->smask = new BYTE[bmsize])) { bsptr->smask = 0; SpriteInitError = 1; return 0; }
	CreateMask(bsptr->smask, spge, sx, sy, sw, sh);
	if (clrd)
	{
		if (!(bsptr->cmask = new BYTE[bmsize]))
		{
			delete[] bsptr->smask; bsptr->smask = bsptr->cmask = 0; SpriteInitError = 1; return 0;
		}
		CreateColorMask(bsptr->cmask, spge, sx, sy, sw, sh, CBaseCol);
	}
	else
		bsptr->cmask = 0;
	return 1;
}

void CreateBSpriteAlias(BSPRITE *bsptr, BYTE spge, int sx, int sy, int sw, int sh, BSPRITE *bsalias)
{
	bsptr->spge = spge;
	bsptr->sx = sx; bsptr->sy = sy;
	bsptr->sw = sw; bsptr->sh = sh;
	bsptr->alias = 1;
	bsptr->smask = bsalias->smask;
	bsptr->cmask = bsalias->cmask;
}

void DestroyBSprite(BSPRITE *bsptr)
{
	if (!bsptr->alias) if (bsptr->smask) delete[] bsptr->smask;
}

void BSprite(BSPRITE *bsptr, BYTE tpge, int tx, int ty)
{
	BMove(bsptr->spge, bsptr->sx, bsptr->sy, tpge, tx, ty, bsptr->sw, bsptr->sh, bsptr->smask);
}

void BSpriteY(BSPRITE *bsptr, BYTE tpge, int tx, int ty, int yoff, int nhgt)
{
	BMove(bsptr->spge, bsptr->sx, bsptr->sy + yoff, tpge, tx, ty + yoff, bsptr->sw, nhgt, bsptr->smask + (bsptr->sw*(bsptr->sh - nhgt) / 8));
}

inline void BSpriteC(BSPRITE *bsptr, BYTE tpge, int tx, int ty, BYTE col)
{
	BMove(bsptr->spge, bsptr->sx, bsptr->sy, tpge, tx, ty, bsptr->sw, bsptr->sh, bsptr->smask);
	if (col > 0) if (bsptr->cmask) // Sprite Color Base Max 12 x 9
		BMove(bsptr->spge, CBaseX + 12 * col, CBaseY, tpge, tx, ty, bsptr->sw, bsptr->sh, bsptr->cmask);
	//BMoveSwapColor(bsptr->spge, bsptr->sx, bsptr->sy, tpge, tx, ty, bsptr->sw, bsptr->sh, CBaseCol, col);
}

BYTE CreateASprite(ASPRITE *asptr, BYTE spge, int sx, int sy, int sw, int sh, BYTE cspr = 0, BYTE clrd = 0)
{
	int cnt;
	if (cspr) CreateCSprite(spge, sx, sy, sw, sh);
	for (cnt = 0; cnt < 4; cnt++)
		if (!CreateBSprite(&(asptr->shift[cnt]), spge, sx + (sw + 4)*cnt, sy, sw + 4, sh, clrd))
		{
			SpriteInitError = 1; return 0;
		}
	return 1;
}

void CreateASpriteAlias(ASPRITE *asptr, BYTE spge, int sx, int sy, int sw, int sh, ASPRITE *asalias, BYTE cspr = 0)
{
	int cnt;
	if (cspr) CreateCSprite(spge, sx, sy, sw, sh);
	for (cnt = 0; cnt < 4; cnt++)
		CreateBSpriteAlias(&(asptr->shift[cnt]), spge, sx + (sw + 4)*cnt, sy, sw + 4, sh, &(asalias->shift[cnt]));
}

void DestroyASprite(ASPRITE *asptr)
{
	int cnt;
	for (cnt = 0; cnt < 4; cnt++) DestroyBSprite(&(asptr->shift[cnt]));
}

void ASprite(ASPRITE *asptr, BYTE tpage, int tx, int ty)
{
	BSprite(&(asptr->shift[tx % 4]), tpage, (tx >> 2) << 2, ty);
}

void ASpriteC(ASPRITE *asptr, BYTE tpage, int tx, int ty, BYTE col)
{
	BSpriteC(&(asptr->shift[tx % 4]), tpage, (tx >> 2) << 2, ty, col);
}