/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design CLONK Miner's BattleSequenceGraphicsSystem Module

//---------------------------- Include Headers -------------------------------

#define _USE_MATH_DEFINES

//#include <dos.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "standard.h"
#include "std_gfx.h"
#include "vga4.h"
#include "sprite.h"

#include "clonk_bs.h"

#include "bsexcsys.h"
#include "bsmansys.h"
#include "bspxssys.h"
#include "bsweasys.h"
#include "bsstrsys.h"
#include "bsvhcsys.h"

#include "RandomWrapper.h"

//---------------------------- Extern AGC Loader -----------------------------

extern BYTE LoadAGC2PageV1(char *fname, BYTE tpge, int tx, int ty, WORD *iwdt, WORD *ihgt, BYTE *palptr, BYTE *scram);

//-------------------------- Vehicle Data Definitions -------------------------

#define VHDCatPhase   back[0]

#define VHDXBowPhase  back[0]
#define VHDXBowDir    back[2]

#define VHDWipfLoad   back[29]

#define VHDAddToWait  back[25]
#define VHDBallCrash  back[28]

//-------------------------- Main Global Externals ---------------------------

extern USERPREF UserPref;
extern BSATYPE BSA;
extern CONFIG Config;

extern char OSTR[500];

extern void EventCall(int evnum);

extern MOUSECON MouseCon;

//-------------------------- Global Game Variables ---------------------------

BYTE CPGE;
int FPS = 0, LowFPSMark = 30;
BYTE LowFPS = 0;

BYTE FPhase = 0;

//-------------------------- Game Message Variables ---------------------------

typedef struct GAMEMSG {
	char txt[101];
	int plr, tx, ty, col;
	MANTYPE *tman;
	int time;
};

const int GMNum = 4;

GAMEMSG GMsg[GMNum];

//------------------------------ InitBack Data ---------------------------------

#define SPSAND    0
#define SPGRANITE 1
#define SPROCK    2
#define SPGOLD    3
#define SPOTHER   4
#define SPWATER   5
#define SPOIL     6

const int IBSpotType[3][10] = { SPSAND,SPSAND,SPGRANITE,SPROCK,SPROCK,SPWATER,SPWATER,SPROCK,SPOIL,SPOIL,
				 SPSAND,SPGRANITE,SPGRANITE,SPROCK,SPROCK,SPROCK,SPWATER,SPOIL,SPGOLD,SPGOLD,
				 SPGRANITE,SPGRANITE,SPROCK,SPROCK,SPROCK,SPROCK,SPWATER,SPGOLD,SPGOLD,SPGOLD };
// adjust to gmode
//---------------------------- Sprite Variables -------------------------------

const int MASprNum = 53;

ASPRITE ManSpr[MASprNum];
ASPRITE RockSpr[RockTypeNum];
ASPRITE FireSpr[4];
ASPRITE FlagSpr[4];

ASPRITE SharkSpr[2];
ASPRITE WipfSpr[9];
ASPRITE MonsterSpr[6];
BSPRITE DeadMonster;

BSPRITE StructSpr[10];
BSPRITE ConstrSignSpr;
BSPRITE WindMillSpr[4];
BSPRITE ElevWheelSpr[2];
BSPRITE PumpSpr[3];
BSPRITE TowerSpr;

ASPRITE BoatSpr, BoatSprSail;
ASPRITE BalloonSpr;
ASPRITE LorrySpr[3];
ASPRITE CatSpr[3][2];
BSPRITE ElevatorSpr;

BSPRITE ThreeSpr;

const int MASpr[ManActNum - 1][2] = { 0,1,2,3,4,5,6,6,7,8,9,10,9,10,2,3,11,12,13,13 };
const int STSpr[StructTypeNum] = { 0,1,6,0,1,5,7,8, 9,0,0, 2,3,4 };

//-------------------------- Sprite Initialization ----------------------------

#define NOCOL   0
#define CLRD    1
#define CLRONLY 2

#define NOSHIFT 0
#define CSHIFT  1

BYTE InitBSSprites(void)
{
	int cnt1;

	SpriteInitError = 0;
	SetColorBase(64, 0, 191);

	// Men
	for (cnt1 = 0; cnt1 < MASprNum - 1; cnt1++)
		CreateASprite(&ManSpr[cnt1], GFXPage, 48 * (cnt1 % 4), 9 * (cnt1 / 4), 8, 9, CSHIFT, CLRD);
	CreateASprite(&ManSpr[MASprNum - 1], GFXPage, 0, 117, 8, 9, NOSHIFT, CLRD);
	// Fire
	for (cnt1 = 0; cnt1 < 4; cnt1++)
		CreateASprite(&FireSpr[cnt1], GFXPage, 240, 10 * cnt1, 8, 10, CSHIFT);
	// Flags
	for (cnt1 = 0; cnt1 < 4; cnt1++)
		CreateASprite(&FlagSpr[cnt1], GFXPage, 240, 40 + 4 * cnt1, 4, 4, CSHIFT, CLRD);
	// Rocks
	CreateASprite(&RockSpr[ROCK], GFXPage, 288, 4 * ROCK, 4, 4);
	for (cnt1 = GOLD; cnt1 <= LIQEARTH; cnt1++)
		CreateASpriteAlias(&RockSpr[cnt1], GFXPage, 288, 4 * cnt1, 4, 4, &RockSpr[ROCK]);
	CreateASprite(&RockSpr[STEEL], GFXPage, 288, 4 * STEEL, 4, 4);
	CreateASprite(&RockSpr[MONSTEGG], GFXPage, 288, 4 * MONSTEGG, 4, 4);
	CreateASprite(&RockSpr[ZAPN], GFXPage, 288, 4 * ZAPN, 4, 4);
	for (cnt1 = ZUPN; cnt1 <= OILBARREL; cnt1++)
		CreateASpriteAlias(&RockSpr[cnt1], GFXPage, 288, 4 * cnt1, 4, 4, &RockSpr[ZAPN], CSHIFT*(cnt1 >= BARREL));
	CreateASprite(&RockSpr[CONKIT], GFXPage, 288, 4 * CONKIT, 4, 4, CSHIFT);
	CreateASpriteAlias(&RockSpr[LINECON], GFXPage, 288, 4 * LINECON, 4, 4, &RockSpr[CONKIT], CSHIFT);
	CreateASprite(&RockSpr[ARROW], GFXPage, 288, 4 * ARROW, 4, 4, CSHIFT);
	for (cnt1 = FARROW; cnt1 <= BARROW; cnt1++)
		CreateASpriteAlias(&RockSpr[cnt1], GFXPage, 288, 4 * cnt1, 4, 4, &RockSpr[ARROW], CSHIFT*(cnt1 >= BARREL));
	for (cnt1 = PLANT1; cnt1 <= PLANT3; cnt1++)
		CreateASprite(&RockSpr[cnt1], GFXPage, 288, 4 * cnt1, 4, 4, CSHIFT);
	CreateASprite(&RockSpr[FLAG], GFXPage, 288, 4 * FLAG, 4, 4, CSHIFT, CLRD);
	CreateASprite(&RockSpr[COMET], GFXPage, 288, 4 * COMET, 4, 4, CSHIFT);
	CreateASprite(&RockSpr[ROCKPXS], GFXPage, 288, 4 * ROCKPXS, 4, 4, CSHIFT); // nouse
	// Animals
	for (cnt1 = 0; cnt1 < 2; cnt1++)
		CreateASprite(&SharkSpr[cnt1], GFXPage, 64, 147 + 9 * cnt1, 20, 9);
	for (cnt1 = 0; cnt1 < 9; cnt1++)
		CreateASprite(&WipfSpr[cnt1], GFXPage, 192, 7 * cnt1, 8, 7, CSHIFT);
	for (cnt1 = 0; cnt1 < 2; cnt1++) // Monster
	{
		CreateASprite(&MonsterSpr[cnt1 * 3 + 0], GFXPage, 144 + 64 * cnt1, 165, 12, 6, CSHIFT);
		CreateASprite(&MonsterSpr[cnt1 * 3 + 1], GFXPage, 144 + 64 * cnt1, 171, 12, 7, CSHIFT);
		CreateASprite(&MonsterSpr[cnt1 * 3 + 2], GFXPage, 144 + 64 * cnt1, 178, 8, 2, CSHIFT);
	}
	CreateBSprite(&DeadMonster, GFXPage, 252, 112, 12, 8);
	// Structs
	CreateBSprite(&StructSpr[0], GFXPage, 192, 120, 16, 20); // lgwod
	CreateBSprite(&StructSpr[1], GFXPage, 208, 120, 16, 12); // smlwd
	CreateBSprite(&StructSpr[2], GFXPage, 224, 120, 16, 20); // tree1
	CreateBSprite(&StructSpr[3], GFXPage, 240, 120, 16, 20); // tree2
	CreateBSprite(&StructSpr[4], GFXPage, 256, 120, 16, 20); // tree3
	CreateBSprite(&StructSpr[5], GFXPage, 192, 100, 20, 12); // castl
	CreateBSprite(&StructSpr[6], GFXPage, 212, 100, 16, 12); // oilpw
	CreateBSprite(&StructSpr[7], GFXPage, 228, 100, 16, 12); // house
	CreateBSprite(&StructSpr[8], GFXPage, 176, 120, 16, 20); // tower
	CreateBSprite(&StructSpr[9], GFXPage, 264, 100, 16, 20); // magic

	CreateBSprite(&ConstrSignSpr, GFXPage, 208, 132, 8, 8);
	for (cnt1 = 0; cnt1 < 4; cnt1++)
		CreateBSprite(&WindMillSpr[cnt1], GFXPage, 176 + 24 * cnt1, 140, 24, 20);
	for (cnt1 = 0; cnt1 < 2; cnt1++)
		CreateBSprite(&ElevWheelSpr[cnt1], GFXPage, 208 + 12 * cnt1, 69, 12, 11);
	for (cnt1 = 0; cnt1 < 3; cnt1++)
		CreateBSprite(&PumpSpr[cnt1], GFXPage, 240 + 16 * cnt1, 56, 16, 12);
	CreateBSprite(&TowerSpr, GFXPage, 272, 85, 8, 15);
	// Vehics
	CreateASprite(&BalloonSpr, GFXPage, 192, 80, 16, 20, CSHIFT);
	CreateASprite(&BoatSpr, GFXPage, 0, 128 + 9, 12, 3, CSHIFT);
	CreateASprite(&BoatSprSail, GFXPage, 0, 128, 12, 9, CSHIFT);
	for (cnt1 = 0; cnt1 < 3; cnt1++)
	{
		CreateASprite(&LorrySpr[cnt1], GFXPage, 0, 140 + 10 * cnt1, 12, 10, CSHIFT);
		CreateASprite(&CatSpr[cnt1][0], GFXPage, 0, 170 + 7 * cnt1, 12, 7, CSHIFT);
		CreateASprite(&CatSpr[cnt1][1], GFXPage, 64, 170 + 7 * cnt1, 12, 7, CSHIFT);
	}
	CreateBSprite(&ElevatorSpr, GFXPage, 192, 67, 16, 13);

	CreateBSprite(&ThreeSpr, GFXPage, 196, 160, 20, 5);

	if (SpriteInitError) return 0;

	return 1;
}

