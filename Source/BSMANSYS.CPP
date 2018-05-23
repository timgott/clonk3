/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design CLONK 3 Radikal Man & Animal System Module

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "standard.h"

#include "clonk_bs.h"

#include "bsexcsys.h"
#include "bsgfxsys.h"
#include "bspxssys.h"
#include "bsweasys.h"
#include "bsvhcsys.h"
#include "bsstrsys.h"

#include "RandomWrapper.h"

//------------------------- Main Global Externals ----------------------------

extern char OSTR[500];

extern USERPREF UserPref;
extern BSATYPE BSA;

//-------------------------- Vehicle Data Definitions -------------------------

#define VHDWipfLoad   back[29]
#define VHDXBowDir    back[2]
#define VHDXBowDirCh  back[3]

//---------------------- Global Game Object Variables ------------------------

CREWTYPE Crew[3];

MANTYPE *FirstAnimal;

int DADelay = 0;

//---------------------- Misc ManControl Game Data -----------------------------

const int DigTableX[7] = { 2,2,2,3,6,6,5 };
const int DigTableY[7] = { 1,3,5,4,5,3,1 };

//------------------- Game Object Info (Crew/Plr/System) ---------------------------

BYTE NotHostile(int plr1, int plr2)
{
	if (!Inside(plr1, 0, 2) || !Inside(plr2, 0, 2)) return 0;
	if (!Crew[plr1].Hostile[plr2] && !Crew[plr2].Hostile[plr1]) return 1;
	return 0;
}

void ScoreGain(int plr, int score)
{
	if (Inside(plr, 0, 2))
		BSA.Plr[plr].ScoreGain = BoundBy(BSA.Plr[plr].ScoreGain + score, 0, 65000);
}

int HighRank(BYTE plr)
{
	int hirank = 0;
	MANTYPE *cman;
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
		if (cman->act != MADEAD)
			if (cman->mi->rank > hirank) hirank = cman->mi->rank;
	return hirank;
}

MANTYPE *HighRankMan(BYTE plr)
{
	MANTYPE *cman, *himan = NULL;
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
		if ((cman->act != MADEAD) && !cman->mi->dead)
			if (!himan || (cman->mi->rank > himan->mi->rank))
				himan = cman;
	if (!himan) return NULL;
	return himan;
}

MANINFO *HighRankManInfo(BYTE plr)
{
	MANTYPE *himan;
	himan = HighRankMan(plr);
	if (!himan) return NULL;
	return himan->mi;
}

// For castle flag
#define CstFlag energy

BYTE LostCaptureFlag(BYTE plr) // Evaluates in CaptureTFlag-Melees only
{
	BYTE lost = 0, nocst;
	STRUCTYPE *cstrc;
	MANTYPE *cman;
	ROCKTYPE *crck;
	if ((BSA.SMode == S_MELEE) && (BSA.GPlrElm == 2))
	{
		lost = 1; nocst = 1; // If plr has a castle, flag must be on the castle
		for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)
			if ((cstrc->type == STCASTLE) && (cstrc->con >= 1000) && (cstrc->owner == plr))
			{
				nocst = 0; if (cstrc->CstFlag) lost = 0;
			}
		if (lost && nocst)
		{ // If plr has no castle, flag must be free or carried by own man
			for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
				if ((cman->carr == FLAG) && (cman->carrp == plr))
					lost = 0;
			if (lost)
				for (crck = FirstRock; crck; crck = crck->next)
					if ((crck->type == FLAG) && (crck->phase == plr))
						lost = 0;
		}
	}
	return lost;
}

int AnimalCount(int antype)
{
	MANTYPE *canm;
	VEHICTYPE *cvhc;
	ROCKTYPE *crck;
	int rval = 0;
	for (canm = FirstAnimal; canm; canm = canm->next)
		if (canm->type == antype)
			if (canm->act != MADEAD)
				rval++;
	if (antype == MNWIPF)
		for (cvhc = FirstVehic; cvhc; cvhc = cvhc->next)
			if (cvhc->type == VHLORRY)
				rval += cvhc->VHDWipfLoad;
	if (antype == MNMONSTER)
		for (crck = FirstRock; crck; crck = crck->next)
			if (crck->type == MONSTEGG)
				rval++;
	return rval;
}

//------------------------------ (Man Control) -------------------------------

//----- Data Organization

MANTYPE *NewMan(MANTYPE **firstman, MANINFO *mi, BYTE con, BYTE type, int x, int y, int tx, BYTE act, int phase, int strn, int ydir, int xdir, int carr, void *tptr)
{
	MANTYPE *nman;
	if (!(nman = new MANTYPE)) { LowMem = 1; return NULL; }

	nman->mi = mi;

	nman->con = con;
	nman->type = type;
	nman->x = x; nman->y = y; nman->tx = tx;
	nman->act = act; nman->phase = phase; nman->strn = strn;
	nman->ydir = ydir; nman->xdir = xdir; nman->carr = carr; nman->carrp = 0;
	nman->onf = 0; nman->del = 0;

	// del-usage:
	//
	// MAWALK	stand in front of vhc: account push delay
	// MAFIGHT    fight duration, negative del: win fight
	// MATHROW    ---
	// MAFLY      jump to stand flag
	// MASWIM     diving oxygene
	// MADIG	dig-begin-count
	// MABRIDGE	loam/steel-flag
	// MABUILD	---
	// MAPUSH	pushing direction
	// MADEAD	body decay
	// MAHOME	---

	nman->tptr = tptr;

	nman->next = *firstman;
	*firstman = nman;

	return nman;
}

MANTYPE *NewAnimal(MANTYPE **firstman, BYTE type, int x, int y, int tx, BYTE act, int phase, int strn, int ydir, int xdir, int carr, void *tptr)
{
	return NewMan(firstman, NULL, 0, type, x, y, tx, act, phase, strn, ydir, xdir, carr, tptr);
}

void DeleteMen(MANTYPE *firstman)
{
	MANTYPE *temptr;
	while (firstman) { temptr = firstman->next; delete firstman; firstman = temptr; }
}

void ClearMenTPtr(void *tptr)
{
	int cnt;
	MANTYPE *cman;
	for (cnt = 0; cnt < 3; cnt++)
		for (cman = Crew[cnt].FirstMan; cman; cman = cman->next)
			if (cman->tptr == tptr) cman->tptr = NULL;
	for (cman = FirstAnimal; cman; cman = cman->next)
		if (cman->tptr == tptr) cman->tptr = NULL;
}

void CursorAdjust(BYTE plr);

void CheckCursorDeletion(MANTYPE *mptr)
{
	int plr;
	for (plr = 0; plr < 3; plr++)
		if (Crew[plr].Cursor == mptr)
		{
			Crew[plr].Cursor = NULL;
			CursorAdjust(plr);
		}
}

DWORD DeathRemovalCheck(MANTYPE **firstman)
{
	MANTYPE *mptr = *firstman;
	MANTYPE *next, *prev = NULL;
	DWORD lcnt = 0;
	while (mptr)
	{
		next = mptr->next;

		if (((mptr->act == MADEAD) && (mptr->del > 2000)) || (mptr->type == MNNOMAN))
		{
			ClearMenTPtr(mptr); // Can anything other point to a man?
			ClearGMsgTPtr(mptr);
			CheckCursorDeletion(mptr);
			if (prev) prev->next = next;
			else *firstman = next;
			delete mptr;
		}
		else
		{
			prev = mptr;
			lcnt++;
		}

		mptr = next;
	}
	return lcnt;
}

//----- Action Control Support Functions

void ManWalk(MANTYPE *mptr, BYTE stand)
{
	mptr->act = MAWALK;
	if (stand) mptr->tx = mptr->x;
	mptr->phase = 0;
	mptr->del = 0;
	mptr->xdir = Sign(mptr->xdir);
	mptr->ydir = 0;
}

void ManFly(MANTYPE *mptr, int xdir, int ydir, BYTE stand)
{
	mptr->act = MAFLY;
	mptr->phase = random(4);
	mptr->xdir = xdir; mptr->ydir = ydir;
	mptr->del = stand; // del is stand on land [make del usage table somewhere?!]
}

void ManSwim(MANTYPE *mptr, int ydir)
{
	mptr->act = MASWIM;
	mptr->phase = random(4);
	mptr->ydir = ydir;
	mptr->xdir = Sign(mptr->xdir);
	mptr->del = 50; // del is oxygen
}

void ManDig(MANTYPE *mptr, int dir) // actual dir gets sign from xdir
{                                 // dir is height direction only
	mptr->act = MADIG;
	mptr->phase = 0;
	if (mptr->xdir == 0) mptr->xdir = 1;
	mptr->xdir = Sign(mptr->xdir)*Abs(dir);
	mptr->ydir = 0;

	mptr->del = 5 + 25 * (100 - UserPref.DoubleDigSpeed) / 100; // Dig-begin count down

	if (Inside(GBackPix(mptr->x + 0, mptr->y + 8), CGranite1, CGold2) || Inside(GBackPix(mptr->x + 6, mptr->y + 8), CGranite1, CGold2))
	{
		mptr->xdir = 3 * Sign(mptr->xdir);
		if (mptr->type == MNMAN) GSTP = SNDMETAL;
	}
}

//--------------------- (Round Object CrossChecking) -------------------------

void LoseTPtr(MANTYPE *mptr)
{
	if (mptr->type == MNMAN)
		if (mptr->tptr && (mptr->carr == LINECON))
			if ((mptr->act != MABUILD) && (mptr->act != MAPUSH))
				mptr->carr = NOROCK; // (Lines are dropped)
	mptr->tptr = NULL;
}

