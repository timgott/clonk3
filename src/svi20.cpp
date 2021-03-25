/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  SVI 2.0  STANDARD VISUAL INTERFACE Library (C) by M.Bender

// 1.0 June      1994
// 1.1 July-Sept 1994
// 1.2 Sept-Oct  1994
// 1.3 October   1994
// 1.4 December  1994
// 1.5 February  1995
// 1.6 April     1995 (HyperHelp)
// 1.7 May       1995 (ExternCall)
// 2.0 Oct 95-June 96 (All new code, Object Orientation, PowerHelp 1.1)

//--------------------------- Include Headers --------------------------------

#include <stdio.h>
#include <SDL2/SDL.h>
#include <inttypes.h>

#include "standard.h"
#include "vga4.h"
#include "std_gfx.h"
#include "stdfile.h"

#include "svi20typ.h"

#include "UserInput.h"
#include "SDLmain.h"


extern char OSTR[500]; // Use extern OSTR?

//------------------------- Data Usage Definitions ---------------------------

#define BTXRVal    bdata[0]
#define BTNoULRep  bdata[1]

#define FBTitle    bdata[0]

#define FRTitle    bdata[0]
#define FRFCol     idata[0]
#define FRBCol     idata[1]
#define FRTBCol    idata[2]

#define PCSrcPage  bdata[0]
#define PCTitle    bdata[1]
#define PCSTabWdt  bdata[2]
#define PCSrcX     idata[0]
#define PCSrcY     idata[1]

#define TXForm     bdata[0]
#define TXCType    bdata[1]
#define TXTitle    bdata[2]
#define TXFCol     idata[0]
#define TXBCol     idata[1]

#define ICSrcPage  bdata[0]
#define ICSTabWdt  bdata[1]
#define ICTitle    bdata[2]
#define ICSrcX	   idata[0]
#define ICSrcY	   idata[1]
#define ICSrcWdt   idata[2]
#define ICSrcHgt   idata[3]

#define SCTitle    bdata[0] // HScroll only
#define SCRangeLo    idata[0]
#define SCRangeHi    idata[1]
#define SCSTEP     idata[2]
#define SCKSTEP    idata[3]

#define LBListID   bdata[0]
#define LBCellNumX bdata[1]
#define LBCellNumY bdata[2]
#define LBCellWdt  idata[0]
#define LBCellHgt  idata[1]
#define LB1stCell  idata[2]
#define LBClNum    idata[3]
#define LBSelect   idata[4]

#define SBSrcPage  bdata[0]
#define SBSrcWdt   bdata[1]
#define SBSrcHgt   bdata[2]
#define SBRngLo    idata[0]
#define SBRngHi    idata[1]
#define SBSrcX     idata[2]
#define SBSrcY     idata[3]
#define SBTitle    idata[4]

#define TBTitle    bdata[0]
#define TBXRVal    bdata[1]
#define TBMaxLen   idata[0]
#define TBCursor   idata[1]

#define VBTitle    bdata[0]
#define VBUnit	   bdata[1]
#define VBLongU    bdata[2]
#define VBRngLo    idata[0]
#define VBRngHi    idata[1]
#define VBStep     idata[2]
#define VBDesign   idata[3]

#define SLTitle    bdata[0]
#define SLElNum    idata[0]

#define HTLineIndex idata[0]
#define HTMaxLineI  idata[1]

//--------------------- KbObjectAction KbSpc-Definitions ---------------------

#define KBNONE   0
#define KBSTN    1
#define KBENTER  2
#define KBLEFT   3
#define KBRIGHT  4
#define KBUP     5
#define KBDOWN   6
#define KBHOME   7
#define KBEND    8
#define KBBACK   9
#define KBDEL   10
#define KBPGUP  11
#define KBPGDN  12

//--------------------------- Global Variables -------------------------------

BYTE SVIPage;

WORD SVIInitErrors = 0;

BYTE SVIRunning = 0, HelpRunning = 0;

WINDOW *FirstWindow = NULL;
ULINK  *FirstULink = NULL;

void(*ExtDrawLBC)(int, int, int, int, BYTE) = NULL;
BYTE(*ExtLBCAction)(int, int, int) = NULL;

BYTE GrayMap[256];

int IMsgX, IMsgY;

BYTE OpenHelp;


//---------------------------- Misc Functions --------------------------------

BYTE DefCols[16 * 3] = { 0, 0, 0, 13,13,13, 22,22,22, 36,36,36, 45,45,45, 52,52,52, 63,63,63,
			20, 0, 0,  0,20, 0,  0, 0,20, 50, 0, 0,  0,50, 0,  0, 0,50, 63,53, 0,
			 0, 0, 0, 64,64,64 };

void DefColors(void)
{
	SetDAC(0, 16, DefCols);
}

void DefCols2Pal(BYTE *tpal)
{
	MemCopy(DefCols, tpal, 16 * 3);
}

//--------------------------- Mouse Functions --------------------------------

BYTE CBA[8];
BYTE CBAX, CBAY;

BYTE MSON = 0;
BYTE MouseType; // Number of buttons

int InitMouse(void)
{
	/*union REGS Regs;
	Regs.x.ax = 0x0000;
	int86(0x33, &Regs, &Regs);
	if (Regs.x.ax == 0xffff)
	{
		MSON = 1; return (Regs.x.bx);
	} // returns number of mousebuttons
	MSON = 0;
	return 0; // zero if failed*/
	MSON = 1;
	return 3; // Ja denk mal man hat ne 3 knopf maus oder sch�tz ich mal wenn nich dann nich oder so ende punkt
}

int MouseState(int *x, int *y)
{
	int but = SDL_GetMouseState(x, y);
	if (x != nullptr) *x /= PortScreenScaleFactor;
	if (y != nullptr) *y /= PortScreenScaleFactor;
	return but;
}

BYTE Mousebut(void)
{
	if (MSON) // Ist ja jetzt eigentlich nutzlos aber gut.
	{
		SDL_PumpEvents();
		return MouseState(NULL, NULL);
	}
	return 0;
}

int MouseX(void)
{
	if (MSON)
	{
		int x;
		MouseState(&x, NULL);
		return x;
	}
	return 0;
}

int MouseY(void)
{
	if (MSON)
	{
		int y;
		MouseState(NULL, &y);
		return y;
	}
	return 0;
}

void SetMouseLoc(int tx, int ty)
{
	if (MSON)
	{
		SDL_WarpMouseInWindow(SdlWindow, tx, ty);
	}
}

void SetMouseRange(int x1, int y1, int x2, int y2) // Kein Plan
{
	/*union REGS Regs;
	if (MSON)
	{
		Regs.x.ax = 0x0007; Regs.x.cx = x1; Regs.x.dx = x2;
		int86(0x33, &Regs, &Regs);
		Regs.x.ax = 0x0008; Regs.x.cx = y1; Regs.x.dx = y2;
		int86(0x33, &Regs, &Regs);
	}*/
}

void DefCBA(int cnum)
{
	static char *def[92] = {
		  "   x    ",
		  "   x    ",
		  "   x    ",
		  "xxx xxx ",
		  "   x    ",
		  "   x    ",
		  "   x    ",
		  "        ",
		  "xxxx    ",
		  "xx      ",
		  "x x     ",
		  "x  x    ",
		  "    x   ",
		  "        ",
		  "        ",
		  "        ",
		  "xxxxx   ",
		  "x   x   ",
		  "x   x   ",
		  "x   x   ",
		  "xxxxx   ",
		  "        ",
		  "        ",
		  "        ",
		  "x     x ",
		  " x   x  ",
		  "  x x   ",
		  "   x    ",
		  "  x x   ",
		  " x   x  ",
		  "x     x ",
		  "        ",
		  "        ",
		  " xxxxx  ",
		  " x  xx  ",
		  " x   x  ",
		  " xx  x  ",
		  " xxxxx  ",
		  "        ",
		  "        ",
		  "xxxxxx  ",
		  "x       ",
		  "x       ",
		  "x       ",
		  "x       ",
		  "x       ",
		  "        ",
		  "        ",
		  "xxxxx   ",
		  "x       ",
		  "x       ",
		  "x       ",
		  "xxxxx   ",
		  "        ",
		  "        ",
		  "        ",
		  "xxxxx   ",
		  "    x   ",
		  "    x   ",
		  "    x   ",
		  "xxxxx   ",
		  "        ",
		  "        ",
		  "        ",
		  "  xxx   ",
		  " x   x  ",
		  "x     x ",
		  "x     x ",
		  "x     x ",
		  " x   x  ",
		  "  xxx   ",
		  "        " };

	static BYTE cbaxydata[9][2] = { 3,3,0,0,2,2,3,3,3,3,0,0,2,2,2,2,3,3 };

	char *line;
	int lcnt, bcnt;
	for (lcnt = 0; lcnt < 8; lcnt++)
	{
		CBA[lcnt] = 0;
		line = def[8 * cnum + lcnt];
		for (bcnt = 0; bcnt < 8; bcnt++)
		{
			CBA[lcnt] <<= 1;
			if (*(line + bcnt) != 32) CBA[lcnt] |= 1;
		}
	}
	CBAX = cbaxydata[cnum][0]; CBAY = cbaxydata[cnum][1];
}

void SetMouseCursor(int type)
{
	static int mctype;
	if (type != mctype)
	{
		DefCBA(type); mctype = type;
	}
}

void DrawMouse(int dmx, int dmy)
{
	BYTE lcnt, bcnt, lbuf;
	for (lcnt = 0; lcnt < 8; lcnt++)
	{
		lbuf = CBA[lcnt];
		for (bcnt = 0; bcnt < 8; bcnt++)
		{
			if (lbuf & 128)
			{
				SPixF(dmx + bcnt - CBAX + 1, dmy + lcnt - CBAY + 1, CGray1);
				SPixF(dmx + bcnt - CBAX, dmy + lcnt - CBAY, CWhite);
			}
			lbuf <<= 1;
		}
	}
}

BYTE InitSVIMouse(void)
{
	if (!MSON) if (!(MouseType = InitMouse())) return 0;
	SetMouseRange(0, 0, 639, 199);
	SetMouseCursor(1);
	return 1;
}

//------------------------ System Error Handling -----------------------------

void RedrawArea(int x1, int y1, int x2, int y2);

void SystemError(char *txt)
{
	int twdt, thgt;
	LPage(SVIPage); PPage(SVIPage);
	TOutSt(txt, &twdt, &thgt);
	twdt = Max(twdt, 20);
	DefColors();
	DBox(160 - twdt * 2 - 10, 100 - thgt * 3 - 10, 160 + twdt * 2 + 10, 100 + thgt * 2 + 10, CGray5);
	DFrame(160 - twdt * 2 - 10, 100 - thgt * 3 - 10, 160 + twdt * 2 + 10, 100 + thgt * 2 + 10, SVIRunning ? CRed : CGray1);
	SOut("SVI 2.0 System Error", 160, 100 - thgt * 3 - 10 + 4, CGray2, -1, 1);
	TOut(txt, 160, 100 - thgt * 3 - 10 + 14, CGray1, -1, 1);
	WaitForInput();
	RedrawArea(160 - twdt * 2 - 10, 100 - thgt * 3 - 10, 160 + twdt * 2 + 10, 100 + thgt * 2 + 10);
}

BYTE InitErrorCheck(void)
{
	if (SVIInitErrors)
	{
		SystemError("Errors have occured while trying|to initialize SVI objects.|Most likely due to insufficient memory.");
		SVIInitErrors = 0; // yeah?
		return 1;
	}
	return 0;
}

//---------------------------- External Callers -------------------------------

void DrawListBoxCell(OBJECT *obj, int num, BYTE sel)
{
	int c1x = obj->inwin->x + obj->x, c1y = obj->inwin->y + obj->y;

	//if (obj->inwin!=FirstWindow) return; // Old style not-covered check

	if (!ExtDrawLBC) { SystemError("exfunc drawlistboxcell not set"); return; }

	num -= obj->LB1stCell; // Relativate num index to 1st cell in list box

	if (Inside(num, 0, obj->LBCellNumX*obj->LBCellNumY - 1))
		ExtDrawLBC(obj->LBListID, num + obj->LB1stCell, c1x + 1 + obj->LBCellWdt*(num%obj->LBCellNumX), c1y + 1 + obj->LBCellHgt*(num / obj->LBCellNumX), sel);
}