void DeInitBSSprites(void)
{
	int cnt1;
	for (cnt1 = 0; cnt1 < MASprNum; cnt1++) DestroyASprite(&ManSpr[cnt1]);
	for (cnt1 = 0; cnt1 < 4; cnt1++) DestroyASprite(&FireSpr[cnt1]);
	for (cnt1 = 0; cnt1 < 4; cnt1++) DestroyASprite(&FlagSpr[cnt1]);
	for (cnt1 = 0; cnt1 < RockTypeNum; cnt1++) DestroyASprite(&RockSpr[cnt1]);

	for (cnt1 = 0; cnt1 < 2; cnt1++) DestroyASprite(&SharkSpr[cnt1]);
	for (cnt1 = 0; cnt1 < 9; cnt1++) DestroyASprite(&WipfSpr[cnt1]);
	for (cnt1 = 0; cnt1 < 6; cnt1++) DestroyASprite(&MonsterSpr[cnt1]);
	DestroyBSprite(&DeadMonster);

	for (cnt1 = 0; cnt1 < 10; cnt1++) DestroyBSprite(&StructSpr[cnt1]);

	for (cnt1 = 0; cnt1 < 4; cnt1++) DestroyBSprite(&WindMillSpr[cnt1]);
	for (cnt1 = 0; cnt1 < 3; cnt1++) DestroyBSprite(&PumpSpr[cnt1]);
	for (cnt1 = 0; cnt1 < 2; cnt1++) DestroyBSprite(&ElevWheelSpr[cnt1]);
	DestroyBSprite(&TowerSpr);
	DestroyBSprite(&ConstrSignSpr);

	DestroyASprite(&BalloonSpr);
	DestroyASprite(&BoatSpr);
	DestroyASprite(&BoatSprSail);
	for (cnt1 = 0; cnt1 < 3; cnt1++)
	{
		DestroyASprite(&LorrySpr[cnt1]);
		DestroyASprite(&CatSpr[cnt1][0]);
		DestroyASprite(&CatSpr[cnt1][1]);
	}
	DestroyBSprite(&ElevatorSpr);

	DestroyBSprite(&ThreeSpr);
}

//----------------------------- SystemMsg System -----------------------------

/*char SysMsgBuf[51]="";

void SystemMsg(char *msg) // take out in release version
  {
  SCopy(msg,SysMsgBuf,50);
  }*/

  //------------------------------- BackV System -----------------------------------

long BackVSize, BackVPos, BackVSPos;
BYTE *BackVBuf;
BYTE BackVLineB[320];

void SBackPix(long tx, long ty, BYTE col) // LPage(BackPage) call required!
{
	if (ty < 0) return; // Out on top cheking
	if (ty < (BackVPos - BackVSPos))
	{
		BackVBuf[tx + 320L * ty] = col; return;
	}
	if (ty - (BackVPos - BackVSPos) < 200)
	{
		SPixA(tx, ty - (BackVPos - BackVSPos), col); return;
	}
	BackVBuf[tx + 320L * (ty - 200L)] = col;
}

BYTE GBackPix(long fx, long fy) // LPage(BackPage) call required
{
	if (fy < 0) return 0; // Out on top cheking
	if (fy < (BackVPos - BackVSPos))
		return BackVBuf[fx + 320L * fy];
	if (fy - (BackVPos - BackVSPos) < 200)
		return GPixA(fx, fy - (BackVPos - BackVSPos));
	return BackVBuf[fx + 320L * (fy - 200L)];
}

void Blast320(void *src, void *trg)
{
	int cnt;
	DWORD *dtrg = (DWORD*)trg; // use inline asm
	DWORD *dsrc = (DWORD*)src;
	for (cnt = 0; cnt < 80; cnt++) *dtrg++ = *dsrc++;
}

void BackVScrollDown(void)
{
	long cnt;
	BYTE *bvbptr;
	if (BackVPos == BackVSize - 160) return;
	LPage(BackPage);
	if (BackVSPos < 40) BackVSPos++;
	else
	{
		bvbptr = BackVBuf + 320L * (BackVPos - 40L);
		// read BufLine BackVPos
		Blast320(bvbptr, BackVLineB);
		// buffer PageLine 0 over old BufLine BackVPos
		for (cnt = 0; cnt < 320; cnt++) *bvbptr++ = GPixA(cnt, 0);
		// shift BackPage up by 1
		B4Move(BackPage, 0, 1, BackPage, 0, 0, 80, 199);
		// draw read BufLine to PageLine 199
		bvbptr = BackVLineB;
		for (cnt = 0; cnt < 320L; cnt++) SPixA(cnt, 199, *bvbptr++);
	}
	BackVPos++;
}

void BackVScrollUp(void)
{
	long cnt;
	BYTE *bvbptr;
	if (BackVPos == 0) return;
	LPage(BackPage);
	if (BackVSPos > 0) BackVSPos--;
	else
	{
		bvbptr = BackVBuf + 320L * (BackVPos - 1L);
		// read BufLine BackVPos-1
		Blast320(bvbptr, BackVLineB);
		// buffer PageLine 199 over old BufLine BackVPos-1
		for (cnt = 0; cnt < 320; cnt++) *bvbptr++ = GPixA(cnt, 199);
		// shift BackPage down by 1
		for (cnt = 198; cnt >= 0; cnt--)
			B4Move(BackPage, 0, cnt, BackPage, 0, cnt + 1, 80, 1);
		// draw read BufLine to PageLine 0
		bvbptr = BackVLineB;
		for (cnt = 0; cnt < 320; cnt++) SPixA(cnt, 0, *bvbptr++);
	}
	BackVPos--;
}

void AdjustBackVPos(int *nvy)
{
	*nvy = BoundBy(*nvy, 0, BackVSize - 160);
	if (BackVPos < *nvy) BackVScrollDown();
	if (BackVPos > *nvy) BackVScrollUp();
}

//------------------------- Background Drawing -------------------------------

BYTE IsSkyBack(BYTE color)
{
	if (Inside(color, CSky1, CSky2)) return 1;
	if (Inside(color, CVhcL, CVhcH)) if ((color - CVhcL) % 2 == 0) return 1;
	if (Inside(color, CPXSL, CPXSH)) if ((color - CPXSL) % 2 == 0) return 1;
	return 0;
}

void DrawSkyPix(int tx, int ty)
{
	SBackPix(tx, ty, CSky1 + (ty / 5) + (random(5) < (ty % 5)));
}

void DrawGroundPix(int tx, int ty)
{
	SBackPix(tx, ty, CEarth1 + 1 + random(CEarth2 - CEarth1));
}

void DrawTunnelPix(int tx, int ty)
{
	SBackPix(tx, ty, CTunnel1 + random(CTunnel2 - CTunnel1 + 1));
}

void DrawBackPix(int tx, int ty)
{
	if (IsSkyBack(GBackPix(tx, ty))) DrawSkyPix(tx, ty);
	else DrawTunnelPix(tx, ty);
}

void DrawLoamChunk(int tx, int ty)
{
	int cnt, cnt2;
	for (cnt = 0; cnt < 2; cnt++) for (cnt2 = 0; cnt2 < 2; cnt2++)
		if (Inside(tx + cnt, 0, 319) && Inside(ty + cnt2, 0, BackVSize - 1))
			if (!Inside(GBackPix(tx + cnt, ty + cnt2), CGranite1, CGold2))
				SBackPix(tx + cnt, ty + cnt2, CEarth1 + 2 + random(2));
}

void DrawSteelChunk(int tx, int ty)
{
	int cnt, cnt2;
	for (cnt = 0; cnt < 2; cnt++) for (cnt2 = 0; cnt2 < 2; cnt2++)
		if (Inside(tx + cnt, 0, 319) && Inside(ty + cnt2, 0, BackVSize - 1))
			SBackPix(tx + cnt, ty + cnt2, CGranite1);
}

void IBDrawGroundPix(int tx, int ty)
{
	if ((BSA.WClim < 200) && (random(50) + 80 > ty)) SBackPix(tx, ty, CSandT);
	else SBackPix(tx, ty, CEarth1 + 1 + random(4));
}

void IBDrawLayerPix(int tx, int ty, BYTE col)
{
	if (Inside(tx, 0, 319) && Inside(ty, 0, BackVSize - 1))
		if (Inside(GBackPix(tx, ty), CEarth1, CSandT))
			SBackPix(tx, ty, col);
}

void IBDrawLayerSpot(int tx, int ty, BYTE type)
{
	int cnt, cnt2;
	switch (type)
	{
	case SPSAND:
		for (cnt = 0; cnt < 7; cnt++) IBDrawLayerPix(tx + random(5) - 2, ty + random(7) - 3, CSandT);
		break;
	case SPGRANITE: case SPROCK: case SPGOLD:
		for (cnt = 0; cnt < 5; cnt++) for (cnt2 = 0; cnt2 < 4; cnt2++)
			if ((cnt + cnt2 != 0) && (cnt + cnt2 != 7))
				IBDrawLayerPix(tx + cnt, ty + cnt2, CGranite1 + 3 * (type - SPGRANITE) + random(3));
		break;
	case SPWATER: case SPOIL:
		for (cnt = 0; cnt < 4; cnt++) for (cnt2 = 0; cnt2 < 4; cnt2++)
			if ((cnt + cnt2 != 0) && (cnt + cnt2 != 6))
				IBDrawLayerPix(tx + cnt, Min(ty + cnt2, BackVSize - 5), (type == SPWATER) ? CWaterT : COilT);
		break;
	}
}


void DrawSurface(int addsize, void(*drawpixfunc)(int tx, int ty))
{
	float phpos, phstep;
	int cnt, cnt2, cten, cx, rcy, acy;
	cten = random(30);   // range 0 to 29
	rcy = random(81) - 40; // range -50 to +50 (init value -40 to +40)
	// Drawing
	phpos = 2.0*M_PI*((float)BSA.BPhase) / 100.0; // Phase Start
	phstep = (0.25*M_PI + 3.75*M_PI*((float)BSA.BPLen) / 100.0) / 320.0;
	for (cnt = 0; cnt < 320; cnt++)
	{					// just horizontal flip???
		cx = cnt;
		// Calculate actual surface
		phpos += phstep; // Phase advance
		if (BSA.BCanyon)
		{
			acy = 50 * BSA.BCrvAmp / 100; if (sin(phpos) < 0.5) acy *= -1;
		}
		else
			acy = sin(phpos) * 50 * BSA.BCrvAmp / 100;
		acy += rcy*BSA.BRndAmp / 100;
		acy = BoundBy(acy, -50, +50);
		// Standard projection to 320x200+addsize
		for (cnt2 = 80 + acy; cnt2 < 200 + addsize; cnt2++) drawpixfunc(cx, cnt2);
		// Adjust random-cy
		cten = BoundBy(cten + random(10) - 4, 0, 29);
		if ((cnt < 10) || (cnt > 310)) cten = 19;
		rcy = BoundBy(rcy + cten / 10.0 - 1.9, -50, +50);
		if (rcy < -40) cten += 3; if (rcy > +40) cten -= 3;
	}
}

void IBDrawBack(void)
{
	BYTE wtype;
	int cnt, cnt2, cx, cy;
	int vsize = BSA.BSize;
	LPage(BackPage); // Needs to be called before call to SBackPix/GBackPix
	// Draw Sky
	for (cnt = 0; cnt < 155; cnt++)
		for (cnt2 = 0; cnt2 < 320; cnt2++)
			DrawSkyPix(cnt2, cnt);
	// Draw Earth Base
	DrawSurface(vsize - 200, &IBDrawGroundPix);
	// Water Level
	wtype = CWaterS; //if (50 - 100 * BSA.WClim / 1000 < -10) wtype = CSnowS;
	if (Weather.mflvl <= -1) wtype = CSnowS;
	if (BSA.WEnvr == 1) wtype = CAcidS;
	for (cnt = 130; cnt > 130 - 60 * BSA.BWatLvl / 100; cnt--)
		for (cnt2 = 0; cnt2 < 320; cnt2++)
			if (IsSkyBack(GBackPix(cnt2, cnt)))
				SBackPix(cnt2, cnt, wtype);
	// Draw Ground Layers                // ENFORCE GOLD/OIL if GMode==MINE!!!!
	if (BSA.RuleSet > R_EASY)
		for (cnt = vsize / 8; cnt > 0; cnt--) // spread better...!
		{
			wtype = IBSpotType[BoundBy(BSA.WClim / 333, 0, 2)][random(10)];
			cx = random(320);
			for (cy = 0; !Inside(GBackPix(cx, cy), CGroundL, CGroundH); cy++);
			cy += 30 + random(vsize - cy - 50);
			for (cnt2 = random(40) + 40; cnt2 > 0; cnt2--)
			{
				cx = BoundBy(cx + random(11) - 5, 0, 319);
				cy = BoundBy(cy + random(5) - 2, 0, vsize - 1);
				IBDrawLayerSpot(cx, cy, wtype);
			}
		}
	// SetUp Loose Rocks      ought to be in PXSSYS or WEASYS...
	for (cnt = (random(vsize / 5) + vsize / 5)*(1 + (BSA.RuleSet == R_EASY)); cnt > 0; cnt--)
	{
		cx = random(298) + 4;
		for (cy = 0; GBackPix(cx, cy) < CSolidL; cy++);
		cy += 20 + random(vsize - cy - 30);
		if (Inside(GBackPix(cx, cy), CEarth1, CEarth2))
			NewRock(cx, cy, RADEAD, BSA.BRockType[random(20)], 0, 10, 0, -1);
	}
}

