/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  VGA4 Module  by M.Bender

#include <SDL.h>
//#include "STANDARD.H"
#include "SDLmain.h"
#include "vga4.h"

int MaxX = 319, MaxY = 199;

#define VGA_PAGECOUNT 4
//#define VERTICAL_RETRACE_MS 10 // Vertical Retrace Emulation; Millisekunden die in PPage gewartet werden

SDL_Surface* vgaPages[VGA_PAGECOUNT];
BYTE currentPage = 0;
BYTE videoPage = 0;


int IsVGA(void)
{
	return 1;
}

int InitVGA(void)
{
	for (BYTE i = 0; i < VGA_PAGECOUNT; i++)
	{
		vgaPages[i] = SDL_CreateRGBSurface(0, MaxX + 1, MaxY + 1, 8, 0, 0, 0, 0); // 8-Bit hat Palette.
		SDL_SetSurfaceBlendMode(vgaPages[i], SDL_BLENDMODE_NONE);
	}
	return 0;
}

void CloseVGA(void)
{
	for (int i = 0; i < VGA_PAGECOUNT; i++)
		SDL_FreeSurface(vgaPages[i]);
}

void SPixA(int x, int y, BYTE col)
{
	SDL_Surface* surf = vgaPages[currentPage];
	if (x >= surf->w || x < 0) return;
	if (y >= surf->h || y < 0) return;
	uint8_t* pcol = ((uint8_t*)surf->pixels) + surf->pitch * y + surf->format->BytesPerPixel * x;
	*pcol = col;
}

BYTE GPixA(int x, int y)
{
	SDL_Surface* surf = vgaPages[currentPage];
	return ((uint8_t*)surf->pixels)[surf->pitch * y + surf->format->BytesPerPixel * x];
}

void LPage(BYTE page)
{
	currentPage = page;
}

// Prepare Page? Present Page?
void PPage(BYTE page)
{
	videoPage = page;
	UpdateScreen();
	//SDL_Delay(VERTICAL_RETRACE_MS);
}

void UpdateScreen()
{
	//HandleSDLEvents();

	// Blitten funktioniert hier schon wieder nicht. Scheint unzuverlässig zu sein.
	//SDL_BlitScaled(vgaPages[videoPage], NULL, sdlScreenSurface, &destRect);

	int w = vgaPages[videoPage]->w;
	int h = vgaPages[videoPage]->h;

	BYTE *sourcePixels = (BYTE*)vgaPages[videoPage]->pixels;
	Uint32 *destPixels = (Uint32*)SdlScreenSurface->pixels;

	int sourceBpp = vgaPages[videoPage]->format->BytesPerPixel;
	//int sourceLineskip = w * sourceBpp;

	int destBpp = SdlScreenSurface->format->BytesPerPixel;
	int destPixelSkip = ScreenScaleFactor;
	int destLineSkip = w * destPixelSkip;

	SDL_Color *vgaColors = vgaPages[videoPage]->format->palette->colors;

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			Uint32 *p = destPixels;
			SDL_Color col = vgaColors[*sourcePixels];
			Uint32 destCol = (col.a << 8 * 3) + (col.r << 8 * 2) + (col.g << 8 * 1) + col.b;
			for (int yy = 0; yy < ScreenScaleFactor; yy++)
			{
				for (int xx = 0; xx < ScreenScaleFactor; xx++)
				{
					*p = destCol;
					p += 1;
				}
				p += destLineSkip - destPixelSkip;
			}

			destPixels += destPixelSkip;
			sourcePixels += sourceBpp;
		}

		for (int yy = 1; yy < ScreenScaleFactor; yy++)
		{
			destPixels += destLineSkip;
		}
	}

	SDL_UpdateWindowSurface(SdlWindow);
}

void BMove(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE* bmskp)
{
	//B4Move(fpge, fx, fy, tpge, tx, ty, wdt, hgt);

	// Mit Bitmask.

	int screenWdt = vgaPages[fpge]->w;
	int screenHgt = vgaPages[fpge]->h;
	int bpp = vgaPages[fpge]->format->BytesPerPixel;
	int lineSkip = (screenWdt - wdt) * bpp;

	if (wdt > screenWdt - fx) wdt = screenWdt - fx;
	if (wdt > screenWdt - tx) wdt = screenWdt - tx;
	if (hgt > screenHgt - fy) hgt = screenHgt - fy;
	if (hgt > screenHgt - ty) hgt = screenHgt - fy;

	Uint8* psource = (Uint8*)vgaPages[fpge]->pixels;
	psource += (screenWdt * fy + fx) * bpp;
	Uint8* pdest = (Uint8*)vgaPages[tpge]->pixels;
	pdest += (screenWdt * ty + tx) * bpp;

	BYTE bit = 0;

	for (int h = 0; h < hgt; h++)
	{
		for (int w = 0; w < wdt; w++)
		{
			if (bmskp != NULL)
			{
				if (((*bmskp >> bit) & 0b00000001) != 0)
					*pdest = *psource;
				bit++;
				if (bit > 7)
				{
					bit = 0;
					bmskp++;
				}
			}
			else
				*pdest = *psource;
			psource++;
			pdest++;
		}

		psource += lineSkip;
		pdest += lineSkip;
	}
}

// Neue Funktion. Einfacher als Colormask zu implementieren: Vertausche einfach die Farben auf der Palette.
// Kann ich wieder rückgängig machen, Bitmask implementiert
void BMoveSwapColor(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, BYTE wdt, BYTE hgt, BYTE oldcol, BYTE newcol)
{
	SDL_Palette *pal = vgaPages[fpge]->format->palette;
	SDL_Color tempColor = pal->colors[oldcol];
	pal->colors[oldcol] = pal->colors[newcol];
	B4Move(fpge, fx, fy, tpge, tx, ty, wdt, hgt);
	pal->colors[oldcol] = tempColor;
}

