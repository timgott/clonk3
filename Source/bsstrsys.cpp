/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design CLONK Battle Sequence Structure/Line Control System Module

#include <stdlib.h>
#include <stdio.h>

#include "standard.h"

#include "clonk_bs.h"

#include "bsexcsys.h"
#include "bsmansys.h"
#include "bsgfxsys.h"
#include "bspxssys.h"
#include "bsvhcsys.h"
#include "bsweasys.h"

#include "RandomWrapper.h"

//-------------------------- Main Global Externals -----------------------------

extern BSATYPE BSA;

extern char OSTR[500];

extern void EventCall(int evnum);

//-------------------- STR Global Game Object Variables ------------------------

STRUCTYPE *FirstStruct;
LINETYPE *FirstLine;

BYTE StructBurn;

//---------------------- Local constants & definitions -----------------------

const int OilPLiqStSize = 50;

// For castle flag
#define CstFlag energy

//-------------------------- (Structure Control) -------------------------------

//-------------------- Structure CrossChecking/Reaction -----------------------

void IncinerateStruct(STRUCTYPE *cstrc)
{
	if (!cstrc->onf)
	{
		cstrc->onf = 1; cstrc->liqstr = 0; GSTP = SNDFIRE;
	}
}


void CheckStructIgnition(int tx, int ty)
{
	STRUCTYPE *cstrc;
	for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)
		if (cstrc->type > STNOSTRUCT)
			if ((cstrc->type != STCASTLE) && (cstrc->type != STOILPOWER) && (cstrc->type != STTOWER) && (cstrc->type != STMAGIC))
				if (BSA.Realism.StrcBurn || (cstrc->type >= STCACTUS))
					if (Inside(tx - cstrc->x, 0, StructWdt[cstrc->type]))
						if (Inside(ty - cstrc->y, -5, StructHgt[cstrc->type] + 5))
							IncinerateStruct(cstrc);
}

void StructHitDamage(int tx, int ty, int dmg, int csdby)
{
	STRUCTYPE *cstrc;
	for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)
		if (cstrc->type > STNOSTRUCT)
			if (Inside(tx - cstrc->x, 0, StructWdt[cstrc->type]))
				if (Inside(ty - cstrc->y, -5, StructHgt[cstrc->type] + 5))
				{
					cstrc->damage = Min(cstrc->damage + dmg, 100);
					// If damage can be increased by anything other than
					// StructHitDamage(), move the following check out to ExecStructs()
					if (cstrc->damage > StructMaxDam[cstrc->type])
						if (BSA.Realism.StrcBurn || (cstrc->type >= STCACTUS))
						{
							// Burning castle lose flag // Castle incineration caused by
							if (cstrc->type == STCASTLE)  // anything other than StrHitDam()?
								if (BSA.GPlrElm == 2)
									if (cstrc->con >= 1000) if (cstrc->CstFlag)
									{
										NewRock(cstrc->x + 4, cstrc->y - 20, RADEAD, FLAG, 0, 0, cstrc->owner, -1);
										Crew[cstrc->owner].CaFlX = Crew[cstrc->owner].CaFlY = -1;
									}
							// Regulars...
							IncinerateStruct(cstrc);
							cstrc->damage = 0;
						}

					if (Inside(csdby, 0, 2) && (cstrc->owner > -1))
						if (!NotHostile(cstrc->owner, csdby))
							ScoreGain(csdby, dmg);
				}
}

BYTE StructIsConnected(STRUCTYPE *strc, int linetype)
{
	LINETYPE *cline = FirstLine;
	while (cline)
	{
		if (cline->type == linetype)
			if ((cline->fstrc == strc) || (cline->tstrc == strc))
				return 1;
		cline = cline->next;
	}
	return 0;
}

STRUCTYPE *OnWhichStruct(int tx, int ty) // Returns buildings only
{
	STRUCTYPE *cstrc;
	for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)
		if (cstrc->type != STNOSTRUCT) if (cstrc->type < STCACTUS)
			if (Inside(tx - cstrc->x, 0, StructWdt[cstrc->type]))
				if (Inside(ty - cstrc->y, 0, StructHgt[cstrc->type]))
					return cstrc;
	return NULL;
}

BYTE Water2BurningStruct(int tx, int ty)
{
	STRUCTYPE *cstrc;
	for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)
		if (cstrc->type != STNOSTRUCT) if (cstrc->onf)
			if (Inside(tx - cstrc->x, 0, StructWdt[cstrc->type]))
				if (Inside(ty - cstrc->y, 0, StructHgt[cstrc->type]))
				{
					cstrc->liqstr++;
					return 1;
				}
	return 0;
}

//------------------------- Structure GFX Functions --------------------------

void DrawStructBase(STRUCTYPE *strc, BYTE news)
{
	int cnt1, cnt2;
	if (strc->type >= STCACTUS) return;
	for (cnt1 = 0; cnt1 < StructWdt[strc->type]; cnt1++)
		if ((strc->type != STELEVATOR) || !Inside(cnt1, 2, 13))
			for (cnt2 = 0; cnt2 < 4; cnt2++)
				if (news) SBackPix(strc->x + cnt1, strc->y + StructHgt[strc->type] + cnt2, CGranite1 + random(3));
				else if (Inside(GBackPix(strc->x + cnt1, strc->y + StructHgt[strc->type] + cnt2), CGranite1, CGranite2))
					SBackPix(strc->x + cnt1, strc->y + StructHgt[strc->type] + cnt2, CRock1 + random(3));
}

void FreeStructBack(STRUCTYPE *strc)
{
	int cnt1, cnt2;
	if (strc->type >= STCACTUS) return;
	for (cnt1 = 0; cnt1 < StructWdt[strc->type]; cnt1++)
		for (cnt2 = 0; cnt2 < StructHgt[strc->type]; cnt2++)
			DrawBackPix(strc->x + cnt1, strc->y + cnt2);
}