BYTE ListBoxAction(OBJECT *obj, int aid)
{
	BYTE xrval;
	if (!ExtLBCAction) { SystemError("extlbcaction not set"); return 0; }
	xrval = ExtLBCAction(obj->LBListID, obj->LBSelect, aid);
	DrawListBoxCell(obj, obj->LBSelect, 1);
	return xrval;
}

/*extern void GameIntegC(void);

void IntegrityC(int fromw)
  {
  WINDOW *cwin; OBJECT *cobj; ULINK *clnk;
  int wcnt=0,ocnt=0,lcnt=0;
  DWORD etmem=0;
  char incostr[100];

  GameIntegC();

  if (FirstULink!=SaveFULink) SystemError("INC: FirstULink modified illegally");
  if (FirstWindow!=SaveFWin) SystemError("INC: FirstWindow modified illegally");

  for (cwin=FirstWindow; cwin; cwin=cwin->next,wcnt++)
	{
	if (!Inside(cwin->type,0,4))
	  {
	  sprintf(incostr,"INC %d: illegal window type",fromw);
	  SystemError(incostr);
	  }
	for (cobj=cwin->fobj; cobj; cobj=cobj->next,ocnt++)
	  {
	  if (!Inside(cobj->type,0,14))
	  {
	  sprintf(incostr,"INC %d: illegal object type",fromw);
	  SystemError(incostr);
	  }
	  if ((cobj->type==TEXT) && (cobj->TXCType==1))
	etmem+=SLen(cobj->bptr);
	  }
	}
  for (clnk=FirstULink; clnk; clnk=clnk->next,lcnt++)
	if (!Inside(clnk->type,0,10))
	  {
	  sprintf(incostr,"INC %d: illegal ulink type",fromw);
	  SystemError(incostr);
	  }

  //sprintf(incostr,"wins %d  objs  %d  links %d  xtbuf %iu      ",wcnt,ocnt,lcnt,etmem);
  //LPage(SVIPage);
  //SOut(incostr,0,0,CWhite,CBlack);
  }*/

  //---------------------------- Graphic Support -------------------------------

void InitSVIGrayMap(void)
{
	BYTE tgray[32];
	COLOR cpal[256];
	int cnt, cnt2;
	BYTE closest = 0;
	GetDAC(0, 256, cpal);
	// Find 32 gray scales
	for (cnt = 0; cnt < 32; cnt++)
	{
		for (cnt2 = 0; cnt2 < 256; cnt2++)
			if (cpal[cnt2].b.red == cpal[cnt2].b.green) if (cpal[cnt2].b.green == cpal[cnt2].b.blue)
				if (Abs(cnt * 2 - cpal[cnt2].b.red) < Abs(cnt * 2 - cpal[closest].b.red))
					closest = cnt2;
		tgray[cnt] = closest;
	}
	// Map 256 colors to 32 gray scales
	for (cnt = 0; cnt < 256; cnt++)
		GrayMap[cnt] = tgray[(cpal[cnt].b.red + cpal[cnt].b.green + cpal[cnt].b.blue) / 6];
}

void SVIGCopy(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE gray)
{
	BYTE linebuf[320];
	BYTE *bufptr;
	register int xcnt, ycnt;
	for (ycnt = 0; ycnt < hgt; ycnt++)
	{
		LPage(fpge); bufptr = linebuf;
		for (xcnt = 0; xcnt < wdt; xcnt++, bufptr++) *bufptr = GPixF(fx + xcnt, fy);
		LPage(tpge); bufptr = linebuf;
		if (gray)
		{
			for (xcnt = 0; xcnt < wdt; xcnt++, bufptr++)
				if (*bufptr) SPixF(tx + xcnt, ty, GrayMap[*bufptr]);
		}
		else
		{
			for (xcnt = 0; xcnt < wdt; xcnt++, bufptr++)
				if (*bufptr) SPixF(tx + xcnt, ty, *bufptr);
		}
		fy++; ty++;
	}
	LPage(tpge);
}

void DrawStd3D(int x1, int y1, int x2, int y2, BYTE invert)
{
	DFrame(x1, y1, x2, y2, invert ? CGray2 : CGray4);
	DFrame(x1 + 1, y1 + 1, x2, y2, invert ? CGray4 : CGray2);
	SPixF(x1, y1, invert ? CGray1 : CGray5); SPixF(x2, y1, CGray3);
	SPixF(x1, y2, CGray3); SPixF(x2, y2, invert ? CGray5 : CGray1);
	DBox(x1 + 1, y1 + 1, x2 - 1, y2 - 1, CGray3);
}

//--------------------------- Process Box ------------------------------------

BYTE ProcessBPage = 0;
//int ProcessBLine=-1;

void InitProcessB(const char *msg, BYTE page = SVIPage)
{
	int tx = 80, ty = 80, wdt = 160, hgt = 19;
	ProcessBPage = page;
	//ProcessBLine=0;
	LPage(ProcessBPage);
	DrawStd3D(tx, ty, tx + wdt, ty + hgt, 0);
	DBox(tx + wdt + 1, ty + 2, tx + wdt + 2, ty + hgt + 2, CGray1);
	DBox(tx + 2, ty + hgt + 1, tx + wdt + 2, ty + hgt + 2, CGray1);
	DBox(tx + 3, ty + 3, tx + wdt - 3, ty + hgt - 3 - 7, CGray5);
	DFrame(tx + 2, ty + 2, tx + wdt - 2, ty + hgt - 2 - 7, CGray2);
	DBox(tx + 2, ty + hgt - 3 - 4, tx + wdt - 2, ty + hgt - 2, CGray4);
	SOut(msg, tx + 4, ty + 4, CGray1);
}

/*void DeInitProcessB(void)
  {
  ProcessBLine=-1;
  }*/

void ProcessB(int val)
{
	//if (ProcessBLine==-1) return;
	LPage(ProcessBPage);
	/*if (ProcessBLine>3)
	  {
	  SVIGCopy(ProcessBPage,80+3,80+4+6*1,ProcessBPage,80+3,80+4,152,3*6,0);
	  DBox(80+4,80+4+6*3,80+160-4,80+4+6*3+5,CGray5);
	  ProcessBLine=3;
	  }
	SOut(msg,80+4,80+4+6*ProcessBLine,CGray1);
	ProcessBLine++;*/
	DBox(80 + 3, 80 + 19 - 3 - 3, 80 + 3 + BoundBy(val * 154 / 100, 0, 154), 80 + 19 - 3, CRed);
	UpdateScreen();
}

//---------------------------- Object Graphics ---------------------------------

int HScrollMarkPos(OBJECT *obj)
{
	return ((long)(obj->wdt - 5)) * ((long)(*obj->iptr - obj->SCRangeLo)) / Max(obj->SCRangeHi - obj->SCRangeLo, 1);
}

void DrawHScrollMark(OBJECT *obj)
{
	int tx, ty;
	tx = obj->inwin->x + obj->x + HScrollMarkPos(obj);
	ty = obj->inwin->y + obj->y;
	DrawStd3D(tx, ty, tx + 4, ty + obj->hgt - 1, 0);
}

void RemoveHScrollMark(OBJECT *obj)
{
	int tx, ty;
	tx = obj->inwin->x + obj->x + HScrollMarkPos(obj);
	ty = obj->inwin->y + obj->y;
	DFrame(tx, ty, tx + 4, ty + obj->hgt - 1, CGray3);
	DFrame(tx, ty + 1, tx + 4, ty + 1, CGray2);
	DBox(tx, ty + 2, tx + 4, ty + obj->hgt - 2, CGray4);
	if (tx == obj->inwin->x + obj->x) DFrame(tx, ty, tx, ty + obj->hgt - 1, CGray3);
	if (tx < obj->inwin->x + obj->x + 2) DFrame(obj->inwin->x + obj->x + 1, obj->inwin->y + obj->y + 1, obj->inwin->x + obj->x + 1, obj->inwin->y + obj->y + obj->hgt - 2, CGray2);
	if (tx + 4 == obj->inwin->x + obj->x + obj->wdt - 1) DFrame(tx + 4, ty, tx + 4, ty + obj->hgt - 1, CGray3);
}

void DrawHScroll(OBJECT *obj)
{
	int wx, wy;
	wx = obj->inwin->x; wy = obj->inwin->y;
	if (obj->SCTitle) SOut(obj->title, wx + obj->x, wy + obj->y - 6, CGray1, -1, 0);
	DFrame(wx + obj->x + 1, wy + obj->y + 1, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, CGray2);
	DFrame(wx + obj->x, wy + obj->y, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, CGray3);
	DBox(wx + obj->x + 2, wy + obj->y + 2, wx + obj->x + obj->wdt - 2, wy + obj->y + obj->hgt - 2, CGray4);
	if (obj->hid != HIDE) DrawHScrollMark(obj);
}

int VScrollMarkPos(OBJECT *obj)
{
	return ((long)(obj->hgt - 5)) * ((long)(*obj->iptr - obj->SCRangeLo)) / Max(obj->SCRangeHi - obj->SCRangeLo, 1);
}

void DrawVScrollMark(OBJECT *obj)
{
	int tx, ty;
	tx = obj->inwin->x + obj->x;
	ty = obj->inwin->y + obj->y + VScrollMarkPos(obj);
	DrawStd3D(tx, ty, tx + obj->wdt - 1, ty + 4, 0);
}

void RemoveVScrollMark(OBJECT *obj)
{
	int tx, ty, wy, wx;
	tx = obj->inwin->x + obj->x;
	ty = obj->inwin->y + obj->y + VScrollMarkPos(obj);
	wy = obj->inwin->y; wx = obj->inwin->x;
	DFrame(tx, ty, tx + obj->wdt - 1, ty + 4, CGray3);
	DFrame(tx + 1, ty, tx + 1, ty + 4, CGray2);
	DBox(tx + 2, ty, tx + obj->wdt - 2, ty + 4, CGray4);
	if (ty == wy + obj->y) DFrame(tx, ty, tx + obj->wdt - 1, ty, CGray3);
	if (ty < wy + obj->y + 2) DFrame(wx + obj->x + 1, wy + obj->y + 1, wx + obj->x + obj->wdt - 2, wy + obj->y + 1, CGray2);
	if (ty + 4 == wy + obj->y + obj->hgt - 1) DFrame(tx, ty + 4, tx + obj->wdt - 1, ty + 4, CGray3);
}

void DrawVScroll(OBJECT *obj)
{
	int wx, wy; wx = obj->inwin->x; wy = obj->inwin->y;
	DFrame(wx + obj->x + 1, wy + obj->y + 1, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, CGray2);
	DFrame(wx + obj->x, wy + obj->y, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, CGray3);
	DBox(wx + obj->x + 2, wy + obj->y + 2, wx + obj->x + obj->wdt - 2, wy + obj->y + obj->hgt - 2, CGray4);
	if (obj->hid != HIDE) DrawVScrollMark(obj);
}

void DrawButton(OBJECT *obj, BYTE pressed)
{
	BYTE tcol;
	int wx, wy; wx = obj->inwin->x; wy = obj->inwin->y;
	DrawStd3D(wx + obj->x, wy + obj->y, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, pressed);
	if (obj->hid == HIDE) tcol = CGray4;  else tcol = (pressed ? CDRed : CGray1);
	SOut(obj->title, wx + obj->x + obj->wdt / 2 + 1, wy + obj->y + obj->hgt / 2 - 2, tcol, -1, 1);
}