//------------------ Background RunTime Modification -------------------------

const BYTE pxsreltype[8] = { PXSSAND,PXSSNOW,PXSMASSWATER,PXSMASSACID,PXSMASSOIL,PXSMASSLAVA,PXSMASSLAVA,PXSMASSLAVA };

void ReleasePXS(int tx, int ty)
{
	BYTE bpx;
	if ((tx < 0) || (tx > 319) || (ty < 0)) return;
	bpx = GBackPix(tx, ty);

	if (Inside(bpx, CPXSL, CPXSH))
	{

		if (Inside(bpx, CLiqL, CLiqH)) // SuperSpecial LIQDRAINing
		{
			PXSOut(PXSLIQDRAIN, tx, ty, 0);
			return;
		}

		DrawBackPix(tx, ty);
		MassPXSOut(pxsreltype[(bpx - CPXSL) >> 1], tx, ty, 0);
		ReleasePXS(tx, ty - 1);
		ReleasePXS(tx - 1, ty - (bpx < CWaterS));
		ReleasePXS(tx + 1, ty - (bpx < CWaterS));
		return;
	}
	if (Inside(bpx, CVhcL, CVhcH))
	{
		DrawBackPix(tx, ty);
		ReleasePXS(tx, ty - 1);
	}
}

inline int LiqType(BYTE color)
{
	if (!Inside(color, CLiqL, CLiqH)) return -1;
	return (color - CLiqL) >> 1;
}

int LiqAtBPix(int x, int y)
{
	return LiqType(GBackPix(x, y));
}

// bounds are liqtypes
int ExtractLiquid2(int fx, int fy, BYTE bndl, BYTE bndh)
{
	int cnt;                  // -1: NoLiquid    Other: Index to LiqRelClass
	BYTE chkl, chkr;
	int ltype = LiqAtBPix(fx, fy);
	if (Inside(ltype, bndl, bndh))
	{
		// Find top pixel of the same liquid
		do                                   // cannot extract upwards thorugh vhc
		{
			while (LiqAtBPix(fx, fy - 1) == ltype) fy--;
			for (cnt = 0, chkl = chkr = 1; chkl || chkr; cnt++)
			{
				if (chkl)         // make LiqType(color) func
				{
					if ((fx - cnt < 0) || ((LiqAtBPix(fx - cnt, fy) != ltype) && (LiqAtBPix(fx - cnt, fy - 1) != ltype)))
						chkl = 0;
					else
						if (LiqAtBPix(fx - cnt, fy - 1) == ltype) { fx -= cnt; break; }
				}
				if (chkr)
					if ((fx + cnt > 319) || ((LiqAtBPix(fx + cnt, fy) != ltype) && (LiqAtBPix(fx + cnt, fy - 1) != ltype)))
						chkr = 0;
					else
						if (LiqAtBPix(fx + cnt, fy - 1) == ltype) { fx += cnt; break; }
			}
		} while (LiqAtBPix(fx, fy - 1) == ltype);
		DrawBackPix(fx, fy);
		ReleasePXS(fx, fy - 1);
	}
	return ltype;
}

int ExtractLiquid(int tx, int ty)
{
	return ExtractLiquid2(tx, ty, 0, 5);
}

BYTE ApplyHeat2Back(int tx, int ty, BYTE lavahot) // Returns cooling
{
	if (lavahot)
		if (Inside(GBackPix(tx, ty), CWaterS, CAcidT))
		{
			ExtractLiquid2(tx, ty, 0, 2); PXSOut(PXSSMOKE, tx, ty, 30); return 1;
		}
	if (Inside(GBackPix(tx, ty), COilS, COilT))
	{
		DrawBackPix(tx, ty); PXSOut(PXSBURNOIL, tx, ty, 70);
		GSTP = SNDFIRE; return 0;
	}
	if (Inside(GBackPix(tx, ty), CSnowS, CSnowT))
	{
		DrawBackPix(tx, ty); PXSOut(PXSWATER, tx, ty, 0); return 1;
	}
	return 0;
}

int DigFree(int tx, int ty, int rad) // Return amount of dug earth
{
	int ycnt, xcnt, lwdt;
	int dpx, dpy, amtearth = 0;
	LPage(BackPage);
	for (ycnt = -rad; ycnt <= rad; ycnt++)
	{
		lwdt = sqrt(rad*rad - ycnt*ycnt);
		dpy = ty + ycnt;
		for (xcnt = -lwdt; xcnt < lwdt + (lwdt == 0); xcnt++)
		{
			dpx = tx + xcnt;
			if (Inside(dpx, 0, 319) && Inside(dpy, 0, BackVSize - 5))
			{
				if (Inside(GBackPix(dpx, dpy), CEGroundL, CEGroundH))
				{
					SBackPix(dpx, dpy, CTunnel1 + random(5));
					amtearth++;
				}
				ReleasePXS(dpx, dpy - 1);
				ReleasePXS(dpx - 1, dpy);
				ReleasePXS(dpx + 1, dpy);
			}
		}
	}
	return amtearth;
}

extern void BlowPXS(int amt, int type, int phase, int fx, int fy, int lvl);

void BlastFree(int tx, int ty, int rad)
{
	int cnt, ycnt, xcnt, lwdt;
	int dpx, dpy, wextrx = -1, wextry = -1;
	int amrck = 0, amgld = 0, amfre = 0, amwtr = 0, amdirt = 0, amsand = 0, amsnow = 0;
	BYTE cbpix;
	LPage(BackPage);

	// Matter Accounting
	for (ycnt = -rad; ycnt <= rad; ycnt++)
	{
		lwdt = sqrt(rad*rad - ycnt*ycnt);
		dpy = ty + ycnt;
		for (xcnt = -lwdt; xcnt <= lwdt; xcnt++)
		{
			dpx = tx + xcnt;
			if (Inside(dpx, 0, 319) && Inside(dpy, 0, BackVSize - 5))
			{
				cbpix = GBackPix(dpx, dpy);
				if (Inside(cbpix, CSky1, CTunnel2))  amfre++;
				if (Inside(cbpix, CRock1, CRock2))   amrck++;
				if (Inside(cbpix, CGold1, CGold2))   amgld++;
				if (Inside(cbpix, CEarth1, CAshes2)) amdirt++;
				if (Inside(cbpix, CSandS, CSandT))   amsand++;
				if (Inside(cbpix, CSnowS, CSnowT))   amsnow++;
				if (Inside(cbpix, CWaterS, CWaterT))
				{
					amwtr++; if ((wextrx == -1) || (wextry == -1)) { wextrx = dpx; wextry = dpy; }
				}
			}
		}
	}

	if ((amwtr > 50) && (amfre == 0))
		for (ycnt = -rad; ycnt <= rad; ycnt++) // MassWater BlastFree
		{
			lwdt = sqrt(rad*rad - ycnt*ycnt);
			dpy = ty + ycnt;
			for (xcnt = -lwdt; xcnt <= lwdt; xcnt++)
			{
				dpx = tx + xcnt;
				if (Inside(dpx, 0, 319) && Inside(dpy, 0, BackVSize - 5))
				{
					cbpix = GBackPix(dpx, dpy);
					if (Inside(cbpix, CRock1, CAshes2) || Inside(cbpix, CSandS, CSandT))
						SBackPix(dpx, dpy, CWaterT);
					if (Inside(cbpix, CGranite1, CGranite2))
						if (Rnd3()) SBackPix(dpx, dpy, CRock1 + random(3));
				}
			}
		}
	else
		for (ycnt = -rad; ycnt <= rad; ycnt++) // Regular BlastFree
		{
			lwdt = sqrt(rad*rad - ycnt*ycnt);
			dpy = ty + ycnt;
			for (xcnt = -lwdt; xcnt <= lwdt; xcnt++)
			{
				dpx = tx + xcnt;
				if (Inside(dpx, 0, 319) && Inside(dpy, 0, BackVSize - 5))
				{
					cbpix = GBackPix(dpx, dpy);
					if (Inside(cbpix, CRock1, CAshes2))
						SBackPix(dpx, dpy, CTunnel1 + random(5));
					if (Inside(cbpix, CGranite1, CGranite2))
						if (Rnd3()) SBackPix(dpx, dpy, CRock1 + random(3));
					ApplyHeat2Back(dpx, dpy, 0);
					ReleasePXS(dpx, dpy - 1); ReleasePXS(dpx - 1, dpy); ReleasePXS(dpx + 1, dpy);
				}
			}
		}

	// Rock/Gold exposure
	while (amrck > 20)
	{
		dpx = BoundBy(tx - (rad / 2) + random(rad), 4, 312);
		dpy = BoundBy(ty - (rad / 2) + random(rad), -15, BackVSize - 10);
		NewRock(dpx, dpy, RAFLY, ROCK, 10 * (dpx - tx) / 3, 10 - (ty - dpy), 0, -1);
		amrck -= 20;
	}
	while (amgld > 20)
	{
		dpx = BoundBy(tx - (rad / 2) + random(rad), 4, 312);
		dpy = BoundBy(ty - (rad / 2) + random(rad), -15, BackVSize - 10);
		NewRock(dpx, dpy, RAFLY, GOLD, 10 * (dpx - tx) / 3, 10 - (ty - dpy), 0, -1);
		amgld -= 20;
		EventCall(204);
	}

	// PXS blows
	BlowPXS(amdirt / 5, PXSSAND, 1, tx, ty, 10);
	BlowPXS(amsnow / 5, PXSSNOW, 0, tx, ty, 10);
	BlowPXS(amsand / 5, PXSSAND, 0, tx, ty, 10);
}

void MeltFree(int tx, int ty, int rad)
{
	int ycnt, xcnt, lwdt;
	int dpx, dpy;
	for (ycnt = -rad; ycnt < rad; ycnt++)
	{
		lwdt = sqrt(rad*rad - ycnt*ycnt);
		dpy = ty + ycnt;
		for (xcnt = -lwdt; xcnt < lwdt; xcnt++)
		{
			dpx = tx + xcnt;
			if (Inside(dpx, 0, 319) && Inside(dpy, 0, BackVSize - 5))
				if (Inside(GBackPix(dpx, dpy), CSnowS, CSnowT))
					ReleasePXS(dpx, dpy);
		}
	}
}

void ClearLineOfSnow(int tx, int ty, int wdt)
{
	int cnt;
	for (cnt = 0; cnt < wdt; cnt++)
		if (Inside(GBackPix(tx + cnt, ty), CSnowS, CSnowT))
		{
			DrawBackPix(tx + cnt, ty); ReleasePXS(tx + cnt, ty - 1);
		}
}

//------------------ Misc Background Access Functions ------------------------