void DrawPlatform(STRUCTYPE *strc)
{
	int cnt;
	switch (strc->type)
	{
	case STCASTLE:
		for (cnt = 0; cnt < 20; cnt++)
			SBackPix(strc->x + cnt, strc->y, CGranite1);
		break;
	case STTOWER:
		for (cnt = 1; cnt < 15; cnt++)
			SBackPix(strc->x + cnt, strc->y + 4, CGranite1);
		for (cnt = 0; cnt < 5; cnt++)
		{
			SBackPix(strc->x + 1, strc->y + cnt, CGranite1);
			SBackPix(strc->x + 14, strc->y + cnt, CGranite1);
		}
		break;
	}
}

void RemovePlatfPix(int tx, int ty)
{
	if ((tx < 1) || IsSkyBack(GBackPix(tx - 1, ty))) DrawSkyPix(tx, ty);
	else DrawTunnelPix(tx, ty);
	ReleasePXS(tx, ty - 1); ReleasePXS(tx - 1, ty); ReleasePXS(tx + 1, ty);
}

void RemovePlatform(STRUCTYPE *strc)
{
	int cnt;
	switch (strc->type)
	{
	case STCASTLE:
		for (cnt = 0; cnt < 20; cnt++)
			RemovePlatfPix(strc->x + cnt, strc->y);
		break;
	case STTOWER:
		for (cnt = 1; cnt < 15; cnt++)
			RemovePlatfPix(strc->x + cnt, strc->y + 4);
		for (cnt = 0; cnt < 5; cnt++)
		{
			RemovePlatfPix(strc->x + 1, strc->y + cnt);
			RemovePlatfPix(strc->x + 14, strc->y + cnt);
		}
		// Remove Gate
		int xside = 2; if (strc->x > Crew[strc->owner].BaseX) xside = 12;
		for (cnt = 17; cnt <= 31; cnt++)
		{
			RemovePlatfPix(strc->x + xside, strc->y + 4 + 16 - (cnt - 16));
			RemovePlatfPix(strc->x + xside + 1, strc->y + 4 + 16 - (cnt - 16));
		}
		break;
	}
}

void DrawTowerGate(STRUCTYPE *strc)
{
	int xside = 2; if (strc->x > Crew[strc->owner].BaseX) xside = 12;
	if (Inside(strc->p, 1, 15)) // Close
	{
		SBackPix(strc->x + xside, strc->y + 4 + strc->p, CGranite2);
		SBackPix(strc->x + xside + 1, strc->y + 4 + strc->p, CGranite2);
	}
	if (Inside(strc->p, 17, 31)) // Open
	{
		RemovePlatfPix(strc->x + xside, strc->y + 4 + 16 - (strc->p - 16));
		RemovePlatfPix(strc->x + xside + 1, strc->y + 4 + 16 - (strc->p - 16));
	}
}

//-------------------- Structure Construction Functions ---------------------

BYTE ConstructionCheck(int type, int &ctx, int &cty)
{
	STRUCTYPE *cstrc;
	int cnt, cnt2, swdt, shgt, ngcnt;

	swdt = StructWdt[type]; shgt = StructHgt[type];
	ctx += 2;
	if (!Inside(ctx, 0, 319) || !Inside(cty, 0, BackGrSize - 2)) return 0;
	while (Inside(GBackPix(ctx, cty), CSolidL, CSolidH))
	{
		cty--; if (cty < 0) return 0;
	}
	while (!Inside(GBackPix(ctx, cty + 1), CSolidL, CSolidH))
	{
		cty++; if (cty > BackGrSize - 2) return 0;
	}

	ctx -= swdt / 2; ctx /= 4; ctx *= 4; cty -= shgt - 1;

	if (type == STWINDMILL) if (!Inside(ctx, 4, 300)) return 0;

	cstrc = FirstStruct;
	while (cstrc)
	{
		if (Inside(cstrc->type, STWINDMILL, STCASTLE))
			if (Inside(ctx - cstrc->x, -swdt + 1, StructWdt[cstrc->type] - 1)
				&& Inside(cty - cstrc->y, -shgt + 1, StructHgt[cstrc->type] - 1))
				return 0;
		cstrc = cstrc->next;
	}

	ngcnt = 0;
	for (cnt = ctx; cnt < ctx + swdt; cnt++)
	{
		if (!Inside(cnt, 0, 319)) return 0;
		for (cnt2 = cty; cnt2 < cty + shgt; cnt2++)
			if ((cnt2 < 0) || Inside(GBackPix(cnt, cnt2), CSolidL, CSolidH))
				ngcnt++;
		for (cnt2 = cty + shgt; cnt2 < cty + shgt + 4; cnt2++)
			if ((cnt2 > BackGrSize - 1) || !Inside(GBackPix(cnt, cnt2), CSolidL, CSandT))
				ngcnt++;
	}

	if (ngcnt > ((1.5 + 1.5*(type == STELEVATOR)))*swdt) return 0;

	return 1;
}

void StructCompletion(STRUCTYPE *strc, BYTE byhand, int newown)
{
	strc->p = 0;
	strc->liqstr = 0;
	strc->energy = 0;
	strc->owner = newown;
	DrawStructBase(strc, 1);
	FreeStructBack(strc);
	DrawPlatform(strc);
	if (byhand) if (Inside(strc->owner, 0, 2)) ScoreGain(strc->owner, (BSA.SMode == S_MELEE) ? 50 : 20);
	if (strc->type == STCASTLE) if (BSA.GPlrElm != 2) strc->CstFlag = 1;
	EventCall(209);
}

//------------------------- Structure Data Organization ----------------------