void DrawValBox(OBJECT *obj, BYTE update)
{
	int wx, wy; wx = obj->inwin->x; wy = obj->inwin->y;
	if (!obj->VBLongU)
		sprintf(OSTR, "%d%c", *obj->iptr, obj->VBUnit);
	else
		sprintf(OSTR, "%" PRIu32 "%c", *((DWORD*)obj->iptr), obj->VBUnit);
	switch (obj->VBDesign)
	{
	case 0:
		if (!update)
		{
			if (obj->VBTitle) SOut(obj->title, wx + obj->x, wy + obj->y - 6, CGray1, -1);
			DFrame(wx + obj->x, wy + obj->y, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, CGray1);
		}
		DBox(wx + obj->x + 1, wy + obj->y + 1, wx + obj->x + obj->wdt - 2, wy + obj->y + obj->hgt - 1 - 1, (obj->hid == HIDE) ? CGray4 : CWhite);
		SOut(OSTR, wx + obj->x + obj->wdt - 2, wy + obj->y + 2, (obj->hid == HIDE) ? CGray3 : CGray1, -1, 2);
		break;
	case 1:
		if (!update)
			if (obj->VBTitle) SOut(obj->title, wx + obj->x, wy + obj->y - 6, CGray1, -1);
		DBox(wx + obj->x, wy + obj->y, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, (obj->hid == HIDE) ? CGray3 : CGray4);
		SOut(OSTR, wx + obj->x + obj->wdt - 1, wy + obj->y + 1, (obj->hid == HIDE) ? CGray2 : CGray1, -1, 2);
		break;
	case 2:
		if (!update)
			if (obj->VBTitle) SOutS(obj->title, wx + obj->x - 2, wy + obj->y + 1, CWhite, CGray1, 2);
		DBox(wx + obj->x, wy + obj->y, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, (obj->hid == HIDE) ? CGray4 : CGray5);
		SOut(OSTR, wx + obj->x + obj->wdt - 1, wy + obj->y + 1, (obj->hid == HIDE) ? CGray2 : CGray1, -1, 2);
		break;
	case 3:
		if (!update)
			if (obj->VBTitle) SOut(obj->title, wx + obj->x - 2, wy + obj->y + 1, CGray1, -1, 2);
		DBox(wx + obj->x, wy + obj->y, wx + obj->x + obj->wdt - 1, wy + obj->y + obj->hgt - 1, (obj->hid == HIDE) ? CGray4 : CWhite);
		SOut(OSTR, wx + obj->x + obj->wdt - 1, wy + obj->y + 1, (obj->hid == HIDE) ? CGray2 : CGray1, -1, 2);
		break;
	}
}

void DrawCheckMark(int tx, int ty, int size, BYTE color)
{
	DLine(tx, ty + size / 2, tx + size / 2 - 2, ty + size - 1, color);
	DLine(tx + 1, ty + size / 2, tx + size / 2 - 2 + 1, ty + size - 1, color);
	DLine(tx + size / 2 - 1, ty + size - 1, tx + size - 2, ty, color);
	DLine(tx + size / 2 - 1 + 1, ty + size - 1, tx + size - 2 + 1, ty, color);
}

void DrawFlagBox(OBJECT *obj, BYTE update)
{
	int c1x = obj->inwin->x + obj->x, c1y = obj->inwin->y + obj->y;
	DFrame(c1x, c1y, c1x + obj->wdt - 1, c1y + obj->hgt - 1, CGray1);
	DBox(c1x + 1, c1y + 1, c1x + obj->wdt - 1 - 1, c1y + obj->hgt - 1 - 1, (obj->hid == HIDE) ? CGray4 : CWhite);
	if (*obj->iptr) DrawCheckMark(c1x, c1y, obj->wdt, (obj->hid == HIDE) ? CGray2 : CRed);
	if (!update) if (obj->FBTitle)
		TOut(obj->title, c1x + obj->wdt + 1, c1y, (obj->hid == HIDE) ? CGray2 : CGray1, -1);
}

void DrawListBox(OBJECT *obj)
{
	int c1x = obj->inwin->x + obj->x, c1y = obj->inwin->y + obj->y;
	int cnt;
	DFrame(c1x, c1y, c1x + obj->wdt - 1, c1y + obj->hgt - 1, CGray1);
	for (cnt = 0; cnt < obj->LBCellNumX*obj->LBCellNumY; cnt++)
		DrawListBoxCell(obj, obj->LB1stCell + cnt, (cnt == obj->LBSelect - obj->LB1stCell));
}

void DrawIcon(OBJECT *obj, BYTE prsd)
{
	int tx, ty; tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	if (obj->ICTitle) SOut(obj->title, tx, ty - 6, CGray1);
	DrawStd3D(tx, ty, tx + obj->wdt - 1, ty + obj->hgt - 1, prsd);
	SVIGCopy(obj->ICSrcPage, obj->ICSrcX + obj->ICSrcWdt*((*obj->iptr) % obj->ICSTabWdt), obj->ICSrcY + obj->ICSrcHgt*((*obj->iptr) / obj->ICSTabWdt), SVIPage, tx + obj->wdt / 2 - obj->ICSrcWdt / 2, ty + obj->hgt / 2 - obj->ICSrcHgt / 2, obj->ICSrcWdt, obj->ICSrcHgt, ((obj->hid == HIDE) || prsd));
}

void DrawSelectBox(OBJECT *obj)
{
	int tx, ty, clwdt, cnt;
	BYTE pgray;
	tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	clwdt = obj->wdt / (obj->SBRngHi + 1 - obj->SBRngLo);
	if (obj->SBTitle) SOut(obj->title, tx, ty - 6, CGray1);
	for (cnt = obj->SBRngLo; cnt <= obj->SBRngHi; cnt++)
	{
		DBox(tx, ty, tx + clwdt - 1, ty + obj->hgt - 1, ((cnt == *obj->iptr) && (obj->hid != HIDE)) ? CWhite : CGray5);
		pgray = 0; //if (obj->SBMarkType==1) if (cnt!=*obj->iptr) pgray=1;
		if (obj->hid == HIDE) pgray = 1;
		SVIGCopy(obj->SBSrcPage, obj->SBSrcX + obj->SBSrcWdt*cnt, obj->SBSrcY, SVIPage, tx + clwdt / 2 - obj->SBSrcWdt / 2, ty + obj->hgt / 2 - obj->SBSrcHgt / 2, obj->SBSrcWdt, obj->SBSrcHgt, pgray);
		if (obj->hid != HIDE) if (cnt == *obj->iptr) //if (obj->SBMarkType==0)
			DFrame(tx, ty, tx + clwdt - 1, ty + obj->hgt - 1, CGray1);
		tx += clwdt;
	}
}

void DrawText(OBJECT *obj)
{
	int tx, ty; tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	char *otxt;

	switch (obj->TXCType)
	{
	case 0: // Standard (short) text
		otxt = obj->title;
		break;
	case 1: // Extended text (with allocated memory)
		if (!obj->bptr) { SystemError("null bptr on draw text"); return; }
		otxt = obj->bptr;
		break;
	case 2: // Const long text (may be indexed)
		if (!obj->bptr) { SystemError("null bptr on draw text"); return; }
		if (obj->iptr) // Indexed text (brutal ptr conversion)
			otxt = ((char**)obj->bptr)[*obj->iptr];
		else
			otxt = obj->bptr;
		break;
	}

	if ((obj->wdt > 0) && (obj->hgt > 0))
		FTOut(otxt, tx, ty, obj->wdt, obj->hgt, 0, obj->TXFCol, obj->TXBCol, obj->TXForm);
	else
		TOut(otxt, tx, ty, obj->TXFCol, obj->TXBCol, obj->TXForm);

	if (obj->TXTitle) if (obj->TXCType == 2)
		SOut(obj->title, tx, ty - 6, CGray1);
}

void DrawPicture(OBJECT *obj)
{
	int tx, ty; tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	int srcx, srcy;

	srcx = obj->PCSrcX; srcy = obj->PCSrcY;
	if (obj->iptr)
	{
		srcx += obj->wdt*(*obj->iptr%obj->PCSTabWdt);
		srcy += obj->hgt*(*obj->iptr / obj->PCSTabWdt);
	}

	SVIGCopy(obj->PCSrcPage, srcx, srcy, SVIPage, tx, ty, obj->wdt, obj->hgt, (obj->hid == HIDE));

	switch (obj->PCTitle)
	{
	case 1: SOut(obj->title, tx, ty - 6, CGray1, -1, 0); break;
	case 2: SOut(obj->title, tx + 1, ty + 1, CGray1, -1, 0); break;
	}
}

void DrawTextBox(OBJECT *obj, BYTE update)
{
	int tx, ty; tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	if (!update)
	{
		DBox(tx, ty, tx + obj->wdt - 1, ty + obj->hgt - 1, CWhite);
		DFrame(tx, ty, tx + obj->wdt - 1, ty + obj->hgt - 1, CGray2);
		switch (obj->TBTitle)
		{
		case 1: SOut(obj->title, tx, ty - 6, CGray1); break;
		case 2: SOut(obj->title, tx - 2, ty + 2, CGray2, -1, 2); break;
		}
	}
	SOut(obj->bptr, tx + 2, ty + 2, CGray1, CWhite);
	if (SLen(obj->bptr) < obj->TBMaxLen)
		DBox(tx + 2 + 4 * SLen(obj->bptr), ty + 2, tx + obj->wdt - 2, ty + 6, CWhite);
	DFrame(tx + 1, ty + 7, tx + obj->wdt - 2, ty + 7, CWhite);
	DFrame(tx + 2 + 4 * obj->TBCursor, ty + 7, tx + 2 + 2 + 4 * obj->TBCursor, ty + 7, CBlue);
}

void DrawFrame(OBJECT *obj)
{
	int tx, ty; tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	if (obj->FRBCol > -1) DBox(tx, ty, tx + obj->wdt - 1, ty + obj->hgt - 1, obj->FRBCol);
	if (obj->FRFCol > -1) DFrame(tx, ty, tx + obj->wdt - 1, ty + obj->hgt - 1, obj->FRFCol);
	if (obj->FRTitle == 1) SOut(obj->title, tx + 3, ty - 4, obj->FRFCol, obj->FRTBCol);
	if (obj->FRTitle == 2) SOut(obj->title, tx + obj->wdt / 2, ty - 4, obj->FRFCol, obj->FRTBCol, 1);
	if (obj->FRTitle == 3) SOut(obj->title, tx + obj->wdt - 4, ty - 4, obj->FRFCol, obj->FRTBCol, 2);
}

void DrawSelector(OBJECT *obj)
{
	int cnt, tx, ty; tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	char *elname = obj->bptr;
	int pipos;
	DFrame(tx, ty, tx + obj->wdt - 1, ty + obj->hgt - 1, CGray1);
	if (obj->SLTitle) SOut(obj->title, tx + obj->wdt / 2, ty - 6, CGray1, -1, 1);
	for (cnt = 0; cnt < obj->SLElNum; cnt++)
	{
		DBox(tx + 1, ty + 1 + 8 * cnt, tx + obj->wdt - 2, ty + 1 + 8 * cnt + 6, (cnt == *obj->iptr) ? CRed : CGray5);
		pipos = SCharPos('|', elname);
		if (pipos > -1) { SCopy(elname, OSTR, pipos); elname += pipos + 1; }
		else SCopy(elname, OSTR);
		SOut(OSTR, tx + obj->wdt / 2, ty + 2 + 8 * cnt, (cnt == *obj->iptr) ? CWhite : CGray1, -1, 1);
		DFrame(tx, ty + 2 + 8 * cnt + 6, tx + obj->wdt - 1, ty + 2 + 8 * cnt + 6, CGray1);
	}
}

void DrawHyperText(OBJECT *obj, BYTE update)
{
	int cnt, tx, ty; tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	if (!update)
	{
		DBox(tx + 1, ty + 1, tx + obj->wdt - 2, ty + obj->hgt - 2, CWhite);
		DFrame(tx, ty, tx + obj->wdt - 1, ty + obj->hgt - 1, CGray1);
	}
	FTOut(obj->bptr, tx + 2, ty + 2, (obj->wdt - 3) / 4, (obj->hgt - 3) / 6, obj->HTLineIndex, 110, CWhite, 0);
}

WINDOW *AreaCoveredByWin(int x1, int y1, int x2, int y2)
{
	WINDOW *cwin;
	int wx1, wy1, wx2, wy2;
	for (cwin = FirstWindow; cwin; cwin = cwin->next)
		if (cwin->type != DELETED)
		{
			wx1 = cwin->x; wy1 = cwin->y; wx2 = cwin->x + cwin->wdt - 1; wy2 = cwin->y + cwin->hgt - 1;
			if (!((x2 < wx1) || (x1 > wx2)) && !((y2 < wy1) || (y1 > wy2))) return cwin;
		}
	return cwin;
}

BYTE ObjectVisible(OBJECT *obj)
{
	WINDOW *cwin;
	int tx, ty; tx = obj->inwin->x + obj->x; ty = obj->inwin->y + obj->y;
	cwin = AreaCoveredByWin(tx, ty, tx + obj->wdt - 1, ty + obj->hgt - 1);
	if (cwin && (cwin != obj->inwin)) return 0;
	return 1;
}