BYTE FindSurface(BYTE type, int atx, int wdt, int *rtx, int *rty)
{
	BYTE rval;                 // returns center of located surface strip
	int tx, ty, dir, cnt;
	BYTE cgrnd;

	LPage(BackPage);

	*rtx = *rty = -1;

	for (dir = -1; dir < 2; dir += 2) // Two tries, return closer one
	{

		tx = BoundBy(atx - wdt / 2 * dir, 0, 319); ty = 0; cnt = wdt; rval = 2;

		do
		{
			while (!Inside(GBackPix(tx, ty), CSolidL, CSolid2H)) ty++;
			while (Inside(GBackPix(tx, ty - 1), CSolidL, CSolid2H)) ty--;


			cgrnd = GBackPix(tx, ty);
			switch (type)
			{
			case 0: // Liquid
				if (Inside(cgrnd, CLiqL, CLiqH)) cnt--; else cnt = wdt;
				break;
			case 1: // Solid
				if (Inside(cgrnd, CSolidL, CSolidH)) cnt--; else cnt = wdt;
				break;
			case 2: // Ground
				if (Inside(cgrnd, CGroundL, CGroundH)) cnt--; else cnt = wdt;
				break;
			case 3: // EGround
				if (Inside(cgrnd, CEGroundL, CEGroundH)) cnt--; else cnt = wdt;
				break;
			case 4: // Structure placement
				if (Inside(cgrnd, CRock1, CSandT)) cnt--; else cnt = wdt;
				break;
			}

			if (dir == +1) if (cnt < 0)  rval = 1; // Slight length counting difference?
			if (dir == -1) if (cnt <= 0) rval = 1;

			if (rval == 2)
			{
				tx += dir;
				if (!Inside(tx, 0, 319)) rval = 0;
			}
		} while (rval == 2);

		if (rval == 1) // Found surface
		{
			if (dir == -1) // On Try 1, set rt's
			{
				*rtx = tx - wdt / 2 * dir; *rty = ty;
			}
			else // On Try 2, set rt's if closer or Try 1 wasn't successfull
				if ((*rtx == -1) || (Abs((tx - wdt / 2 * dir) - atx) < Abs((*rtx) - atx)))
				{
					*rtx = tx - wdt / 2 * dir; *rty = ty;
				}
		}
	}

	if (*rtx != -1) return 1;
	return 0;
}

void RaiseUpTerrain(int tx, int ty, int wdt)
{
	int cx, cy;
	for (cx = tx; cx < tx + wdt; cx++)
		for (cy = ty; !Inside(GBackPix(cx, cy), CGroundL, CGroundH); cy++)
			DrawGroundPix(cx, cy);
}

//--------------------- Background Initialization ----------------------------

BYTE InitBack(void)
{
	if (!Inside(BSA.BSize, 160, 350)) return 0;
	BackVSize = BSA.BSize;
	BackVPos = BackVSPos = 0;
	BackVBuf = NULL;
	if (BSA.BSize > 200) if (!(BackVBuf = new BYTE[320L * (BSA.BSize - 200L)])) return 0;
	IBDrawBack();
	return 1;
}

void DeInitBack(void)
{
	if (BackVBuf) delete[] BackVBuf;
}

//----------------------------- GameMessage ---------------------------------

void InitGameMessage(void)
{
	int cnt;
	for (cnt = 0; cnt < GMNum; cnt++) GMsg[cnt].txt[0] = 0;
}

void GameMessage(char *msg, int tx, int ty, int col, MANTYPE *tman)
{
	int cnt;
	if (tx == -1) // -> Player menu
	{
		tx = Crew[ty].StBX + 50; ty = 172;
	}
	// Clear old msgs if double
	for (cnt = 0; cnt < GMNum; cnt++)
		if ((!GMsg[cnt].tman && (GMsg[cnt].tx == tx))
			|| (GMsg[cnt].tman && (GMsg[cnt].tman == tman)))
			GMsg[cnt].txt[0] = 0;
	// Find free one or take last
	for (cnt = 0; cnt < GMNum - 1; cnt++) if (!GMsg[cnt].txt[0]) break;
	SCopy(msg, GMsg[cnt].txt, 100);
	GMsg[cnt].col = col;
	GMsg[cnt].tman = tman;
	GMsg[cnt].tx = tx; GMsg[cnt].ty = ty;
	GMsg[cnt].time = SLen(GMsg[cnt].txt) * 4;
}

void DrawGameMessage(void)
{
	int cnt, twdt, thgt;

	LPage(CPGE);

	for (cnt = 0; cnt < GMNum; cnt++)
		if (GMsg[cnt].txt[0])
		{
			if (GMsg[cnt].tman)
			{
				GMsg[cnt].tx = GMsg[cnt].tman->x + 4; GMsg[cnt].ty = GMsg[cnt].tman->y - 10 - BackVPos + 20;
			}

			if (Inside(GMsg[cnt].ty, 13, 180))
			{
				TOutSt(GMsg[cnt].txt, &twdt, &thgt);
				TOutS(GMsg[cnt].txt, BoundBy(GMsg[cnt].tx, twdt * 2, 319 - twdt * 2), GMsg[cnt].ty - 6 * (thgt - 1), GMsg[cnt].col, 0, 1);
			}
			GMsg[cnt].time--; if (GMsg[cnt].time <= 0) GMsg[cnt].txt[0] = 0;
		}

	LPage(BackPage);
}

void ClearGMsgTPtr(MANTYPE *tman)
{
	int cnt;
	for (cnt = 0; cnt < GMNum; cnt++)
		if (GMsg[cnt].tman == tman)
			GMsg[cnt].txt[0] = 0;
}

//----------------------------- KeyConDisplay --------------------------------

int KeyConDStatus;

void KeyConDisplay(int stat)
{
	BYTE cover[10], keys[10], cplr;
	int cnt;
	for (cnt = 0; cnt < 3; cnt++) if (BSA.Plr[cnt].Info) cplr = cnt;
	KeyConDStatus = stat;
	// Prepare display output
	RWCopy(GFXPage, 148, 117, GFXPage, 120, 117, 28, 26, 0);
	FillMem(cover, 10, 1); FillMem(keys, 10, 0);
	switch (stat)
	{
	case 1: // All cover, all keys
		FillMem(keys, 10, 1); break;
	case 2: // MovmCon1 keys
		FillMem(keys + 6, 3, 1); break;
	case 3: // MovmCon1 base
		FillMem(cover + 6, 3, 0); break;
	case 4: // MovmCon1+Jump base
		FillMem(cover + 6, 3, 0); cover[4] = 0; break;
	case 5: // MovmCon1+Jump keys
		FillMem(keys + 6, 3, 1); keys[4] = 1; break;
	case 6: // MovmCon1+Dig base
		FillMem(cover + 6, 3, 0); cover[5] = 0; break;
	case 7: // MovmCon1+Dig keys
		FillMem(keys + 6, 3, 1); keys[5] = 1; break;
	case 8: // MovmCon1+Throw base
		FillMem(cover + 6, 3, 0); cover[3] = 0; break;
	case 9: // MovmCon1+Throw keys
		FillMem(keys + 6, 3, 1); keys[3] = 1; break;
	case 10: // MovmCon1+Con2 base
		FillMem(cover + 3, 6, 0); break;
	case 11: // MovmCon1+Con2+CursorRL base
		FillMem(cover, 9, 0); cover[1] = 1; break;
	case 12: // MovmCon1+Con2+CursorRL keys
		FillMem(cover + 3, 6, 0); keys[0] = 1; keys[2] = 1; break;
	case 13: // MovmCon1+Con2+CursorT base
		FillMem(cover, 9, 0); break;
	case 14: // MovmCon1+Con2+CursorT keys
		FillMem(cover, 9, 0); cover[1] = 1; keys[1] = 1; break;
	case 15: // MovmCon1+Con2 base+keys
		FillMem(cover + 3, 6, 0); FillMem(keys + 3, 6, 1); break;
	case 16: // All Base + Menu key
		FillMem(cover, 10, 0); keys[9] = 1; break;
	case 17: // All Base
		FillMem(cover, 10, 0); break;
	}
	for (cnt = 0; cnt < 10; cnt++)
	{
		if (cover[cnt]) RWCopy(GFXPage, 189, 160, GFXPage, 120 + 2 + 8 * (cnt % 3), 117 + 1 + 6 * (cnt / 3), 7, 5, 0);
		if (keys[cnt])
		{
			COut(Config.KCom[cplr][cnt], 120 + 2 + 8 * (cnt % 3) + 2 + 1, 117 + 1 + 6 * (cnt / 3) + 1, CGray1, -1);
			COut(Config.KCom[cplr][cnt], 120 + 2 + 8 * (cnt % 3) + 2, 117 + 1 + 6 * (cnt / 3), CWhite, -1);
		}
	}
}

int KComFlashDel = 0, KComFlashBut = 0;

void KComFlash(int com)
{
	KComFlashDel = 20; KComFlashBut = com;
}

inline void DrawKeyConD(BYTE tpge)
{
	if (KeyConDStatus)
	{
		B4Move(GFXPage, 30, 117, tpge, 36, 20 + 10, 7, 26);
		if (KComFlashDel > 0)
		{
			LPage(tpge);
			DFrame(144 + 2 + 8 * (KComFlashBut % 3) - 1, 30 + 1 + 6 * (KComFlashBut / 3) - 1, 144 + 2 + 8 * (KComFlashBut % 3) + 8, 30 + 1 + 6 * (KComFlashBut / 3) + 6, CYellow);
		}
	}
	if (KComFlashDel > 0) KComFlashDel--;
}

//-------------------------- Game Loop Screen --------------------------------

extern void DrawPXS(void);

void DrawFireSpot(int tx, int ty)
{
	if (Inside(ty - BackVPos, 0, 159 + 4))
		ASprite(&FireSpr[Rnd4()], CPGE, BoundBy(tx - 3, 0, 311), ty - 8 + Rnd3() - BackVPos + 20);
}

void DrawFire(int tx, int ty, int twdt, int thgt)
{
	int cnt;
	if (!Inside(ty - BackVPos, -20, 159)) return;
	for (cnt = twdt*thgt / 50; cnt > 0; cnt--)
		DrawFireSpot(tx + random(twdt - 8) + 4, ty + random(thgt - 2));
}

void DrawConMark(int tx, int ty, BYTE col)
{
	const int cmx[12] = { 0,1,0, 6,7,7, 0,0,1, 7,6,7 };
	const int cmy[12] = { 0,0,1, 0,0,1, 7,8,8, 7,8,8 };
	int cnt;
	for (cnt = 0; cnt < 12; cnt++)
		SPixA(tx + cmx[cnt], ty + cmy[cnt], col);
}

inline void DrawMan(MANTYPE *mptr, BYTE pcol, BYTE cnfl)
{
	if (mptr->act < MAHOME)
		if (Inside(mptr->y - BackVPos, -10, 159))
		{
			if (mptr->onf) ASprite(&FireSpr[Rnd4()], CPGE, mptr->x, mptr->y - 3 - BackVPos + 20 + Rnd3());
			ASpriteC(&ManSpr[MASpr[mptr->act][(mptr->xdir < 0)] * 4 + mptr->phase], CPGE, mptr->x, mptr->y - BackVPos + 20, pcol);
			if (cnfl) if (mptr->con) if (mptr->act < MADEAD) DrawConMark(mptr->x, mptr->y - BackVPos + 20, CRed);
		}
}

void DrawShark(MANTYPE *mptr)
{
	int mtx, mty;
	if (Inside(mptr->y - BackVPos, -9, 159))
	{
		ASprite(&SharkSpr[(mptr->xdir < 0)], CPGE, mptr->x - 10, mptr->y - BackVPos + 20);
		mtx = mptr->x + 6 - 15 * (mptr->xdir < 0); mty = mptr->y + 7 - BackVPos + 20 + (mptr->ydir > 5);
		SPixA(mtx, mty, 21); SPixA(mtx + 1, mty, 21); SPixA(mtx + 2, mty, 21);
		if (mptr->act == MADEAD)
		{
			mtx = mptr->x + 6 - 14 * (mptr->xdir < 0); mty = mptr->y + 4 - BackVPos + 20;
			SPixA(mtx, mty, 21); SPixA(mtx + 1, mty, 21); SPixA(mtx, mty + 1, 20); SPixA(mtx + 1, mty + 1, 20);
		}
	}
}