STRUCTYPE *NewStruct(int type, int x, int y, int p, int con, int owner)
{
	STRUCTYPE *newstruct, *cstrc;
	if (!(newstruct = new STRUCTYPE)) { LowMem = 1; return NULL; }
	newstruct->x = x; newstruct->y = y; newstruct->p = p;
	newstruct->type = type; newstruct->con = con;
	newstruct->tptr = NULL; newstruct->onf = 0;
	newstruct->owner = owner;
	newstruct->energy = 0;
	newstruct->damage = 0;
	newstruct->liqstr = 0;

	// liqstr usage:
	//
	// Under construction: water collection (for fire)
	//
	// STPUMP     1 pix storage for liqtypes (-1,0,1,2,3)
	// STOILPOWER 0-50 pix oil storage
	// STELEVATOR elevator speed 0-2
	// STTOWER    don't draw flag
	// STCASTLE   0-50 pix oil storage

	if (FirstStruct) // New ones to the end of line
	{
		cstrc = FirstStruct; while (cstrc->next) cstrc = cstrc->next;
		cstrc->next = newstruct;
	}
	else
		FirstStruct = newstruct;
	newstruct->next = NULL;

	return newstruct;
}

void DestroyConnects(STRUCTYPE *tstrc)
{
	LINETYPE *cline = FirstLine;
	while (cline)
	{
		if ((cline->fstrc == tstrc) || (cline->tstrc == tstrc))
			cline->type = LNNOLINE;
		cline = cline->next;
	}
}

void DeleteStruct(STRUCTYPE *tstrc)
{
	STRUCTYPE *cstrc = FirstStruct, *prev = NULL;
	if (!tstrc) return;
	ClearMenTPtr(tstrc);
	DestroyConnects(tstrc);
	while (cstrc)
	{
		if (cstrc->next == tstrc) { prev = cstrc; break; }
		cstrc = cstrc->next;
	}
	if (tstrc == FirstStruct) FirstStruct = tstrc->next;
	if (prev) prev->next = tstrc->next;
	delete tstrc;
}

void DeleteStructs(void)
{
	STRUCTYPE *temptr;
	while (FirstStruct) { temptr = FirstStruct->next; delete FirstStruct; FirstStruct = temptr; }
}

//-------------------------- Structure Activities ----------------------------

void CheckMen4StructCon(STRUCTYPE *tstrc)
{
	MANTYPE *mptr;
	int cnt;
	for (cnt = 0; cnt < 3; cnt++)
		for (mptr = Crew[cnt].FirstMan; mptr; mptr = mptr->next)
			if (mptr->act == MAWALK) if (!mptr->tptr)
				if (Inside(mptr->x - tstrc->x, 0, StructWdt[tstrc->type] - 8))
					if (Inside(mptr->y - tstrc->y, 0, StructHgt[tstrc->type] - 5))
					{
						mptr->act = MABUILD;
						mptr->xdir = Sign(mptr->xdir); mptr->phase = 0; mptr->ydir = 0;
						mptr->tptr = tstrc;
					}
}

void InstallFlag(MANTYPE *mptr, STRUCTYPE *tstrc)
{
	if (!tstrc->CstFlag) if ((mptr->carr == FLAG) && (mptr->carrp == tstrc->owner))
	{
		mptr->carr = NOROCK; tstrc->CstFlag = 1;
		Crew[tstrc->owner].CaFlX = tstrc->x + 6; Crew[tstrc->owner].CaFlY = tstrc->y - 18;
	}
}

extern BYTE HomeMan(MANTYPE *mptr, BYTE plr, void *htptr, int homeat);
extern void HomeRVehic(VEHICTYPE *vhc, void *htptr, int homeat);


void CheckGateTransport(STRUCTYPE *tstrc) // Inside castle phase 3-15
{
	MANTYPE *mptr;
	VEHICTYPE *cvhc;
	ROCKTYPE *crck;
	int cnt;
	// Vehicles
	if (tstrc->p < 10)
		for (cvhc = FirstVehic; cvhc; cvhc = cvhc->next)
			if ((cvhc->type == VHLORRY) || (cvhc->type == VHSAILBOAT) || (cvhc->type == VHCATAPULT) || (cvhc->type == VHCROSSBOW))
				if (Inside(cvhc->p, -10, +10))
				{
					// Gate to platform/home
					if (Inside(cvhc->x + 6 - (tstrc->x + 10), -3, +3))
						if (Inside(cvhc->y - tstrc->y, 1, 5))
							if (BSA.Realism.CstHome)
							{
								HomeRVehic(cvhc, tstrc, tstrc->owner);
							}
							else
							{
								cvhc->x = tstrc->x + 2; cvhc->y = tstrc->y - 9; // v Balloon on platform?
								cvhc->p = 40 + 20 * Inside(GBackPix(tstrc->x + 15, tstrc->y - 1), CVhcL, CVhcH);
							}
				}
	// Men
	for (cnt = 0; cnt < 3; cnt++)
		if (Crew[cnt].FirstMan)
			if (!Crew[tstrc->owner].Hostile[cnt] && !Crew[cnt].Hostile[tstrc->owner])
				for (mptr = Crew[cnt].FirstMan; mptr; mptr = mptr->next)
					if (mptr->act == MAWALK) //if (!mptr->tptr)
					{
						// Gate to platform/home
						if (mptr->x == tstrc->x + 6) if (mptr->tx == mptr->x)
							if (Inside(mptr->y - tstrc->y, 1, 5))
								if (BSA.Realism.CstHome)
								{
									InstallFlag(mptr, tstrc);
									HomeMan(mptr, cnt, tstrc, tstrc->owner);
								}
								else
								{
									mptr->x = tstrc->x + 2;
									mptr->y = tstrc->y - 8;
									mptr->tx = tstrc->x + 8 + random(6);
									InstallFlag(mptr, tstrc);
								}
						// Platform (tower) to gate
						if (mptr->x == tstrc->x + 1) if (mptr->x == mptr->tx)
							if (mptr->y == tstrc->y - 9)
							{
								mptr->y = tstrc->y + 3;
								if (random(2)) { mptr->x = tstrc->x + 6 - 1; mptr->tx = tstrc->x + 6 - 2 - random(5); }
								else { mptr->x = tstrc->x + 6 + 1; mptr->tx = tstrc->x + 6 + 2 + random(5); }
								InstallFlag(mptr, tstrc);
							}
					}
	// Rocks from storage
	if (BSA.Realism.CstHome)
		for (cnt = 0; cnt < RockTypeNum; cnt++)
			while (Crew[tstrc->owner].RockStorage[cnt] > 0)
			{
				NewRock(tstrc->x + 8 + random(11) - 5, tstrc->y + 8, RADEAD, cnt, 0, 0, 5 * Inside(cnt, ARROW, BARROW), -1);
				Crew[tstrc->owner].RockStorage[cnt]--;
				if (Crew[tstrc->owner].CMenu == CMRCKORDER) Crew[tstrc->owner].RedrStB = 1;
			}
}