void CheckCastleFlagCollect(MANTYPE *mptr)
{
	STRUCTYPE *cstrc;
	int cnt;
	if (mptr->type == MNMAN) if (mptr->carr == NOROCK) if (mptr->act == MAFLY)
		for (cnt = 0; cnt < 3; cnt++)
			if (BSA.Plr[cnt].Col > -1)
				if (Crew[cnt].CaFlX > -1)
					if (Inside(mptr->x + 3 - Crew[cnt].CaFlX, -4, +4))
						if (Inside(mptr->y + 3 - Crew[cnt].CaFlY, -4, +3))
						{
							mptr->carr = FLAG; mptr->carrp = cnt;
							Crew[cnt].CaFlX = Crew[cnt].CaFlX = -1;
							for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)
								if ((cstrc->type == STCASTLE) && (cstrc->owner == cnt))
									if ((cstrc->con >= 1000) && cstrc->CstFlag)
										cstrc->CstFlag = 0;
						}
}

void CheckMen2Rocks(void) // Every Tick1
{
	ROCKTYPE *crck;
	MANTYPE *mptr;
	int cnt;
	for (cnt = 0; cnt < 3 + Tick2; cnt++) // To animals every second
		for (mptr = ((cnt == 3) ? FirstAnimal : Crew[cnt].FirstMan); mptr; mptr = mptr->next)
			if (mptr->act < MADEAD)
			{
				if (mptr->act != MAFLY) // To Rocks
					for (crck = FirstRock; crck; crck = crck->next)
						if (crck->type != NOROCK)
						{
							// Collect
							if (mptr->type == MNMAN)
								if (crck->act == RADEAD) if (!Crew[cnt].DontPickUp)
									if ((crck->x == mptr->x + 2) && Inside(crck->y - mptr->y, 0, 8))
										if ((mptr->carr == NOROCK) || (Inside(crck->type, ARROW, BARROW) && (mptr->carr == crck->type) && (mptr->carrp < 5)))
											switch (crck->type)
											{
											case TFLINT:
												if (crck->phase) Explode(crck->x, crck->y, 10, crck->thrby);
												else mptr->carr = TFLINT;
												crck->type = NOROCK;
												break;
												/*case PLANT3:
										  mptr->carr=PLANT3;
										  crck->phase-=30;
										  if (crck->phase>=0) crck->type=BoundBy(PLANT1+crck->phase/10,PLANT1,PLANT3);
										  else crck->type=NOROCK;
										  mptr->carrp=30;
												  break;*/
											case ARROW: case FARROW: case BARROW:
												if ((crck->type == FARROW) && (crck->phase >= 10))
												{
													mptr->onf = 1;
												}
												else
												{
													if (mptr->carr == NOROCK) { mptr->carr = crck->type; mptr->carrp = 0; }
													mptr->carrp += crck->phase;
													if (mptr->carrp > 5) { crck->phase = mptr->carrp - 5; mptr->carrp = 5; }
													else crck->type = NOROCK;
													Crew[cnt].RedrStB = 1;
												}
												break;
											case MONSTEGG:
												if (crck->phase < 10)
												{
													mptr->carr = crck->type;
													mptr->carrp = crck->phase;
													crck->type = NOROCK;
												}
												break;
											case PLANT1: case PLANT2: case PLANT3: case ROCKPXS:
												break;
											default:
												mptr->carr = crck->type;
												mptr->carrp = crck->phase;
												crck->type = NOROCK;
												break;
											}
							// Get Hit
							if ((crck->act == RAROLL) || (crck->act == RAFLY))
								if (crck->type < PLANT1)
									if (Inside(crck->x - mptr->x, 0, 4) && Inside(crck->y - mptr->y, 0, 6))
									{
										if (Inside(crck->type, ARROW, BARROW)) mptr->strn -= 2;
										else mptr->strn -= 5;
										LoseTPtr(mptr);
										if ((mptr->type == MNMAN) || (mptr->type == MNWIPF))
										{
											if ((crck->type == FARROW) && (crck->phase >= 10)) mptr->onf = 1;
											ManFly(mptr, Sign(mptr->xdir)*(-1), 0, 0);
											if (mptr->type == MNMAN) GSTP = SNDHURT1 + random(3);
										}
										else
										{
											mptr->tx = BoundBy(mptr->x + crck->xdir * 30, 12, 305);
										}

										if (Inside(crck->thrby, 0, 2)) if ((cnt == 3) || !NotHostile(cnt, crck->thrby))
										{			    // animal hit/kill count only
											ScoreGain(crck->thrby, 2); // by game mode
											if (mptr->strn <= 0)
												ScoreGain(crck->thrby, 10); // Kills extra
										}
									}
						}
				CheckCastleFlagCollect(mptr);
			}
}

void CheckMenFight(int plr1, int plr2)
{
	MANTYPE *fman1, *cman2;
	for (fman1 = Crew[plr1].FirstMan; fman1; fman1 = fman1->next)
		if ((fman1->act == MAWALK) || Inside(fman1->act, MASWIM, MAPUSH))
			for (cman2 = Crew[plr2].FirstMan; cman2; cman2 = cman2->next)
				if ((cman2->act == MAWALK) || Inside(cman2->act, MASWIM, MAPUSH))
					if (Inside(fman1->x - cman2->x, -3, 3))
						if (Inside(fman1->y - cman2->y, -5, 5))
						{
							fman1->act = cman2->act = MAFIGHT;
							fman1->phase = random(4); cman2->phase = random(4);
							if (fman1->onf) cman2->onf = 1; if (cman2->onf) fman1->onf = 1;
							LoseTPtr(fman1); LoseTPtr(cman2);
							// Adjust facing
							fman1->xdir = +1; cman2->xdir = -1; if (fman1->x > cman2->x) { fman1->xdir = -1; cman2->xdir = +1; }
							// Win/lose
							fman1->del = cman2->del = 35 - Min(Abs(fman1->strn - cman2->strn), 10);
							if (fman1->strn > cman2->strn) fman1->del *= -1; // Negative del
							if (fman1->strn < cman2->strn) cman2->del *= -1; // means win fight
							// Experience gains (in advance to fight)
							if (fman1->del < 0) fman1->mi->exp += 30;
							else fman1->mi->exp += 10;
							if (cman2->del < 0) cman2->mi->exp += 30;
							else cman2->mi->exp += 10;               // Kills extra
							if (fman1->strn - 5 - 5 * (fman1->del > 0) <= 0) { cman2->mi->exp += 50; ScoreGain(plr2, 10); }
							if (cman2->strn - 5 - 5 * (cman2->del > 0) <= 0) { fman1->mi->exp += 50; ScoreGain(plr1, 10); }
							break;
						}
}

void MenCrossChecking(void) // Every Tick3 or Tick5
{
	if (BSA.Plr[0].Col > -1) if (!BSA.Plr[0].Eliminate)
	{
		if (BSA.Plr[1].Col > -1) if (!BSA.Plr[1].Eliminate)
			if (Crew[0].Hostile[1] || Crew[1].Hostile[0])
				CheckMenFight(0, 1);
		if (BSA.Plr[2].Col > -1) if (!BSA.Plr[2].Eliminate)
			if (Crew[0].Hostile[2] || Crew[2].Hostile[0])
				CheckMenFight(0, 2);
	}
	if (BSA.Plr[1].Col > -1) if (!BSA.Plr[1].Eliminate && !BSA.Plr[2].Eliminate)
		if (BSA.Plr[2].Col > -1) if (Crew[1].Hostile[2] || Crew[2].Hostile[1])
			CheckMenFight(1, 2);
}

void CheckSharkAttack(MANTYPE *shk, MANTYPE *cman)
{
	if (shk->act != MASWIM) return;
	if (!shk->tptr) if (!Tick10) // Not hunting yet
		while (cman)
		{
			if (cman->act == MASWIM) if ((cman->type != MNSHARK) && (cman->type != MNMONSTER))
				if (Inside(cman->y - shk->y, -50, +50)) if (random(3))
				{
					shk->tptr = cman; return;
				}
			cman = cman->next;
		}
}

void CheckMonsterHit(MANTYPE *mns, MANTYPE *cman)
{
	if ((mns->act == MAWALK) || (mns->act == MASWIM))
		while (cman)
		{
			if ((cman->act < MADEAD) && (cman->act != MAFLY))
				if (Inside(mns->x - cman->x, -8, +8) && Inside(mns->y - cman->y, -5, +5))
				{
					cman->strn -= 10;
					LoseTPtr(cman);
					ManFly(cman, mns->xdir*(-1), 0, 0);
					GSTP = SNDHURT1 + random(3);
				}
			cman = cman->next;
		}
}

void CheckAnimals2Men(void) // Every Tick3 or Tick5
{
	MANTYPE *canm;
	int ccrew, chkcnt;
	ccrew = random(3);                   // Just random (for 2 plrs)
	if (!Crew[ccrew].FirstMan) if (random(2)) { ++ccrew %= 3; ++ccrew %= 3; }
	for (chkcnt = 0; chkcnt < 3; chkcnt++, ++ccrew %= 3)
	{
		if (Crew[ccrew].FirstMan)
			for (canm = FirstAnimal; canm; canm = canm->next)
			{
				switch (canm->type)
				{
				case MNSHARK: CheckSharkAttack(canm, Crew[ccrew].FirstMan); break;
				case MNMONSTER: CheckMonsterHit(canm, Crew[ccrew].FirstMan); break;
					//case MNWIPF: break; // nothing for now...
				}
			}
	}
}

void CheckSharks2Animals(void) // Every Tick10
{
	MANTYPE *anm1, *anm2;
	for (anm1 = FirstAnimal; anm1; anm1 = anm1->next)
		if (anm1->type == MNSHARK)
			CheckSharkAttack(anm1, FirstAnimal);
}