void DrawObject(OBJECT *obj, BYTE update, BYTE option = 0)
{
	if (!ObjectVisible(obj)) return;
	if (obj->inwin->type == DELETED) return;

	LPage(SVIPage);
	switch (obj->type)
	{
	case    BUTTON: DrawButton(obj, option);	break;
	case      TEXT: DrawText(obj);    		break;
	case   PICTURE: DrawPicture(obj); 		break;
	case   HSCROLL: DrawHScroll(obj); 		break;
	case   VSCROLL: DrawVScroll(obj); 		break;
	case    VALBOX: DrawValBox(obj, update); 	break;
	case     FRAME: DrawFrame(obj); 		break;
	case   FLAGBOX: DrawFlagBox(obj, update); 	break;
	case      ICON: DrawIcon(obj, option);    	break;
	case   LISTBOX: DrawListBox(obj);   	break;
	case SELECTBOX: DrawSelectBox(obj); 	break;
	case   TEXTBOX: DrawTextBox(obj, update); 	break;
	case  SELECTOR: DrawSelector(obj);          break;
	case HYPERTEXT: DrawHyperText(obj, update);  break;
	default:
		SystemError("DrwObj: Undefined Object Type");
		break;
	}

	UpdateScreen();
}

// DrawScrollMark,DrawListBoxCell called directly...!

//----------------------- Object Action Executors -------------------------------

BYTE EvaluateXRVal(BYTE xrval, WINDOW *inwin)
{
	switch (xrval)
	{
	case XRVCLOSE: inwin->type = DELETED; xrval = 0; break;
	case XRVHELP:  OpenHelp = 1;          xrval = 0; break;
	}
	return xrval;
}

void ExecULinks(WINDOW *fwin, OBJECT *fobj);

void JumpHScroll(OBJECT *obj, int ax)
{
	int newval;
	newval = obj->SCRangeLo + (long)(obj->SCRangeHi - obj->SCRangeLo)*(long)(ax - 2) / (obj->wdt - 5);
	newval = BoundBy((newval / obj->SCSTEP)*obj->SCSTEP, obj->SCRangeLo, obj->SCRangeHi);
	*obj->iptr = newval;
}

void ShiftScroll(OBJECT *obj, int dir)
{
	int newval;
	newval = *obj->iptr + obj->SCKSTEP*dir;
	newval = BoundBy((newval / obj->SCSTEP)*obj->SCSTEP, obj->SCRangeLo, obj->SCRangeHi);
	*obj->iptr = newval;
}

void ScrollHScroll(OBJECT *obj)
{
	int lmx = MouseX();
	int lval;
	do
	{
		lval = *obj->iptr;
		while (Mousebut() && (MouseX() == lmx));
		lmx = MouseX();
		LPage(SVIPage);
		RemoveHScrollMark(obj);
		JumpHScroll(obj, lmx - obj->inwin->x - obj->x);
		DrawHScrollMark(obj);
		if (*obj->iptr != lval) ExecULinks(obj->inwin, obj);
	} while (Mousebut());
}

BYTE OnHScrollMark(OBJECT *obj, int ax)
{
	int smx = HScrollMarkPos(obj);
	if (Inside(ax - smx, 0, 4)) return 1;
	return 0;
}

void JumpVScroll(OBJECT *obj, int ay)
{
	int newval;
	newval = obj->SCRangeLo + (long)(obj->SCRangeHi - obj->SCRangeLo)*(long)(ay - 2) / (obj->hgt - 5);
	newval = BoundBy((newval / obj->SCSTEP)*obj->SCSTEP, obj->SCRangeLo, obj->SCRangeHi);
	*obj->iptr = newval;
}

void ScrollVScroll(OBJECT *obj)
{
	int lmy = MouseY();
	int lval;
	do
	{
		lval = *obj->iptr;
		while (Mousebut() && (MouseY() == lmy));
		lmy = MouseY();
		LPage(SVIPage);
		RemoveVScrollMark(obj);
		JumpVScroll(obj, lmy - obj->inwin->y - obj->y);
		DrawVScrollMark(obj);
		if (*obj->iptr != lval) ExecULinks(obj->inwin, obj);
	} while (Mousebut());
}

BYTE OnVScrollMark(OBJECT *obj, int ay)
{
	int smy = VScrollMarkPos(obj);
	if (Inside(ay - smy, 0, 4)) return 1;
	return 0;
}

void ValBoxModify(OBJECT *obj, int dir)
{
	*obj->iptr = BoundBy(*obj->iptr + obj->VBStep*dir, obj->VBRngLo, obj->VBRngHi);
	DrawObject(obj, 1);
}

BYTE ListBoxMsAction(OBJECT *obj, int ax, int ay) // Returns XRVal
{
	int ncell;
	BYTE xrval = 0;
	if (Inside(ax, 1, obj->wdt - 2) && Inside(ay, 1, obj->hgt - 2))
	{
		ncell = BoundBy(obj->LB1stCell + (ax - 1) / obj->LBCellWdt + obj->LBCellNumX*((ay - 1) / obj->LBCellHgt), 0, obj->LBClNum - 1);
		if (ncell == obj->LBSelect) // Double click -> cell action aid:0
			xrval = EvaluateXRVal(ListBoxAction(obj, 0), obj->inwin);
		else // Else -> select cell
		{
			LPage(SVIPage);
			DrawListBoxCell(obj, obj->LBSelect, 0);
			obj->LBSelect = ncell;
			if (obj->iptr) *obj->iptr = ncell;
			DrawListBoxCell(obj, obj->LBSelect, 1);

		}
	}
	return xrval;
}

int ListBoxMax1stC(OBJECT *obj)
{
	int max1stpos;
	max1stpos = obj->LBClNum; while (max1stpos%obj->LBCellNumX != 0) max1stpos++;
	max1stpos = Max(max1stpos - (obj->LBCellNumX*obj->LBCellNumY), 0);
	return max1stpos;
}

BYTE JumpListBoxTo(OBJECT *obj, int new1stcell)
{
	int last1st = obj->LB1stCell;
	new1stcell = BoundBy(new1stcell, 0, obj->LBClNum - 1);
	while (new1stcell%obj->LBCellNumX != 0) new1stcell--;
	obj->LB1stCell = BoundBy(new1stcell, 0, ListBoxMax1stC(obj));
	if (obj->LB1stCell != last1st) { DrawObject(obj, 0); return 1; }
	return 0;
}

void MoveListBoxMark(OBJECT *obj, int step)
{
	if (Inside(obj->LBSelect + step, 0, obj->LBClNum - 1))
	{
		LPage(SVIPage);
		DrawListBoxCell(obj, obj->LBSelect, 0);
		obj->LBSelect += step;
		if (obj->iptr) *obj->iptr = obj->LBSelect;
		if (!JumpListBoxTo(obj, obj->LBSelect - (obj->LBCellNumX*obj->LBCellNumY) / 2))
			DrawListBoxCell(obj, obj->LBSelect, 1);
	}
	ExecULinks(obj->inwin, obj);
}

void AdjustScrBarRangeHigh(WINDOW *inwin, int *ptrto, int nrngh)
{
	OBJECT *cobj;
	for (cobj = inwin->fobj; cobj; cobj = cobj->next)
		if (cobj->type == VSCROLL)
			if (cobj->iptr == ptrto)
			{
				cobj->SCRangeHi = nrngh;
				DrawObject(cobj, 0);
			}
}

void AdjustLBCellNum(int lid, int ncnum)
{
	WINDOW *cwin;
	OBJECT *cobj;
	for (cwin = FirstWindow; cwin; cwin = cwin->next)
		for (cobj = cwin->fobj; cobj; cobj = cobj->next)
			if (cobj->type == LISTBOX)
				if (cobj->LBListID == lid)
				{
					cobj->LBClNum = ncnum;
					cobj->LB1stCell = 0;
					cobj->LBSelect = 0; if (ncnum < 1) cobj->LBSelect = -1;
					if (cobj->iptr) *cobj->iptr = cobj->LBSelect;
					DrawObject(cobj, 1);
					AdjustScrBarRangeHigh(cobj->inwin, &cobj->LB1stCell, ListBoxMax1stC(cobj));
					return;
				}
}

void SelectBoxMsAction(OBJECT *obj, int ax)
{
	int oldsel = *obj->iptr;
	*obj->iptr = obj->SBRngLo + ax / (obj->wdt / (obj->SBRngHi + 1 - obj->SBRngLo));
	if (*obj->iptr != oldsel) DrawObject(obj, 0);
}

void ShiftSelectBox(OBJECT *obj, int dir)
{
	int oldsel = *obj->iptr;
	*obj->iptr = BoundBy(*obj->iptr + dir, obj->SBRngLo, obj->SBRngHi);
	if (*obj->iptr != oldsel) DrawObject(obj, 0);
}

void SelectorMsAction(OBJECT *obj, int ay)
{
	int oldsel = *obj->iptr;
	*obj->iptr = BoundBy(ay / 8, 0, obj->SLElNum - 1);
	if (*obj->iptr != oldsel) DrawObject(obj, 0);
}

void ShiftSelector(OBJECT *obj, int dir)
{
	int oldsel = *obj->iptr;
	*obj->iptr = BoundBy(*obj->iptr + dir, 0, obj->SLElNum - 1);
	if (*obj->iptr != oldsel) DrawObject(obj, 0);
}

int TBMaxCursorPos(OBJECT *obj)
{
	return Min(SLen(obj->bptr), obj->TBMaxLen - 1);
}

BYTE KbTextBoxAction(OBJECT *obj, BYTE kbspc, BYTE kbstn) // Returns eXitRVal
{
	BYTE xrval = 0;
	int cnt;
	switch (kbspc)
	{
	case KBSTN:
		if (SLen(obj->bptr) < obj->TBMaxLen)
			for (cnt = SLen(obj->bptr); cnt >= obj->TBCursor; cnt--)
				obj->bptr[cnt + 1] = obj->bptr[cnt];
		obj->bptr[obj->TBCursor] = kbstn;
		obj->TBCursor = Min(obj->TBCursor + 1, TBMaxCursorPos(obj));
		DrawObject(obj, 1);
		break;
	case KBENTER:
		xrval = EvaluateXRVal(obj->TBXRVal, obj->inwin);
		ExecULinks(obj->inwin, obj);
		break;
	case KBLEFT:
		obj->TBCursor = Max(obj->TBCursor - 1, 0);
		DrawObject(obj, 1);
		break;
	case KBRIGHT:
		obj->TBCursor = Min(obj->TBCursor + 1, TBMaxCursorPos(obj));
		DrawObject(obj, 1);
		break;
	case KBHOME:
		obj->TBCursor = 0; DrawObject(obj, 1);
		break;
	case KBEND:
		obj->TBCursor = TBMaxCursorPos(obj); DrawObject(obj, 1);
		break;
	case KBBACK:
		if (obj->TBCursor > 0)
		{
			for (cnt = obj->TBCursor - 1; obj->bptr[cnt]; cnt++)
				obj->bptr[cnt] = obj->bptr[cnt + 1];
			obj->TBCursor--;
			DrawObject(obj, 1);
		}
		break;
	case KBDEL:
		for (cnt = obj->TBCursor; obj->bptr[cnt]; cnt++)
			obj->bptr[cnt] = obj->bptr[cnt + 1];
		DrawObject(obj, 1);
		break;
	}
	return xrval;
}

void JumpHyperText(OBJECT *obj, int npos)
{
	int oldpos = obj->HTLineIndex;
	obj->HTLineIndex = BoundBy(npos, 0, obj->HTMaxLineI);
	if (obj->HTLineIndex != oldpos) DrawObject(obj, 1);
}

void ScrollHyperText(OBJECT *obj, int dir)  // Use GFXCopy to scroll stepwise
{
	JumpHyperText(obj, obj->HTLineIndex + dir);
}

void NewHelpIndex(char *index);

void HyperTextAction(OBJECT *obj, int ax, int ay)
{
	FTOutGetHL(obj->bptr, (obj->wdt - 3) / 4, (obj->hgt - 3) / 6, obj->HTLineIndex, (ax - 2) / 4, (ay - 2) / 6, OSTR);
	if (OSTR[0]) NewHelpIndex(OSTR);
}

void AdjustHTLines(OBJECT *obj, int newcline, int newmaxl)
{
	obj->HTLineIndex = newcline;
	obj->HTMaxLineI = newmaxl;
	DrawObject(obj, 1);
	AdjustScrBarRangeHigh(obj->inwin, &obj->HTLineIndex, obj->HTMaxLineI);
}

//------------------------ UpdateLink Operation ------------------------------

ULINK *NewULink(BYTE type, WINDOW *fwin, OBJECT *fobj, OBJECT *tobj, int(*exfunc)(int), int expar)
{
	ULINK *nlnk;

	if (!(nlnk = new ULINK)) { SVIInitErrors++; return NULL; }

	nlnk->type = type;
	nlnk->fwin = fwin; nlnk->fobj = fobj;
	nlnk->tobj = tobj;
	nlnk->exfunc = exfunc;
	nlnk->expar = expar;

	nlnk->next = FirstULink;
	FirstULink = nlnk;

	//SaveFULink=FirstULink;
	//IntegrityC(1);

	return nlnk;
}