void CheckOilCollection(STRUCTYPE *tstrc)
{
	MANTYPE *mptr;
	int cnt, brls = 0;
	for (cnt = 0; cnt < 3; cnt++)
		if (Crew[cnt].FirstMan)
			for (mptr = Crew[cnt].FirstMan; mptr; mptr = mptr->next)
				if (mptr->act == MAWALK) if (!mptr->tptr)
					if (Inside(mptr->x - tstrc->x, -3, 11))
						if (Inside(mptr->y - tstrc->y, 1, 5))
							if (mptr->carr == OILBARREL)
								if (tstrc->liqstr < OilPLiqStSize)
								{
									mptr->carr = BARREL;
									tstrc->liqstr = Min(tstrc->liqstr + 15, OilPLiqStSize);
									brls++;
									GSTP = SNDCONNECT;
								}
	if (brls > 0)
	{
		snprintf(OSTR, "+%d Oil", brls);
		GameMessage(OSTR, tstrc->x + 8, tstrc->y - 8, CWhite, NULL);
	}
}

BYTE EnergyFromOilPowerNeeded(STRUCTYPE *tstrc) // Returns 0 for burn off
{                                             // if hooked to running wmill
	LINETYPE *cline;
	for (cline = FirstLine; cline; cline = cline->next)
		if (cline->fstrc == tstrc)
			if (cline->tstrc && (cline->tstrc->type == STWINDMILL))
				if (Abs(Weather.wind2)*BoundBy(130 - cline->tstrc->y, 0, 130) / 260 > 0)
					return 0;
	return 1;
}

void CheckForDrawFlag(STRUCTYPE *tstrc) // Guard towers...
{
	if (OnWhichStruct(tstrc->x + 3, tstrc->y - 9)) tstrc->liqstr = 1;
	else tstrc->liqstr = 0;
}

void TowerDrainWater(STRUCTYPE *tstrc)
{
	if (Inside(GBackPix(tstrc->x + 2, tstrc->y + 3), CWaterS, CWaterT))
		RemovePlatfPix(tstrc->x + 1, tstrc->y + 3);
	else
		SBackPix(tstrc->x + 1, tstrc->y + 3, CGranite1);
}

void TowerVhcElevation(STRUCTYPE *tstrc)
{
	VEHICTYPE *cvhc;
	for (cvhc = FirstVehic; cvhc; cvhc = cvhc->next)
		if ((cvhc->type == VHLORRY) || (cvhc->type == VHCATAPULT) || (cvhc->type == VHCROSSBOW))
			if (Inside(cvhc->p, -10, +10))
				if (Inside(cvhc->x + 6 - (tstrc->x + 8), -3, +3))
					if (Inside(cvhc->y - tstrc->y, 9, 13))
					{
						cvhc->x = tstrc->x + 2; cvhc->y = tstrc->y - 7; cvhc->p = 0;
					}
}