void CheckClonkSting(int x, int y)
{
	int cnt;
	MANTYPE *cman;
	for (cnt = 0; cnt < 3; cnt++)
		for (cman = Crew[cnt].FirstMan; cman; cman = cman->next)
			if (!Rnd3())
				if ((cman->act != MAFLY) && (cman->act < MADEAD))
					if (Inside(x - cman->x, 1, 7) && Inside(y - cman->y, 1, 8))
					{
						cman->strn -= 10;
						LoseTPtr(cman);
						ManFly(cman, 0, 15, 0);
						GSTP = SNDHURT1 + random(3);
						return;
					}
}

void CheckManHit(int tx, int ty, int rad)
{
	int cnt;
	MANTYPE *cman;
	for (cnt = 0; cnt < 4; cnt++)
		for (cman = ((cnt == 3) ? FirstAnimal : Crew[cnt].FirstMan); cman; cman = cman->next)
			if (cman->act < MADEAD)
				if (Inside(cman->x - tx, -rad, rad) && Inside(cman->y - ty, -rad, rad))
				{
					cman->strn -= 10;
					LoseTPtr(cman);
					if (cman->type != MNSHARK) ManFly(cman, cman->xdir*(-1), 0, 0);
				}
}

void CheckManIgnition(int tx, int ty)
{
	int cnt;
	MANTYPE *mptr;
	for (cnt = 0; cnt < 3; cnt++)
		for (mptr = Crew[cnt].FirstMan; mptr; mptr = mptr->next)
			if ((mptr->act == MAWALK) || (mptr->act == MAPUSH))
				if (Inside(mptr->x + 2 - tx, -4, 4) && Inside(mptr->y + 4 - ty, -5, 5))
					mptr->onf = 1;
	for (mptr = FirstAnimal; mptr; mptr = mptr->next)
		if (mptr->type == MNWIPF) if (mptr->act == MAWALK)
			if (Inside(mptr->x + 2 - tx, -4, 4) && Inside(mptr->y + 4 - ty, -5, 5))
				mptr->onf = 1;
}

void ExecCrossChecking(void) // Called from BSEXCSYS
{
	CheckMen2Rocks();
	if ((!LowFPS && !Tick3) || (LowFPS && !Tick5))
	{
		MenCrossChecking();
		CheckAnimals2Men();
	}
	if (!Tick20) CheckSharks2Animals();
}

//----- Action Execution Functions

void ConstructLine(MANTYPE *mptr)
{
	LINETYPE *tline;
	if (mptr->type != MNMAN) { RoundError("safety: conline w\out man"); return; }
	if (mptr->tptr)
	{
		if (mptr->carr != LINECON) { RoundError("safety: MAN-TPTR w/out LINECON! 1"); return; }
		// Set new line target loc
		tline = (LINETYPE*)mptr->tptr;
		if ((tline->x[tline->lsec] != mptr->x + 4) || (tline->y[tline->lsec] != mptr->y + 4)) tline->mfd = 1;
		tline->x[tline->lsec] = mptr->x + 4; tline->y[tline->lsec] = mptr->y + 6;
	}
}

void AbandonLine(MANTYPE *mptr) // for homing and srcpipe-menu only...
{                             // and menu-activation
	LINETYPE *tline;
	if (!mptr || !mptr->tptr) return;
	if (mptr->type != MNMAN) return; // How about animal tptrs? (?? -> see above)
	if (mptr->carr != LINECON) { RoundError("safety: MAN-TPTR w/out LINECON! 2"); return; }
	tline = (LINETYPE*)mptr->tptr;
	tline->type = LNNOLINE;
}

BYTE HomeMan(MANTYPE *mptr, BYTE plr, void *htptr, int homeat)
{                                     // Returns homing okay
// Home a flag-check
	if (mptr->carr == FLAG)
	{
		if (!Inside(mptr->carrp, 0, 2)) { RoundError("safety: home man invalid flag owner"); return 0; }
		if (mptr->carrp == plr) return 0; // Can't home own flag
	}
	// Regular homing
	if (mptr->tptr) if (mptr->act == MAWALK) AbandonLine(mptr);
	mptr->act = MAHOME; mptr->x = mptr->y = mptr->tx = mptr->phase = mptr->xdir = mptr->ydir = 0;
	mptr->tptr = htptr;
	HomeRock(mptr->carr, mptr->carrp, homeat); mptr->carr = NOROCK;
	return 1;
}

BYTE StandardManMovement(MANTYPE *mptr, BYTE mxphase, int plr)
{
	VEHICTYPE *tvhc;
	int stnpix, ctco;
	// Falling check/Height adjust
	ctco = 0;
	while (!Inside(GBackPix(mptr->x + 4, mptr->y + 8 + ctco), CSolidL, CSolidH) && mptr->y + 8 + ctco < BackGrSize) ctco++;
	if (ctco > 4) // Fall, Wipfs might jump
	{
		if ((mptr->type == MNWIPF) && random(2))
		{
			ctco = Sign(mptr->xdir); if (!ctco) ctco = Sign(mptr->tx - mptr->x);
			if (!ctco) ctco = 1 - 2 * random(2);
			ManFly(mptr, ctco, -7, 0);
		}
		else ManFly(mptr, 0, 30, 0);
		return 0;
	}
	// No Fall, walk / Height adjust
	if (ctco == 0) if (Inside(GBackPix(mptr->x + 4, mptr->y + 0), CSolidL, CSolidH)) ctco = +1; // no upward movm into solid
	mptr->y += ctco - 1;
	// Walk right
	if (mptr->x < mptr->tx)
	{
		mptr->xdir = +1;
		if (!Inside(GBackPix(mptr->x + 6, mptr->y + 2), CSolidL, CSolidH) && !Inside(GBackPix(mptr->x + 6, mptr->y + 0), CSolidL, CSolidH))
		{
			mptr->x++; if (Tick2) { mptr->phase++; if (mptr->phase > mxphase - 1) mptr->phase = 0; }
		}
	}
	// Walk left
	if (mptr->x > mptr->tx)
	{
		mptr->xdir = -1;
		if (!Inside(GBackPix(mptr->x + 1, mptr->y + 2), CSolidL, CSolidH) && !Inside(GBackPix(mptr->x + 1, mptr->y + 0), CSolidL, CSolidH))
		{
			mptr->x--; if (Tick2) { mptr->phase++; if (mptr->phase > mxphase - 1) mptr->phase = 0; }
		}
	}
	// Boundary FailSafe
	mptr->x = BoundBy(mptr->x, 0, 311);
	// In solid matter, shift up
	while (Inside(GBackPix(mptr->x + 4, mptr->y + 8), CSolidL, CSolidH) && !Inside(GBackPix(mptr->x + 4, mptr->y + 0), CSolidL, CSolidH)) mptr->y--;
	// In water, swim
	if (Inside(GBackPix(mptr->x + 4, mptr->y + 3), CLiqL, CLiqH))
	{
		ManSwim(mptr, 0); return 0;
	}
	// OnVehic movement
	stnpix = GBackPix(mptr->x + 4, mptr->y + 9);
	if (Inside(stnpix, CVhcL, CVhcH))
	{
		stnpix -= CVhcL; stnpix >>= 1; stnpix--; mptr->x += stnpix; mptr->tx += stnpix;
	}
	// Homing check (Man only)
	if (mptr->type == MNMAN) if (mptr->y < -6) if (!BSA.Realism.CstHome)
		for (tvhc = FirstVehic; tvhc; tvhc = tvhc->next)
			if ((tvhc->type == VHBALLOON) && (tvhc->owner >= 0))
				if (Inside(mptr->x + 4 - tvhc->x, 0, 16) && Inside(mptr->y + 4 - tvhc->y, 0, 20))
					if (NotHostile(tvhc->owner, plr))
						return (!HomeMan(mptr, plr, tvhc, tvhc->owner));
	// EQuake falling check (Man only)
	if (!Tick20) if (mptr->type == MNMAN)
		if (Ground.equake) if (!random(20))
			if (!Inside(GBackPix(mptr->x + 4, mptr->y + 9), CVhcL, CVhcH))
				if (!Inside(GBackPix(mptr->x + 4, mptr->y + 9), CGranite1, CGranite2))
				{
					ManFly(mptr, 0, 10, 0); return 0;
				}
	return 1;
}

void SetManTX2Push(MANTYPE *mptr, VEHICTYPE *tvhc)
{
	// Adjust target pushing position (cat/xbow might have turned)
	if (!Tick10) if ((tvhc->type == VHCATAPULT) || (tvhc->type == VHCROSSBOW))
		if (((mptr->del == +1) && (tvhc->data3 == 1)) || ((mptr->del == -1) && (tvhc->data3 == 0)))
			mptr->del *= -1;
	// Walk to pushing position
	if (mptr->del > 0) // del contains pushing direction
	{ // Push right
		if (!Inside(GBackPix(mptr->x + 4, mptr->y + 9), CVhcL, CGranite2) || (mptr->x < tvhc->x - 5))
			mptr->tx = tvhc->x - 5;
	}                       // On vehic/granite don't walk towards pushing
	else                      // position if in front of pushing target
	{ // Push Left
		if (!Inside(GBackPix(mptr->x + 4, mptr->y + 9), CVhcL, CGranite2) || (mptr->x > tvhc->x + 8))
			mptr->tx = tvhc->x + 8;
	}
	// Standing (Man only) Adjust facing
	if (mptr->x == mptr->tx)
		mptr->xdir = Sign(mptr->del);
}

void DeathAnnounce(MANTYPE *mptr)
{
	static char *datxt[2] = { "%s ist dead","%s|rests in peace" };
	if (mptr->type != MNMAN) return;
	if (DADelay > 0) return;
	sprintf_s(OSTR, datxt[random(2)], mptr->mi->name);
	GameMessage(OSTR, 0, 0, CGray4, mptr);
	DADelay = 30; // A second delay till next DA
}

extern void BlowPXS(int amt, int type, int phase, int fx, int fy, int lvl);