void DrawWipf(MANTYPE *mptr)
{
	BYTE wspr = 0;
	if (mptr->act < MAHOME) // Wipf home?????????
		if (Inside(mptr->y - BackVPos, -7, 159))
		{
			if (mptr->onf) ASprite(&FireSpr[Rnd4()], CPGE, mptr->x, mptr->y - 2 - BackVPos + 20 + Rnd3());
			switch (mptr->act)
			{
			case MAWALK: case MASWIM: wspr = 3 + 2 * Sign(mptr->xdir) + (mptr->phase >> 1); break;
			case MAFLY: wspr = 7; if (mptr->xdir < 0) wspr = 0; break;
			case MADIG: if (mptr->xdir < 0) wspr = 2 - Min(mptr->phase, 2); else wspr = 5 + Min(mptr->phase, 2); break;
			case MADEAD: wspr = 8; break;
			default: wspr = 2; break;
			}
			ASprite(&WipfSpr[wspr], CPGE, mptr->x, mptr->y + 2 - BackVPos + 20);
		}
}

void DrawMonster(MANTYPE *mns)
{
	int foot;
	BYTE dir;

	if (Inside(mns->y - BackVPos, -15, 159))
	{
		if (mns->act < MADEAD)
		{
			dir = (mns->xdir > 0);

			foot = mns->phase; if (foot == 3) foot = 1; foot--; if (dir) foot *= -1;

			ASprite(&MonsterSpr[3 * dir + 1], CPGE, mns->x + 8 - 8 * dir - 6, mns->y + 5 - BackVPos + 20 - 3);
			ASprite(&MonsterSpr[3 * dir + 0], CPGE, mns->x + 8 * dir - 6, mns->y + mns->phase / 2 - BackVPos + 20 - 3);
			if (mns->act == MAFLY) ASprite(&MonsterSpr[3 * dir + 2], CPGE, mns->x, mns->y + 4 - BackVPos + 20);
			else ASprite(&MonsterSpr[3 * dir + 2], CPGE, mns->x + foot, mns->y + 10 - mns->phase / 2 - BackVPos + 20 - 3);
		}
		if (mns->act == MADEAD)
			BSprite(&DeadMonster, CPGE, mns->x, mns->y + 4 - BackVPos + 20);
	}
}


void DrawAnimal(MANTYPE *mptr)
{
	switch (mptr->type)
	{
	case MNSHARK:   DrawShark(mptr);   break;
	case MNWIPF:    DrawWipf(mptr);    break;
	case MNMONSTER: DrawMonster(mptr); break;
	}
}

void DrawArrow(ROCKTYPE *rptr)
{
	int yskew;
	yskew = BoundBy(2 * (rptr->ydir - 20) / 30, -2, +2);
	if (rptr->xdir > 0) yskew *= -1;
	if ((rptr->type == FARROW) && (rptr->phase >= 10))
		DrawFireSpot(rptr->x + 2, rptr->y + 2);
	DLine(rptr->x, rptr->y + 2 - BackVPos + 20 + yskew, rptr->x + 3, rptr->y + 2 - BackVPos + 20 - yskew, (rptr->type == FARROW) ? 32 : 17);
	if (rptr->type == BARROW)
	{
		if (rptr->xdir > 0) yskew *= -1;
		SPixA(rptr->x + 3 * (rptr->xdir > 0), rptr->y + 2 - BackVPos + 20 + yskew, 33);
	}
}

extern void DrawSinglePXS(PXSTYPE *pxp);

void DrawRock(ROCKTYPE *rptr)
{
	int rstp, cnt;
	float scale;
	PXSTYPE tempPXS;
	if (rptr->type > NOROCK)
		if (Inside(rptr->y - BackVPos, -15, 159))
			switch (rptr->type)
			{
			case ARROW: case FARROW: case BARROW:
				if (((rptr->phase == 1) || (rptr->phase >= 10))) DrawArrow(rptr);
				else ASprite(&RockSpr[rptr->type], CPGE, rptr->x, rptr->y - BackVPos + 20);
				break;
			case PLANT3:
				for (rstp = rptr->phase - 30, cnt = 1; rstp > 0; cnt++)
				{
					ASprite(&RockSpr[Min(PLANT1 + rstp / 10, PLANT3)], CPGE, rptr->x, rptr->y - (3 + (rptr->x % 2))*cnt - BackVPos + 20);
					rstp -= Min(rstp, 30);
				}
				break;
			case FLAG:
				if (!Inside(rptr->phase, 0, 2)) return;
				ASpriteC(&RockSpr[rptr->type], CPGE, rptr->x, rptr->y - BackVPos + 20, BSA.Plr[rptr->phase].Col);
				break;
			case MONSTEGG:
				ASprite(&RockSpr[rptr->type], CPGE, rptr->x, rptr->y - BackVPos + 20);
				if (Inside(rptr->phase, 10, 49))
				{
					scale = (rptr->phase - 10.0) / 39.0;
					RWCopyE(GFXPage, 160, 153, CPGE, rptr->x - 4 * scale, rptr->y + 3 - 12 * scale - BackVPos + 20, 12, 12, 1, scale, scale);
				}
				break;
			case COMET:
				DrawFireSpot(rptr->x + 1, rptr->y + 1);
				ASprite(&RockSpr[rptr->type], CPGE, rptr->x, rptr->y - BackVPos + 20);
				break;
			case ROCKPXS:
				tempPXS.type = rptr->phase;
				tempPXS.x = rptr->x + 1; tempPXS.y = rptr->y + 3;
				tempPXS.p = rptr->thrby;
				DrawSinglePXS(&tempPXS);
				break;
			default:
				ASprite(&RockSpr[rptr->type], CPGE, rptr->x, rptr->y - BackVPos + 20);
				break;
			}
}

void DrawCatArm(int tx, int ty, int dir, int phase, int vslope)
{
	int angle, atx, aty;
	if (Inside(phase, 0, 50)) angle = phase * 70 / 50;
	if (Inside(phase, 51, 60)) angle = (60 - phase) * 70 / 9;
	if (dir) angle -= 15 * vslope; else angle += 15 * vslope;

	float fang = M_PI*(float)angle / 180.0;   // table

	LPage(CPGE);
	if (dir) // Face left
	{
		atx = sin(fang) * 12; aty = -cos(fang) * 12;
		DLine(tx + 8, ty + 3, tx + 2 + atx, ty + 9 + aty, 28);   // looks ugly...
		DLine(tx + 2, ty + 4, tx + 2 + atx, ty + 8 + aty, 26);
		DLine(tx + 1, ty + 5, tx + 1 + atx, ty + 8 + aty, 27);
	}
	else // Face right
	{
		atx = -sin(fang) * 12; aty = -cos(fang) * 12;
		DLine(tx + 3, ty + 3, tx + 9 + atx, ty + 9 + aty, 28);
		DLine(tx + 9, ty + 4, tx + 9 + atx, ty + 8 + aty, 26);
		DLine(tx + 10, ty + 5, tx + 10 + atx, ty + 8 + aty, 27);
	}

}

void DrawXBowSet(VEHICTYPE *vhc)
{
	int tx = vhc->x, ty = vhc->y - BackVPos + 20;
	int lvpos, aang;
	float sfang, cfang;

	// Arm angle (+/- 0-90)
	aang = vhc->VHDXBowDir; if (vhc->data3) aang *= -1;
	sfang = sin(M_PI*(float)aang / 180.0);
	cfang = -cos(M_PI*(float)aang / 180.0);

	// Lever position (0-10)
	lvpos = vhc->VHDXBowPhase;
	if (lvpos > 10) lvpos = 10 - ((lvpos - 10) / 2);

	DFrame(tx + 6 + sfang*(-5 + lvpos), ty + 2 + 2 + cfang*(-5 + lvpos), tx + 6 + sfang*(-5 + lvpos), ty - 2 + 2 + cfang*(-5 + lvpos), 28);
	DFrame(tx - 1 + 6 + sfang*(-5 + lvpos), ty + 1 + 2 + cfang*(-5 + lvpos), tx + 1 + 6 + sfang*(-5 + lvpos), ty - 1 + 2 + cfang*(-5 + lvpos), 17);

	DLine(tx + 6 + sfang * 7, ty + 2 + cfang * 5, tx + 6 - sfang * 5, ty + 2 - cfang * 5, 27);
	if (vhc->VHDXBowDir > 45)
		DLine(tx + 6 + sfang * 7, ty + 1 + cfang * 5, tx + 6 - sfang * 5, ty + 1 - cfang * 5, 17);
	else
		DLine(tx + 7 + sfang * 7, ty + 2 + cfang * 5, tx + 7 - sfang * 5, ty + 2 - cfang * 5, 17);

	DLine(tx, ty + 6, tx + 6, ty + 1, 28);
	DLine(tx, ty + 6 + 1, tx + 6, ty + 2, 29);
	DLine(tx + 6, ty + 1, tx + 11, ty + 6, 28);
	DLine(tx + 6, ty + 2, tx + 11, ty + 6 + 1, 29);

}

const int wpflposx[5] = { 0, 5, 3, 1, 4 };
const int wpflposy[5] = { -3,-2,-4,-5,-3 };

void DrawWipfLoad(VEHICTYPE *vhc)
{
	int cnt;
	for (cnt = BoundBy(vhc->VHDWipfLoad, 0, 5) - 1; cnt >= 0; cnt--)
		ASprite(&WipfSpr[3], CPGE, vhc->x + wpflposx[cnt], vhc->y + wpflposy[cnt] - BackVPos + 20);
}

void DrawVehicA(VEHICTYPE *vhc)
{
	int cnt;
	switch (vhc->type)
	{
	case VHLORRY:
		if (vhc->x >= 0)
			if (Inside(vhc->y - BackVPos, -7, 159))
			{
				if (vhc->VHDWipfLoad > 0) DrawWipfLoad(vhc);
				ASprite(&LorrySpr[vhc->data2], CPGE, vhc->x, vhc->y - BackVPos + 20);
			}
		break;
	case VHCATAPULT:
		if (vhc->x >= 0)
			if (Inside(vhc->y - BackVPos, -12, 163))
			{
				DrawCatArm(vhc->x, vhc->y - BackVPos + 20, vhc->data3, vhc->VHDCatPhase, vhc->data2 - 1);
				ASprite(&CatSpr[vhc->data2][vhc->data3], CPGE, vhc->x, vhc->y + 3 - BackVPos + 20);
			}
		break;
	case VHCROSSBOW:
		if (vhc->x >= 0)
			if (Inside(vhc->y - BackVPos, -12, 163))
			{
				DrawXBowSet(vhc);
				ASprite(&CatSpr[vhc->data2][vhc->data3], CPGE, vhc->x, vhc->y + 3 - BackVPos + 20);
			}
		break;
	case VHSAILBOAT:
		if (vhc->x >= 0)
			if (Inside(vhc->y - BackVPos, -12, 159))
				ASprite(&BoatSprSail, CPGE, vhc->x, vhc->y - 3 - BackVPos + 20);
		break;
	}
}

void DrawVehicB(VEHICTYPE *vhc) // or sort vehics in correct order on NewVehic
{
	switch (vhc->type)
	{
	case VHSAILBOAT:
		if (vhc->x >= 0)
			if (Inside(vhc->y - BackVPos, -12, 159))
				ASprite(&BoatSpr, CPGE, vhc->x, vhc->y + 6 - BackVPos + 20);
		break;
	}
}