void ExecStructs(void) // Every Tick1
{
	int cnt;
	BYTE gbpix;
	STRUCTYPE *cstrc = FirstStruct;
	STRUCTYPE *tptr;
	StructCnt = 0;
	StructBurn = 0;
	while (cstrc)
	{
		// Under construction/destruction ----------------------------------------
		if (cstrc->con < 1000)
		{
			if (!Tick5)
			{
				// Construction building needs workers (Phase 1-7) [check for 35 frs]
				if (!cstrc->onf)
					if (Inside(cstrc->type, STNOSTRUCT + 1, STCACTUS - 1))
						if (cstrc->p > 0)
						{
							CheckMen4StructCon(cstrc);
							cstrc->p++; if (cstrc->p > 7) cstrc->p = 0;
						}
				// Burning Struct -> Clonk Ignition
				if (cstrc->onf)
				{
					if (cstrc->con > 200)
						CheckManIgnition(cstrc->x + 8, cstrc->y + 13);
					ApplyHeat2Back(cstrc->x + 8, cstrc->y + StructHgt[cstrc->type] - random(5), 0);
				}
				else // Tree Growth
					if (Inside(cstrc->type, STCACTUS, STCONIFTREE)) cstrc->con++;
				// Destroyed elevator loses car
				if (cstrc->type == STELEVATOR)
					if (cstrc->tptr)			// elevator crash????
					{
						RemoveVehic((VEHICTYPE*)cstrc->tptr); cstrc->tptr = NULL;
					}
				// Destroyed castle loses platform
				if (!Tick50) if ((cstrc->type == STCASTLE) || (cstrc->type == STTOWER))
					RemovePlatform(cstrc);
			}
		}
		// Up and running --------------------------------------------------------
		else
		{
			if (!BSA.Realism.StrcEnrg)
				if ((cstrc->type != STCASTLE) && (cstrc->type != STMAGIC))
					cstrc->energy = 100;
			switch (cstrc->type)
			{
			case STWINDMILL:
				if (GBackPix(cstrc->x + 8, cstrc->y + 8) < CSolidL)
				{
					cnt = Abs(Weather.wind2)*BoundBy(130 - cstrc->y, 0, 130) / 260;
					cstrc->p += cnt; // Phase is wheel gfx
					cstrc->energy = Min(cstrc->energy + 3 * cnt, 50);
				}
				while (cstrc->p > 39) cstrc->p -= 40; // STWINDMILL phase 40
				break;
			case STELEVATOR:
				if (!Tick50 && !cstrc->tptr) // Has no car yet? (Repeated Check)
					cstrc->tptr = NewVehic(VHELEVATOR, cstrc->x, cstrc->y + 7, 0, cstrc->y + 7, BackGrSize, 0, cstrc->owner, NULL);
				// Move elevator (liqstr is speed)
				if (((cstrc->liqstr == 0) && (!Tick3)) || ((cstrc->liqstr == 1) && (!Tick2)) || (cstrc->liqstr == 2))
					if (cstrc->energy >= 10)
					{ // Phase is wheel gfx
						if (!Tick3)
						{
							cstrc->p++; if (cstrc->p > 1) cstrc->p = 0; cstrc->energy -= 10;
						}
						if (cstrc->tptr) MoveElevator((VEHICTYPE*)cstrc->tptr);
					}
				// Increase speed check
				if (!Tick50) if (cstrc->tptr) if (cstrc->liqstr < 2)
					if (((VEHICTYPE*)cstrc->tptr)->y - cstrc->y > 50 * (cstrc->liqstr + 1))
						cstrc->liqstr++;
				break;
			case STCASTLE:
				// Renew platform
				if (Sec10) DrawPlatform(cstrc);
				// Gate phasing
				if (!Tick3) if (cstrc->p > 0)
				{ // Phase: 0 Nothing 1-5 Open 6-12 TransportCheck 13-17 Close
					if (cstrc->p == 1) GSTP = SNDCGATE;
					if (cstrc->p == 13) GSTP = SNDCGATEC;
					cstrc->p++; if (cstrc->p == 18) cstrc->p = 0;
					if (Inside(cstrc->p, 3, 15)) CheckGateTransport(cstrc);
				}
				// Activate gate by transfer rock storage
				if (Sec1)
					if (BSA.Realism.CstHome) if (cstrc->p == 0)
						for (cnt = 0; cnt < RockTypeNum; cnt++)
							if (Crew[cstrc->owner].RockStorage[cnt] > 0)
								cstrc->p = 1;
				// Oil barrel output
				if (Sec1)
					if (cstrc->liqstr >= 15)
						if (BSA.Realism.CstHome)
						{
							cstrc->liqstr -= 15;
							Crew[cstrc->owner].Wealth = Min(Crew[cstrc->owner].Wealth + 5, 999);
							ScoreGain(cstrc->owner, 5);
						}
						else                          // Hard coded RckOrderID!!!!
							if ((Crew[cstrc->owner].RckProd.Store[9] > 0) && (Crew[cstrc->owner].Wealth >= RockValue[BARREL]))
							{
								cstrc->liqstr -= 15;
								Crew[cstrc->owner].RckProd.Store[9]--;
								Crew[cstrc->owner].Wealth -= RockValue[BARREL];
								Crew[cstrc->owner].RedrStB = 1;
								NewRock(cstrc->x + 9 + random(5), cstrc->y - 4, RADEAD, OILBARREL, 0, 0, 0, -1);
								EventCall(212);
							}
				break;
			case STOILPOWER:
				// Activity/Energy production            (Phase 1: Burn)
				if (cstrc->p == 1) if (cstrc->liqstr > 0)
				{
					if (Sec5) // Oil burn speed: 1/Sec5
					{
						cstrc->liqstr--;
						if (cstrc->liqstr == 5) GameMessage("Oil almost|empty", cstrc->x + 8, cstrc->y - 8, CWhite, NULL);
						EventCall(213);
					}
					cstrc->energy = Min(cstrc->energy + 7, 50);
					if (!Tick2) PXSOut(PXSSMOKE, cstrc->x + 10, cstrc->y, 30 + random(40));
				}
				// Check burn on/off
				if (Sec1)
					cstrc->p = EnergyFromOilPowerNeeded(cstrc);
				break;
			case STTOWER:
				// Renew platform
				if (Sec10) { DrawPlatform(cstrc); CheckForDrawFlag(cstrc); }
				// Gate phasing (0 Open 1-15 Closing 16 Closed 17-31 Opening)
				if (Inside(cstrc->p, 1, 15)) { DrawTowerGate(cstrc); cstrc->p++; }
				if (Inside(cstrc->p, 17, 31)) { DrawTowerGate(cstrc); cstrc->p++; if (cstrc->p == 32) cstrc->p = 0; }
				break;
			}
			// Snow Clearing
			if (!Tick50)
				if ((cstrc->type == STCASTLE) || (cstrc->type == STTOWER) || (!BSA.Realism.StrcSnow && (cstrc->type <= STCACTUS)))
				{
					ClearLineOfSnow(cstrc->x - 1, cstrc->y + StructHgt[cstrc->type] - 1, StructWdt[cstrc->type] + 2);
					if (cstrc->type == STCASTLE) ClearLineOfSnow(cstrc->x, cstrc->y - 1, 20);
					if (cstrc->type == STTOWER)
					{
						ClearLineOfSnow(cstrc->x + 2, cstrc->y + 3, 12); TowerDrainWater(cstrc);
					}
				}
		}
		// Always ----------------------------------------------------------------
		gbpix = GBackPix(cstrc->x + StructWdt[cstrc->type] / 2, cstrc->y + StructHgt[cstrc->type] - 6);
		// Burning structs
		if (cstrc->onf)
		{
			StructBurn = 1;
			// Smoking
			if ((cstrc->type < STCACTUS) || (cstrc->con > 200)) // Tree Trunks don't
			{
				cnt = cstrc->y + StructHgt[cstrc->type] * (1000 - cstrc->con) / 1000;
				if (Rnd3()) PXSOut(PXSSMOKE, cstrc->x + 2 + random(StructWdt[cstrc->type] - 4), cnt, 30 + random(40));
				if (cstrc->con > 400) PXSOut(PXSSMOKE, cstrc->x + 2 + random(StructWdt[cstrc->type] - 4), cnt, 30 + random(40));
			}
			// Destruction
			if (!Tick3 || (cstrc->type >= STCACTUS)) cstrc->con--;
			if ((cstrc->con % 100) == 0) DrawStructBase(cstrc, 0);
			if (cstrc->con < 0) cstrc->type = STNOSTRUCT;
			if (cstrc->type == STOILPOWER) // Full OilPowers explode
				if (cstrc->liqstr > 15)
				{
					cstrc->con = 0; DrawStructBase(cstrc, 0);
					Explode(cstrc->x + 8, cstrc->y + 7, 15, -1);
					for (cnt = 0; cnt < 4; cnt++)
						NewRock(cstrc->x + 6, cstrc->y + 4, RAFLY, ROCK, 10 * (random(5) - 2), 0 + random(5) - 2, 0, -1);
					PourPXS(PXSBURNOIL, cstrc->x + 8, cstrc->y + 7, 100 + random(50), 50, 18);
				}
			// Background effects
			if (Inside(gbpix, CWaterS, CWaterT)) { cstrc->onf = 0; GSTP = SNDPSHSH; }
			if (!Tick20) if (Inside(gbpix, CSnowS, CSnowT)) MeltFree(cstrc->x + StructWdt[cstrc->type] / 2, cstrc->y + StructHgt[cstrc->type] - 5, 8);
			// Extinguish by liqstr
			if (cstrc->liqstr > 30) { cstrc->onf = 0; GSTP = SNDPSHSH; }
		}
		// Lava incineration
		if (!Tick5)
			if ((cstrc->type != STCACTUS) && (cstrc->type != STCASTLE) && (cstrc->type != STOILPOWER) && (cstrc->type != STTOWER))
				if (BSA.Realism.StrcBurn || (cstrc->type >= STCACTUS))
					if (Inside(gbpix, CLavaS1, CLavaT3))
						IncinerateStruct(cstrc);
		// Automatic structure damage repair
		if (Sec5) if (cstrc->damage > 0) cstrc->damage--;
		// Removal of destroyed struct
		if (cstrc->type == STNOSTRUCT) { tptr = cstrc; cstrc = cstrc->next; DeleteStruct(tptr); }
		else { cstrc = cstrc->next; StructCnt++; }
	}
}