void MoveMan(MANTYPE *mptr, int plr)
{
	int ctco, stnpix, cnt, homeat, liqtype;
	STRUCTYPE *tstrc;
	VEHICTYPE *tvhc;
	BYTE flag, ubpix, lbpix;

	// Safety
	if (mptr->type == MNMAN) if (!mptr->mi)
	{
		RoundError("safety: man w/out maninfo"); mptr->type = MNNOMAN; return;
	}

	// Burning
	if (mptr->onf)
	{
		if (!Tick3)
		{
			mptr->strn--;
			stnpix = GBackPix(mptr->x + 4, mptr->y + 5);
			if (Inside(stnpix, CWaterS, CWaterT)) { mptr->onf = 0; GSTP = SNDPSHSH; }
			ApplyHeat2Back(mptr->x + 4, mptr->y + 5, 0);
		}
		PXSOut(PXSSMOKE, mptr->x + 2 + Tick3, mptr->y + 3, 10 + random(20));
	}

	// Death Check
	if (mptr->strn < 0) if (mptr->act != MAFLY)
	{
		LoseTPtr(mptr); mptr->act = MADEAD; mptr->del = 0; DeathAnnounce(mptr);
		if (mptr->type == MNMAN)
		{
			GSTP = SNDDEAD;
			mptr->mi->dead = 1;
			if (mptr->carr == FLAG)
			{
				NewRock(mptr->x + 2, mptr->y + 2, RADEAD, FLAG, 0, 0, mptr->carrp, -1);
				mptr->carr = NOROCK;
			}
		}
		if (mptr->type == MNMONSTER) mptr->x = BoundBy((mptr->x / 4) * 4, 0, 308);
	}

	// Safety (right here?) (double)
	mptr->tx = BoundBy(mptr->tx, 0, 311);

	// Actions:
	switch (mptr->act)
	{

	case MAWALK: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		if (!StandardManMovement(mptr, 4, plr)) break;
		if (mptr->type == MNMAN) ConstructLine(mptr);
		// Stand (check for push order) MNMAN only
		if (!Tick10) if (!mptr->tptr) if (mptr->type == MNMAN)
		{
			flag = 0;
			if (mptr->x == mptr->tx)
			{
				tvhc = FirstVehic;
				while (tvhc)
				{
					if ((tvhc->type == VHLORRY) || (tvhc->type == VHCATAPULT) || (tvhc->type == VHCROSSBOW) || ((tvhc->type == VHSAILBOAT) && !tvhc->back[29]))
						if (tvhc->x >= 0)
							if (Inside(mptr->x - tvhc->x, -8, 15) && Inside(mptr->y - tvhc->y, -9, +9))
								break;
					tvhc = tvhc->next;
				}
				if (tvhc)                                                                   // Sailboat on land only
					if ((tvhc->type == VHLORRY) || (tvhc->type == VHCATAPULT) || (tvhc->type == VHCROSSBOW) || ((tvhc->type == VHSAILBOAT) && !tvhc->back[29]))
						if (tvhc->x >= 0)
						{
							mptr->del++; flag = 1;
							if (mptr->del > Crew[plr].DontPushDelay / 10)
								if (Crew[plr].CMenu == CMNOMENU)
								{
									mptr->act = MAPUSH;          // -> ManPush(...)
									mptr->tptr = tvhc;
									// del contains pushing target direction
									mptr->del = -1; if (mptr->x < tvhc->x + 2) mptr->del = +1;
									if ((tvhc->type == VHCATAPULT) || (tvhc->type == VHCROSSBOW))
										mptr->del = +1 - 2 * tvhc->data3;
									SetManTX2Push(mptr, tvhc);
									mptr->phase = 0;
									GSTP = SNDGRAB;
								}
						}
			}
			if (!flag) mptr->del = 0;
		}
		break;

	case MAFIGHT: // - (Man only) - - - - - - - - - - - - - - - - - - - - - -
		if (!Tick3) { mptr->phase++; if (mptr->phase > 3) mptr->phase = 0; }
		if (mptr->del < 0) mptr->del++; else mptr->del--;
		if (!random(20)) GSTP = SNDHURT1 + random(3);
		if (Inside(mptr->del, -1, +1))
		{
			if (mptr->del < 0) // Win fight
			{
				mptr->act = MATHROW; mptr->phase = 0;
				mptr->strn -= 5;
			}
			else // Lose fight
			{
				mptr->strn -= 10;
				ManFly(mptr, mptr->xdir*(-1), 0, 0);
				if (Inside(GBackPix(mptr->x + 4, mptr->y - 2), CSolidL, CSolidH)) { mptr->ydir = 15; mptr->xdir = 0; }
			}
		}
		break;

	case MATHROW: // - (Man only) - - - - - - - - - - - - - - - - - - - - - - - -
	  // Falling check
		ctco = 0; while (!Inside(GBackPix(mptr->x + 4, mptr->y + 8 + ctco), CSolidL, CSolidH)) ctco++;
		if (ctco > 4) { ManFly(mptr, 0, 30, 0); break; }
		else mptr->y += ctco - 1;
		if (Tick2) { mptr->phase++; if (mptr->phase > 3) ManWalk(mptr, 0); }
		// In solid matter, shift up
		while (Inside(GBackPix(mptr->x + 4, mptr->y + 8), CSolidL, CSolidH) && !Inside(GBackPix(mptr->x + 4, mptr->y + 0), CSolidL, CSolidH)) mptr->y--;
		// OnVehic movement
		stnpix = GBackPix(mptr->x + 4, mptr->y + 9);
		if (Inside(stnpix, CVhcL, CVhcH))
		{
			stnpix -= CVhcL; stnpix >>= 1; stnpix--; mptr->x += stnpix; mptr->tx += stnpix;
		}
		break;

	case MAFLY: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		if (Tick2) { mptr->phase++; if (mptr->phase > 3) mptr->phase = 0; }
		// horizontal movement
		ctco = BoundBy(mptr->x + mptr->xdir, 0, 311);
		do
		{
			if (mptr->x < ctco) mptr->x++;
			if (mptr->x > ctco) mptr->x--;
			// hit left wall
			if (Inside(GBackPix(mptr->x + 1, mptr->y + 4), CSolidL, CSolidH))
			{
				mptr->x += 3; ctco = mptr->x; mptr->xdir = 0; mptr->ydir = Max(mptr->ydir, 20);
			}
			// hit right wall
			if (Inside(GBackPix(mptr->x + 6, mptr->y + 4), CSolidL, CSolidH))
			{
				mptr->x -= 3; ctco = mptr->x; mptr->xdir = 0; mptr->ydir = Max(mptr->ydir, 20);
			}
		} while (mptr->x != ctco);
		// vertical movement
		mptr->ydir++;
		ctco = BoundBy(mptr->y + (mptr->ydir - 15) / 10, -20, BackGrSize - 10);
		while (mptr->y != ctco)
		{
			if (mptr->y < ctco) mptr->y++;
			if (mptr->y > ctco) mptr->y--;
			// hit ceiling
			if (Inside(GBackPix(mptr->x + 3, mptr->y + 1), CSolidL, CSolidH))
			{
				ctco = mptr->y; mptr->ydir = Max(mptr->ydir, 25);
			}
			// hit water -> swim
			if (Inside(GBackPix(mptr->x + 3, mptr->y + 4), CLiqL, CLiqH))
			{
				ManSwim(mptr, 15);
				if (!random(2)) GSTP = SNDSPLASH;
				switch ((GBackPix(mptr->x + 3, mptr->y + 4) - CLiqL) / 2)
				{
				case 0: liqtype = PXSWATER; break;
				case 1: liqtype = PXSACID; break;
				case 2: liqtype = PXSOIL; break;
				case 3: case 4: case 5: liqtype = PXSLAVA; break;
				}
				BlowPXS(10, liqtype, 0, mptr->x + 3, mptr->y - 3, 5);
				for (cnt = 0; cnt < 10; cnt++) ExtractLiquid(mptr->x + 3, mptr->y + 4);
				break;
			}
			// fall downwards
			if (mptr->ydir > 10)
				if (Inside(GBackPix(mptr->x + 1, mptr->y + 10), CSolidL, CSolidH) || Inside(GBackPix(mptr->x + 6, mptr->y + 10), CSolidL, CSolidH))
				{
					ctco = mptr->y; ManWalk(mptr, mptr->del);
				}
		}
		if (mptr->type == MNMAN) ConstructLine(mptr);
		break;

	case MASWIM: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		if (!Tick5) { mptr->phase++; if (mptr->phase > 3) mptr->phase = 0; }

		// falling down shift
		if (!Inside(GBackPix(mptr->x + 4, mptr->y + 4), CSolidL, CSolid2H)) mptr->y++;
		// swim right
		if ((mptr->x < mptr->tx) && (!Inside(GBackPix(mptr->x + 6, mptr->y + 4), CSolidL, CSolidH)))
		{
			mptr->x++; mptr->xdir = +1;
		}
		// swim left
		if ((mptr->x > mptr->tx) && (!Inside(GBackPix(mptr->x + 1, mptr->y + 4), CSolidL, CSolidH)))
		{
			mptr->x--; mptr->xdir = -1;
		}
		// vertical movement
		ctco = BoundBy(mptr->y + mptr->ydir / 10, -20, BackGrSize - 10);
		while (mptr->y != ctco)
		{
			if (mptr->y > ctco)
				if (Inside(GBackPix(mptr->x + 4, mptr->y + 3), CLiqL, CLiqH)) mptr->y--;
				else { mptr->ydir = Max(mptr->ydir, 0); break; }
				if (mptr->y < ctco)
					if (Inside(GBackPix(mptr->x + 4, mptr->y + 8), CLiqL, CLiqH)) mptr->y++;
					else { mptr->ydir = Min(mptr->ydir, 0); break; }
		}

		ubpix = GBackPix(mptr->x + 4, mptr->y + 3); // Upper +4/+3 (after movement)
		lbpix = GBackPix(mptr->x + 4, mptr->y + 6); // Lower +4/+6

		// No "air" swimming
		if (!Inside(lbpix, CLiqL, CLiqH)) if (mptr->type == MNMAN) ManWalk(mptr, 0);
		// Under water -> surface
		if (Inside(ubpix, CLiqL, CLiqH))
		{
			mptr->ydir = Max(mptr->ydir - 1, -10); if (!Rnd3()) if (Inside(ubpix, CWaterS, CWaterT)) PXSOut(PXSBUBBLE, mptr->x + 4, mptr->y + 3, 0);
		}
		// Reach ground in shallow -> walk
		if (mptr->type != MNSHARK)
			if (Inside(GBackPix(mptr->x + 4 + mptr->xdir, mptr->y + 8), CSolidL, CSolidH))
				if (!Inside(ubpix, CLiqL, CLiqH))
					ManWalk(mptr, 0);
		// Can breathe?
		if (mptr->type != MNSHARK)
		{
			if (!Inside(ubpix, CSky1, CTunnel2))
			{
				mptr->del--; if (mptr->del < 1) { mptr->del = 0; if (!Tick3) mptr->strn--; }
			}
			else
			{
				if (mptr->type == MNMAN) if (mptr->act == MASWIM)
					if (mptr->del < 1) GSTP = SNDBREATH;
				mptr->del = 50;
			}
		}
		else
		{
			if (!Inside(lbpix, CWaterS, CWaterT)) { mptr->del--; if (mptr->del < 1) { mptr->del = 0; if (!Tick3) mptr->strn--; } }
			else mptr->del = 50;
		}
		// In lava, catch fire or get hurt
		if (!Tick5)
			if (Ground.volcano.age > BackGrSize + 100)
				if (Inside(lbpix, CLavaS1, CLavaT3))
				{
					if ((mptr->type != MNSHARK) && (mptr->type != MNMONSTER)) mptr->onf = 1;
					mptr->strn--;
				}
		// Acid hurts
		if (!Tick5) if (mptr->type != MNMONSTER)
			if (Inside(lbpix, CAcidS, CAcidT))
			{
				mptr->strn -= 10;
				PXSOut(PXSSMOKE, mptr->x + 4, mptr->y + 4, 20 + random(20));
			}
		// Fill barrels (Man only)
		if (!Tick10) if (mptr->type == MNMAN) if (mptr->carr == BARREL)
			for (cnt = 0; cnt < 15; cnt++)
			{
				stnpix = ExtractLiquid(mptr->x + 4, mptr->y + 5);
				if (Inside(stnpix, 0, 2)) mptr->carr = WATBARREL + stnpix;
				else break; // abort lava extraction, little lava is still extracted
			}
		// Construct Line
		if (mptr->type == MNMAN) ConstructLine(mptr);
		break;

	case MADIG: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	  // bottom -> straight/stop
		if (mptr->y > BackGrSize - 15)
		{
			if (mptr->xdir == -1) mptr->xdir = -2;
			if (mptr->xdir == 0) { ManWalk(mptr, 1); break; }
			if (mptr->xdir == +1) mptr->xdir = +2;
		}
		// very solid and water stop checking (immediate)
		if (mptr->type == MNMAN)
			if (Inside(GBackPix(mptr->x + 3, mptr->y + 3), CLiqL, CLiqH))
			{
				ManSwim(mptr, 0); break;
			}
		if (((Inside(GBackPix(mptr->x + 0, mptr->y - 1), CGranite1, CGold2) || Inside(GBackPix(mptr->x + 6, mptr->y - 1), CGranite1, CGold2)) && (Abs(mptr->xdir) > 1))
			|| Inside(GBackPix(mptr->x + 3 + 3 * Sign(mptr->xdir), mptr->y + 4), CGranite1, CGold2))
		{
			ManWalk(mptr, 1); if (mptr->type == MNMAN) GSTP = SNDMETAL;
		}
		if ((Inside(GBackPix(mptr->x + 0, mptr->y + 8), CGranite1, CGold2) || Inside(GBackPix(mptr->x + 6, mptr->y + 8), CGranite1, CGold2)) && (Abs(mptr->xdir) < 3))
		{
			ManWalk(mptr, 1); if (mptr->type == MNMAN) GSTP = SNDMETAL;
		}
		// Phase (always)
		if (Tick2) { mptr->phase++; if (mptr->phase > 3) mptr->phase = 0; }
		// Dig exec (if no delay)
		if (!mptr->del)
			if (mptr->phase == 3)
			{
				// Dig
				stnpix = DigFree(mptr->x + DigTableX[mptr->xdir + 3], mptr->y + DigTableY[mptr->xdir + 3], 5 + (mptr->xdir == 0));
				// Release LIQEARTH if wanted
				if (plr != -1) // mptr->type not checked
					if (Crew[plr].EDig)
					{
						Crew[plr].EDigMass += stnpix;
						if (Crew[plr].EDigMass >= 50)
						{
							Crew[plr].EDigMass -= 50; NewRock(mptr->x + 4 + 4 * Sign(mptr->xdir), mptr->y + 5, RADEAD, LIQEARTH, 0, 0, 0, -1);
						}
					}
				// Horizontal movement
				mptr->x += Sign(mptr->xdir);
				// Stop checking border
				if (!Inside(mptr->x, 0, 311))
				{
					mptr->x = BoundBy(mptr->x, 0, 311); ManWalk(mptr, 1); break;
				}

				if ((Abs(mptr->xdir) < 3) || !(mptr->x % 2))
					mptr->y += Min(2 - Abs(mptr->xdir), 1);
				// Free air stop checkings
				if (!Inside(GBackPix(mptr->x + 3 + Sign(mptr->xdir), mptr->y + 10), CSolidL, CSolidH)
					&& !Inside(GBackPix(mptr->x + 3, mptr->y + 10), CSolidL, CSolidH))
					ManWalk(mptr, 1);
			}
		// line construction
		if (mptr->type == MNMAN) ConstructLine(mptr);
		// dig-begin count down
		if (mptr->del > 0) mptr->del--;
		break;

	case MABRIDGE: // - (Man only) - - - - - - - - - - - - - - - - - - - - - -
	  // Falling check
		ctco = 0; while (!Inside(GBackPix(mptr->x + 4, mptr->y + 8 + ctco), CSolidL, CSolidH)) ctco++;
		if (ctco > 4) { ManFly(mptr, 0, 30, 0); break; }
		else mptr->y += ctco - 1;
		// Exec bridge
		if (Tick2) mptr->phase++;
		if ((mptr->phase == 3) || ((mptr->del == 1) && (mptr->phase == 1)))
		{
			mptr->phase = 0;
			ctco = mptr->x + 4;
			if (Abs(mptr->xdir) == 2) ctco += Sign(mptr->xdir);
			if (mptr->xdir == -1) ctco -= 4; if (mptr->xdir == +1) ctco += 2;
			ctco += (mptr->xdir - Sign(mptr->xdir))*mptr->ydir;
			if (mptr->del) DrawSteelChunk(ctco, mptr->y + 9 - (3 - Abs(mptr->xdir))*mptr->ydir);
			else DrawLoamChunk(ctco, mptr->y + 9 - (3 - Abs(mptr->xdir))*mptr->ydir);
			mptr->ydir++; if (mptr->ydir > 6 + 3 * (Abs(mptr->xdir) == 2)) ManWalk(mptr, 1);
		}
		// Shift up on vehic
		if (Inside(GBackPix(mptr->x + 4, mptr->y + 8), CVhcL, CVhcH)) mptr->y--;
		// In water, swim
		if (Inside(GBackPix(mptr->x + 4, mptr->y + 3), CLiqL, CLiqH))
		{
			ManSwim(mptr, 0); break;
		}
		// OnVehic movement
		stnpix = GBackPix(mptr->x + 4, mptr->y + 9);
		if (Inside(stnpix, CVhcL, CVhcH))
		{
			stnpix -= CVhcL; stnpix >>= 1; stnpix--; mptr->x += stnpix; mptr->tx += stnpix;
		}
		break;

	case MABUILD: // - (Man only) - - - - - - - - - - - - - - - - - - - - - -
		if (mptr->tptr)
		{
			if (!Tick3) { mptr->phase++; if (mptr->phase > 2) mptr->phase = 0; }
			if (!Tick20) mptr->mi->exp++; // Experience gain

			tstrc = (STRUCTYPE*)mptr->tptr;
			if (tstrc->con < 1000)
			{
				if (!Tick3) { tstrc->con++; if (tstrc->con >= 1000) StructCompletion(tstrc, 1, plr); }
				if (!random(10)) // Man shift on structure
				{
					mptr->x = BoundBy(mptr->x + random(5) - 2, tstrc->x, tstrc->x + StructWdt[tstrc->type] - 8);
					ctco = tstrc->y + (StructHgt[tstrc->type] * (1000 - tstrc->con) / 1000);
					mptr->y = BoundBy(mptr->y + random(5) - 2, ctco - 7, tstrc->y + StructHgt[tstrc->type] - 9);
					mptr->xdir = random(3) - 1;
				}
				if (!Inside(mptr->x, tstrc->x, tstrc->x + StructWdt[tstrc->type] - 8)
					|| !Inside(mptr->y, tstrc->y - 7, tstrc->y + StructHgt[tstrc->type] - 6))
				{
					ManWalk(mptr, 1); LoseTPtr(mptr);
				} // Man carried away from struct
				if (tstrc->onf) mptr->onf = 1;
				if (!Tick10) if (!random(10)) GSTP = SNDCONSTR1 + random(2);
			}
			else // Struct done -> stop
			{
				ManWalk(mptr, 1); LoseTPtr(mptr);
			}
		}
		else // Stop if TStruct has been erased
		{
			LoseTPtr(mptr); ManWalk(mptr, 1);
		}
		// Shift up on vehic
		if (Inside(GBackPix(mptr->x + 4, mptr->y + 8), CVhcL, CVhcH)) mptr->y--;
		// In water, swim
		if (Inside(GBackPix(mptr->x + 4, mptr->y + 3), CLiqL, CLiqH))
		{
			LoseTPtr(mptr); ManSwim(mptr, 0); break;
		}
		// OnVehic movement
		stnpix = GBackPix(mptr->x + 4, mptr->y + 9);
		if (Inside(stnpix, CVhcL, CVhcH))
		{
			stnpix -= CVhcL; stnpix >>= 1; stnpix--; mptr->x += stnpix; mptr->tx += stnpix;
		}
		break;

	case MAPUSH: // - (Man only) - - - - - - - - - - - - - - - - - - - - - - -
	  // Missing vehic
		if (!mptr->tptr) { ManWalk(mptr, 1); break; }
		// Pushing target movement
		tvhc = (VEHICTYPE*)mptr->tptr;
		if (!StandardManMovement(mptr, 2, plr)) { if (mptr->act != MAHOME) mptr->tptr = NULL; break; }
		SetManTX2Push(mptr, tvhc);
		// Pushing
		if (Inside(mptr->x - tvhc->x, -8, 15) && Inside(mptr->y - tvhc->y, -9, +9))
		{ // On Vehic
			switch (tvhc->type) // stnpix is applied pushing force
			{
			case VHLORRY:    stnpix = 5; break;
			case VHCATAPULT: stnpix = 6; break;
			case VHCROSSBOW: stnpix = 6; break;
			case VHSAILBOAT: stnpix = 5; break;
			}
			switch (mptr->del)
			{
			case -2: tvhc->p -= stnpix; break;
			case -1: case +1:
				tvhc->p -= stnpix*Sign(tvhc->p);
				if (Inside(tvhc->p, -7, +7)) tvhc->p = 0; // Stand still
				break;
			case +2: tvhc->p += stnpix; break;
			}
		}
		else // Lost target
		{
			ManWalk(mptr, 1); mptr->tptr = NULL; break;
		}
		break;

	case MADEAD: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		mptr->phase = 0; mptr->onf = 0; mptr->strn = 0; mptr->ydir = 0; mptr->tptr = NULL;
		mptr->con = 0;
		// Body decay
		mptr->del++;
		// Shift up on vehic
		if (Inside(GBackPix(mptr->x + 4, mptr->y + 8), CVhcL, CVhcH)) mptr->y--;
		// Shift down
		if (!Inside(GBackPix(mptr->x + 4, mptr->y + 9), CSolidL, CSolidH) && (mptr->y < BackGrSize - 10)) mptr->y++;
		break;

	case MAHOME: // - (Man only) - - - - - - - - - - - - - - - - - - - - - - -
		if (mptr->tptr)
		{
			// Home at base
			mptr->onf = 0;
			if (BSA.Realism.CstHome)
			{
				tstrc = (STRUCTYPE*)mptr->tptr; homeat = tstrc->owner;
			}
			else
			{
				tvhc = (VEHICTYPE*)mptr->tptr; homeat = tvhc->owner;
			}
			// Energy recharge
			if (!Tick2)
				if (mptr->strn < 100 + 5 * mptr->mi->rank)
					if (BSA.RuleSet == 0) // EASY recharge
						mptr->strn++;
					else // ADVANCED/RADICAL recharge
						if (Crew[homeat].StrBank > 0) // Charge from bank
						{
							mptr->strn++; Crew[homeat].StrBank--;
						}
						else // Try bank refresh
							if (Crew[homeat].Wealth > 0) { Crew[homeat].Wealth--; Crew[homeat].StrBank += 20; }
			// Re-Entry
			if (BSA.Realism.CstHome) // Castle
			{
				if ((mptr->strn >= 100 + 5 * mptr->mi->rank) || ((Crew[homeat].StrBank == 0) && (Crew[homeat].Wealth == 0) && (BSA.RuleSet != 0)))
				{
					if (tstrc->p == 0) tstrc->p = 1; // Activate castle-gate
					if (Inside(tstrc->p, 3, 15))
					{ // Exit castle
						mptr->y = tstrc->y + 3;
						if (random(2)) { mptr->x = tstrc->x + 6 - 1; mptr->tx = tstrc->x + 6 - 2 - random(5); }
						else { mptr->x = tstrc->x + 6 + 1; mptr->tx = tstrc->x + 6 + 2 + random(5); }
						ManWalk(mptr, 0); mptr->tptr = NULL;
					}
				}
			}
			else // Balloon
			{
				if ((tvhc->p == 0) && (tvhc->y == -16))
				{
					mptr->x = tvhc->x + 2 + random(5); mptr->y = tvhc->y + 10;
					ManWalk(mptr, 1); mptr->tptr = NULL;
					mptr->phase = Rnd4();
				}
			}
		}
		break;
	}

	// Safety absolute
	mptr->x = BoundBy(mptr->x, 0, 311);
	if (!Tick50) if (mptr->mi) mptr->mi->exp = Min(mptr->mi->exp, 30000);
}