// 4 im Namen: x-Koordinaten und wdt sind nur ein Viertel!!!
void B4Move(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt)
{
	// Blit funktioniert irgendwie nicht. (warum.)
	/*
	SDL_Rect sourceRect;
	sourceRect.x = fx * 4;
	sourceRect.y = fy;
	sourceRect.w = wdt * 4;
	sourceRect.h = hgt;
	SDL_Rect destRect;
	destRect.x = tx;
	destRect.y = ty;
	SDL_BlitSurface(vgaPages[fpge], &sourceRect, vgaPages[tpge], &destRect);
	*/

	wdt *= 4;
	fx *= 4;
	tx *= 4;
	BMove(fpge, fx, fy, tpge, tx, ty, wdt, hgt, NULL);
}

void CPage(BYTE fpge, BYTE tpge)
{
	SDL_Surface* surfFrom = vgaPages[fpge];
	SDL_Surface* surfTo = vgaPages[tpge];
	SDL_memcpy(surfTo->pixels, surfFrom->pixels, surfFrom->w * surfFrom->h * surfFrom->format->BytesPerPixel);
	//SDL_memcpy(surfTo->format->palette->colors, surfFrom->format->palette->colors, sizeof(SDL_Color) * surfFrom->format->palette->ncolors);
}

void CPageY(BYTE fpge, BYTE fy, BYTE tpge, BYTE ty, BYTE hgt)
{
	B4Move(fpge, 0, fy, tpge, 0, ty, 80, hgt);
	//B4Move(fpge, 40, fy, tpge, 40, ty, 40, hgt);
}

//--------------------------------- DAC --------------------------------------

//typedef union { struct { BYTE red, green, blue; } b; BYTE RGB[3]; } COLOR;

// Leider kann man nicht Color auf SDL_Color casten weil SDL_Color noch ein Alpha-Byte hat.
// Deshalb werden die Farben jetzt einfach konvertiert.

#define DACColorScale 4
void GetDAC(int first, int num, void *bufptr)
{
	/*union REGS Regs;
	struct SREGS SRegs;
	Regs.x.ax = 0x1017;
	Regs.x.bx = first;
	Regs.x.cx = num;
	Regs.x.dx = FP_OFF(bufptr);
	SRegs.es = FP_SEG(bufptr);
	int86x(0x10, &Regs, &Regs, &SRegs);*/

	for (int i = 0; i < num; i++)
	{
		SDL_Palette *pal = vgaPages[videoPage]->format->palette;
		SDL_Color color = pal->colors[(first + i) % pal->ncolors];
		uint8_t* outCol = (uint8_t*)bufptr + i * 3;
		outCol[0] = color.r / DACColorScale;
		outCol[1] = color.g / DACColorScale;
		outCol[2] = color.b / DACColorScale;
	}
}

void SetDAC(int first, int num, void *bufptr)
{
	/*union REGS Regs;
	struct SREGS SRegs;
	Regs.x.ax = 0x1012;
	Regs.x.bx = first;
	Regs.x.cx = num;
	Regs.x.dx = FP_OFF(bufptr);
	SRegs.es = FP_SEG(bufptr);
	int86x(0x10, &Regs, &Regs, &SRegs);*/

	SDL_Color colors[256];
	for (int i = 0; i < num; i++)
	{
		Uint8* pcol = (Uint8*)bufptr + i * 3;
		colors[i].r = pcol[0] * DACColorScale;
		colors[i].g = pcol[1] * DACColorScale;
		colors[i].b = pcol[2] * DACColorScale;
		colors[i].a = 0xFF;
	}

	for (int i = 0; i < VGA_PAGECOUNT; i++)
	{
		SDL_SetPaletteColors(vgaPages[i]->format->palette, colors, first, num);
	}
}

void SetColor(COLOR *pal, BYTE col, BYTE red, BYTE green, BYTE blue)
{
	pal[col].b.red = red; pal[col].b.green = green; pal[col].b.blue = blue;
}

void FadeColor(COLOR *pal, BYTE fcol, BYTE tcol)
{
	long cnt, ccnt, flen;
	long cdis[3];

	if (!pal || (fcol >= tcol)) return;

	flen = tcol - fcol;
	for (ccnt = 0; ccnt < 3; ccnt++)
		cdis[ccnt] = pal[tcol].RGB[ccnt] - pal[fcol].RGB[ccnt];

	for (cnt = 0; cnt <= flen; cnt++)
		for (ccnt = 0; ccnt < 3; ccnt++)
			pal[fcol + cnt].RGB[ccnt] = pal[fcol].RGB[ccnt] + cdis[ccnt] * cnt / flen;

}

COLOR FadeFPal[256];
COLOR *FadeTPal;
int FadeSteps, FadeCStep;

void InitFadeDAC(COLOR *tpal, int steps)
{
	GetDAC(0, 256, FadeFPal);
	FadeTPal = tpal;
	FadeSteps = steps; FadeCStep = 0;
}

BYTE FadeDAC(void)
{
	COLOR cpal[256];
	int cnt2, cnt3;
	FadeCStep++; if (FadeCStep > FadeSteps) return 0;
	for (cnt2 = 0; cnt2 < 256; cnt2++)
		for (cnt3 = 0; cnt3 < 3; cnt3++)
			cpal[cnt2].RGB[cnt3] = FadeFPal[cnt2].RGB[cnt3] + FadeCStep*(FadeTPal[cnt2].RGB[cnt3] - FadeFPal[cnt2].RGB[cnt3]) / FadeSteps;
	SetDAC(0, 256, cpal);
	return 1;
}