//---------------------------- (Line Control) --------------------------------

BYTE LineNotCarried(LINETYPE *lptr)
{
	int cnt;
	MANTYPE *mptr;
	for (cnt = 0; cnt < 3; cnt++)
		for (mptr = Crew[cnt].FirstMan; mptr; mptr = mptr->next)
			if (mptr->tptr == lptr)
				return 0;
	return 1;
}

LINETYPE *PickUpOpenLine(int cx, int cy)
{
	LINETYPE *cline = FirstLine;
	while (cline)
	{
		if (cline->type != LNNOLINE)
			if (Inside(cline->x[cline->lsec] - cx, -8, +8))
				if (Inside(cline->y[cline->lsec] - cy, -8, +8))
					return cline;
		cline = cline->next;
	}
	return NULL;
}

BYTE ConnectionExists(STRUCTYPE *str1, STRUCTYPE *str2, BYTE type)
{
	LINETYPE *cline = FirstLine;
	while (cline)
	{
		if ((cline->fstrc == str1) && (cline->tstrc == str2) && (cline->type == type)) return 1;
		if ((cline->fstrc == str2) && (cline->tstrc == str1) && (cline->type == type)) return 1;
		cline = cline->next;
	}
	return 0;
}

LINETYPE *NewLine(int type, int x1, int y1, STRUCTYPE *fstrc, STRUCTYPE *tstrc)
{
	int cnt;
	LINETYPE *newline, *cline;
	if (!(newline = new LINETYPE)) { LowMem = 1; return NULL; }
	for (cnt = 0; cnt < LineLen; cnt++)
	{
		newline->x[cnt] = -1; newline->y[cnt] = -1;
	}
	newline->x[0] = x1; newline->y[0] = y1;
	newline->mfd = 0;
	newline->type = type;
	newline->lsec = 0;
	newline->fstrc = fstrc; newline->tstrc = tstrc;

	if (FirstLine)
	{
		cline = FirstLine; while (cline->next) cline = cline->next;
		cline->next = newline; // put to the end of list (not necessary)
	}
	else
		FirstLine = newline;
	newline->next = NULL;

	return newline;
}

void DeleteLine(LINETYPE *tline)
{
	LINETYPE *cline = FirstLine, *prev = NULL;
	if (!tline) return;
	ClearMenTPtr(tline);
	while (cline)
	{
		if (cline->next == tline) { prev = cline; break; }
		cline = cline->next;
	}
	if (tline == FirstLine) FirstLine = tline->next;
	if (prev) prev->next = tline->next;
	delete tline;
}

void DeleteLines(void)
{
	LINETYPE *temptr;
	while (FirstLine) { temptr = FirstLine->next; delete FirstLine; FirstLine = temptr; }
}

BYTE LineCutEarthAt(int x1, int y1, int x2, int y2, int *retx, int *rety)
{
	int d, dx, dy, aincr, bincr, xincr, yincr, x, y;

	if (Abs(x2 - x1) < Abs(y2 - y1))
	{
		if (y1 > y2) { Swap(x1, x2); Swap(y1, y2); }
		xincr = (x2 > x1) ? 1 : -1;
		dy = y2 - y1; dx = Abs(x2 - x1);
		d = 2 * dx - dy; aincr = 2 * (dx - dy); bincr = 2 * dx; x = x1; y = y1;
		if (Inside(GBackPix(x, y), CSolid2L, CSolidH)) { *retx = x; *rety = y; return 1; }
		for (y = y1 + 1; y <= y2; ++y)
		{
			if (d >= 0) { x += xincr; d += aincr; }
			else d += bincr;
			if (Inside(GBackPix(x, y), CSolid2L, CSolidH)) { *retx = x; *rety = y; return 1; }
		}
	}
	else
	{
		if (x1 > x2) { Swap(x1, x2); Swap(y1, y2); }
		yincr = (y2 > y1) ? 1 : -1;
		dx = x2 - x1; dy = Abs(y2 - y1);
		d = 2 * dy - dx; aincr = 2 * (dy - dx); bincr = 2 * dy; x = x1; y = y1;
		if (Inside(GBackPix(x, y), CSolid2L, CSolidH)) { *retx = x; *rety = y; return 1; }
		for (x = x1 + 1; x <= x2; ++x)
		{
			if (d >= 0) { y += yincr; d += aincr; }
			else d += bincr;
			if (Inside(GBackPix(x, y), CSolid2L, CSolidH)) { *retx = x; *rety = y; return 1; }
		}
	}
	return 0;
}