//-------------------------- (Clonk Crew Control) ------------------------------

void StBarRedrawCheck(BYTE plr)
{
	// Update on change of  rank,carr,strn
	//			  Cursor,ManCnt,Wealth
	static MANTYPE *lcursor[3];
	static int lmcnt[3], lccnt[3], lwealth[3], scgain[3];
	static int lrank[3], lcarr[3], lstrn[3], lcon[3];
	static BYTE lmenu[3], lmenud[3];

	if (lcursor[plr] != Crew[plr].Cursor) { Crew[plr].RedrStB = 1; lcursor[plr] = Crew[plr].Cursor; }
	else if (Crew[plr].Cursor)
	{ // Personals only if Cursor is active and stayed the same
		if (lrank[plr] != Crew[plr].Cursor->mi->rank) { Crew[plr].RedrStB = 1; lrank[plr] = Crew[plr].Cursor->mi->rank; }
		if (lcarr[plr] != Crew[plr].Cursor->carr) { Crew[plr].RedrStB = 1; lcarr[plr] = Crew[plr].Cursor->carr; }
		if (lstrn[plr] != Crew[plr].Cursor->strn) { Crew[plr].RedrStB = 1; lstrn[plr] = Crew[plr].Cursor->strn; }
		if (lcon[plr] != Crew[plr].Cursor->con) { Crew[plr].RedrStB = 1; lcon[plr] = Crew[plr].Cursor->con; }
	}
	if (scgain[plr] != BSA.Plr[plr].ScoreGain) { Crew[plr].RedrStB = 1; scgain[plr] = BSA.Plr[plr].ScoreGain; }
	if (lmcnt[plr] != Crew[plr].ManCnt) { Crew[plr].RedrStB = 1; lmcnt[plr] = Crew[plr].ManCnt; }
	if (lccnt[plr] != Crew[plr].ConCnt) { Crew[plr].RedrStB = 1; lccnt[plr] = Crew[plr].ConCnt; }
	if (lwealth[plr] != Crew[plr].Wealth) { Crew[plr].RedrStB = 1; lwealth[plr] = Crew[plr].Wealth; }
	if (lmenu[plr] != Crew[plr].CMenu) { Crew[plr].RedrStB = 1; lmenu[plr] = Crew[plr].CMenu; }
	if (lmenud[plr] != Crew[plr].CMenuData) { Crew[plr].RedrStB = 1; lmenud[plr] = Crew[plr].CMenuData; }

	// Redr toggled at point of change for:
	//
	// DontPickUp-Mark, RockStorage on CMRCKORDER, lining-carr-flashback,
	// Arrow-CarrP-Change, Player Elimination
}