void DrawVehicC(VEHICTYPE *vhc)
{
	int cnt;
	switch (vhc->type)
	{
	case VHELEVATOR:
		if (Inside(vhc->y - BackVPos, -13, 159))
			BSprite(&ElevatorSpr, CPGE, vhc->x, vhc->y - BackVPos + 20);
		for (cnt = vhc->y - 1 - BackVPos; (cnt > vhc->data - 3 - BackVPos) && (cnt >= 0); cnt--) // Rope
			if (cnt < 160) SPixA(vhc->x + 8, cnt + 20, 28);
		break;
	case VHBALLOON:
		if (Inside(vhc->y - BackVPos, -19, 159))
		{
			ASprite(&BalloonSpr, CPGE, vhc->x, vhc->y - BackVPos + 20);
			ASpriteC(&FlagSpr[FPhase], CPGE, vhc->x + 12, vhc->y + 8 - BackVPos + 20, BSA.Plr[vhc->owner].Col);
			if (vhc->VHDBallCrash == 2) DrawFire(vhc->x, vhc->y, 16, 12);
		}
		break;
	}
}

void DrawStruct(STRUCTYPE *strc)
{
	int schgt;
	BYTE burncol;
	if (strc->type > STNOSTRUCT)
	{
		// On Fire
		if (strc->onf)
		{
			schgt = (StructHgt[strc->type] * strc->con) / 1000;  // slowww!!!
			DrawFire(strc->x, strc->y + StructHgt[strc->type] - schgt, StructWdt[strc->type], schgt);
		}
		// Struct special backs
		if (strc->con >= 1000)
			switch (strc->type)
			{
			case STELEVATOR: // Elevator wheel
				if (Inside(strc->y - 5 - BackVPos, -19, 159))
					BSprite(&ElevWheelSpr[strc->p], CPGE, strc->x + 4, strc->y - 5 - BackVPos + 20);
				break;
			case STOILPOWER: // Oilpower back
				if (Inside(strc->y + 7 - BackVPos, -3, 159))
				{
					burncol = 16;
					if (strc->p == 1)
					{
						if (strc->liqstr > 10) burncol = 56 + random(8); else if (strc->liqstr > 0) burncol = 32 + random(4);
					}
					DFrame(strc->x + 9, strc->y + 7 - BackVPos + 20, strc->x + 9 + 2, strc->y + 7 + 2 - BackVPos + 20, burncol);
					DFrame(strc->x + 1, strc->y + StructHgt[strc->type] - BackVPos + 20, strc->x + 2, strc->y + StructHgt[strc->type] - BackVPos + 20 - strc->liqstr / 4, 16);
				}
				break;
			case STTOWER: // Tower back
				if (Inside(strc->y - BackVPos, -19, 159))
					DBox(strc->x + 5, strc->y + 11 - BackVPos + 20, strc->x + 5 + 5, strc->y + 11 + 7 - BackVPos + 20, CGray1);
				break;
			case STMAGIC: // Magic tower back
				if (Inside(strc->y + 8 - BackVPos, -16, 159))
				{
					burncol = 16; if (strc->energy > 0) burncol = 56 + random(2) + 6 * (strc->energy / 10000);
					DBox(strc->x + 4, strc->y + 7 - BackVPos + 20, strc->x + 10, strc->y + 13 - BackVPos + 20, burncol);
				}
				break;
			}
		// Regular struct sprite
		if (Inside(strc->y - BackVPos, -19, 159))
		{
			if (strc->con < 1000)
			{
				schgt = (StructHgt[strc->type] * strc->con) / 1000;  // slowww!!!
				if (schgt > 0)
					BSpriteY(&StructSpr[STSpr[strc->type]], CPGE, strc->x, strc->y - BackVPos + 20, StructHgt[strc->type] - schgt, schgt);
			}
			else          // don't need tower...
				BSprite(&StructSpr[STSpr[strc->type]], CPGE, strc->x, strc->y - BackVPos + 20);
			//if (strc->type<STCACTUS) DBox(strc->x-5,strc->y+StructHgt[strc->type]-BackVPos+20,strc->x-3,strc->y+StructHgt[strc->type]-BackVPos+20-strc->energy/2,10);
		}
		// Windmill wheel, pump, castle gate
		if (strc->type == STWINDMILL) if (strc->con >= 1000)
			if (Inside(strc->y - 10 - BackVPos, -19, 159))
				BSprite(&WindMillSpr[strc->p / 10], CPGE, strc->x - 4, strc->y - 10 - BackVPos + 20);
		if (strc->type == STPUMP) if (strc->con >= 1000)
			if (Inside(strc->y - 7 - BackVPos, -11, 159))
				BSprite(&PumpSpr[Abs(strc->p)], CPGE, strc->x, strc->y - 7 - BackVPos + 20);
		if (strc->type == STCASTLE) if (strc->con >= 1000)
			if (Inside(strc->p, 1, 17))
				if (Inside(strc->y + 4 - BackVPos, -8, 159))
				{
					schgt = strc->p - 1; if (strc->p > 5) schgt = 4; if (strc->p > 12) schgt = 4 - (strc->p - 13);
					B4Move(GFXPage, 48 + schgt * 3, 112, CPGE, strc->x / 4 + 1, strc->y + 4 - BackVPos + 20, 3, 8);
				}
	}
}

// For castle flag
#define CstFlag energy

void DrawStructSpecial(STRUCTYPE *cstrc)
{
	if (!Inside(cstrc->type, STNOSTRUCT + 1, STCACTUS - 1)) return; // Buildings only

	if (cstrc->con < 1000) // Under construction? -> Sign
	{
		if (!cstrc->onf)
			if (Inside(cstrc->y + StructHgt[cstrc->type] - 8 - BackVPos, -7, 159))
				BSprite(&ConstrSignSpr, CPGE, cstrc->x, cstrc->y + StructHgt[cstrc->type] - 8 - BackVPos + 20);
	}
	else // Complete -> Castle/Tower/Magic flag, Tower
	{
		if (cstrc->type == STCASTLE)
			if (Inside(cstrc->y - 20 - BackVPos, -19, 159))
			{
				BSprite(&TowerSpr, CPGE, cstrc->x, cstrc->y - 15 - BackVPos + 20);
				if (cstrc->CstFlag)
				{
					DFrame(cstrc->x + 4, cstrc->y - 20 - BackVPos + 20, cstrc->x + 4, cstrc->y - 16 - BackVPos + 20, 25);
					ASpriteC(&FlagSpr[FPhase], CPGE, cstrc->x + 5, cstrc->y - 20 - BackVPos + 20, BSA.Plr[cstrc->owner].Col);
				}
			}
		if (cstrc->type == STTOWER)
		{
			if (cstrc->liqstr == 0) // liqstr is flag on/off
				if (Inside(cstrc->y - 10 - BackVPos, -9, 159))
				{
					DFrame(cstrc->x + 2, cstrc->y - 9 - BackVPos + 20, cstrc->x + 2, cstrc->y - 1 - BackVPos + 20, 25);
					ASpriteC(&FlagSpr[FPhase], CPGE, cstrc->x + 3, cstrc->y - 9 - BackVPos + 20, BSA.Plr[cstrc->owner].Col);
				}
			if (Inside(cstrc->y - BackVPos, -19, 159))
				BSprite(&StructSpr[STSpr[STTOWER]], CPGE, cstrc->x, cstrc->y - BackVPos + 20);
		}
		if (cstrc->type == STMAGIC)
			if (Inside(cstrc->y - 10 - BackVPos, -9, 159))
			{
				DFrame(cstrc->x + 7, cstrc->y - 9 - BackVPos + 20, cstrc->x + 7, cstrc->y - 1 - BackVPos + 20, 25);
				ASpriteC(&FlagSpr[FPhase], CPGE, cstrc->x + 8, cstrc->y - 9 - BackVPos + 20, BSA.Plr[cstrc->owner].Col);
			}
	}
}

void DrawLine(LINETYPE *cline)
{
	int cnt;
	LPage(CPGE);
	if (cline->type != LNNOLINE)
		for (cnt = 0; (cnt < LineLen - 1) && (cline->x[cnt] != -1); cnt++)
			if (cline->x[cnt + 1] != -1) // slslslsloooooooowwwww ?!
			{
				DLine(cline->x[cnt], cline->y[cnt] - BackVPos + 20, cline->x[cnt + 1], cline->y[cnt + 1] - BackVPos + 20, (cline->type == LNENERGY) ? 27 : 18);
				if (Inside(cline->y[cnt] - BackVPos, 0, 159))
					SPixA(cline->x[cnt], cline->y[cnt] - BackVPos + 20, 19);
			}
}

void DrawExplosion(EXPLTYPE *expl)
{
	int cnt;
	if (expl->act > -1)
		if (Inside(expl->y - BackVPos, 0, 159))
			for (cnt = 0; cnt < expl->act; cnt++)
				DrawFireSpot(expl->x + 4 - sin(cnt)*BoundBy(expl->act - cnt + 2, 0, 15), expl->y + 4 - cos(cnt)*BoundBy(expl->act - cnt + 2, 0, 15));
}

void DrawLightN(void)
{
	int cnt, cnt2;
	LIGHTNING *lgt;
	for (cnt = 0; cnt < MaxLightN; cnt++)
		if (Weather.LightN[cnt].type > -1)
		{
			lgt = &(Weather.LightN[cnt]);
			for (cnt2 = 1; cnt2 <= lgt->phase; cnt2++)
				DLine(lgt->lx[cnt2 - 1], BoundBy(lgt->ly[cnt2 - 1] - BackVPos + 20, 0, 199),
					lgt->lx[cnt2], BoundBy(lgt->ly[cnt2] - BackVPos + 20, 0, 199),
					CLightning);
		}
}

extern char *RankName[11];

void DrawCursor(MANTYPE *crs)
{
	if (crs)
		if (Inside(crs->y - BackVPos, 0, 170))
		{
			// bind by border
			SOutS(crs->mi->name, crs->x + 4, crs->y - 13 - BackVPos + 20, CRed, CBlack, 1);
			if (crs->mi->rank > 0) SOutS(RankName[crs->mi->rank], crs->x + 4, crs->y - 13 - 6 - BackVPos + 20, CRed, CBlack, 1);
			SOutS("$", crs->x + 2, crs->y - 7 - BackVPos + 20, CRed, CBlack);
		}
}

void DrawDragBox(int x1, int y1, int x2, int y2)
{
	int cnt;
	if (x2 < x1) Swap(x1, x2); if (y2 < y1) Swap(y1, y2);
	for (cnt = x1; cnt <= x2; cnt += 3)
	{
		SPixF(cnt, y1, CWhite); SPixF(cnt, y2, CWhite);
	}
	for (cnt = y1; cnt <= y2; cnt += 3)
	{
		SPixF(x1, cnt, CWhite); SPixF(x2, cnt, CWhite);
	}
}

extern void DrawMouse(int dmx, int dmy);
extern void SetMouseCursor(int type);

extern BYTE DragBoxLarge(void);

int SaveMBx, SaveMBy;

void RestoreMouseBack(BYTE page)
{
	if (Crew[MouseCon.player].CMenu)
		if (MouseCon.cursor == 1)
			if (SaveMBx > -1)
				B4Move(GFXPage, 128 / 4, 168, page, SaveMBx, SaveMBy, 3, 12);
}