void ExecLine(LINETYPE *cline)
{
	int cnt, trnsf, cpos, ceax, ceay, xadj, yadj, chkpr;
	// Section adjust
	if (cline->mfd == 1)
	{
		for (cpos = 0; (cpos < LineLen - 1) && (cline->x[cpos + 1] != -1); cpos++);
		if (cpos == 0) { /*RoundError("safety: Line w/ 1 loc only");*/ return; }
		xadj = yadj = 0;
		if (LineCutEarthAt(cline->x[cpos - 1], cline->y[cpos - 1], cline->x[cpos], cline->y[cpos], &ceax, &ceay))
		{

			chkpr = +1; if (cline->y[cpos - 1] > cline->y[cpos]) chkpr = -1;
			for (cnt = 0; cnt < 20; cnt++) // Y-Adjust for section
			{
				if (!Inside(ceay, 0 + cnt, BackGrSize - 1 - cnt))
				{ /*SystemMsg("Cannot y-adjust line sec (b)");*/ return;
				}
				if (!Inside(GBackPix(ceax, ceay + cnt*chkpr), CSolid2L, CSolidH)) { yadj = +cnt + 1; break; }
				if (!Inside(GBackPix(ceax, ceay - cnt*chkpr), CSolid2L, CSolidH)) { yadj = -cnt - 1; break; }
			}
			if (cnt == 20) { /*SystemMsg("Cannot y-adjust line sec");*/ return; }
			yadj *= chkpr;

			chkpr = +1; if (cline->x[cpos - 1] > cline->x[cpos]) chkpr = -1;
			for (cnt = 0; cnt < 20; cnt++) // X-Adjust for section
			{
				if (!Inside(ceax, 0 + cnt, 319 - cnt))
				{ /*SystemMsg("Cannot x-adjust line sec (b)");*/ return;
				}
				if (!Inside(GBackPix(ceax + cnt*chkpr, ceay), CSolid2L, CSolidH)) { xadj = +cnt + 1; break; }
				if (!Inside(GBackPix(ceax - cnt*chkpr, ceay), CSolid2L, CSolidH)) { xadj = -cnt - 1; break; }
			}
			if (cnt == 20) { /*SystemMsg("Cannot x-adjust line sec");*/ return; }
			xadj *= chkpr;

			ceax += xadj; ceay += yadj;
			if (cpos == LineLen - 1)
			{
				GameMessage("Line too long", cline->x[cpos], cline->y[cpos], CWhite, NULL); /*error sound*/ cline->type = LNNOLINE; return;
			}
			cline->x[cpos + 1] = cline->x[cpos]; cline->y[cpos + 1] = cline->y[cpos];
			cline->x[cpos] = ceax; cline->y[cpos] = ceay;
			cline->lsec = cpos + 1;
		}
		cline->mfd = 0;
	}
	// Source pipe drilling (LNSRCPIPE only)
	if (cline->mfd == 2 + 1) // Down
	{
		cline->y[cline->lsec]++;
		if (cline->y[cline->lsec] > BackGrSize - 5) cline->mfd = 0;
		if (cline->y[cline->lsec] - cline->y[cline->lsec - 1] > 20) cline->mfd = 0;
		if (Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec] + 1), CLiqL, CLiqH) && !Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec] + 2), CLiqL, CLiqH)) cline->mfd = 0;
		if (Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec]), CGranite1, CGold2)) cline->mfd = 0;
	}
	if (cline->mfd == 2 + 0) // Left
	{
		cline->x[cline->lsec]--;
		if (cline->x[cline->lsec] < 5) cline->mfd = 0;
		if (cline->x[cline->lsec] + 20 < cline->x[cline->lsec - 1]) cline->mfd = 0;
		if (Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec] + 1), CLiqL, CLiqH)) cline->mfd = 0;
		if (Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec]), CGranite1, CGold2)) cline->mfd = 0;
	}
	if (cline->mfd == 2 + 2) // Right
	{
		cline->x[cline->lsec]++;
		if (cline->x[cline->lsec] > 314) cline->mfd = 0;
		if (cline->x[cline->lsec] - 20 > cline->x[cline->lsec - 1]) cline->mfd = 0;
		if (Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec] + 1), CLiqL, CLiqH)) cline->mfd = 0;
		if (Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec]), CGranite1, CGold2)) cline->mfd = 0;
	}
	// Transfer execution
	if (cline->fstrc)
		switch (cline->type)
		{
		case LNENERGY: // Energy transfer (needs 2 structs)
			if (cline->tstrc)
			{
				trnsf = Min(cline->fstrc->energy / 5, 10);
				cline->fstrc->energy -= trnsf;
				cline->tstrc->energy = Min(cline->tstrc->energy + trnsf, 50);
			}
			break;
		case LNSRCPIPE: // Liquid source transfer (needs fstrc)
			if (!Tick5)    // if TRGPIPE and OILPOWER connected, pump 1 pix/Sec5
			{
				if ((cline->fstrc->con >= 1000) && (cline->fstrc->energy >= 5))
					if (cline->fstrc->liqstr == -1)
					{ // Pump to struct liquid storage for target pipe if possible
						trnsf = ExtractLiquid(cline->x[cline->lsec], cline->y[cline->lsec] + 1);
						if (trnsf != -1)
						{
							cline->fstrc->liqstr = trnsf;
							cline->fstrc->energy -= 10;
							cline->fstrc->p++; if (cline->fstrc->p > 2) cline->fstrc->p = -1;
						}
					}
			}
			break;
		case LNTRGPIPE: // Liquid target transfer (need fstrc)
			if (!Tick5)
			{
				trnsf = cline->fstrc->liqstr;
				if ((cline->fstrc->con >= 1000) && (cline->fstrc->energy >= 5))
					if (cline->tstrc) // Connected to struct?
					{
						if ((cline->tstrc->type == STOILPOWER) || (cline->tstrc->type == STCASTLE))
							if (trnsf == 2) // Oil
							{
								if (cline->tstrc->liqstr < OilPLiqStSize)
								{                        // Pump to OILPOWER or CASTLE
									cline->tstrc->liqstr++;  // if all okay
									cline->fstrc->liqstr = -1; // Otherwise don't pump
								}
							}
							else // Other liquid -> out
								if (trnsf != -1)
									if (!Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec]), CSolidL, CSolidH))
									{
										PXSOut(LiqRelClass[trnsf], cline->x[cline->lsec], cline->y[cline->lsec], 0);
										cline->fstrc->liqstr = -1;
										EventCall(211);
									}
					}
					else // No, pump to line end if open
					{
						if (trnsf != -1)
							if (!Inside(GBackPix(cline->x[cline->lsec], cline->y[cline->lsec]), CSolidL, CSolidH))
							{
								PXSOut(LiqRelClass[trnsf], cline->x[cline->lsec], cline->y[cline->lsec], 0);
								cline->fstrc->liqstr = -1;
								EventCall(211);
							}
					}
			}
			break;
		}
	// Automatic line shortening
	if (Sec5)
		for (cpos = 0; (cpos < LineLen - 2) && (cline->x[cpos] != -1); cpos++)
			if ((cline->x[cpos + 1] != -1) && (cline->x[cpos + 2] != -1))
				if (!LineCutEarthAt(cline->x[cpos], cline->y[cpos], cline->x[cpos + 2], cline->y[cpos + 2], &ceax, &ceay))
				{ // Shorten line at cpos
					for (cnt = cpos + 1; cnt < LineLen - 1; cnt++)
					{
						cline->x[cnt] = cline->x[cnt + 1]; cline->y[cnt] = cline->y[cnt + 1];
					}
					cline->x[cnt] = cline->y[cnt] = -1;
					cline->lsec--;
					break;
				}
}