void RockProduction(BYTE plr) // Every Sec5
{
	int cnt;
	for (cnt = 0; cnt < RockOrderNum; cnt++)
	{
		Crew[plr].RckProd.PrDel[cnt] += BSA.RckProdSpeed*BSA.RckProdSpeed;
		if (Crew[plr].RckProd.PrDel[cnt] >= BSA.RckProdTime[cnt])
		{
			Crew[plr].RckProd.PrDel[cnt] = 0;
			if (Crew[plr].RckProd.Store[cnt] < 15) Crew[plr].RckProd.Store[cnt]++;
			Crew[plr].RedrStB = 1;
		}
	}
}

extern void PlayerVhcOrder(BYTE plr, int oid, BYTE autoc);

void AutoClonkProduction(BYTE plr)
{
	if (Crew[plr].Wealth >= VhcOrderPrice[0]) PlayerVhcOrder(plr, 0, 1);
}

void MoveCrew(BYTE plr)
{
	MANTYPE *mptr;
	int ccon;

	// Death removal check
	if (Sec5) ManCnt[plr] = DeathRemovalCheck(&Crew[plr].FirstMan);

	// Man movement execution
	if (Tick3)
	{
		for (mptr = Crew[plr].FirstMan; mptr; mptr = mptr->next)
			MoveMan(mptr, plr);
	}
	else // New Ave&Cnt values every third tick
	{
		Crew[plr].AveX = Crew[plr].AveY = -1; Crew[plr].ManCnt = Crew[plr].ConCnt = 0; ccon = 0;
		for (mptr = Crew[plr].FirstMan; mptr; mptr = mptr->next)
		{
			MoveMan(mptr, plr);
			if (mptr->act != MADEAD) Crew[plr].ManCnt++;
			if (mptr->act < MADEAD)
				if (mptr->con) // Controlled count for ave's
				{
					Crew[plr].ConCnt++; Crew[plr].AveX += mptr->x; Crew[plr].AveY += mptr->y; ccon++;
				}
		}
		if (ccon) { Crew[plr].AveX /= ccon; Crew[plr].AveY /= ccon; }
	}

	if (Sec5)
	{
		RockProduction(plr);
		if (BSA.RuleSet == 0) AutoClonkProduction(plr);
	}

	StBarRedrawCheck(plr);

	// DontPushDelay
	if (Crew[plr].DontPushDelay > 10) Crew[plr].DontPushDelay--;
	// SelectAllCount
	if (Crew[plr].SelectAllCount > 0) Crew[plr].SelectAllCount--;
	// Drop/DontPickUp
	if (Crew[plr].DropCount > 0) Crew[plr].DropCount--;
	// Con/Cursor Flash
	if (Crew[plr].ConFlash > 0) Crew[plr].ConFlash--;
	if (Crew[plr].CursorFlash > 0) Crew[plr].CursorFlash--;
	// MsWalkDClick Count
	if (Crew[plr].MsWalkDClick > 0) Crew[plr].MsWalkDClick--;
}