void DeleteULinks(void)
{
	ULINK *tlnk;
	if (FirstULink) SystemError("Delete ulinks: ulinks should have|been deleted by close wins");
	while (FirstULink) { tlnk = FirstULink->next; delete FirstULink; FirstULink = tlnk; /*SaveFULink=FirstULink;*/ }
}

void DeleteULinksFromWindow(WINDOW *cwin)
{
	ULINK *clnk = FirstULink, *prev = NULL, *nxt;
	OBJECT *cobj;
	BYTE del;

	while (clnk)
	{

		del = 0;
		if (clnk->fwin == cwin) del = 1;
		for (cobj = cwin->fobj; cobj; cobj = cobj->next)
			if ((cobj == clnk->fobj) || (cobj == clnk->tobj)) del = 1;

		nxt = clnk->next;

		if (del)
		{
			if (prev) prev->next = nxt;
			else { FirstULink = nxt; /*SaveFULink=FirstULink;*/ }
			delete clnk;
		}
		else
		{
			prev = clnk;
		}

		clnk = nxt;

	}
}

void ExecULinks(WINDOW *fwin, OBJECT *fobj)
{
	ULINK *clnk;
	OBJECT *cobj;

	// exfunc may create new windows, objects, and ulinks and recall
	// runSVI; but it may not modify or delete any existing windows,
	// objects, and ulinks!

	if (fwin && (fwin != FirstWindow)) return; // Avoid repetition of opening
	if (fobj && (fobj->inwin != FirstWindow)) return; // of new FirstWindow...

	for (clnk = FirstULink; clnk; clnk = clnk->next)
		if ((fwin && (clnk->fwin == fwin)) || (fobj && (clnk->fobj == fobj)))
			switch (clnk->type)
			{
			case OBJFUNC:
				if (!clnk->tobj) { SystemError("Null tobj on exec ulink"); break; }
				if (clnk->exfunc)
					*(clnk->tobj->iptr) = clnk->exfunc(clnk->expar);
				DrawObject(clnk->tobj, 1);
				break;
			case VALBINC:
				if (!clnk->tobj) { SystemError("Null tobj on exec ulink"); break; }
				if (clnk->tobj->type != VALBOX) { SystemError("incorrect tobj type on exec ulink"); break; }
				if (clnk->tobj->VBLongU) return; // goes not
				*(clnk->tobj->iptr) = BoundBy(*(clnk->tobj->iptr) + clnk->tobj->VBStep*(+1), clnk->tobj->VBRngLo, clnk->tobj->VBRngHi);
				DrawObject(clnk->tobj, 1);
				break;
			case VALBDEC:
				if (!clnk->tobj) { SystemError("Null tobj on exec ulink"); break; }
				if (clnk->tobj->type != VALBOX) { SystemError("incorrect tobj type on exec ulink"); break; }
				if (clnk->tobj->VBLongU) return; // goes not
				*(clnk->tobj->iptr) = BoundBy(*(clnk->tobj->iptr) + clnk->tobj->VBStep*(-1), clnk->tobj->VBRngLo, clnk->tobj->VBRngHi);
				DrawObject(clnk->tobj, 1);
				break;
			case EXCREDR:
				if (clnk->exfunc) clnk->exfunc(clnk->expar);
				if (clnk->tobj) DrawObject(clnk->tobj, 1);
				break;
			case TOGHID:
				if (!fobj) { SystemError("Null fobj on exec ulink"); break; }
				for (cobj = fobj->inwin->fobj; cobj; cobj = cobj->next)
					if ((cobj != fobj) && (cobj != clnk->tobj))
					{
						if (cobj->hid != HIDENEVER) Toggle(cobj->hid);
						DrawObject(cobj, 0);
					}
				break;
			case LBCACT:
				if (!fobj) { SystemError("Null fobj on exec ulink"); break; }
				if (!clnk->tobj) { SystemError("Null tobj on exec ulink"); break; }
				if (clnk->tobj->type != LISTBOX) { SystemError("incorrect tobj type on exec ulink"); break; }
				EvaluateXRVal(ListBoxAction(clnk->tobj, clnk->expar), clnk->tobj->inwin);
				break;
			case LBSCRUP:
				if (!clnk->tobj) { SystemError("Null tobj on exec ulink"); break; }
				switch (clnk->tobj->type)
				{
				case LISTBOX: JumpListBoxTo(clnk->tobj, clnk->tobj->LB1stCell - clnk->tobj->LBCellNumX); break;
				case HYPERTEXT: ScrollHyperText(clnk->tobj, -1); break;
				default: SystemError("incorrect tobj type on exec ulink"); break;
				}
				break;
			case LBSCRDN:
				if (!clnk->tobj) { SystemError("Null tobj on exec ulink"); break; }
				switch (clnk->tobj->type)
				{
				case LISTBOX: JumpListBoxTo(clnk->tobj, clnk->tobj->LB1stCell + clnk->tobj->LBCellNumX); break;
				case HYPERTEXT: ScrollHyperText(clnk->tobj, +1); break;
				default:SystemError("incorrect tobj type on exec ulink"); break;
				}
				break;
			case CLOSEXC:
				// Is not called by ExecULinks but by CloseWin/DelULinksFromWin
				break;
			case OBJFUNCA:
				// Is not called by ExecULinks but by ExecAULinks
				break;
			case CLWINEXC:
				if (fobj == clnk->fobj) // Not activated by fwin; by fobj only
				{
					if (fwin) fwin->type = DELETED;
					if (clnk->exfunc) clnk->exfunc(clnk->expar);
				}
				break;
			default: SystemError("Undefined ULink type on exec"); break;
			}
}

void ExecAULinks(void)
{
	ULINK *clnk;
	OBJECT *cobj;

	for (clnk = FirstULink; clnk; clnk = clnk->next)
		switch (clnk->type)
		{
		case OBJFUNCA:
			if (!clnk->tobj) { SystemError("Null tobj on exec ulink"); break; }
			if (clnk->exfunc)
				*(clnk->tobj->iptr) = clnk->exfunc(clnk->expar);
			DrawObject(clnk->tobj, 1);
			break;
		}
}

//------------------------ Object Action Callers -----------------------------

BYTE MsObjAction(OBJECT *obj, int ax, int ay) // Returns eXitRVal
{
	BYTE xrval = 0;
	int mui;

	if (!obj) return 0;

	// OnClick Action
	switch (obj->type)
	{
	case BUTTON:
		LPage(SVIPage);
		DrawObject(obj, 0, 1);
		if (obj->BTNoULRep)
		{
			ExecULinks(obj->inwin, obj);
		}
		else
		{
			SDL_Delay(100); // Give chance to release button // TODO: delay ersetzen
			if (Mousebut()) mui = 300; else mui = 0;
			do { ExecULinks(obj->inwin, obj); SDL_Delay(mui); mui = 100; } while (Mousebut());
		}
		break;
	case HSCROLL:
		if (OnHScrollMark(obj, ax)) ScrollHScroll(obj); // <- execs ULinks
		else { LPage(SVIPage); RemoveHScrollMark(obj); JumpHScroll(obj, ax); DrawHScrollMark(obj); ExecULinks(obj->inwin, obj); }
		break;
	case VSCROLL:
		if (OnVScrollMark(obj, ay)) ScrollVScroll(obj); // <- execs ULinks
		else { LPage(SVIPage); RemoveVScrollMark(obj); JumpVScroll(obj, ay); DrawVScrollMark(obj); ExecULinks(obj->inwin, obj); }
		break;
	case FLAGBOX:
		if (*obj->iptr) *obj->iptr = 0; else *obj->iptr = 1;
		DrawObject(obj, 1);
		ExecULinks(obj->inwin, obj);
		break;
	case ICON:
		LPage(SVIPage);
		DrawObject(obj, 0, 1);
		break;
	case TEXTBOX:
		obj->TBCursor = BoundBy((ax - 2) / 4, 0, TBMaxCursorPos(obj));
		DrawObject(obj, 1);
		break;
	case HYPERTEXT:
		HyperTextAction(obj, ax, ay);
		break;
	}
	while (Mousebut());

	// AfterClick Action
	switch (obj->type)
	{
	case BUTTON:
		LPage(SVIPage);
		DrawObject(obj, 0, 0);
		xrval = EvaluateXRVal(obj->BTXRVal, obj->inwin);
		break;
	case ICON:
		LPage(SVIPage);
		DrawObject(obj, 0, 0);
		ExecULinks(obj->inwin, obj);
		break;
	case LISTBOX:
		xrval = ListBoxMsAction(obj, ax, ay);
		ExecULinks(obj->inwin, obj);
		break;
	case SELECTBOX:
		SelectBoxMsAction(obj, ax);
		ExecULinks(obj->inwin, obj);
		break;
	case SELECTOR:
		SelectorMsAction(obj, ay);
		ExecULinks(obj->inwin, obj);
		break;
	}

	return xrval;
}


BYTE KbObjAction(OBJECT *obj, BYTE kbspc, BYTE kbstn) // Returns eXitRVal
{
	BYTE xrval = 0;

	if (!obj) return 0;

	// OnClick Action
	switch (obj->type)
	{
	case BUTTON:
		if (kbspc == KBENTER)
		{
			LPage(SVIPage);
			DrawObject(obj, 0, 1); SDL_Delay(250);
			ExecULinks(obj->inwin, obj);
		}
		break;
	case VALBOX:
		if (!obj->VBLongU)
		{
			if (kbspc == KBUP) { ValBoxModify(obj, +1); ExecULinks(obj->inwin, obj); }
			if (kbspc == KBDOWN) { ValBoxModify(obj, -1); ExecULinks(obj->inwin, obj); }
		}
		break;
	case FLAGBOX:
		if (*obj->iptr) *obj->iptr = 0; else *obj->iptr = 1;
		LPage(SVIPage);
		DrawFlagBox(obj, 1);
		ExecULinks(obj->inwin, obj);
		break;
	case ICON:
		if (kbspc == KBENTER) { LPage(SVIPage); DrawObject(obj, 0, 1); SDL_Delay(250); }
		break;
	}

	// AfterClick Action
	switch (obj->type)
	{
	case BUTTON:
		if (kbspc == KBENTER) { LPage(SVIPage); DrawObject(obj, 0, 0); xrval = EvaluateXRVal(obj->BTXRVal, obj->inwin); }
		break;
	case HSCROLL:
		if (Inside(kbspc, KBLEFT, KBRIGHT))
		{
			LPage(SVIPage);
			RemoveHScrollMark(obj);
			ShiftScroll(obj, (kbspc == KBLEFT) ? -1 : +1);
			DrawHScrollMark(obj);
			ExecULinks(obj->inwin, obj);
		}
		break;
	case VSCROLL:
		if (Inside(kbspc, KBUP, KBDOWN))
		{
			LPage(SVIPage);
			RemoveVScrollMark(obj);
			ShiftScroll(obj, (kbspc == KBUP) ? -1 : +1);
			DrawVScrollMark(obj);
			ExecULinks(obj->inwin, obj);
		}
		break;
	case ICON:
		if (kbspc == KBENTER)
		{
			LPage(SVIPage); DrawObject(obj, 0, 0); ExecULinks(obj->inwin, obj);
		}
		break;
	case LISTBOX:
		switch (kbspc)
		{
		case KBENTER: xrval = EvaluateXRVal(ListBoxAction(obj, 0), obj->inwin); break;
		case KBLEFT:  MoveListBoxMark(obj, -1); break;
		case KBRIGHT: MoveListBoxMark(obj, +1); break;
		case KBUP:    MoveListBoxMark(obj, -obj->LBCellNumX); break;
		case KBDOWN:  MoveListBoxMark(obj, +obj->LBCellNumX); break;
		}
		break;
	case SELECTBOX:
		if (kbspc == KBLEFT) { ShiftSelectBox(obj, -1); ExecULinks(obj->inwin, obj); }
		if (kbspc == KBRIGHT) { ShiftSelectBox(obj, +1); ExecULinks(obj->inwin, obj); }
		break;
	case SELECTOR:
		if (kbspc == KBUP) { ShiftSelector(obj, -1); ExecULinks(obj->inwin, obj); }
		if (kbspc == KBDOWN) { ShiftSelector(obj, +1); ExecULinks(obj->inwin, obj); }
		break;
	case TEXTBOX:
		xrval = KbTextBoxAction(obj, kbspc, kbstn);
		break;
	case HYPERTEXT:
		if (Inside(kbspc, KBUP, KBDOWN))
		{
			ScrollHyperText(obj, (kbspc == KBUP) ? -1 : +1);
			ExecULinks(obj->inwin, obj);
		}
		if (Inside(kbspc, KBPGUP, KBPGDN))
		{
			ScrollHyperText(obj, ((obj->hgt - 3) / 6)*((kbspc == KBPGUP) ? -1 : +1));
			ExecULinks(obj->inwin, obj);
		}
		break;
	}

	return xrval;
}