void ExecLines(void) // Every Tick1
{
	LINETYPE *cline = FirstLine, *tmpptr;
	LineCnt = 0;
	while (cline)
	{
		if (cline->type != LNNOLINE) // Exec existing lines
		{
			ExecLine(cline);
			LineCnt++;
			cline = cline->next;
		}
		else // Delete non-existing lines
		{
			tmpptr = cline->next;
			DeleteLine(cline);
			cline = tmpptr;
		}
	}

}

//--------------------------- ReadyBase --------------------------------------

STRUCTYPE *InitReadyBase(BYTE plr) // Returns castle if set up
{
	const int BaseSType[5] = { STCASTLE,STELEVATOR,STWINDMILL,STPUMP,STTOWER };
	int cnt, cnt2, tx, ty, ldis, cstype, lvl = -1;
	STRUCTYPE *nstrc[6] = { NULL,NULL,NULL,NULL,NULL,NULL };
	LINETYPE *nline;

	for (cnt = 0; cnt < 6; cnt++)
		if ((cnt < 5) || (Crew[plr].BaseX == 160)) // Center player might get 2 towers
			if (BSA.Plr[plr].ReadyBase[Min(cnt, 4)])
			{
				cstype = BaseSType[Min(cnt, 4)];

				// Double-struct avoided, since no placement on granite
				if (!FindSurface(4, Crew[plr].BaseX, StructWdt[cstype], &tx, &ty)) break;
				if ((cnt == 1) && (lvl != -1)) ty = lvl - 20; // ELEV same as CASTLE
				else { ty = 0; while (!Inside(GBackPix(tx, ty), CGroundL, CSandT)) ty++; ty -= StructHgt[cstype]; }
				tx = BoundBy(tx - StructWdt[cstype] / 2, 0, 320 - StructWdt[cstype]); tx /= 4; tx *= 4;

				for (cnt2 = 0, ldis = 1000; cnt2 < cnt; cnt2++)
					if (nstrc[cnt2]) if (Abs(tx - nstrc[cnt2]->x) < ldis)
						ldis = Abs(tx - nstrc[cnt2]->x);
				if (ldis != 1000) if (ldis > 50) break; // no spreading

				if (cstype == STCASTLE) tx = BoundBy(tx, 0, 296);
				if (cstype == STWINDMILL) tx = BoundBy(tx, 4, 300);
				if (!(nstrc[cnt] = NewStruct(cstype, tx, ty, 0, 1000, plr))) break;
				lvl = ty + StructHgt[cstype]; // Basement level
				RaiseUpTerrain(tx, lvl, StructWdt[cstype]);
				StructCompletion(nstrc[cnt], 0, plr);
			}

	if (nstrc[1] && nstrc[2]) // Elevator and WMill have been built
	{
		if (!(nline = NewLine(LNENERGY, nstrc[2]->x + 3, nstrc[2]->y + StructHgt[STWINDMILL] - 4, nstrc[2], nstrc[1]))) return 0;
		nline->x[1] = nstrc[1]->x + 3; nline->y[1] = nstrc[1]->y + StructHgt[STELEVATOR] - 4;
		nline->lsec = 1;
	}
	if (nstrc[2] && nstrc[3]) // WMill and pump have been built
	{
		if (!(nline = NewLine(LNENERGY, nstrc[2]->x + 3, nstrc[2]->y + StructHgt[STWINDMILL] - 4, nstrc[2], nstrc[3]))) return 0;
		nline->x[1] = nstrc[3]->x + 3; nline->y[1] = nstrc[3]->y + StructHgt[STPUMP] - 4;
		nline->lsec = 1;
	}

	return nstrc[0];
}