void DrawMouseCon(BYTE tpge)
{
	int plr = MouseCon.player;
	LPage(tpge);
	switch (MouseCon.cursor)
	{
	case 0: case 4: // Cross
		Viewport(0, 20, 319, 179);
		SetMouseCursor(0); DrawMouse(MouseCon.x1, MouseCon.y1);
		break;
	case 1: // Menu Arrow
		Viewport(Crew[plr].StBX, 180, Crew[plr].StBX + Crew[plr].StBWdt - 1, 199);
		SaveMBx = BoundBy(MouseCon.x1, Crew[plr].StBX, Crew[plr].StBX + Crew[plr].StBWdt - 12) / 4;
		SaveMBy = BoundBy(MouseCon.y1, 180, 200 - 12);
		B4Move(tpge, SaveMBx, SaveMBy, GFXPage, 128 / 4, 168, 3, 12);
		SetMouseCursor(1); DrawMouse(MouseCon.x1, MouseCon.y1);
		break;
	case 2: // Cross/DragBox
		Viewport(0, 20, 319, 179);
		if (DragBoxLarge()) DrawDragBox(MouseCon.x1, MouseCon.y1, MouseCon.x2, MouseCon.y2);
		else { SetMouseCursor(0); DrawMouse(MouseCon.x1, MouseCon.y1); }
		break;
	case 3: // Menu Arrow (Delayed)
		Viewport(Crew[plr].StBX, 180, Crew[plr].StBX + Crew[plr].StBWdt - 1, 199);
		SetMouseCursor(1); DrawMouse(MouseCon.x1, MouseCon.y1);
		break;
	case 5:
		DrawConMark(BoundBy(MouseCon.x1 - 3, 0, 308), BoundBy(MouseCon.y1 - 5, 20, 168), CWhite);
		break;
	}
	NoViewport();
}

//------------------------------ Status Bars ---------------------------------

extern char *MagicName[MagicNum];
extern int MagicEnergy[MagicNum];

void DrawWealthBox(BYTE plr, int tx, int ty, BYTE tpge)
{
	sprintf(OSTR, "%03d", Crew[plr].Wealth);
	DBox(tx + 5, ty, tx + 17, ty + 6, 0);
	ASprite(&RockSpr[GOLD], tpge, tx, ty + 1);
	SOut(OSTR, tx + 6, ty + 1, CWhite);
}

void DrawRockStorage(BYTE plr, int tx, int ty, BYTE tpge)
{
	int cnt, cpos;
	for (cnt = 0, cpos = 0; cnt < RockTypeNum; cnt++)
		if (Crew[plr].RockStorage[cnt] > 0)
		{
			if (cpos >= 3) { SOut("...", Crew[plr].StBX + tx + 5, ty + 14, CWhite, -1); return; }
			ASprite(&RockSpr[cnt], tpge, Crew[plr].StBX + tx, ty + 1 + 6 * cpos);
			if (Crew[plr].RockStorage[cnt] < 100)
				sprintf(OSTR, "+%02d", Crew[plr].RockStorage[cnt]);
			else
				sprintf(OSTR, "%03d", Crew[plr].RockStorage[cnt]);
			SOutS(OSTR, Crew[plr].StBX + tx + 5, ty + 6 * cpos, CWhite, CBlack);
			cpos++;
		}
}

void DrawStrentghBox(MANTYPE *mptr, BYTE tpge, int tx, int ty)
{
	int maxstrn = 100 + 5 * mptr->mi->rank;
	LPage(tpge);
	DBox(tx, ty, tx + 101, ty + 3, CBlack);
	if (mptr->mi->rank > 0) DBox(tx + 100 * 100 / maxstrn, ty, tx + 101, ty + 3, CDRed);
	if (mptr->strn > 0) DBox(tx + 1, ty + 1, tx + Max(mptr->strn * 100 / maxstrn, 1), ty + 2, CRed);
}

void DrawCarrBox(MANTYPE *mptr, BYTE tpge, int tx, int ty, BYTE dpu)
{
	BYTE bcol = CBlack;
	LPage(tpge);
	if (mptr->tptr && (mptr->carr == LINECON))
		if ((mptr->act != MABUILD) && (mptr->act != MAPUSH))
			switch (((LINETYPE*)mptr->tptr)->type)
			{
			case LNENERGY:  bcol = 63; break;
			case LNSRCPIPE: bcol = 41; break;
			case LNTRGPIPE: bcol = 45; break;
			}
	DBox(tx, ty, tx + 5, ty + 4, bcol);
	if (mptr->carr > NOROCK)
		if (mptr->carr == FLAG) ASpriteC(&RockSpr[mptr->carr], tpge, tx + 1, ty + 1, BSA.Plr[mptr->carrp].Col);
		else ASprite(&RockSpr[mptr->carr], tpge, tx + 1, ty + 1);
		if (Inside(mptr->carr, ARROW, BARROW))
			DFrame(tx, ty + 5 - mptr->carrp, tx, ty + 4, CGray2);
		if (dpu) SOutS("$", tx + 2, ty, CWhite, CGray1);
}

void DrawCountBox(int tval, BYTE mcol, BYTE tpge, int tx, int ty)
{
	sprintf(OSTR, "%02d", tval);
	DBox(tx + 6, ty + 1, tx + 6 + 8, ty + 7, CBlack);
	SOut(OSTR, tx + 6 + 1, ty + 2, CWhite, -1);
	ASpriteC(&ManSpr[0], tpge, tx, ty, mcol);
}

void StatusDisplay(BYTE tpge, BYTE plr)
{
	char tstr[10];
	if (Crew[plr].Cursor) // Personal Clonk Status
	{
		RWCopy(GFXPage, 217 + 5 * Crew[plr].Cursor->mi->rank, 160, tpge, Crew[plr].StBX + 1, 181, 5, 5, 0);
		tstr[0] = 0;
		if (BSA.GPlrElm == 0)
			if (Crew[plr].Cursor->mi == Crew[plr].HiRankCap)
				sprintf(tstr, "%c!%c", CYellow, CWhite);
		if (Crew[plr].ConCnt > 1) sprintf(OSTR, "%s%s+%d", Crew[plr].Cursor->mi->name, tstr, Crew[plr].ConCnt - 1);
		else sprintf(OSTR, "%s%s", Crew[plr].Cursor->mi->name, tstr);
		SOutS(OSTR, Crew[plr].StBX + 7, 181, Crew[plr].Cursor->con ? CWhite : CGray3, CBlack);

		sprintf(OSTR, "%d", Crew[plr].Cursor->mi->exp); SOut(OSTR, Crew[plr].StBX + 95, 181, CBlack, -1, 2);

		DrawStrentghBox(Crew[plr].Cursor, tpge, Crew[plr].StBX + 1, 187);
		DrawCarrBox(Crew[plr].Cursor, tpge, Crew[plr].StBX + 97, 181, Crew[plr].DontPickUp);
	}

	if (Crew[plr].FirstMan) // Overall Plr/Crew Status
	{
		DrawCountBox(Crew[plr].ManCnt, BSA.Plr[plr].Col, tpge, Crew[plr].StBX + 1, 191);
		DrawWealthBox(plr, Crew[plr].StBX + 18, 192, tpge);

		sprintf(OSTR, "%d", BSA.Plr[plr].ScoreGain);
		SOut(OSTR, Crew[plr].StBX + 102, 193, CWhite, -1, 2);
	}

}

extern long MagicLevel(int plr);

void DrawMagicLevel(int level, int tx, int ty)
{
	DBox(tx, ty, tx + 4, ty + 17, CBlack);
	if (level > 0)
		DBox(tx + 1, ty + 16, tx + 3, ty + 16 - Min(15 * level / 100, 15), 63);
}

extern void SVIGCopy(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE gray);

void StatusBarIcon(int inum, int tx, int ty, BYTE tpge, BYTE gray = 0)
{
	if (inum == -1)
		DFrame(tx, ty, tx + 11, ty + 7, 20);
	else
		if (inum < 40)
			SVIGCopy(GFXPage, 272 + 12 * (inum % 4), 120 + 8 * (inum / 4), tpge, tx, ty, 12, 8, gray);
		else
			SVIGCopy(GFXPage, 64 + 12 * (inum % 4), 128 + 8 * ((inum - 40) / 4), tpge, tx, ty, 12, 8, gray);
}

#define SBUTBLANK 27
#define SBUTINACT 31
#define SBUTLEFT   2
#define SBUTEXIT   0
#define SBUTRIGHT  3

const int StrBuildBut[StructBuildNum] = { 10,28,9, 8,25,11, 34,SBUTINACT,SBUTINACT };
const int VhcOrderBut[VhcOrderNum] = { 12,13,14,15,30,16 };
const int MagicBut[MagicNum] = { 36,37,38, 40,39,41, -1,-1,-1 };