//--------------------------- Object Handling -------------------------------

OBJECT *NewBasicObject(const char *title, BYTE type, BYTE acc, BYTE hid, int x, int y, int wdt, int hgt, BYTE bd1, BYTE bd2, BYTE bd3, int id1, int id2, int id3, int id4, int id5, char *bptr, int *dptr)
{
	OBJECT *nobj;
	WINDOW *twin;

	if (!FirstWindow) { SVIInitErrors++; return NULL; }

	if (!(nobj = new OBJECT)) return NULL;

	nobj->type = type; nobj->acc = acc; nobj->hid = hid;
	nobj->x = x; nobj->y = y; nobj->wdt = wdt; nobj->hgt = hgt;
	SCopy(title, nobj->title, ObjTitleLen);
	nobj->bdata[0] = bd1; nobj->bdata[1] = bd2; nobj->bdata[2] = bd3;
	nobj->idata[0] = id1; nobj->idata[1] = id2; nobj->idata[2] = id3; nobj->idata[3] = id4; nobj->idata[4] = id5;
	nobj->iptr = dptr;
	nobj->bptr = bptr;

	twin = FirstWindow;
	nobj->next = twin->fobj;
	twin->fobj = nobj;

	nobj->inwin = twin;

	return nobj;
}

OBJECT *NewObject(const char *title, BYTE type, BYTE acc, BYTE hid, int x, int y, int wdt, int hgt, BYTE bd1, BYTE bd2, BYTE bd3, int id1, int id2, int id3, int id4, int id5, char *bptr, int *dptr)
{
	OBJECT *nobj, *bscobj1, *bscobj2, *bscobj3;

	switch (type) // Check for extended objects
	{
	case VALBOXB:
		nobj = NewBasicObject(title, VALBOX, acc, hid, x, y, wdt, hgt, bd1, bd2, bd3, id1, id2, id3, id4, id5, bptr, dptr);
		bscobj2 = NewBasicObject("#", BUTTON, 0, hid, x + wdt + 1, y, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
		NewULink(VALBINC, NULL, bscobj2, nobj, NULL, 0);
		bscobj2 = NewBasicObject("$", BUTTON, 0, hid, x + wdt + 11, y, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
		NewULink(VALBDEC, NULL, bscobj2, nobj, NULL, 0);
		break;
	case LISTBOX:
		nobj = NewBasicObject(title, LISTBOX, acc, hid, x, y, wdt, hgt, bd1, bd2, bd3, id1, id2, id3, id4, id5, bptr, dptr);
		bscobj1 = NewBasicObject("#", BUTTON, 0, hid, x + wdt + 1, y, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
		bscobj2 = NewBasicObject("$", BUTTON, 0, hid, x + wdt + 1, y + hgt - 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
		bscobj3 = NewBasicObject("LBScrollBar", VSCROLL, 0, hid, x + wdt + 1, y + 10, 9, hgt - 20, 0, 0, 0, 0, ListBoxMax1stC(nobj), nobj->LBCellNumX, 0, 0, bptr, &nobj->LB1stCell);
		NewULink(EXCREDR, NULL, bscobj3, nobj, NULL, 0);
		NewULink(EXCREDR, NULL, nobj, bscobj3, NULL, 0);
		NewULink(EXCREDR, NULL, bscobj1, bscobj3, NULL, 0);
		NewULink(EXCREDR, NULL, bscobj2, bscobj3, NULL, 0);
		NewULink(LBSCRUP, NULL, bscobj1, nobj, NULL, 0);
		NewULink(LBSCRDN, NULL, bscobj2, nobj, NULL, 0);
		break;
	case HYPERTEXT:
		nobj = NewBasicObject(title, HYPERTEXT, acc, hid, x, y, wdt, hgt, bd1, bd2, bd3, id1, id2, id3, id4, id5, bptr, dptr);
		bscobj1 = NewBasicObject("#", BUTTON, 0, hid, x + wdt + 1, y, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
		bscobj2 = NewBasicObject("$", BUTTON, 0, hid, x + wdt + 1, y + hgt - 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
		bscobj3 = NewBasicObject("HTScrollBar", VSCROLL, 0, hid, x + wdt + 1, y + 10, 9, hgt - 20, 0, 0, 0, nobj->HTLineIndex, nobj->HTMaxLineI, 1, 0, 0, bptr, &nobj->HTLineIndex);
		NewULink(EXCREDR, NULL, bscobj3, nobj, NULL, 0);
		NewULink(EXCREDR, NULL, nobj, bscobj3, NULL, 0);
		NewULink(EXCREDR, NULL, bscobj1, bscobj3, NULL, 0);
		NewULink(EXCREDR, NULL, bscobj2, bscobj3, NULL, 0);
		NewULink(LBSCRUP, NULL, bscobj1, nobj, NULL, 0);
		NewULink(LBSCRDN, NULL, bscobj2, nobj, NULL, 0);
		break;
	default:
		nobj = NewBasicObject(title, type, acc, hid, x, y, wdt, hgt, bd1, bd2, bd3, id1, id2, id3, id4, id5, bptr, dptr);
		break;
	}

	//IntegrityC(2);

	return nobj;
}

OBJECT *TextObject(const char *txt, int tx, int ty, int fcol, int bcol = -1, BYTE form = 0)
{
	OBJECT *nobj;
	char *tptr;

	if (SLen(txt) > ObjTitleLen) // Extended text
	{
		if (!(tptr = new char[SLen(txt) + 1])) return NULL;
		SCopy(txt, tptr);
		nobj = NewObject("[Extended Text]", TEXT, 0, 0, tx, ty, 0, 0, form, 1, 0, fcol, bcol, 0, 0, 0, tptr, NULL);
		if (!nobj) delete[] tptr;
	}
	else // Standard (short) text
	{
		nobj = NewObject(txt, TEXT, 0, 0, tx, ty, 0, 0, form, 0, 0, fcol, bcol, 0, 0, 0, NULL, NULL);
	}

	return nobj;
}

OBJECT *CTextObject(char *txt, int tx, int ty, int fcol, int bcol = -1, BYTE form = 0)
{
	return NewObject("[Const Text]", TEXT, 0, 0, tx, ty, 0, 0, form, 2, 0, fcol, bcol, 0, 0, 0, txt, NULL);
}

OBJECT *ITextObject(const char **txt, int *index, int tx, int ty, int fcol, int bcol = -1, BYTE form = 0)
{ // brutal ptr conversion
	return NewObject("[Const Indexed Text]", TEXT, 0, 0, tx, ty, 0, 0, form, 2, 0, fcol, bcol, 0, 0, 0, ((char*)txt), index);
}

OBJECT *NewButton(const char *title, BYTE hide, int tx, int ty, int wdt, int hgt, BYTE xrval, BYTE noulr)
{
	return NewObject(title, BUTTON, 1, hide, tx, ty, wdt, hgt, xrval, noulr, 0, 0, 0, 0, 0, 0, NULL, NULL);
}

OBJECT *NewFlagBox(const char *title, int tx, int ty, int *valptr, BYTE hide = HIDEOFF)
{
	return NewObject(title, FLAGBOX, 1, hide, tx, ty, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, valptr);
}

//-------------------------- Object Mark & Tab --------------------------------

OBJECT *WhichObject(OBJECT *cobj, int tx, int ty)
{
	while (cobj)
	{
		if (Inside(tx - cobj->x, 0, cobj->wdt - 1) && Inside(ty - cobj->y, 0, cobj->hgt - 1)) break;
		cobj = cobj->next;
	}
	return cobj;
}

void MarkObject(OBJECT *obj)
{
	if (!obj || !obj->inwin) return;
	DFrame(obj->inwin->x + obj->x - 1, obj->inwin->y + obj->y - 1, obj->inwin->x + obj->x + obj->wdt, obj->inwin->y + obj->y + obj->hgt, CBlue);
}

void UnMarkObject(OBJECT *obj)
{
	BYTE unmcol;
	if (!obj || !obj->inwin) return;
	unmcol = CGray1;
	if (obj->inwin->type == CARDBOX) unmcol = CGray5; if (obj->inwin->type == STANDARD) unmcol = CGray3;
	if (obj->inwin->type == PLAIN) unmcol = CGray5;
	DFrame(obj->inwin->x + obj->x - 1, obj->inwin->y + obj->y - 1, obj->inwin->x + obj->x + obj->wdt, obj->inwin->y + obj->y + obj->hgt, unmcol);
}

OBJECT *FirstAccObject(WINDOW *win)
{
	OBJECT *rval;
	if (!win) return NULL;
	for (rval = win->fobj; rval && (!rval->acc || (rval->hid == HIDE)); rval = rval->next);
	return rval;
}

void TabObject(WINDOW *win, OBJECT **obj)
{
	if (!win || !*obj) return;
	UnMarkObject(*obj);
	do
	{ // Next Object
		*obj = (*obj)->next; if (!*obj) *obj = win->fobj;
	} while (!((*obj)->acc) || ((*obj)->hid == HIDE));
}

void ReTabObject(WINDOW *win, OBJECT **obj)
{
	OBJECT *prev;
	if (!win || !*obj) return;
	UnMarkObject(*obj);
	do
	{ // Previous Object
		prev = win->fobj; while (prev) { if (prev->next == *obj) break; prev = prev->next; }
		if (prev) *obj = prev;
		else for (*obj = win->fobj; *obj && (*obj)->next; *obj = (*obj)->next);
	} while (!((*obj)->acc) || ((*obj)->hid == HIDE));
}

//--------------------------- Window Operation -------------------------------

WINDOW *NewWindow(const char *title, BYTE type, BYTE prior, int x, int y, int wdt, int hgt, int num, int outof, BYTE escxrv, char *hindex)
{
	WINDOW *nwin;

	if (!(nwin = new WINDOW)) { SVIInitErrors++; return NULL; }

	nwin->type = type; nwin->priority = prior;
	if (x == -1) x = 160 - wdt / 2; if (y == -1) y = 100 - hgt / 2; // center window
	nwin->x = x; nwin->y = y;
	nwin->wdt = wdt; nwin->hgt = hgt;
	nwin->cbnum = num; nwin->cboutof = outof;
	nwin->escxrv = escxrv;
	if (nwin->cboutof == 0) nwin->cboutof = 1; // Safety (->err msg?)
	SCopy(title, nwin->title, WinTitleLen);
	nwin->hindex = hindex;
	nwin->drawn = 0;

	nwin->fobj = NULL;

	nwin->next = FirstWindow;
	FirstWindow = nwin;

	return nwin;
}

void DrawWindow(WINDOW *win)
{
	int dvmx, dvmwdt;
	OBJECT *cobj;
	WINDOW *cwin;
	BYTE drwobjs = 1;

	if (!FirstWindow) return;
	if (!win) win = FirstWindow;

	LPage(SVIPage);
	// Draw Window
	switch (win->type)
	{
	case NOBACK:
		break;
	case STANDARD:
		DrawStd3D(win->x, win->y, win->x + win->wdt - 1, win->y + win->hgt - 1, 0);
		DBox(win->x + win->wdt, win->y + 2, win->x + win->wdt + 1, win->y + win->hgt + 1, CGray1);
		DBox(win->x + 2, win->y + win->hgt, win->x + win->wdt + 1, win->y + win->hgt + 1, CGray1);
		if (win->cbnum) // For STANDARD & PLAIN wins, cbnum means title
		{
			SOut(win->title, win->x + win->wdt / 2, win->y + 2, CGray1, -1, 1);
			DFrame(win->x + win->wdt / 2 - SLen(win->title) * 2, win->y + 8, win->x + win->wdt / 2 + SLen(win->title) * 2 - 2, win->y + 8, CGray1);
		}
		break;
	case CARDBOX:
		dvmwdt = win->wdt / win->cboutof; dvmx = dvmwdt*(win->cbnum - 1);
		DBox(win->x + 1, win->y + 1, win->x + win->wdt - 2, win->y + win->hgt - 2, CGray5);
		DFrame(win->x, win->y, win->x + win->wdt - 1, win->y + win->hgt - 1, CGray1);
		DBox(win->x + win->wdt, win->y + 2, win->x + win->wdt + 1, win->y + win->hgt + 1, CGray1);
		DBox(win->x + 2, win->y + win->hgt, win->x + win->wdt + 1, win->y + win->hgt + 1, CGray1);
		DBox(win->x + dvmx + 1, win->y - 9, win->x + dvmx + dvmwdt - 3, win->y, CGray5);
		DLine(win->x + dvmx, win->y, win->x + dvmx + 1, win->y - 10, CGray1);
		DLine(win->x + dvmx + dvmwdt - 2, win->y, win->x + dvmx + dvmwdt - 3, win->y - 10, CGray1);
		DFrame(win->x + dvmx + 1, win->y - 10, win->x + dvmx + dvmwdt - 3, win->y - 10, CGray1);
		SOut(win->title, win->x + dvmx + dvmwdt / 2, win->y - 7, CGray1, -1, 1);
		break;
	case PLAIN:
		DBox(win->x + 1, win->y + 1, win->x + win->wdt - 2, win->y + win->hgt - 2, CGray5);
		DFrame(win->x, win->y, win->x + win->wdt - 1, win->y + win->hgt - 1, CGray1);
		DBox(win->x + win->wdt, win->y + 2, win->x + win->wdt + 1, win->y + win->hgt + 1, CGray1);
		DBox(win->x + 2, win->y + win->hgt, win->x + win->wdt + 1, win->y + win->hgt + 1, CGray1);
		if (win->cbnum) // For STANDARD & PLAIN wins, cbnum means title
		{
			SOut(win->title, win->x + win->wdt / 2, win->y + 2, CGray1, -1, 1);
			DFrame(win->x + win->wdt / 2 - SLen(win->title) * 2, win->y + 8, win->x + win->wdt / 2 + SLen(win->title) * 2 - 2, win->y + 8, CGray1);
		}
		break;
	case DELETED:
		SystemError("DrwWin: Draw deleted error");
		return;
	default:
		SystemError("DrwWin: Undefined Window Type");
		return;
	}
	// Hightlight priority windows
	/*if (win->priority)
	  DFrame(win->x-1,win->y-1,win->x+win->wdt+2,win->y+win->hgt+2,CDGreen);*/
	  // Don't draw objects on cardbox if it isn't the top one
	  /*if (win->type==CARDBOX)
		for (cwin=FirstWindow; cwin; cwin=cwin->next)
		  if (cwin->type==CARDBOX)
		{
		if (cwin!=win) drwobjs=0;
		break;
		}*/
		// Draw Objects
	if (drwobjs)
		for (cobj = win->fobj; cobj; cobj = cobj->next)
		{
			DrawObject(cobj, 0);
			if (cobj->acc) UnMarkObject(cobj);
		}

	win->drawn = 1;
}

/*void DrawWindows(void)
  {
  WINDOW *cwin,*prev;
  for (cwin=FirstWindow; cwin && cwin->next; cwin=cwin->next);
  do
	{
	DrawWindow(cwin);
	prev=FirstWindow; while (prev) { if (prev->next==cwin) break; prev=prev->next; }
	cwin=prev;
	}
  while (cwin);
  }*/

BYTE ExecCloseExULinks(WINDOW *win) // Returns abort CloseWin
{
	BYTE abort = 0;
	ULINK *clnk;

	for (clnk = FirstULink; clnk; clnk = clnk->next)
		if (clnk->fwin == win)
			if (clnk->type == CLOSEXC)
				if (clnk->exfunc)
					if (clnk->exfunc(clnk->expar))
						abort = 1;

	return abort;
}

void RedrawArea(int x1, int y1, int x2, int y2)
{        // Can't do viewport-redraw unless complete cover is assured...
//Viewport(x1,y1,x2,y2);
	DrawWindow(AreaCoveredByWin(x1, y1, x2, y2)); // Assume win covers completely
	//NoViewport();
}

void CloseWindow(WINDOW *win, BYTE noredr = 0)
{
	OBJECT *nobj;  // Will delete any ULink with fwin/fobj/tobj in this window
	WINDOW *prev;

	if (ExecCloseExULinks(win)) return; // Abort?

	DeleteULinksFromWindow(win);

	while (win->fobj) // Delete Objects
	{
		nobj = win->fobj->next;
		if (win->fobj->type == TEXT) // Clear XTBufs
			if (win->fobj->TXCType == 1)
				delete[](win->fobj->bptr);
		delete win->fobj;
		win->fobj = nobj;
	}

	for (prev = FirstWindow; prev; prev = prev->next) if (prev->next == win) break;
	if (prev) prev->next = win->next;
	if (win == FirstWindow) FirstWindow = win->next;

	if (!noredr)
		RedrawArea(win->x, win->y, win->x + win->wdt - 1 + 2, win->y + win->hgt - 1 + 2);

	delete win;
}

void CloseWindows(void)
{
	while (FirstWindow) CloseWindow(FirstWindow, 1);
}

void XRVWindowClosing(void)
{
	WINDOW *cwin, *next;
	for (cwin = FirstWindow; cwin; cwin = next)
	{
		next = cwin->next; if (cwin->type == DELETED) CloseWindow(cwin);
	}
}


BYTE OnWindow(int tx, int ty, WINDOW *win)
{
	int dvmx, dvmwdt;
	if (Inside(tx - win->x, 0, win->wdt - 1) && Inside(ty - win->y, 0, win->hgt - 1)) return 1;
	if (win->type == CARDBOX)
	{
		dvmwdt = win->wdt / win->cboutof; dvmx = dvmwdt*(win->cbnum - 1);
		if (Inside(tx - win->x - dvmx, 0, dvmwdt - 1) && Inside(ty - win->y, -10, 0)) return 1;
	}
	return 0;
}

WINDOW *WhichWindow(int x, int y)
{
	WINDOW *cwin = FirstWindow;
	while (cwin)
	{
		if (OnWindow(x, y, cwin)) break;
		cwin = cwin->next;
	}
	return cwin;
}

void PrimeWindow(WINDOW *win)
{
	WINDOW *prev;
	prev = FirstWindow; while (prev) { if (prev->next == win) break; prev = prev->next; }
	if (prev) prev->next = win->next;
	win->next = FirstWindow;
	FirstWindow = win;
	DrawWindow(win);
}

BYTE WindowAction(WINDOW *win, int acx, int acy, OBJECT **rcobj)
{                                  // Returns eXitRVal, Zero is no exit
	OBJECT *cobj = WhichObject(win->fobj, acx, acy);
	BYTE xrval = 0;

	switch (win->type)
	{
	case NOBACK: case STANDARD: case CARDBOX: case PLAIN:
		break;
	case DELETED:
		SystemError("WindowAction: DELETED win");
		return 0;
	default:
		SystemError("WindowAction: undefined window type");
		return 0;
	}

	if (cobj && (cobj->hid != HIDE))
	{
		xrval = MsObjAction(cobj, acx - cobj->x, acy - cobj->y);
		if (cobj->type == TEXTBOX)
		{
			UnMarkObject(*rcobj); *rcobj = cobj;
		}
	}

	return xrval;
}

void TabWindow(OBJECT **cobj)
{
	WINDOW *lwin;
	if (!FirstWindow->priority)
	{
		UnMarkObject(*cobj);
		for (lwin = FirstWindow; lwin && lwin->next; lwin = lwin->next);
		PrimeWindow(lwin);
		*cobj = FirstAccObject(FirstWindow);
	}
}

//------------------------ SVI Hyper Help System -----------------------------

const int HyperTBufSize = 6000;
const int MaxHelpIndex = 5;

char *HyperTBuf = NULL;
FILE *HelpFile = NULL;
char *HelpFileName;
OBJECT *HelpHTObject, *HelpIndexTBObject;

char HelpTBIndex[50 + 1];
char HelpIndex[MaxHelpIndex][50 + 1];
int  HelpIndexLine[MaxHelpIndex];
int  CHelpIndex;

int HyperTLineNum(char *htbuf, int wdt)
{
	int rval;
	FTOutSt(htbuf, wdt, 0, &rval);
	return rval;
}

int ClosePowerHelp(int dummy)
{
	if (HelpFile) fclose(HelpFile);
	if (HyperTBuf) delete[] HyperTBuf;
	HelpRunning = 0;
	return dummy;
}

void LoadHyperTBuf(void)
{
	char hindex[52 + 1];
	int hilen;
	hindex[0] = '['; SCopy(HelpIndex[CHelpIndex], hindex + 1, 50); hilen = SLen(hindex); hindex[hilen] = ']'; hindex[hilen + 1] = 0;
	Capitalize(hindex);
	HyperTBuf[0] = 0;
	if (!LocateInFile(HelpFile, hindex))
		SCopy("No entry found in help file for given index.", HyperTBuf);
	else
	{
		AdvanceFileLine(HelpFile);
		ReadFileUntil(HelpFile, HyperTBuf, '[', HyperTBufSize);
	}
}

void AdjustPowerHelpIndex(void)
{
	int maxhtli;
	// Load indexed paragraph to buffer
	LoadHyperTBuf();
	// Adjust HT object & scroll bar
	maxhtli = Max(HyperTLineNum(HyperTBuf, 272 / 4) - 108 / 6, 0);
	AdjustHTLines(HelpHTObject, HelpIndexLine[CHelpIndex], maxhtli);
	// Adjust index TB object
	SCopy(HelpIndex[CHelpIndex], HelpTBIndex);
	HelpIndexTBObject->TBCursor = SLen(HelpTBIndex);
	DrawObject(HelpIndexTBObject, 1);
}

int PreviousHelpIndex(int dummy)
{
	if (CHelpIndex > 0)
	{
		CHelpIndex--;
		AdjustPowerHelpIndex();
	}
	return dummy;
}

int TBEntryHelpIndex(int dummy)
{
	NewHelpIndex(HelpTBIndex);
	return dummy;
}

int ContentsHelpIndex(int dummy)
{
	NewHelpIndex("Contents");
	return dummy;
}

BYTE InitHelpPage(void)
{
	WINDOW *cwin;
	int maxhtli;
	cwin = NewWindow("[HelpWin]", STANDARD, 1, -1, 40, 300, 140, 0, 0, XRVCLOSE, "[No help on help]");
	TextObject("SVI Power-Help|Version 1.1", 265, 2, CGray2, -1, 1);

	NewButton("OK", 0, 225, 127, 70, 9, XRVCLOSE, 0);

	NewULink(EXCREDR, NULL,
		NewButton("Contents", 0, 80, 127, 70, 9, 0, 0),
		NULL, &ContentsHelpIndex, 0);

	NewULink(EXCREDR, NULL,
		NewButton("Previous", 0, 5, 127, 70, 9, 0, 0),
		NULL, &PreviousHelpIndex, 0);

	maxhtli = Max(HyperTLineNum(HyperTBuf, 272 / 4) - 108 / 6, 0);
	HelpHTObject = NewObject("[HelpHyperT]", HYPERTEXT, 1, 0, 5, 14, 272 + 3, 108 + 3, 0, 0, 0, 0, maxhtli, 0, 0, 0, HyperTBuf, NULL);

	NewULink(EXCREDR, NULL,
		HelpIndexTBObject = NewObject("Index:", TEXTBOX, 1, 0, 30, 3, 200 + 3, 9, 2, 0, 0, 50, SLen(HelpTBIndex), 0, 0, 0, HelpTBIndex, NULL),
		NULL, &TBEntryHelpIndex, 0);

	NewULink(CLOSEXC, cwin, NULL, NULL, &ClosePowerHelp, 0);
	if (InitErrorCheck()) { CloseWindow(cwin); return 0; }
	return 1;
}

void NewHelpIndex(char *index)
{
	int cnt;

	// Store current HT line index in HelpIndexLine[C] because
	// HT object has so far only modified its own HTLineIndex
	HelpIndexLine[CHelpIndex] = HelpHTObject->HTLineIndex;

	// Advance CHelpIndex pointer or scroll down history entries
	if (CHelpIndex + 1 < MaxHelpIndex)
		CHelpIndex++;
	else
		for (cnt = 0; cnt < MaxHelpIndex - 1; cnt++)
		{
			SCopy(HelpIndex[cnt + 1], HelpIndex[cnt]);
			HelpIndexLine[cnt] = HelpIndexLine[cnt + 1];
		}

	// Activate new index
	SCopy(index, HelpIndex[CHelpIndex], 50);
	HelpIndexLine[CHelpIndex] = 0;
	AdjustPowerHelpIndex();
}

void OpenPowerHelp(char *index)
{
	CHelpIndex = 0;
	SCopy(index, HelpIndex[CHelpIndex], 50);
	HelpIndexLine[CHelpIndex] = 0;
	SCopy(HelpIndex[CHelpIndex], HelpTBIndex);

	// Allocate buffer
	if (!(HyperTBuf = new char[HyperTBufSize + 1]))
	{
		SystemError("insufficient memory|for help system"); ClosePowerHelp(0); return;
	}
	// Open file
	if (!(HelpFile = fopen(HelpFileName, "r")))
	{
		SystemError("error opening help file"); ClosePowerHelp(0); return;
	}
	// Load indexed paragraph to buffer
	LoadHyperTBuf();
	// Init help page
	if (!InitHelpPage())
	{
		ClosePowerHelp(0); return;
	}

	HelpRunning = 1;
}

//--------------------------- User Interface ---------------------------------

COMMAND InCom(BYTE wfm)
{
	BYTE msbk[81];
	COMMAND mstat;
	int xcnt, ycnt;
	char ostr[10];
	LPage(SVIPage);
	mstat.cn = 0; mstat.xn = 0; mstat.b = Mousebut(); mstat.x = MouseX(); mstat.y = MouseY();
	if (Mousebut()) return mstat;
	do
	{
		mstat.x = MouseX(); mstat.y = MouseY();
		for (ycnt = 0; ycnt < 9; ycnt++)
			for (xcnt = 0; xcnt < 9; xcnt++)
				msbk[ycnt * 9 + xcnt] = GPixF(mstat.x + xcnt - CBAX, mstat.y + ycnt - CBAY);
		DrawMouse(mstat.x, mstat.y);
		while ((MouseX() == mstat.x) && (MouseY() == mstat.y) && !CheckKeyOrMouse());
		for (ycnt = 0; ycnt < 9; ycnt++)
			for (xcnt = 0; xcnt < 9; xcnt++)
				SPixF(mstat.x + xcnt - CBAX, mstat.y + ycnt - CBAY, msbk[ycnt * 9 + xcnt]);
	} while (!CheckKeyOrMouse());
	/*mstat.b = Mousebut(); // I used to know what cn and xn stood for, but this is useless anyway
	if (!mstat.b)
	{
		mstat.cn = getch(); if (!mstat.cn) mstat.xn = getch();
	}*/
	if (wfm) while (Mousebut());
	return mstat;
}

//----------------------------- Run SVI --------------------------------------

BYTE RunSVI(void) // Error/No windows returns 0
{
	COMMAND inc;
	WINDOW *cwin, *LFWin = NULL;
	OBJECT *cobj;
	BYTE xrval = 0, kbspc, kbstn;

	SVIRunning = 1; OpenHelp = 0;

	PPage(SVIPage);

	do
	{

		// Window/object new/status check - - - - - - - - - - - - - - - - - - - -

		if (!FirstWindow) { SVIRunning = 0; return 0; }

		if (FirstWindow != LFWin) // A window has been opened or closed or primed
		{
			if (!FirstWindow->drawn) DrawWindow(FirstWindow);
			cobj = FirstAccObject(FirstWindow);
			LFWin = FirstWindow;
		}

		if (!cobj) { SystemError("No accessible object in prime window|Aborting SVI-run"); SVIRunning = 0; return 0; }

		if (cobj->hid == HIDE) // cobj may have been hidden by executed ULink
		{
			UnMarkObject(cobj);
			cobj = FirstAccObject(FirstWindow);
		}

		MarkObject(cobj);

		// InCom - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		//while (Mousebut()); ???

		//IntegrityC(5);

		//PPage(SVIPage);
		//inc = InCom(0);

		COMMAND inc;
		inc.cn = 0; inc.xn = 0;
		SDL_ShowCursor(1);

		UpdateScreen();
		bool exitloop = false;
		do
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				inc.b = MouseState(&inc.x, &inc.y);
				switch (event.type)
				{
				case SDL_MOUSEBUTTONDOWN:
					exitloop = true;
					inc.x = event.button.x / PortScreenScaleFactor;
					inc.y = event.button.y / PortScreenScaleFactor;
					inc.b = SDL_BUTTON(event.button.button);
					break;

				case SDL_KEYDOWN:
					exitloop = true;
					inc.cn = event.key.keysym.sym;
					inc.xn = inc.cn;
					break;

				case SDL_QUIT:
					exitloop = true;
					inc.cn = SDLK_ESCAPE;
					inc.xn = 0;
					break;
				}
			}
			SDL_Delay(20);
		} while (!exitloop);

		// Mouse Action - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		if (inc.b == 1)
		{
			cwin = WhichWindow(inc.x, inc.y);
			if (cwin)
			{
				if (cwin == FirstWindow)
					xrval = WindowAction(FirstWindow, inc.x - cwin->x, inc.y - cwin->y, &cobj);
				else
					if (!FirstWindow->priority)
					{
						UnMarkObject(cobj);
						PrimeWindow(cwin);
						xrval = WindowAction(cwin, inc.x - cwin->x, inc.y - cwin->y, &cobj);
					}
			}
		}

		// Keyboard Action - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		kbspc = KBNONE; kbstn = 0;

		switch (inc.cn)
		{
		case KCTAB:   TabObject(FirstWindow, &cobj); break;
		case KCESC:   xrval = EvaluateXRVal(FirstWindow->escxrv, FirstWindow); break;
		case KCENTER: kbspc = KBENTER;                break;
		case KCBACK:  kbspc = KBBACK;		  break;

		default:
			if (Inside(inc.cn, 32, 126) || Inside(inc.cn, 129, 154))
			{
				kbspc = KBSTN; kbstn = inc.cn;
			}
			break;

			/*default:
		  sprintf(OSTR,"keyboard code %d",inc.cn);
		  SystemError(OSTR);
		  break;*/

		}

		switch (inc.xn)
		{
		case KCSHTAB: ReTabObject(FirstWindow, &cobj); break;
		case KCALTAB: TabWindow(&cobj); break;
		case KCCRSLF:  kbspc = KBLEFT;  break;
		case KCCRSRT:  kbspc = KBRIGHT; break;
		case KCCRSUP:  kbspc = KBUP;    break;
		case KCCRSDN:  kbspc = KBDOWN;  break;
		case KCHOME:   kbspc = KBHOME;  break;
		case KCEND:    kbspc = KBEND;   break;
		case KCDEL:    kbspc = KBDEL;   break;
		case KCPAGEUP: kbspc = KBPGUP;  break;
		case KCPAGEDN: kbspc = KBPGDN;  break;

		case KCF1: OpenHelp = 1; break;

			/*default:
		  if (inc.xn)
			{
			sprintf(OSTR,"extended keyboard code %d",inc.xn);
			SystemError(OSTR);
			}
		  break;*/
		}

		if (kbspc != KBNONE) xrval = KbObjAction(cobj, kbspc, kbstn);

		// Special XRValue evaluation - - - - - - - - - - - - - - - - - - - - - -

		if (xrval == XRVTAB_O) { TabObject(FirstWindow, &cobj); xrval = 0; }

		// Window closing check - - - - - - - - - - - - - - - - - - - - - - - - - -

		XRVWindowClosing();

		// ExecAULinks - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		ExecAULinks();

		// OpenPowerHelp - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		if (OpenHelp) if (FirstWindow) if (!HelpRunning)
			OpenPowerHelp(FirstWindow->hindex);
		OpenHelp = 0;

	} while (!xrval);
	SVIRunning = 0;
	return xrval;
}

void ClearSVI(void)
{
	CloseWindows(); // Clears all objects
	DeleteULinks(); // All ULs should have been deleted by CloseWins
}

//--------------------------- Instant Windows --------------------------------

void Message(const char *txt, char *hindex = NULL)
{
	WINDOW *cwin;
	int twdt, thgt;
	if (!hindex) hindex = "No help";
	TOutSt(txt, &twdt, &thgt);
	twdt = Max(twdt * 4 + 20, 70); thgt = thgt * 6 + 20;
	cwin = NewWindow("Message", STANDARD, 1, -1, -1, twdt, thgt, 0, 0, XRVCLOSE, hindex);
	TextObject(txt, twdt / 2, 4, CGray1, -1, 1);
	NewObject("OK", BUTTON, 1, 0, twdt / 2 - 20, thgt - 13, 40, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

	if (InitErrorCheck())  CloseWindow(cwin);
	else { /*DrawWindow(cwin);*/ if (!SVIRunning) RunSVI(); }
}

void ConfirmedCall(const char *txt, int ctype, int(*exfunc)(int), int expar, char *hindex = NULL)
{
	char *posbut[3] = { "OK","Yeah","Yes" };
	char *negbut[3] = { "Cancel","Nope","No" };
	WINDOW *cwin;
	int twdt, thgt;
	if (!hindex) hindex = "No help";
	TOutSt(txt, &twdt, &thgt);
	twdt = Max(twdt * 4 + 20, 100); thgt = thgt * 6 + 20;
	cwin = NewWindow("Message", STANDARD, 1, -1, -1, twdt, thgt, 0, 0, XRVCLOSE, hindex);
	TextObject(txt, twdt / 2, 4, CGray1, -1, 1);
	NewObject(negbut[ctype], BUTTON, 1, 0, twdt / 2 + 2, thgt - 13, 40, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	NewULink(EXCREDR, NULL,
		NewObject(posbut[ctype], BUTTON, 1, 0, twdt / 2 - 42, thgt - 13, 40, 9, XRVCLOSE, 1, 0, 0, 0, 0, 0, 0, NULL, NULL),
		NULL, exfunc, expar);
	if (InitErrorCheck()) CloseWindow(cwin);
	else { /*DrawWindow(cwin);*/ if (!SVIRunning) RunSVI(); }
}

void LineInput(const char *msg, char *txt, int maxlen, int(*exfunc)(int) = NULL, int expar = 0, char *hindex = NULL)
{
	WINDOW *cwin;
	int winwdt = Max(Max(maxlen * 4 + 4 + 10, SLen(msg) * 4 + 10), 50);
	if (!hindex) hindex = "No help";
	cwin = NewWindow("[LineInput]", STANDARD, 1, -1, -1, winwdt, 32, 0, 0, XRVCLOSE, hindex);
	TextObject(msg, 5, 2, CGray1);
	NewObject("OK", BUTTON, 1, 0, winwdt - 46, 20, 40, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	NewObject("[TextBox]", TEXTBOX, 1, 0, 5, 9, maxlen * 4 + 4, 9, 0, XRVCLOSE, 0, maxlen, 0, 0, 0, 0, txt, NULL);
	NewULink(CLOSEXC, cwin, NULL, NULL, exfunc, expar);
	if (InitErrorCheck()) CloseWindow(cwin);
	else if (!SVIRunning) RunSVI();
}

//----------------------- Initialization Functions ---------------------------

void SVISetExternFunctions(void(*dlbc)(int, int, int, int, BYTE), BYTE(*lbca)(int, int, int))
{
	ExtDrawLBC = dlbc;
	ExtLBCAction = lbca;
}

void InitMsg(const char *msg, BYTE col = CIMsg)
{
	LPage(SVIPage);
	SOut(msg, IMsgX, IMsgY, col);
	IMsgX = 0; IMsgY += 6;

	UpdateScreen();
	//SDL_Delay(100); // Sonst gehts ja viel zu schnell. Wir wollen ja noch ein bisschen das DOS-Gef�hl haben.
}

void InitMsgOpen(const char *msg, BYTE col = CIMsg)
{
	LPage(SVIPage);
	SOut(msg, IMsgX, IMsgY, col);
	IMsgX += 4 * SLen(msg);

	UpdateScreen();
}

void ResetInitMsg(void)
{
	DefColors();
	IMsgX = 0; IMsgY = 0;
	InitMsg("RW\\D SVI 2.0", CGray4);
	InitMsg("");
}

BYTE InitSVI(BYTE svipge, char *helpfname)
{
	if (!IsVGA()) return 0;
	InitVGA();
	DefColors();
	SVIPage = svipge;
	HelpFileName = helpfname;
	ResetInitMsg();
	if (!InitSVIMouse()) InitMsg("No mouse available");
	return 1;
}

void CloseSVI(void)
{
	CloseWindows();
	CloseVGA();
}