//---------------------------- (Animal Control) ------------------------------

void MoveShark(MANTYPE *shk)
{
	MANTYPE *tman;
	int lastx = shk->x, lasttx = shk->tx;
	MoveMan(shk, -1);
	if (shk->tptr) MoveMan(shk, -1); // 2x speed while hunting
	if (shk->act != MADEAD)
	{
		shk->x = BoundBy(shk->x, 12, 305); // Necessary safety
		if (shk->tptr) // Hunting...
		{
			shk->x = BoundBy(shk->x, 12, 305); // Necessary safety
			tman = (MANTYPE*)shk->tptr;
			if (tman->act != MASWIM) // Target lost
				shk->tptr = NULL;
			else
			{
				if (!Rnd3()) shk->tx = tman->x + 4 - 8 * Sign(shk->xdir);
				if (tman->y < shk->y) shk->ydir -= 4;
				if (tman->y > shk->y) shk->ydir += 4;
				shk->ydir = BoundBy(shk->ydir, -30, +30);
				// Hit Checking
				if (Inside(shk->x - (tman->x + 4 - 8 * Sign(shk->xdir)), -3, +3) && Inside(shk->y - tman->y, -3, +3))
				{
					tman->strn -= 2; // hurt sound or chap-chap sound
					shk->ydir = 5 + Tick2;
				}
				else // Leave target?
					if ((shk->x == lastx) && (shk->x != lasttx))
					{
						shk->tptr = NULL;
						//SystemMsg("shark leaving target");
					}
			}
		}
		else // Not hunting
		{
			if ((shk->tx == shk->x) || (shk->x == lastx))
				shk->tx = BoundBy(10 + random(300), 10, 309);
			if (!Tick5) { if (Rnd3()) shk->ydir = Min(shk->ydir + 5, 10); if (!Tick20) if (!random(10)) shk->ydir = 10 + random(20); }
		}
	}
}

long Distance(long x1, long y1, long x2, long y2)
{
	return sqrt(Abs(x1 - x2)*Abs(x1 - x2) + Abs(y1 - y2)*Abs(y1 - y2));
}

ROCKTYPE *FindFood(int fx, int fy)
{
	long attr, ndist, cldist;
	ROCKTYPE *crck, *closest = NULL;
	for (crck = FirstRock; crck; crck = crck->next)
		if (Inside(crck->type, PLANT1, PLANT3))
			if (Inside(crck->x - fx, -200, +200))
				if (Inside(crck->y - fy, -30, +70) || (Inside(crck->y - fy, -55, -30) && !Inside(crck->x - fx, -40, +40)))
				{ // Free ones are more attractive
					attr = 1; if (!Inside(GBackPix(crck->x + 2, crck->y - 1), CSolidL, CSolid2H)) attr += 5;
					ndist = Distance(crck->x, crck->y, fx, fy) / attr;
					if (!closest || (ndist < cldist)) { closest = crck; cldist = ndist; }
				}
	return closest;
}

BYTE TarUpJumpChance(MANTYPE *wpf, int hgt)
{
	int cx = BoundBy(wpf->x - 4 + 16 * (wpf->xdir > 0), 0, 319);
	if (!Inside(GBackPix(cx, wpf->y + hgt + 0), CSolidL, CSolidH))
		if (Inside(GBackPix(cx, wpf->y + hgt + 1), CSolidL, CSolidH)
			|| Inside(GBackPix(cx, wpf->y + hgt + 2), CSolidL, CSolidH)
			|| Inside(GBackPix(cx, wpf->y + hgt + 3), CSolidL, CSolidH))
			if (!Inside(GBackPix(cx, wpf->y + hgt + 4), CSolidL, CSolidH))
				return 1;
	return 0;
}

void MoveWipf(MANTYPE *wpf)
{
	int lastx = wpf->x;
	ROCKTYPE *trock;
	VEHICTYPE *tvhc;
	if (Sec5) if (wpf->act != MADEAD) // Hunger
	{
		wpf->carr--;
		if (wpf->carr <= 0) { wpf->carr = 0; wpf->strn -= 3; }
	}
	// Slow down if weak
	if (!((wpf->act == MAFLY) || (wpf->act >= MADEAD) || (wpf->strn > 50) || !Tick3))
		return;
	// Wipf movement
	MoveMan(wpf, -1);
	if (!wpf->tptr) // Without target ---------
	{
		switch (wpf->act)
		{
		case MAWALK: case MASWIM: // - - - - - - - - - - - - - - - - - - - - - - -
			if (wpf->x == wpf->tx) // Standing
			{
				wpf->xdir = 0;
				if (wpf->act == MAWALK) // WalkStand
				{                    // Phase not advanced by StdManMvm if standing
					if (!Tick5) { wpf->phase++; if (wpf->phase > 3) wpf->phase = 0; }
					if (!Tick10) if (!random(10)) wpf->tx = random(312);
					if (!Tick20) if (!random(10)) GSTP = SNDWIPF;
				}
				else // SwimStand
					if (!Tick10) wpf->tx = random(312);
			}
			else // Moving
			{
				if (wpf->x == lastx) // Stuck? Turn around or dig.
				{
					if (!random(100)) ManDig(wpf, random(2) + 2);
					else wpf->tx = random(312);
				}
				else // Not stuck, jump maybe
				{
					if (!Tick5) if (wpf->act == MAWALK)
						if (!random(30)) ManFly(wpf, Sign(wpf->xdir), -7 + 3 * (wpf->strn < 30), 0);
					// Targeted Upward Jump
					if (!Tick2) //if (random(3))
						if (TarUpJumpChance(wpf, -3)) // only if !low fps?
							ManFly(wpf, Sign(wpf->xdir), -7, 0);
				}
			}
			// Check for hunger
			if (!Tick50)
				if (wpf->carr < 40) 	                // carr is food
					wpf->tptr = FindFood(wpf->x, wpf->y);
			break;
		case MADIG: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			if (!Tick5) if (!random(5)) // Change dig dir
				wpf->xdir = (random(3) + 1)*Sign(wpf->xdir);
			if (!Tick10) if (!random(10)) ManWalk(wpf, random(2)); // Stop digging
			break;
		}
	}
	else // With target (food) ---------
	{
		trock = (ROCKTYPE *)wpf->tptr;
		switch (wpf->act)
		{
		case MAWALK: case MASWIM: // - - - - - - - - - - - - - - - - - - - - - - -
			if (wpf->x == wpf->tx) // Standing
			{
				wpf->xdir = 0;
				if (Inside(wpf->x + 2 - trock->x, -5, +5) && Inside(wpf->y + 4 - (trock->y + 2), -6, +7))
				{ // Got it!                  // Eating
					wpf->phase++; if (wpf->phase > 3) wpf->phase = 0;
					GSTP = SNDMUNCH;
					wpf->carr++; if (wpf->carr > 250) wpf->tptr = NULL; // full, leave it
					if (!Tick5) wpf->strn = Min(wpf->strn + 1, 100); // energy re-gen
					if (!Tick10)
					{
						trock->phase--; // eat plant         vvv out to own func?
						trock->type = BoundBy(PLANT1 + trock->phase / 10, PLANT1, PLANT3);
						if (trock->phase <= 0) trock->type = NOROCK;
					}
				}
				else // Got it not, standing free.
				{
					if (wpf->act == MAWALK) if (!Tick5) { wpf->phase++; if (wpf->phase > 3) wpf->phase = 0; }
					if (!Tick20)
						if (wpf->x == trock->x) wpf->tx = random(312);
						else wpf->tx = trock->x;
				}
			}
			else // Moving
			{
				// Stuck? Turn around or dig (if facing right way or close).
				if (wpf->x == lastx)
				{
					if (random(4) && ((wpf->xdir == Sign(trock->x - 2 - wpf->x)) || Inside(wpf->x + 1 - trock->x, -50, +50)))
						ManDig(wpf, (random(5)) ? 2 + Sign(wpf->y - trock->y) : 1 + random(3));
					else
						wpf->tx = random(312);
				}
				// Not stuck, special get-to's Jump,Dig,or dive
				else
				{
					if (!Tick5) if (!random(5))
					{
						if (wpf->act == MAWALK)
							if (trock->y < wpf->y) // Jump to target
								ManFly(wpf, Sign(wpf->xdir), -7 + 3 * (wpf->strn < 30), 0);
							else // Dig to target if relatively deep
								if (Abs(trock->x - wpf->x) < (trock->y - wpf->y))
									ManDig(wpf, 1);
						if (wpf->act == MASWIM)
							if (trock->y > wpf->y + 2)
								wpf->ydir = 15; // Dive if target is below
					}
					// And dir adjust
					if (!Tick50) wpf->tx = trock->x;
					// Targeted Upward Jump
					if (!Tick2) //if (random(3))
						if (TarUpJumpChance(wpf, -3)) // only if !low fps?
							ManFly(wpf, Sign(wpf->xdir), -7, 0);
				}
			}
			break;
		case MADIG: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	  // Adjust vertical dir to target
			if (!Tick20) wpf->xdir = Sign(wpf->xdir)*(2 + Sign(wpf->y + 4 - trock->y));
			// Adjust horizontal dir (less often for zik zak)
			if (!Tick50) //if (wpf->x%20==0)
				wpf->xdir = Abs(wpf->xdir)*Sign(trock->x - wpf->x);
			// Random stop if under water
			if (!Tick5) if (!random(2))
				if (Inside(GBackPix(wpf->x + 4, wpf->y + 3), CLiqL, CLiqH))
					ManSwim(wpf, 0);
			// Stop if there
			if (Inside(wpf->x + 2 - trock->x, -5, +5) && Inside(wpf->y + 4 - (trock->y + 2), -6, +7)) ManWalk(wpf, 1);
			break;
		}
		if (Sec20) if (!(Inside(wpf->x + 2 - trock->x, -5, +5) && Inside(wpf->y + 4 - (trock->y + 2), -6, +6)))
			wpf->tptr = NULL; // leave target if not at it
	}

	// Wipf2Lorry Checking
	if (!Tick5) if ((wpf->act == MAWALK) || (wpf->act == MAFLY) || (wpf->act == MADIG))
		for (tvhc = FirstVehic; tvhc; tvhc = tvhc->next)
			if (tvhc->type == VHLORRY)
				if (Inside(wpf->x + 4 - tvhc->x, 1, 10))
					if (Inside(wpf->y + 4 - tvhc->y, 0, 8))
						if (tvhc->VHDWipfLoad < 200)
						{
							tvhc->VHDWipfLoad++;
							wpf->type = MNNOMAN;
							ClearMenTPtr(wpf);
							GSTP = SNDYIPPIE;
						}
}