void MenuDisplay(BYTE tpge, BYTE plr)
{
	int cnt, cbutn;
	BYTE hdbut, rsr = 0; if (BSA.RuleSet == R_RADICAL) rsr = 1;
	switch (Crew[plr].CMenu)
	{
	case CMMAIN:
		B4Move(GFXPage, 132 / 4 + 0, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		StatusBarIcon(29, Crew[plr].StBX + 28, 181, tpge);
		StatusBarIcon(17, Crew[plr].StBX + 41, 181, tpge);
		StatusBarIcon(35, Crew[plr].StBX + 54, 181, tpge);
		StatusBarIcon(1, Crew[plr].StBX + 28, 190, tpge);
		StatusBarIcon(33, Crew[plr].StBX + 41, 190, tpge);
		StatusBarIcon(SBUTBLANK, Crew[plr].StBX + 54, 190, tpge);
		break;
	case CMRCKORDER:
		B4Move(GFXPage, 132 / 4 + 0, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		for (cnt = 0; cnt < 3; cnt++)
		{
			cbutn = Crew[plr].CMenuData + cnt;
			hdbut = 1; if ((RockOrder[cbutn] > NOROCK) && (rsr || !RockOrderRad[cbutn])) hdbut = 0;

			if (!hdbut)
			{
				StatusBarIcon(SBUTBLANK, Crew[plr].StBX + 28 + 13 * cnt, 181, tpge);
				sprintf(OSTR, "%2d", Crew[plr].RckProd.Store[cbutn]);
				SOut(OSTR, Crew[plr].StBX + 28 + 11 + 13 * cnt, 182, CGray2, -1, 2);
				ASprite(&RockSpr[RockOrder[cbutn]], tpge, Crew[plr].StBX + 29 + 13 * cnt, 184);
			}
			else
				StatusBarIcon(SBUTINACT, Crew[plr].StBX + 28 + 13 * cnt, 181, tpge);
		}
		StatusBarIcon(SBUTLEFT, Crew[plr].StBX + 28, 190, tpge);
		StatusBarIcon(SBUTEXIT, Crew[plr].StBX + 41, 190, tpge);
		StatusBarIcon(SBUTRIGHT, Crew[plr].StBX + 54, 190, tpge);
		DrawWealthBox(plr, Crew[plr].StBX + 68, 181, tpge);
		DrawRockStorage(plr, 87, 181, tpge);
		break;
	case CMVHCORDER:
		B4Move(GFXPage, 132 / 4 + 0, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		if ((BSA.RuleSet < R_RADICAL) && (Crew[plr].CMenuData + 0 > 3))	StatusBarIcon(SBUTINACT, Crew[plr].StBX + 28, 181, tpge);
		else StatusBarIcon(VhcOrderBut[Crew[plr].CMenuData + 0], Crew[plr].StBX + 28, 181, tpge);
		if ((BSA.RuleSet < R_RADICAL) && (Crew[plr].CMenuData + 1 > 3))	StatusBarIcon(SBUTINACT, Crew[plr].StBX + 41, 181, tpge);
		else StatusBarIcon(VhcOrderBut[Crew[plr].CMenuData + 1], Crew[plr].StBX + 41, 181, tpge);
		if ((BSA.RuleSet < R_RADICAL) && (Crew[plr].CMenuData + 2 > 3))	StatusBarIcon(SBUTINACT, Crew[plr].StBX + 54, 181, tpge);
		else StatusBarIcon(VhcOrderBut[Crew[plr].CMenuData + 2], Crew[plr].StBX + 54, 181, tpge);
		StatusBarIcon(2, Crew[plr].StBX + 28, 190, tpge);
		StatusBarIcon(SBUTEXIT, Crew[plr].StBX + 41, 190, tpge);
		StatusBarIcon(3, Crew[plr].StBX + 54, 190, tpge);
		DrawWealthBox(plr, Crew[plr].StBX + 68, 181, tpge);
		DrawCountBox(Crew[plr].ManCnt, BSA.Plr[plr].Col, tpge, Crew[plr].StBX + 68, 189);
		break;
	case CMHOSTILITY:
		B4Move(GFXPage, 132 / 4 + 0, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		for (cnt = 0; cnt < 3; cnt++)
			if (BSA.Plr[cnt].Col > -1)
			{
				StatusBarIcon(SBUTBLANK, Crew[plr].StBX + 28 + 13 * cnt, 181, tpge);
				if (plr != cnt) RWCopy(GFXPage, 108 + 10 * Crew[plr].Hostile[cnt], 194, tpge, Crew[plr].StBX + 30 + 13 * cnt, 182, 10, 6, 1);
				ASpriteC(&ManSpr[0], tpge, Crew[plr].StBX + 28 + 13 * cnt, 181, BSA.Plr[cnt].Col);
			}
			else StatusBarIcon(SBUTINACT, Crew[plr].StBX + 28 + 13 * cnt, 181, tpge);

			StatusBarIcon(SBUTBLANK, Crew[plr].StBX + 28, 190, tpge);
			StatusBarIcon(SBUTEXIT, Crew[plr].StBX + 41, 190, tpge);
			StatusBarIcon(SBUTBLANK, Crew[plr].StBX + 54, 190, tpge);
			break;
	case CMBRIDGE:
		B4Move(GFXPage, 132 / 4 + 10, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		StatusBarIcon(4, Crew[plr].StBX + 28, 181, tpge);
		StatusBarIcon(5, Crew[plr].StBX + 41, 181, tpge);
		StatusBarIcon(6, Crew[plr].StBX + 54, 181, tpge);
		StatusBarIcon(7, Crew[plr].StBX + 28, 190, tpge);
		StatusBarIcon(SBUTEXIT, Crew[plr].StBX + 41, 190, tpge);
		StatusBarIcon(7, Crew[plr].StBX + 54, 190, tpge);
		break;
	case CMBUILD:
		B4Move(GFXPage, 132 / 4 + 5, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		if (StructBuildRS[Crew[plr].CMenuData + 0] <= BSA.RuleSet)
			StatusBarIcon(StrBuildBut[Crew[plr].CMenuData + 0], Crew[plr].StBX + 28, 181, tpge);
		else StatusBarIcon(SBUTINACT, Crew[plr].StBX + 28, 181, tpge);
		if (StructBuildRS[Crew[plr].CMenuData + 1] <= BSA.RuleSet)
			StatusBarIcon(StrBuildBut[Crew[plr].CMenuData + 1], Crew[plr].StBX + 41, 181, tpge);
		else StatusBarIcon(SBUTINACT, Crew[plr].StBX + 41, 181, tpge);
		if (StructBuildRS[Crew[plr].CMenuData + 2] <= BSA.RuleSet)
			StatusBarIcon(StrBuildBut[Crew[plr].CMenuData + 2], Crew[plr].StBX + 54, 181, tpge);
		else StatusBarIcon(SBUTINACT, Crew[plr].StBX + 54, 181, tpge);
		StatusBarIcon(2, Crew[plr].StBX + 28, 190, tpge);
		StatusBarIcon(SBUTEXIT, Crew[plr].StBX + 41, 190, tpge);
		StatusBarIcon(3, Crew[plr].StBX + 54, 190, tpge);
		break;
	case CMSRCPIPE:
		B4Move(GFXPage, 132 / 4 + 15, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		StatusBarIcon(20, Crew[plr].StBX + 28, 181, tpge);
		StatusBarIcon(22, Crew[plr].StBX + 41, 181, tpge);
		StatusBarIcon(21, Crew[plr].StBX + 54, 181, tpge);
		StatusBarIcon(24, Crew[plr].StBX + 28, 190, tpge);
		StatusBarIcon(0, Crew[plr].StBX + 41, 190, tpge);
		StatusBarIcon(23, Crew[plr].StBX + 54, 190, tpge);
		break;
	case CMSURRENDER:
		StatusBarIcon(32, Crew[plr].StBX + 41, 181, tpge);
		StatusBarIcon(0, Crew[plr].StBX + 41, 190, tpge);
		break;
	case CMCOMMAND:
		B4Move(GFXPage, 132 / 4 + 0, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		StatusBarIcon(18, Crew[plr].StBX + 28, 181, tpge);
		StatusBarIcon(34, Crew[plr].StBX + 41, 181, tpge);
		StatusBarIcon(SBUTBLANK, Crew[plr].StBX + 54, 181, tpge);

		StatusBarIcon(BSA.AllowSurrender ? 32 : SBUTINACT, Crew[plr].StBX + 28, 190, tpge);
		StatusBarIcon(SBUTEXIT, Crew[plr].StBX + 41, 190, tpge);
		StatusBarIcon(rsr ? 19 : SBUTINACT, Crew[plr].StBX + 54, 190, tpge);
		break;
	case CMMAGIC:
		B4Move(GFXPage, 132 / 4 + 20, 180, tpge, Crew[plr].StBX / 4 + 1, 180, 5, 20);
		for (cnt = 0; cnt < 3; cnt++)
			StatusBarIcon(MagicBut[Crew[plr].CMenuData + cnt], Crew[plr].StBX + 28 + 13 * cnt, 181, tpge, (MagicLevel(plr) < MagicEnergy[Crew[plr].CMenuData + cnt]));
		StatusBarIcon(SBUTLEFT, Crew[plr].StBX + 28, 190, tpge);
		StatusBarIcon(SBUTEXIT, Crew[plr].StBX + 41, 190, tpge);
		StatusBarIcon(SBUTRIGHT, Crew[plr].StBX + 54, 190, tpge);
		DrawMagicLevel(100L * MagicLevel(plr) / 10000L, Crew[plr].StBX + 68, 181);
		break;
	}
}

void DrawStatusBar(BYTE tpge, BYTE plr)
{
	int cnt;
	// Wood back
	B4Move(GFXPage, 58, 180, tpge, Crew[plr].StBX / 4 + 0, 180, 10, 20);
	B4Move(GFXPage, 58, 180, tpge, Crew[plr].StBX / 4 + 10, 180, 10, 20);
	B4Move(GFXPage, 58, 180, tpge, Crew[plr].StBX / 4 + 20, 180, (Crew[plr].StBWdt - 80) / 4, 20);

	LPage(tpge);
	if (BSA.Plr[plr].Eliminate)
	{
		if (BSA.Plr[plr].Info)
		{
			if (BSA.Plr[plr].Eliminate == 1)
				sprintf(OSTR, "%s|eliminated!", BSA.Plr[plr].Info->name);
			else
				sprintf(OSTR, "%s|surrendered!", BSA.Plr[plr].Info->name);
			TOutS(OSTR, Crew[plr].StBX + 55, 183, CRed, CBlack, 1);
		}
	}
	else
	{
		if (Crew[plr].CMenu == CMNOMENU) StatusDisplay(tpge, plr);
		else MenuDisplay(tpge, plr);
	}

	//SystemMsg("status bar redrawn");
	Crew[plr].RedrStB = 0;
}

extern WORD RunSec;

void DrawClock(BYTE tpge)
{
	RWCopy(GFXPage, 49, 118, tpge, 266, 8, 52, 10, 0);
}

void TimeOut(void)
{
	WORD hrs, mns, secs;
	hrs = RunSec / 3600; mns = (RunSec - 3600 * hrs) / 60; secs = (RunSec - 3600 * hrs - 60 * mns);
	sprintf(OSTR, "%01u:%02u:%02u %c(%02d)", hrs, mns, secs, CGray4, Config.GameSpeed);
	LPage(CPGE); SOut(OSTR, 315, 10, CWhite, CBlack, 2);

	if (FPS < LowFPSMark) LowFPS = 1; else LowFPS = 0;
	//sprintf(OSTR,"%04d FPS",FPS); SOut(OSTR,319,0,LowFPS ? CRed : CWhite,CBlack,2);
	FPS = 0;

	if (LowMem) SOut("LOWMEM", 319, 5, CRed, CBlack, 2);
}

//-------------------------- DrawScreen Function -----------------------------

void DrawScreen(void)
{
	int cnt;
	MANTYPE   *cman;
	ROCKTYPE  *crck;
	VEHICTYPE *cvhc;
	STRUCTYPE *cstrc;
	LINETYPE  *cline;

	CPageY(BackPage, BackVSPos, CPGE, 20, 160);

	LPage(CPGE);

	DrawPXS();
	DrawExplosion(&Weather.expln);
	DrawLightN();

	if (!UserPref.LineBeforeStrc) { cline = FirstLine; while (cline) { DrawLine(cline); cline = cline->next; } }
	cstrc = FirstStruct; while (cstrc) { DrawStruct(cstrc); cstrc = cstrc->next; }
	if (UserPref.LineBeforeStrc) { cline = FirstLine; while (cline) { DrawLine(cline); cline = cline->next; } }
	cvhc = FirstVehic; while (cvhc) { DrawVehicA(cvhc); cvhc = cvhc->next; }
	for (cman = FirstAnimal; cman; cman = cman->next) DrawAnimal(cman);
	for (cnt = 0; cnt < 3; cnt++)
		for (cman = Crew[cnt].FirstMan; cman; cman = cman->next)
			DrawMan(cman, BSA.Plr[cnt].Col, (Crew[cnt].ConFlash > 0));
	cvhc = FirstVehic; while (cvhc) { DrawVehicB(cvhc); cvhc = cvhc->next; } // !!!
	crck = FirstRock; while (crck) { DrawRock(crck); crck = crck->next; }
	cstrc = FirstStruct; while (cstrc) { DrawStructSpecial(cstrc); cstrc = cstrc->next; }
	cvhc = FirstVehic; while (cvhc) { DrawVehicC(cvhc); cvhc = cvhc->next; }

	for (cnt = 0; cnt < 3; cnt++) if (Crew[cnt].CursorFlash) DrawCursor(Crew[cnt].Cursor);

	DrawKeyConD(CPGE);

	if (UserPref.GameMsgOn || (BSA.SMode == S_MISSION)) DrawGameMessage();

	CPageY(!CPGE, 0, CPGE, 0, 20); CPageY(!CPGE, 180, CPGE, 180, 20);
	RestoreMouseBack(CPGE);

	if (Crew[0].RedrStB) DrawStatusBar(CPGE, 0);
	if (Crew[1].RedrStB) DrawStatusBar(CPGE, 1);
	if (Crew[2].RedrStB) DrawStatusBar(CPGE, 2);

	if (MouseCon.status) DrawMouseCon(CPGE);

	BSprite(&ThreeSpr, CPGE, 208, 20);

	/*if (SysMsgBuf[0])
	  {
	  LPage(CPGE);  SOut(SysMsgBuf,0,0,CWhite,CBlack);
	  LPage(!CPGE); SOut(SysMsgBuf,0,0,CWhite,CBlack);
	  //if (SEqual("ERR",SysMsgBuf)) getch();
	  SysMsgBuf[0]=0;
	  }*/

	  /*  extern DWORD FramePos;
		if (!Tick10)
		  {
		  sprintf(OSTR,"%lu",FramePos);
		  LPage(CPGE); SOut(OSTR,250,0,CWhite,CBlack);
		  }*/

	FPS++;

	if (CPGE) { FPhase++; if (FPhase > 3) FPhase = 0; }
}

//------------------------- BS GFX Initialization ----------------------------

BYTE InitBSGraphics(void)
{
	BYTE cpal[256 * 3];
	if (LoadAGC2PageV1("C3RGRAFX.GRP|C3BATSEQ.AGC", GFXPage, 0, 0, 0, 0, cpal, 0)) return 0;
	//if (LoadAGC2PageV1("C3BATSEQ.AGC",GFXPage,0,0,0,0,cpal,0)) return 0;
	SetDAC(16, 240, cpal + 16 * 3);
	if (!InitBSSprites()) return 0;
	return 1;
}

void DeInitBSGraphics(void)
{
	DeInitBSSprites();
}