void MoveMonster(MANTYPE *mns)
{
	int lastx = mns->x;
	MoveMan(mns, -1);
	mns->x = BoundBy(mns->x, 6, 305);
	mns->tx = BoundBy(mns->tx, 6, 305);
	if (mns->x == lastx) // Stuck
		mns->tx = 6 + random(306);
	if (!Tick20) // Jump
		if (mns->act == MAWALK)
			if (!random(30))
			{
				ManFly(mns, Sign(mns->xdir), -7, 0);
				GSTP = SNDMONSTER;
			}
}

void MoveAnimals(void) // Every Tick1
{
	MANTYPE *canm;
	if (Sec5) AnimalCnt = DeathRemovalCheck(&FirstAnimal);
	for (canm = FirstAnimal; canm; canm = canm->next)
	{
		switch (canm->type)
		{
		case MNSHARK:   MoveShark(canm);   break;
		case MNWIPF:    MoveWipf(canm);    break;
		case MNMONSTER: MoveMonster(canm); break;
		}
	}

	// Needs a Tick1...
	if (DADelay > 0) DADelay--;
}

BYTE InitSharks(int num)
{
	int tx, ty;
	while (num > 0)
	{
		if (!FindSurface(0, random(320), 35, &tx, &ty)) return 0;
		if (!NewAnimal(&FirstAnimal, MNSHARK, tx, ty - 3, tx, MASWIM, 0, 100, 0, 0, NOROCK, NULL)) return 0;
		num--;
	}
	return 1;
}

BYTE InitWipfs(int num)
{
	int cnt, tx, ty;
	BYTE below;
	while (num > 0)
	{
		below = (!random(3));
		if ((BSA.SMode == S_COOPERATE) && (BSA.CoopGMode == C_WIPFRESCUE)) below = random(3);
		if (BSA.SMode == S_MISSION) below = random(3);
		if (below)
		{
			for (cnt = 0; cnt < 5; cnt++) // 5 tries if necessary
			{
				tx = random(300) + 10;
				ty = 0; while (!Inside(GBackPix(tx, ty), CGranite1, CAshes2)) ty++;
				ty += random(BackGrSize - ty - 50) + 30;
				if (Inside(GBackPix(tx, ty), CEarth1, CAshes2)) break;
			}
			if (cnt < 5)
			{
				for (cnt = 0; cnt < 8; cnt++) DigFree(tx - 4 + cnt, ty + 3, 4);
				if (!NewAnimal(&FirstAnimal, MNWIPF, tx - 4, ty, tx, MAWALK, 0, 100, 0, 0, 40 + random(20), NULL)) return 0;
			}
		}
		else // Above
		{
			tx = random(300) + 10;
			ty = 0; while (!Inside(GBackPix(tx, ty), CSolidL, CSolid2H)) ty++; ty -= 6;
			if (!NewAnimal(&FirstAnimal, MNWIPF, tx - 4, ty, tx, MAWALK, 0, 100, 0, 0, 40 + random(20), NULL)) return 0;
		}
		num--;
	}
	return 1;
}

BYTE InitMonsters(int num)
{
	int cx, cy, cp;
	bool isMonsterHunt = (BSA.CoopGMode == C_MONSTERKILL) || (BSA.SMode == S_MISSION);
	while (num > 0)
	{
		cx = random(300) + 10; cy = 0;
		while (!Inside(GBackPix(cx, cy), CSolid2L, CSolidH)) cy++;
		if (!isMonsterHunt) if (!random(3)) cy += random(BackGrSize - 10 - cy);
		cp = random(10); if (isMonsterHunt || !random(3)) cp = 10 - (random(5)*(num > 1));
		if (!NewRock(cx, cy - 3, RADEAD, MONSTEGG, 0, 0, cp, -1)) return 0;
		//if (!NewAnimal(&FirstAnimal,MNMONSTER,6+random(306),-20,6+random(306),MAWALK,0,60+20*BSA.RuleSet,0,0,-1,NULL)) return 0;
		num--;
	}
	return 1;
}

BYTE InitAnimals(void)
{
	BYTE succ = 1;
	if (!InitSharks(BSA.Sharks)) succ = 0;
	if (!InitWipfs(BSA.Wipfs)) succ = 0;
	if (!InitMonsters(BSA.Monsters)) succ = 0;
	return succ;
}

//-------------------------- Round Object Setup ------------------------------

int CrewBaseXOverride = -1;

void InitCrewData(BYTE plr, BYTE hostile)
{
	int cnt;
	Crew[plr].FirstMan = NULL; Crew[plr].Cursor = NULL;
	Crew[plr].AveX = Crew[plr].AveY = -1;
	Crew[plr].ManCnt = 0;
	Crew[plr].TogMode = 0;

	Crew[plr].HiRankCap = NULL;

	Crew[plr].DontPushDelay = 10;
	Crew[plr].SelectAllCount = 0;
	Crew[plr].DropCount = 0;
	Crew[plr].DontPickUp = 0;
	Crew[plr].EDig = 0; Crew[plr].EDigMass = 0;
	Crew[plr].MsWalkDClick = 0;

	Crew[plr].RedrStB = 1;
	Crew[plr].CMenu = Crew[plr].CMenuData = 0;
	Crew[plr].ConFlash = CFlashTime; Crew[plr].CursorFlash = CFlashTime;
	Crew[plr].StBX = 108 * plr; Crew[plr].StBWdt = 108 - 4 * (plr == MaxGamePlr - 1);

	Crew[plr].Wealth = BSA.Plr[plr].Wealth;
	Crew[plr].StrBank = 0;

	for (cnt = 0; cnt < RockTypeNum; cnt++) Crew[plr].RockStorage[cnt] = 0;
	for (cnt = 0; cnt < MaxGamePlr; cnt++) Crew[plr].Hostile[cnt] = hostile;
	Crew[plr].Hostile[plr] = 0;
	Crew[plr].BaseX = BSA.Plr[plr].GPos*(320 / (MaxGamePlr - 1));
	if (CrewBaseXOverride > -1) Crew[plr].BaseX = CrewBaseXOverride; // MISSION

	for (cnt = 0; cnt < RockOrderNum; cnt++)
	{
		Crew[plr].RckProd.Store[cnt] = BSA.RckProdStart[cnt];
		Crew[plr].RckProd.PrDel[cnt] = 0;
	}

	Crew[plr].CaFlX = Crew[plr].CaFlY = -1;
}

//------------------------ Promotion / Experience ----------------------------

extern char *RankName[11];
extern void CursorAdjust(BYTE plr);

extern int ClonkRankExp(int crnk);

void PromoteClonk(MANTYPE *cman, BYTE plr, BYTE regular)
{
	cman->mi->rank++;
	sprintf_s(OSTR, "%s promoted|to %s!|%s", cman->mi->name, RankName[cman->mi->rank], regular ? "" : "|(Exception)");
	GameMessage(OSTR, 0, 0, CYellow, cman);
	GSTP = SNDTRUMPET;
	if (!Crew[plr].Cursor) { cman->con = 1; CursorAdjust(plr); }
	if (!((BSA.SMode == S_MELEE) && (BSA.GPlrElm == 0) && Crew[plr].HiRankCap->dead))
		Crew[plr].HiRankCap = HighRankManInfo(plr);
}

void EmergencyPromotionCheck(void)
{
	int plr, hirank;
	MANTYPE *cman, *hiexp;
	for (plr = 0; plr < 3; plr++)
	{
		hirank = -1; hiexp = NULL;
		for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
			if (cman->act != MADEAD)
			{
				if (cman->mi->rank > hirank) hirank = cman->mi->rank;
				if (!hiexp || (cman->mi->exp > hiexp->mi->exp)) hiexp = cman;
			}
		if (hirank == 0) // Only recruit clonks out there, jump hiexp to "F„hnrich"
			PromoteClonk(hiexp, plr, 0);
	}
}

void PromotionCheck(void) // Every Sec5
{
	int plr;
	MANTYPE *cman;

	if (BSA.Realism.EmrPromo) EmergencyPromotionCheck();

	// Regular Promotions
	for (plr = 0; plr < 3; plr++)
		for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
			if (cman->mi->exp >= ClonkRankExp(cman->mi->rank + 1)) if (cman->mi->rank < 10)
				if (cman->act < MADEAD) // Only if out and alive
					PromoteClonk(cman, plr, 1);
}
