/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design CLONK tng BattleSequence GameCon System Module

#include <stdio.h>
#include <stdlib.h>

#include "standard.h"
#include "vga4.h"

#include "clonk_bs.h"

#include "bsexcsys.h"
#include "bsmansys.h"
#include "bsgfxsys.h"
#include "bspxssys.h"
#include "bsstrsys.h"
#include "bsvhcsys.h"
#include "bsweasys.h"

#include "RandomWrapper.h"

//-------------------------- Vehicle Data Definitions -------------------------

#define VHDWipfLoad   back[29]
#define VHDCatPhase   back[0]
#define VHDCatLoad    back[1]
#define VHDCatFPower  back[2]
#define VHDToCastle   back[26]
#define VHDBallCrash  back[28]
#define VHDPushOutDir back[28] 		// -> header?
#define VHDLApprCorr  back[29]
#define VHDOnWater    back[29]
#define VHDDrilling   data3
#define VHDXBowPhase  back[0]
#define VHDXBowLoadA  back[1]
#define VHDXBowDir    back[2]
#define VHDXBowDirCh  back[3]
#define VHDXBowJustL  back[4]
#define VHDXBowLoadF  back[5]
#define VHDXBowLoadB  back[6]


//------------------------- Main Global Externals ----------------------------

extern char OSTR[500];

extern USERPREF UserPref;
extern BSATYPE BSA;
extern CONFIG Config;

extern void EventCall(int evnum);

extern void KComFlash(int kcom);

//------------------------------ Extern Mouse -------------------------------

extern BYTE Mousebut(void);
extern int MouseX(void);
extern int MouseY(void);
extern BYTE MSON;
extern void SetMouseRange(int x1, int y1, int x2, int y2);
extern void SetMouseCursor(int type);
extern void SetMouseLoc(int tx, int ty);

MOUSECON MouseCon;

//--------------------------- Man Control Functions --------------------------

extern void ManWalk(MANTYPE *mptr, BYTE stand);
extern void ManFly(MANTYPE *mptr, int xdir, int ydir, BYTE stand);
extern void ManSwim(MANTYPE *mptr, int ydir);
extern void ManDig(MANTYPE *mptr, int dir);

//-------------------------------- Name Data ---------------------------------

char *LineTypeName[3] = { "Power line","Feeding pipe","Drain pipe" };
char *StructTypeName[StructTypeNum - 3] = { "Windmill","Water wheel","Oil power","Elevator","Pump","Castle","House","Tower","Magic Tower","(None)","(None)" };
char *ColorName[8] = { "Blue","Red","Green","Yellow","Purple","Brown","Light blue","Light green" };

extern const char *RankName[11];

//-------------------------- ------------------- -------------------------------

void ManCanDoMsg(MANTYPE *mptr, char *verb)
{
	sprintf(OSTR, "%s %s|can %s!", RankName[mptr->mi->rank], mptr->mi->name, verb);
	GameMessage(OSTR, 0, 0, CWhite, mptr);
}

void MouseFromMenu(void)
{
	MouseCon.cursor = 0;
	// The original version teleported the mouse here.
}

void ActivateMenu(BYTE plr, BYTE type)
{
	static char *menuname[11] = { "Exit menu","Main menu","Order|material","Order|Tools","Attack",
				   "Loam bridge","Construction","Feeding pipe drill","Surrender?",
				   "Command","Magic" };
	if (type > CMNOMENU)
	{
		if ((type == CMBRIDGE) && Crew[plr].Cursor && (Crew[plr].Cursor->carr == STEEL))
			GameMessage("Steel bridge", -1, plr, CWhite, NULL);
		else
			GameMessage(menuname[type], -1, plr, CWhite, NULL);
		GSTP = SNDDING;
	}
	Crew[plr].CMenu = type; Crew[plr].CMenuData = 0;
	EventCall(300 + type);
	if (type == CMNOMENU) if (MouseCon.status) if (MouseCon.player == plr)
		MouseFromMenu();
}

MANTYPE *OnWhichClonk(BYTE plr, int x, int y)
{
	MANTYPE *cman;
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
		if (cman->act < MADEAD)
			if (Inside(cman->x + 4 - x, -4, +4) && Inside(cman->y + 5 - y, -4, 4))
				return cman;
	return NULL;
}

//------------------------------ Magic --------------------------------------

char *MagicName[MagicNum] = { "Wind","Lightning","Cold",
			   "Earth quake","Comet storm","Volcano",
			   "","","" };

int MagicEnergy[MagicNum] = { 350,650,950, 1250,1550,1850, 0,0,0 };

long MagicLevel(int plr)
{
	long rval = 0L;
	STRUCTYPE *cstrc;
	for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)
		if ((cstrc->type == STMAGIC) && (cstrc->con >= 1000))
			if (cstrc->owner == plr)
				rval += cstrc->energy;
	return rval;
}

void SubtractMagicLevel(int plr, long level)
{
	int subt;
	STRUCTYPE *cstrc;
	for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)
		if ((cstrc->type == STMAGIC) && (cstrc->con >= 1000))
			if (cstrc->owner == plr)
				if (level > 0)
				{
					subt = Min(level, cstrc->energy);
					cstrc->energy -= subt;
					level -= subt;
				}
	Crew[plr].RedrStB = 1;
}

int OneOpponent(int plr)
{
	int cto, loops;
	cto = random(MaxGamePlr); loops = 0;
	while ((BSA.Plr[cto].Col == -1) || (cto == plr) || !Crew[plr].Hostile[cto] || BSA.Plr[cto].Eliminate)
	{
		++cto %= MaxGamePlr; loops++; if (loops > 2) return -1;
	}
	return cto;
}

extern void LaunchCometS(int tx, int csdby);
extern void LaunchEQuake(int target);
extern void LaunchFreeze(void);
extern void LaunchVolcano(int tx);
extern BYTE LaunchLightN(int type, int fx, int fy, int phase2);

void MagicLightning(int plr, int tx)
{
	STRUCTYPE *fstrc;

	for (fstrc = FirstStruct; fstrc; fstrc = fstrc->next)
		if ((fstrc->type == STMAGIC) && (fstrc->con >= 1000))
			if (fstrc->owner == plr)
				break;

	if (!fstrc) // No tower, magic energy lost
	{
		GameMessage("No magic tower", -1, plr, CWhite, NULL);
		GSTP = SNDERROR; return;
	}

	LaunchLightN(1, fstrc->x + 7, fstrc->y - 9, tx + random(60) - 30);

	GSTP = SNDTHUNDER;
}

void PlayerMagicCommand(int plr, int mgc)
{
	int target, targetx;

	if (MagicLevel(plr) < MagicEnergy[mgc])
	{
		GameMessage("Insufficient energy", -1, plr, CWhite, NULL);
		GSTP = SNDERROR; return;
	}

	target = OneOpponent(plr); if (target == -1) target = random(MaxGamePlr);

	targetx = (0.5 + (float)target)*(320 / MaxGamePlr);      // a little justice
	if (target == 0) targetx -= 20; if (target == MaxGamePlr - 1) targetx += 20;

	SubtractMagicLevel(plr, MagicEnergy[mgc]);
	ScoreGain(plr, MagicEnergy[mgc] / 100);
	GameMessage(MagicName[mgc], -1, plr, CWhite, NULL);

	switch (mgc)
	{
	case 0: // Wind
		if (target < plr) Weather.wind = Weather.wind2 = -60;
		else Weather.wind = Weather.wind2 = +60;
		break;
	case 1: // Blue Lightning
		MagicLightning(plr, targetx);
		break;
	case 2: // Freeze
		LaunchFreeze();
		break;
	case 3: // Quake
		LaunchEQuake(targetx);
		GSTP = SNDEQUAKE;
		break;
	case 4: // Comets
		LaunchCometS(targetx, plr);
		break;
	case 5: // Volcano
		LaunchVolcano(targetx);
		break;
	}

}

//--------------------------- Line Construction ------------------------------

extern void AbandonLine(MANTYPE *mptr);

void DrillPipe(MANTYPE *mptr, BYTE dir) // dir: 0 left 1 down 2 right
{
	LINETYPE *tline = (LINETYPE *)mptr->tptr;
	if (tline->type != LNSRCPIPE) { /*RoundError("Safety: Drill safety");*/ return; }
	if (tline->lsec < LineLen - 1)            // does occur!
	{
		tline->lsec++;
		tline->x[tline->lsec] = tline->x[tline->lsec - 1];
		tline->y[tline->lsec] = tline->y[tline->lsec - 1];
		tline->mfd = 2 + dir;
		GameMessage("Drilling", 0, 0, CWhite, mptr);
		GSTP = SNDDRILL;
	}
	else
	{
		GameMessage("Line too long!", 0, 0, CWhite, mptr);
	}
	mptr->carr = NOROCK;
	mptr->tptr = NULL;
}

BYTE LineConstruction(MANTYPE *mptr, BYTE plr) // Returns 0 if construction impossible
{
	int linetype;
	LINETYPE *tline;
	STRUCTYPE *tstrc;

	// Get TStruct
	tstrc = OnWhichStruct(mptr->x + 4, mptr->y);

	if (mptr->tptr) // Lining already
	{
		tline = (LINETYPE*)mptr->tptr;

		// Source pipe drill menu?
		if (tline->type == LNSRCPIPE)
			if ((mptr->act == MADIG) || (mptr->act == MAWALK))
			{
				ActivateMenu(plr, CMSRCPIPE); return 1;
			}

		if (tstrc && (tstrc->con >= 1000)) // In front of struct -> try connect
			if (((tline->type == LNENERGY) && ((tstrc->type == STELEVATOR) || (tstrc->type == STPUMP)))
				|| ((tline->type == LNTRGPIPE) && ((tstrc->type == STOILPOWER) || (tstrc->type == STCASTLE)))
				|| ((tline->type == LNENERGY) && tline->fstrc && (tline->fstrc->type == STOILPOWER) && (tstrc->type == STWINDMILL)))
				if (tline->fstrc != tstrc)
					if (!ConnectionExists(tline->fstrc, tstrc, tline->type))
					{
						mptr->tptr = NULL; mptr->carr = NOROCK;
						tline->x[tline->lsec] = tstrc->x + 3;
						tline->y[tline->lsec] = tstrc->y + StructHgt[tstrc->type] - 4;
						tline->tstrc = tstrc;
						GSTP = SNDCONNECT;
						GameMessage("Connection", tstrc->x + 3, tstrc->y, CWhite, NULL);
						Crew[plr].RedrStB = 1;
						EventCall(210);
						return 1;
					}

		// Not in front of struct or can't connect -> drop line open
		GameMessage("Line dropped", 0, 0, CWhite, mptr);
		mptr->carr = NOROCK; mptr->tptr = NULL;
		return 1;
	}
	else // Not lining
	{
		if (tstrc && (tstrc->con >= 1000)) // In front of struct -> try create
		{
			// Get LineType
			linetype = LNNOLINE;
			switch (tstrc->type)
			{
			case STWINDMILL: case STOILPOWER: // Producer (always okay)
				linetype = LNENERGY;
				break;
			case STELEVATOR: // User (okay if energy connected)
				if (StructIsConnected(tstrc, LNENERGY)) linetype = LNENERGY;
				break;
			case STPUMP: // Pump (source pipe if none, target pipe if source, otherwise energy if energy connected)
				if (!StructIsConnected(tstrc, LNSRCPIPE)) linetype = LNSRCPIPE;
				else if (!StructIsConnected(tstrc, LNTRGPIPE)) linetype = LNTRGPIPE;
				else if (StructIsConnected(tstrc, LNENERGY)) linetype = LNENERGY;
				break;
			}
			if (linetype != LNNOLINE) // Create Line
			{
				if (!(tline = NewLine(linetype, tstrc->x + 3, tstrc->y + StructHgt[tstrc->type] - 4, tstrc, NULL))) return 1;
				tline->x[1] = tline->x[0]; tline->y[1] = tline->y[0]; tline->lsec = 1;
				mptr->tptr = tline;
				GSTP = SNDCONNECT;
				switch (linetype)
				{
				case LNENERGY: GameMessage("New power line", 0, 0, CWhite, mptr); break;
				case LNSRCPIPE: GameMessage("New feeding pipe", 0, 0, CWhite, mptr); break;
				case LNTRGPIPE: GameMessage("New drain pipe", 0, 0, CWhite, mptr); break;
				}
				Crew[plr].RedrStB = 1;
				EventCall(210);
				return 1;
			}
			else // Can't create new line
			{
				GSTP = SNDERROR;
				GameMessage("Cannot create|a new line here", 0, 0, CWhite, mptr);
				return 1;
			}
		}
		else // Not in front of struct -> no construction
		{
			return 0;
		}
	}
}

BYTE LinePickUp(MANTYPE *mptr, BYTE plr) // Returns 0 if pick up impossible
{
	LINETYPE *lptr;
	int dummy1, dummy2, clsec;
	lptr = PickUpOpenLine(mptr->x + 4, mptr->y + 4);
	if (lptr && LineNotCarried(lptr)) // Try pickup
		if (!lptr->tstrc || NotHostile(plr, lptr->tstrc->owner))
		{
			clsec = lptr->lsec - 1;
			if (LineCutEarthAt(lptr->x[clsec], lptr->y[clsec], lptr->x[clsec + 1], lptr->y[clsec + 1], &dummy1, &dummy2))
				if (lptr->lsec >= LineLen - 1)
				{
					lptr->type = LNNOLINE; // Pick up, but "abandon"
					mptr->carr = LINECON;
					GameMessage("Line too long", 0, 0, CWhite, mptr);
				}
				else // Extend line if last section cuts earth
				{
					lptr->lsec++;
					lptr->x[lptr->lsec] = lptr->x[lptr->lsec - 1];
					lptr->y[lptr->lsec] = lptr->y[lptr->lsec - 1];
				}
			lptr->tstrc = NULL;
			mptr->tptr = lptr;
			mptr->carr = LINECON;
			GameMessage(LineTypeName[lptr->type], 0, 0, CWhite, mptr);
			GSTP = SNDCONNECT;
			return 1;
		}
	return 0;
}

//-------------------------- Clonk Con Functions ------------------------------

extern void TowerVhcElevation(STRUCTYPE *cstrc);

BYTE Check4JumpStructureAccess(STRUCTYPE *tstrc, MANTYPE *mptr, BYTE plr)
{
	if (!tstrc) return 0;
	if (tstrc->con >= 1000) // Complete structs only
		switch (tstrc->type)
		{
		case STCASTLE:
			if (NotHostile(tstrc->owner, plr))
				if (Inside(mptr->x + 4 - (tstrc->x + 10), -3, +3) || (Crew[plr].ConCnt > 1))
				{
					mptr->tx = tstrc->x + 6;
					// Activate castle gate
					if (tstrc->p == 0) { tstrc->p = 1; EventCall(201); }
					return 1;
				}
			break;
		case STTOWER: // Check for tower elevation
			if (NotHostile(plr, tstrc->owner))
				if (Inside(mptr->x + 4 - (tstrc->x + 8), -2, +2) && Inside(mptr->y - (tstrc->y + 11), -6, +4))
				{
					mptr->act = MAWALK;
					mptr->x = tstrc->x + 4; mptr->y = tstrc->y - 5; mptr->tx = mptr->x;
					TowerVhcElevation(tstrc);
					EventCall(206);
					return 1;
				}
			break;
		}
	return 0;
}

extern void CheckOilCollection(STRUCTYPE *tstrc);

BYTE Check4StopStructureAccess(STRUCTYPE *tstrc, MANTYPE *mptr, BYTE plr, BYTE stand)
{
	if (!tstrc) return 0;
	if (tstrc->con >= 1000) // Complete structs
		switch (tstrc->type)
		{
		case STOILPOWER:
			if (mptr->carr == OILBARREL)
			{
				mptr->tx = tstrc->x + 1 + random(6);
				CheckOilCollection(tstrc);
				return 1;
			}
			break;
		case STTOWER:
			// Trigger gate
			if (Inside(mptr->x + 4 - (tstrc->x + 8), -5, +5))
				if (stand)
					if (NotHostile(tstrc->owner, plr))
						if ((tstrc->p == 0) || (tstrc->p == 16))
						{
							tstrc->p++; GSTP = SNDGATE; EventCall(207);
						}
			// Group collect to tower
			if (Crew[plr].ConCnt > 1)
				if (mptr->y > tstrc->y + 5)
					if (!Inside(mptr->x + 4 - tstrc->x, 3, 12))
						mptr->tx = BoundBy(tstrc->x + 4 + random(8) - 4, 0, 311);
			break;
		}
	else // Structs under construction
		if (tstrc->type < STCACTUS)
		{
			mptr->tx = tstrc->x + 1 + random(6);
			// Activate Crew4Con check waiting
			if (tstrc->p == 0) tstrc->p = 1;
			return 1;
		}
	return 0;
}

void ClonkBaseCon(MANTYPE *mptr, BYTE plr, BYTE con, VEHICTYPE *tvhc, STRUCTYPE *tstrc, int par = -1)
{                      // con: 0 Left 1 Stop 2 Right 3 Target
	BYTE standing = (mptr->x == mptr->tx);
	int dirch;

	// Change target into stop
	if (con == 3) if (Inside(par - (mptr->x + 4), -8, +8)) con = 1;

	switch (mptr->act)
	{
	case MADIG:

		if (con == 3) // Change target into left/right
		{
			if (par > (mptr->x + 4) + 8) con = 2;
			if (par < (mptr->x + 4) - 8) con = 0;
		}

		switch (con)
		{
		case 0: mptr->xdir = Max(mptr->xdir - 1, -3); break;
		case 1: ManWalk(mptr, 1); break;
		case 2: mptr->xdir = Min(mptr->xdir + 1, 3); break;
		}
		break;
	case MABRIDGE: if (con == 1) ManWalk(mptr, 1); break;
	case MABUILD:
		if (con == 1)
		{
			((STRUCTYPE*)mptr->tptr)->p = 0; // Eliminate worker waiting phase
			LoseTPtr(mptr); ManWalk(mptr, 1);
		}
		break;
	case MASWIM:
		switch (con)
		{
		case 0: mptr->tx = 0;    break;
		case 1:
			mptr->ydir = 15;
			if (par != -1) mptr->tx = par; // MouseCon knows stop + target
			break;
		case 2: mptr->tx = 311;  break;
		case 3: mptr->tx = par;  break;
		}
		break;
	case MAPUSH:
		tvhc = (VEHICTYPE*)mptr->tptr;

		if (con == 3) // Change target into left/right/stop
		{
			if (par > (mptr->x + 4) + 8) con = 2;
			if (par < (mptr->x + 4) - 8) con = 0;
			if (Inside(par - ((VEHICTYPE*)mptr->tptr)->x, 0, 12)) con = 1;
		}

		switch (con)
		{
		case 0:
			dirch = -1; if (tvhc->data3) dirch = +1; // For crossbow
			if ((Abs(mptr->del) < 2) && (tvhc->type == VHCROSSBOW) && (tvhc->VHDXBowDir + 15 * dirch < 100))
				tvhc->VHDXBowDirCh = 10 + dirch;
			else
				mptr->del = -2;
			break;
		case 1:
			if (Abs(mptr->del) == 2) mptr->del = Sign(mptr->del);
			else { LoseTPtr(mptr); ManWalk(mptr, 1); Crew[plr].DontPushDelay = 150; GSTP = SNDGRAB; }
			break;
		case 2:
			dirch = +1; if (tvhc->data3) dirch = -1; // For crossbow
			if ((Abs(mptr->del) < 2) && (tvhc->type == VHCROSSBOW) && (tvhc->VHDXBowDir + 15 * dirch < 100))
				tvhc->VHDXBowDirCh = 10 + dirch;
			else
				mptr->del = +2;
			break;
		}
		break;
	case MAWALK:
		switch (con)
		{
		case 0: mptr->tx = 0; break;
		case 1:
			// Individual may have tstrc/tvhc though group doesn't
			if (!tvhc) tvhc = OnWhichVehic(mptr->x + 4, mptr->y + 4, NULL);
			if (!tstrc) tstrc = OnWhichStruct(mptr->x + 4, mptr->y + 5);
			// Regular stop targeting
			mptr->tx = mptr->x;
			if (par != -1) mptr->tx = par; // MouseCon knows stop + target
			// Group collect
			if (Crew[plr].ConCnt > 1)
			{
				if (!Inside(mptr->tx - Crew[plr].AveX, -8, +8))
					mptr->tx = BoundBy(Crew[plr].AveX + random(17) - 8, 0, 311);
				// Vehic
				if (tvhc)
					if (!Inside(mptr->x - tvhc->x, -1, +6))
						mptr->tx = BoundBy(tvhc->x - 1 + random(6), 0, 311);
			}
			// Check for structure access
			Check4StopStructureAccess(tstrc, mptr, plr, standing);
			break;
		case 2: mptr->tx = 311; break;
		case 3: mptr->tx = par; break;
		}
		break;
	default: // other (?) ->fly,...
		switch (con)
		{
		case 0: mptr->tx = 0;   if (mptr->act == MAFLY) mptr->del = 0; break;
		case 1: mptr->tx = mptr->x; break;
		case 2: mptr->tx = 311; if (mptr->act == MAFLY) mptr->del = 0; break;
		case 3: mptr->tx = par; if (mptr->act == MAFLY) mptr->del = 0; break;
		}
		break;
	}
}

void ManThrow(MANTYPE *mptr, int tx, int ty, int xdir, int ydir, int plr)
{
	int rtype, rphase;
	rtype = mptr->carr; rphase = mptr->carrp;

	if (Inside(rtype, ARROW, BARROW))
	{
		rphase = 1; mptr->carrp--; Crew[plr].RedrStB = 1;
		xdir *= 1.8; ydir = ((ydir - 20)*1.8) + 20;
	}
	if (rtype == TFLINT) rphase = 50 + random(20);
	if (rtype == FARROW) rphase = 10 + 200;

	NewRock(mptr->x + tx, mptr->y + ty, RAFLY, rtype, xdir, ydir, rphase, plr);

	if (Inside(mptr->carr, FLINT, FBOMB)) mptr->mi->exp += 20 + 5 * (mptr->carr - FLINT);

	if (!(Inside(mptr->carr, ARROW, BARROW) && (mptr->carrp > 0))) mptr->carr = NOROCK;
}

void ClonkThrowCon(MANTYPE *mptr, BYTE plr, int par)
{
	VEHICTYPE *tvhc;
	BYTE drop = 0;
	BYTE *xbtload;

	if (mptr->tptr && (mptr->act != MAPUSH)) return; // Not if lining or building

	if ((Crew[plr].DropCount > 100) || (par == 1))
	{
		drop = 1; Crew[plr].DontPickUp = 2; Crew[plr].RedrStB = 1;
	}

	switch (mptr->act)
	{
	case MAWALK:
		if (mptr->carr > NOROCK)
		{
			if (drop) // Drop Rock
			{
				NewRock(mptr->x + 4 - 4 * (mptr->xdir < 0), mptr->y + 2, RADEAD, mptr->carr, 0, 0, mptr->carrp, plr);
				mptr->carr = NOROCK;
			}
			else // Throw Rock
			{
				mptr->act = MATHROW; mptr->phase = 0;
				ManThrow(mptr, +4, -2, (mptr->xdir < 0) ? -10 : +10, 0, plr);
			}
		}
		break;
	case MASWIM:
		if (mptr->carr > NOROCK) // SwimDrop Rock
			ManThrow(mptr, +4 + 2 * Sign(mptr->xdir), +8, 10 * Sign(mptr->xdir), 25, plr);
		break;
	case MAPUSH:
		tvhc = (VEHICTYPE*)mptr->tptr; // Load/Fire Catapult/Crossbow

		if (tvhc->type == VHCATAPULT)
		{
			if (tvhc->VHDCatPhase == 0) // Cat at rest, might load
			{
				if ((mptr->carr > NOROCK) && (mptr->carr < ARROW))
				{
					tvhc->VHDCatPhase = 1; tvhc->VHDCatLoad = mptr->carr;
					tvhc->VHDCatFPower = 0;
					if (Inside(mptr->carr, FLINT, FBOMB)) mptr->mi->exp += 30 + 5 * (mptr->carr - FLINT);
					mptr->carr = NOROCK;
				}
			}
			else
				if (Inside(tvhc->VHDCatPhase, 2, 50)) // Charging, fire
				{
					tvhc->VHDCatPhase = 51 + 8 * (50 - tvhc->VHDCatPhase) / 50;
					GSTP = SNDCATAPULT;
				}
		}
		if (tvhc->type == VHCROSSBOW)
		{
			if (Inside(mptr->carr, ARROW, BARROW)) // Load crossbow
			{
				xbtload = &(tvhc->VHDXBowLoadA);
				if (mptr->carr == FARROW) xbtload = &(tvhc->VHDXBowLoadF);
				if (mptr->carr == BARROW) xbtload = &(tvhc->VHDXBowLoadB);

				if (*xbtload < 200)
				{
					(*xbtload) += mptr->carrp;
					mptr->carr = NOROCK;
					GSTP = SNDCONNECT;
					sprintf(OSTR, "%d/%d/%d", tvhc->VHDXBowLoadA, tvhc->VHDXBowLoadF, tvhc->VHDXBowLoadB);
					GameMessage(OSTR, tvhc->x + 6, tvhc->y - 15, CWhite, NULL);
					tvhc->VHDXBowJustL = 1;
				}
			}
			else // Fire crossbow if possible
			{
				if (tvhc->VHDXBowPhase == 0)
					if (tvhc->VHDXBowLoadA + tvhc->VHDXBowLoadF + tvhc->VHDXBowLoadB > 0)
						if (!tvhc->VHDXBowJustL)
						{
							tvhc->VHDXBowPhase = 1;
							GSTP = SNDXBOW;
							EventCall(203);
						}
			}
		}
		break;
	}
}

void ClonkJumpCon(MANTYPE *mptr, BYTE plr, STRUCTYPE *tstrc, int par)
{
	int jdir;
	if (mptr->act == MAWALK)
	{
		if (!Check4JumpStructureAccess(tstrc, mptr, plr))
		{
			// Regular jump
			if (!Inside(GBackPix(mptr->x + 4, mptr->y - 2), CSolidL, CSolidH))
			{
				jdir = Sign(mptr->xdir);
				if (par > -1) jdir = Sign(par - mptr->x);
				if (!jdir) jdir = Sign(mptr->tx - mptr->x); if (!jdir) jdir = 1;
				ManFly(mptr, jdir, -7, (mptr->tx == mptr->x));
			}
		}
	}
	if ((mptr->act == MAPUSH) || (mptr->act == MASWIM))
	{
		Check4JumpStructureAccess(tstrc, mptr, plr);
	}
}

BYTE NeedsElevatorShaft(VEHICTYPE *tvhc)
{
	int cnt, cnt2, sldcnt;
	sldcnt = 0;
	for (cnt = 0; cnt < 5; cnt++)
		for (cnt2 = 2; cnt2 < 14; cnt2++)
			if (Inside(GBackPix(tvhc->x + cnt2, tvhc->y + 13 + cnt), CSolidL, CSolidH))
				sldcnt++;
	if (sldcnt > 5) return 1;
	return 0;
}

void DoubleDigSpecial(MANTYPE *mptr, BYTE plr)
{
	VEHICTYPE *tvhc = OnWhichVehic(mptr->x + 4, mptr->y + 4, NULL);
	if (!mptr) return;
	switch (mptr->carr)
	{
	case LOAM: case STEEL: // Bridge
		ManWalk(mptr, 1);
		ActivateMenu(plr, CMBRIDGE);
		return;
	case CONKIT: // Construct
		ManWalk(mptr, 1);
		ActivateMenu(plr, CMBUILD);
		return;
	case LINECON: // Try line con
		if (LineConstruction(mptr, plr)) { ManWalk(mptr, 1); return; }
		break;
	case NOROCK: // Try line pick up
		break;
	}
	// Else, try elevator shafting
	if (tvhc && (tvhc->type == VHELEVATOR))
		if (NeedsElevatorShaft(tvhc))
		{
			tvhc->VHDDrilling = 1;
			GameMessage("Drilling|elevator shaft", tvhc->x + 8, tvhc->y - 8, CWhite, NULL);
			GSTP = SNDDRILL;
			ManWalk(mptr, 1);
			EventCall(205);
			return;
		}
	// Else try line pickup
	if (mptr->carr == NOROCK)
		if (LinePickUp(mptr, plr)) { ManWalk(mptr, 1); return; }
}

void ClonkDDSpecialCon(MANTYPE *mptr, BYTE plr)
{
	if (mptr->mi->rank < 1) { ManCanDoMsg(mptr, "not perform|special actions"); return; }
	GameMessage("Special action", -1, plr, CWhite, NULL);
	DoubleDigSpecial(mptr, plr);
}

void ClonkDigCon(MANTYPE *mptr, BYTE plr, int dir)
{
	if (mptr->mi->rank < 1) { ManCanDoMsg(mptr, "not dig"); return; }
	switch (mptr->act)
	{
	case MAWALK:
		ManDig(mptr, dir); Crew[plr].EDig = 0;
		return;
	case MADIG:
		if (mptr->del == 0) // Liquid earth release (digging a while)
		{
			if (BSA.RuleSet > R_EASY) Toggle(Crew[plr].EDig); return;
		}
		else // "Double-click" specials (dig-begin)
			DoubleDigSpecial(mptr, plr);
		break;
	case MASWIM: case MAFLY: // "Action" line con
	  // Try line con
		if (mptr->carr == LINECON)
			if (LineConstruction(mptr, plr)) return;
		// Else, try line pick up
		if (mptr->carr == NOROCK)
			if (LinePickUp(mptr, plr)) return;
		break;
	}
}

extern void DrawStructBase(STRUCTYPE *strc, BYTE news);
extern void FreeStructBack(STRUCTYPE *strc);

void ClonkBuildCon(MANTYPE *mptr, BYTE plr, int type)
{
	STRUCTYPE *nstrc;
	int ctx, cty;
	if (!mptr || (mptr->act >= MADEAD))
	{
		GameMessage("No Clonk|selected", -1, plr, CRed, NULL); GSTP = SNDERROR;
		return;
	}
	if (mptr->act != MAWALK || (mptr->carr != CONKIT))
	{
		sprintf(OSTR, "%s can|not build here", mptr->mi->name);
		GameMessage(OSTR, 0, 0, CWhite, mptr); GSTP = SNDERROR;
		return;
	}
	if (type == STNOSTRUCT) return;
	ctx = mptr->x + 3; cty = mptr->y + 8;
	LPage(BackPage); // ??
	if (ConstructionCheck(type, ctx, cty)) // Okay -> build
	{
		mptr->carr = NOROCK;
		mptr->mi->exp += 25;
		nstrc = NewStruct(type, ctx, cty, 0, 0, plr);
		if (nstrc)
		{
			DrawStructBase(nstrc, 1);
			FreeStructBack(nstrc);
			sprintf(OSTR, "Constructing|%s", StructTypeName[type]);
			GameMessage(OSTR, 0, 0, CWhite, mptr);
			EventCall(208);
		}
	}
	else // Not okay -> cancel
	{
		GameMessage("Construction|impossible here", 0, 0, CWhite, mptr);
		GSTP = SNDERROR;
	}
}

void ClonkBridgeCon(MANTYPE *mptr, int plr, int dir)
{
	if (!mptr || (mptr->act >= MADEAD))
	{
		GameMessage("No Clonk|selected", -1, plr, CRed, NULL); GSTP = SNDERROR;
		return;
	}
	if ((mptr->act != MAWALK) || Inside(GBackPix(mptr->x + 3, mptr->y + 9), CVhcL, CVhcH))
	{               // check for steel/loam carr
		sprintf(OSTR, "%s can|not build here", mptr->mi->name);
		GameMessage(OSTR, 0, 0, CWhite, mptr); GSTP = SNDERROR;
		return;
	}
	if ((dir == +1) && (mptr->xdir < 0)) dir = -1;
	mptr->act = MABRIDGE; // -> ManBridge()
	mptr->ydir = 0;
	mptr->xdir = dir;
	mptr->phase = 0;
	mptr->del = (mptr->carr == STEEL);
	mptr->carr = NOROCK;
	mptr->mi->exp += 20;
	EventCall(250 + Abs(dir) - 1);
}

void ClonksCeaseAction(int plr)
{
	MANTYPE *cman;
	if (!UserPref.CeaseAction) return;
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
	{
		cman->tx = cman->x;
		if (cman->act == MADIG) ManWalk(cman, 1);
		if (cman->act == MAPUSH) cman->del = Sign(cman->del);
	}
}

//-------------------------- Plr Cursor Control ------------------------------

void AvoidNoCon(BYTE plr) // If no man is con, hiexp con on
{
	MANTYPE *cman, *hiexp = NULL;
	BYTE anycon = 0;
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
		if (cman->act < MADEAD)
		{
			if (cman->con) anycon = 1;
			if (!hiexp || (cman->mi->exp > hiexp->mi->exp)) hiexp = cman;
		}
	if (!anycon) if (hiexp) hiexp->con = 1;
}

void CursorAdjust(BYTE plr) // Set to highest rank con (NULL if none)
{
	MANTYPE *cman, *hirank = NULL;

	AvoidNoCon(plr);

	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
		if (cman->con && (cman->act < MADEAD))
			if (!hirank || (cman->mi->rank > hirank->mi->rank))
				hirank = cman;
	Crew[plr].Cursor = hirank;
	// After any cursor modification
	ClonksCeaseAction(plr);
	Crew[plr].CursorFlash = CFlashTime;
}

void CursorNext(BYTE plr)
{
	MANTYPE *from;
	if (!Crew[plr].Cursor) // Set to any (first) man if empty
	{
		if (!Crew[plr].FirstMan) return;
		Crew[plr].Cursor = Crew[plr].FirstMan;
	}
	from = Crew[plr].Cursor;
	do
	{
		Crew[plr].Cursor = Crew[plr].Cursor->next;
		if (!Crew[plr].Cursor) Crew[plr].Cursor = Crew[plr].FirstMan;
	} while ((Crew[plr].Cursor->act >= MADEAD) && (Crew[plr].Cursor != from));
	// After any cursor modification
	ClonksCeaseAction(plr);
	Crew[plr].CursorFlash = CFlashTime;
}

void CursorPrev(BYTE plr)
{
	MANTYPE *cman, *prev, *from;
	if (!Crew[plr].Cursor) // Set to any (first) man if empty
	{
		if (!Crew[plr].FirstMan) return;
		Crew[plr].Cursor = Crew[plr].FirstMan;
	}
	from = Crew[plr].Cursor;
	do
	{
		prev = NULL;
		for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
			if (cman->next == Crew[plr].Cursor) prev = cman;
		Crew[plr].Cursor = prev;
		if (!Crew[plr].Cursor)
		{
			for (cman = Crew[plr].FirstMan; cman && cman->next; cman = cman->next);
			Crew[plr].Cursor = cman;
		}
	} while ((Crew[plr].Cursor->act >= MADEAD) && (Crew[plr].Cursor != from));
	// After any cursor modification
	Crew[plr].CursorFlash = CFlashTime;
	ClonksCeaseAction(plr);
}

void CursorTo(BYTE plr, MANTYPE *nman)
{
	Crew[plr].Cursor = nman;
	// After any cursor modification
	Crew[plr].CursorFlash = CFlashTime;
	ClonksCeaseAction(plr);
}

//-------------------------- Clonk Con Control -------------------------------

void AllClonksCon(BYTE plr, BYTE def) // def: 0 Off  1 On  2 Toggle
{
	MANTYPE *cman;
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
	{
		if (def == 2) Toggle(cman->con);
		else cman->con = def;
	}
	Crew[plr].ConFlash = CFlashTime;
}

void CursorSelectsSingleCon(BYTE plr)
{
	if (!Crew[plr].Cursor) return;        // Not if not necessary
	if ((Crew[plr].ConCnt == 1) && Crew[plr].Cursor->con) return;
	AllClonksCon(plr, 0);
	Crew[plr].Cursor->con = 1;
	CursorAdjust(plr);
}

void CursorTogglesSingleCon(BYTE plr)
{
	if (!Crew[plr].Cursor) return;
	Toggle(Crew[plr].Cursor->con);
	Crew[plr].ConFlash = CFlashTime;
}

void ToggleAllCon(BYTE plr)
{
	AllClonksCon(plr, 2);
	CursorAdjust(plr);
}

void SelectAllCon(BYTE plr)
{
	AllClonksCon(plr, 1);
	CursorAdjust(plr);
	GameMessage("All selected", -1, plr, CWhite, NULL);
}

void CheckVehicSelectsCon(BYTE plr)
{
	void *tptr = NULL;
	MANTYPE *cman;
	BYTE mfd = 0;
	// Any con pushing?
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
		if (cman->con)
			if (cman->tptr && (cman->act == MAPUSH)) tptr = cman->tptr;
	// Yes, all those pushing con, all those not pushing no-con
	if (tptr)
		for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
			if ((cman->act == MAPUSH) && (cman->tptr == tptr))
			{
				if (cman->con == 0) mfd = 1; cman->con = 1;
			}
			else
			{
				if (cman->con == 1) mfd = 1; cman->con = 0;
			}
	if (mfd)
	{
		CursorAdjust(plr);
		Crew[plr].ConFlash = CFlashTime;
	}
}

//-------------------------- ClonkControl Func ------------------------------
						   // com:
void ClonkControl(BYTE plr, BYTE com, int par) // 0 left    1 stop  2 right
{                                            // 3 throw   4 jump  5 dig
	MANTYPE *cman;                               // 6 bridge  7 build 8 x-target
	BYTE allstand = 1;                             // 9 ddspec

	// DontPickUp
	if (Crew[plr].DontPickUp > 0) { Crew[plr].DontPickUp--; Crew[plr].RedrStB = 1; }

	// Single clonk if dig, bridge, build
	if (Inside(com, 5, 7) || (com == 9))
		CursorSelectsSingleCon(plr);

	// Group Target Vehic & Struct
	VEHICTYPE *tvhc = OnWhichVehic(Crew[plr].AveX + 4, Crew[plr].AveY + 4, NULL);
	STRUCTYPE *tstrc = OnWhichStruct(Crew[plr].AveX + 4, Crew[plr].AveY + 4);

	// Exec con men
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
		if (cman->con)
			switch (com)
			{
			case 0: case 1: case 2:
				if ((cman->act != MAWALK) || (cman->tx != cman->x)) allstand = 0;
				ClonkBaseCon(cman, plr, com, tvhc, tstrc);
				break;
			case 3: ClonkThrowCon(cman, plr, par); break;
			case 4: ClonkJumpCon(cman, plr, tstrc, par); break;
			case 5: ClonkDigCon(cman, plr, 1); break;
			case 6: ClonkBridgeCon(cman, plr, par); break;
			case 7: ClonkBuildCon(cman, plr, par); break;
			case 8:
				if ((cman->act != MAWALK) || !Inside(cman->tx - cman->x, -8, 8)) allstand = 0;
				if (!Inside(par - (cman->x + 4), -8, +8)) allstand = 0;
				ClonkBaseCon(cman, plr, 3, tvhc, tstrc, par);
				break;
			case 9: ClonkDDSpecialCon(cman, plr); break;
			}

	// Specials on "stop"-base con:

	// All stand, stop means vehic con
	if ((com == 1) || (com == 8)) if (allstand)
		if (tvhc) { ConVehic(tvhc, plr); return; } // <- to 'else' DropCount
	  // Drop (no drop with mouse con)
	if (com == 1) Crew[plr].DropCount = Min(Crew[plr].DropCount + 60, 200);
	else Crew[plr].DropCount = 0;
}

//--------------------------- Menu Functions ---------------------------------

extern MANINFO *NewMan2List(MANINFO **crew, char *name, int rank, int exp, int rnds, int rean);
extern MANTYPE *NewMan(MANTYPE **firstman, MANINFO *mi, BYTE con, BYTE type, int x, int y, int tx, BYTE act, int phase, int strn, int ydir, int xdir, int carr, void *tptr);
extern char *GetClonkName(void); // temp

void *GetHomeTPtr(BYTE plr, BYTE nomsg)
{
	VEHICTYPE *cvhc;
	STRUCTYPE *cstrc;
	void *hometptr = NULL;
	if (BSA.Realism.CstHome)
	{
		for (cstrc = FirstStruct; cstrc; cstrc = cstrc->next)      // with flag only?
			if ((cstrc->type == STCASTLE) && (cstrc->owner == plr))
				if (cstrc->con >= 1000) break;
		if (!cstrc && !nomsg) { GameMessage("No order|without a castle", -1, plr, CWhite, NULL); GSTP = SNDERROR; }
		else hometptr = cstrc;
	}
	else
	{
		for (cvhc = FirstVehic; cvhc; cvhc = cvhc->next)
			if ((cvhc->type == VHBALLOON) && (cvhc->owner == plr)) break;
		if (!cvhc && !nomsg) { GameMessage("No order|without a balloon", -1, plr, CWhite, NULL); GSTP = SNDERROR; }
		else hometptr = cvhc;
	}
	return hometptr;
}

extern BYTE LostCaptureFlag(BYTE plr);

void PlayerRckOrder(BYTE plr, int oid)
{
	int rid = RockOrder[oid];

	if (!GetHomeTPtr(plr, 0)) return;

	if (LostCaptureFlag(plr))
	{
		GameMessage("No order with|flag lost", -1, plr, CWhite, NULL); GSTP = SNDERROR; return;
	}

	if ((BSA.RuleSet < 2) && RockOrderRad[oid]) return;

	if (rid > NOROCK)
		if (Crew[plr].Wealth >= RockValue[rid])
			if (Crew[plr].RckProd.Store[oid] > 0)
			{
				Crew[plr].RockStorage[rid]++;
				Crew[plr].RckProd.Store[oid]--;
				Crew[plr].Wealth -= RockValue[rid];
				Crew[plr].RedrStB = 1; // Cash $$$ sound
				EventCall(400 + rid);
				return;
			}
	GSTP = SNDERROR;
}

extern void BlowUpBalloon(VEHICTYPE *tvhc);

BYTE ManIsInCrewList(MANINFO *mani, MANTYPE *cman)
{
	for (cman; cman; cman = cman->next)
		if (cman->mi == mani) return 1;
	return 0;
}

void PlrVhcOrderNewMan(BYTE plr, void *hometptr, BYTE autoc)
{
	MANINFO *nmi;
	MANTYPE *nman;
	if (BSA.PCrew)
	{
		// Use Stammcrew man if available
		for (nmi = BSA.Plr[plr].Info->crew; nmi; nmi = nmi->next)
			if (!nmi->dead)
				if (!ManIsInCrewList(nmi, Crew[plr].FirstMan))
					break;
		// Enter new if necessary
		if (!nmi) nmi = NewMan2List(&(BSA.Plr[plr].Info->crew), GetClonkName(), 0, 0, 0, 0);
	}
	else
		nmi = NewMan2List(&(BSA.Plr[plr].TCrew), GetClonkName(), 0, 0, 0, 0);

	if (nmi) nman = NewMan(&Crew[plr].FirstMan, nmi, 0, MNMAN, 0, 0, 0, MAHOME, 0, 100, 0, 0, NOROCK, hometptr);
	if (autoc) if (nman) GameMessage("A new recruit", 0, 0, CWhite, nman);
}

void PlayerVhcOrder(BYTE plr, int oid, BYTE autoc)
{
	VEHICTYPE *nvhc;
	void *hometptr;
	int hirank = HighRank(plr);

	if ((BSA.RuleSet < 2) && (oid > 3)) return;

	if (!((oid == 5) && !BSA.Realism.CstHome))
		if (!(hometptr = GetHomeTPtr(plr, autoc))) return;

	if (LostCaptureFlag(plr))
	{
		GameMessage("No order|with flag lost", -1, plr, CWhite, NULL); GSTP = SNDERROR; return;
	}

	if (Crew[plr].Wealth >= VhcOrderPrice[oid])
	{
		switch (oid)
		{
		case 0: // New Clonk
			if (Crew[plr].ManCnt - 1 + 1 > hirank * 5)
			{
				if (hirank < 10) sprintf(OSTR, "Without a %s|no additional clonks|may be ordered", RankName[hirank + 1]);
				else sprintf(OSTR, "No additional clonks|may be ordered");
				if (!autoc) GameMessage(OSTR, -1, plr, CWhite, NULL);
				return;
			}
			PlrVhcOrderNewMan(plr, hometptr, autoc);
			break;
		case 1: // Lorry
			NewVehic(VHLORRY, -1, 0, 0, 0, 0, 0, plr, hometptr);
			break;
		case 2: // Catapult
			NewVehic(VHCATAPULT, -1, 0, 0, 0, 0, 0, plr, hometptr);
			break;
		case 3: // Sail Boat
			NewVehic(VHSAILBOAT, -1, 0, 0, 0, 0, 0, plr, hometptr);
			break;
		case 4: // Crossbow
			nvhc = NewVehic(VHCROSSBOW, -1, 0, 0, 0, 0, 0, plr, hometptr);
			if (nvhc) nvhc->VHDXBowDir = 65;
			break;
		case 5: // Balloon
			for (nvhc = FirstVehic; nvhc; nvhc = nvhc->next)
				if ((nvhc->type == VHBALLOON) && (nvhc->owner == plr)) break;
			if (nvhc) // Balloon exists, blow up
			{
				nvhc->VHDBallCrash = 1; // No explosion
				// Balloon will be delayed as if shot down
			}
			else
				NewVehic(VHBALLOON, 0, -20, 159, -1, -1, 0, plr, NULL);
			break;
		}
		Crew[plr].Wealth -= VhcOrderPrice[oid];
		// Cash $$$ sound
	}
	else
		GSTP = SNDERROR;
}

void CursorCallBalloon(BYTE plr)         // not by recruit?
{
	VEHICTYPE *cvhc;
	if (BSA.RuleSet < 2) return;
	if (Crew[plr].Cursor && (Crew[plr].Cursor->act < MADEAD))
	{
		for (cvhc = FirstVehic; cvhc; cvhc = cvhc->next)
			if ((cvhc->type == VHBALLOON) && (cvhc->owner == plr))
			{
				cvhc->tptr = Crew[plr].Cursor;
				sprintf(OSTR, "Balloon to|%s", Crew[plr].Cursor->mi->name);
				GameMessage(OSTR, 0, 0, CWhite, Crew[plr].Cursor);
				return;
			}
		GameMessage("No balloon!", -1, plr, CRed, NULL);
	}
	else
		GameMessage("No Clonk|selected", -1, plr, CRed, NULL);
}

void GameConExecMenu(BYTE com, BYTE plr)
{
	int cnt;
	switch (Crew[plr].CMenu)
	{
	case CMMAIN: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		switch (com)
		{
		case 3: ActivateMenu(plr, CMRCKORDER); break;
		case 4: ActivateMenu(plr, CMVHCORDER); break;
		case 5: ActivateMenu(plr, CMCOMMAND); break;
		case 6: ClonkControl(plr, 9, 0); break;
		case 7: ClonkControl(plr, 3, 1); GameMessage("Drop", -1, plr, CWhite, NULL); ActivateMenu(plr, CMNOMENU); break;
		case 8: break;
		case 9: ActivateMenu(plr, CMNOMENU); break;
		}
		break;
	case CMRCKORDER: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		switch (com)
		{
		case 3: case 4: case 5:
			PlayerRckOrder(plr, Crew[plr].CMenuData + com - 3);
			break;
		case 6:
			Crew[plr].CMenuData -= 3;
			if (Crew[plr].CMenuData == -3) Crew[plr].CMenuData = RockOrderNum - 3;
			Crew[plr].RedrStB = 1;
			break;
		case 7: ActivateMenu(plr, CMNOMENU); break;
		case 8:
			Crew[plr].CMenuData += 3;
			if (Crew[plr].CMenuData == RockOrderNum) Crew[plr].CMenuData = 0;
			Crew[plr].RedrStB = 1;
			break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	case CMVHCORDER: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		switch (com)
		{
		case 3: case 4: case 5:
			PlayerVhcOrder(plr, Crew[plr].CMenuData + com - 3, 0);
			break;
		case 6:
			Crew[plr].CMenuData -= 3;
			if (Crew[plr].CMenuData == -3) Crew[plr].CMenuData = VhcOrderNum - 3;
			Crew[plr].RedrStB = 1;
			break;
		case 7: ActivateMenu(plr, CMNOMENU); break;
		case 8:
			Crew[plr].CMenuData += 3;
			if (Crew[plr].CMenuData == VhcOrderNum) Crew[plr].CMenuData = 0;
			Crew[plr].RedrStB = 1;
			break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	case CMHOSTILITY: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		switch (com)
		{
		case 3: case 4: case 5:
			if (BSA.Plr[com - 3].Col > -1) if (com - 3 != plr)
			{
				Toggle(Crew[plr].Hostile[com - 3]);
				sprintf(OSTR, "%s attack|%s!", Crew[plr].Hostile[com - 3] ? "" : "Don't ", ColorName[BSA.Plr[com - 3].Col]);
				GameMessage(OSTR, -1, plr, CWhite, NULL);
				GSTP = SNDTRUMPET;
				Crew[plr].RedrStB = 1;
			}
			break;
		case 6: break;
		case 7: ActivateMenu(plr, CMNOMENU); break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	case CMBRIDGE: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		switch (com)
		{
		case 6: ClonkControl(plr, 6, -3); ActivateMenu(plr, CMNOMENU); break;
		case 3: ClonkControl(plr, 6, -2); ActivateMenu(plr, CMNOMENU); break;
		case 4: ClonkControl(plr, 6, +1); ActivateMenu(plr, CMNOMENU); break;
		case 5: ClonkControl(plr, 6, +2); ActivateMenu(plr, CMNOMENU); break;
		case 8: ClonkControl(plr, 6, +3); ActivateMenu(plr, CMNOMENU); break;
		case 7: ActivateMenu(plr, CMNOMENU); break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	case CMBUILD: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		switch (com)
		{
		case 3: case 4: case 5:
			if (StructBuildRS[(com - 3) + Crew[plr].CMenuData] <= BSA.RuleSet)
				ClonkControl(plr, 7, StructBuild[(com - 3) + Crew[plr].CMenuData]);
			ActivateMenu(plr, CMNOMENU);
			break;
		case 6:
			if (Crew[plr].CMenuData > 0) { Crew[plr].CMenuData -= 3; Crew[plr].RedrStB = 1; }
			break;
		case 8:
			if (Crew[plr].CMenuData < StructBuildNum - 3) { Crew[plr].CMenuData += 3; Crew[plr].RedrStB = 1; }
			break;
		case 7: ActivateMenu(plr, CMNOMENU); break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	case CMSRCPIPE: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		ActivateMenu(plr, CMNOMENU);
		switch (com)
		{
		case 3:
			AbandonLine(Crew[plr].Cursor);
			GameMessage("line construction|cancelled", 0, 0, CWhite, Crew[plr].Cursor);
			break;
		case 4: DrillPipe(Crew[plr].Cursor, 1); break;
		case 5:
			Crew[plr].Cursor->carr = NOROCK; Crew[plr].Cursor->tptr = NULL;
			GameMessage("line dropped", 0, 0, CWhite, Crew[plr].Cursor);
			break;
		case 6: DrillPipe(Crew[plr].Cursor, 0); break;
		case 8: DrillPipe(Crew[plr].Cursor, 2); break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	case CMSURRENDER:
		switch (com)
		{
		case 4:
			BSA.Plr[plr].Eliminate = 2;
			Crew[plr].RedrStB = 1;
			GSTP = SNDSURREND;
			break;
		case 7: ActivateMenu(plr, CMNOMENU); break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	case CMCOMMAND: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		switch (com)
		{
		case 3: ActivateMenu(plr, CMHOSTILITY); break;
		case 4: ActivateMenu(plr, CMMAGIC); break;
		case 5: break;
		case 6: if (BSA.AllowSurrender) ActivateMenu(plr, CMSURRENDER); break;
		case 7: ActivateMenu(plr, CMNOMENU); break;
		case 8: CursorCallBalloon(plr); break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	case CMMAGIC: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		switch (com)
		{
		case 3: case 4: case 5:
			PlayerMagicCommand(plr, Crew[plr].CMenuData + com - 3);
			break;
		case 6:
			if (Crew[plr].CMenuData > 0) { Crew[plr].CMenuData -= 3; Crew[plr].RedrStB = 1; }
			break;
		case 7: ActivateMenu(plr, CMNOMENU); break;
		case 8:                           // v only six spells for now
			if (Crew[plr].CMenuData < MagicNum - 3 - 3) { Crew[plr].CMenuData += 3; Crew[plr].RedrStB = 1; }
			break;
		case 9: ActivateMenu(plr, CMMAIN); break;
		}
		break;
	}
}

//------------------------------ Mouse ---------------------------------------

extern long BackVPos;

void DragBoxSelection(void)
{
	int plr = MouseCon.player;
	MANTYPE *cman;
	if (MouseCon.x1 > MouseCon.x2) Swap(MouseCon.x1, MouseCon.x2);
	if (MouseCon.y1 > MouseCon.y2) Swap(MouseCon.y1, MouseCon.y2);
	for (cman = Crew[plr].FirstMan; cman; cman = cman->next)
		if (cman->act < MADEAD)
			if (Inside(cman->x + 4, MouseCon.x1, MouseCon.x2) && Inside(cman->y + 5 - BackVPos + 20, MouseCon.y1, MouseCon.y2))
				cman->con = 1;
			else
				cman->con = 0;

	CursorAdjust(plr);
	Crew[plr].ConFlash = CFlashTime;
}

int ManDigDir(MANTYPE *mptr, int mx, int my)
{
	int dsign = -1; if (mx > mptr->x + 4) dsign = +1;
	if (Inside(mptr->x + 4 - mx, -4, +4))
		if (my > mptr->y + 9) return 0;
		else if (my > mptr->y) return 1;
		if (Inside(my - mptr->y, 0, 10)) return dsign * 2;
		if (my < mptr->y) return dsign * 3;
		return dsign;
}

void TargetClick(void)                       // Mouse digging currently
{                                          // avoids regular ClonkControl
	int plr = MouseCon.player;                   // calling procedure
	MANTYPE *mptr = Crew[plr].Cursor;;
	int dir;

	if (mptr && (mptr->act == MADIG)) // Adjust dig
	{
		if (OnWhichClonk(plr, MouseCon.x1, MouseCon.y1 - 20 + BackVPos) == mptr)
		{
			ClonkControl(plr, 1, 0);
			return;
		}
		dir = ManDigDir(mptr, MouseCon.x1, MouseCon.y1 - 20 + BackVPos);
		mptr->xdir = dir;
		return;
	}

	if (Crew[plr].MsWalkDClick > 0) // Double -> Activate dig
	{
		if (mptr && (mptr->act == MAWALK))
		{
			CursorSelectsSingleCon(plr);
			dir = ManDigDir(mptr, MouseCon.x1, MouseCon.y1 - 20 + BackVPos);
			ManDig(mptr, dir);
			Crew[plr].EDig = 0; // For now, no chance for EDigging
		}
		return;
	}

	// Else, regular target click                   // Double click count down
	Crew[plr].MsWalkDClick = 5 + 25 * (100 - UserPref.DoubleDigSpeed) / 100;
	ClonkControl(plr, 8, MouseCon.x1 - 4);
}

void JumpClick(void)
{
	ClonkControl(MouseCon.player, 4, MouseCon.x1 - 4);
	MouseCon.cursor = 4;
}

void ThrowClick(void)
{
	ClonkControl(MouseCon.player, 3, 0);
	MouseCon.cursor = 4;
}

void InitMouseCon(void)
{
	int cnt;
	MouseCon.status = 0;
	MouseCon.cursor = 0;
	if (MSON)
		for (cnt = 0; cnt < MaxGamePlr; cnt++)
			if ((BSA.Plr[cnt].Col > -1) && !BSA.Plr[cnt].Eliminate)
				if (BSA.Plr[cnt].Con == 3)
				{
					MouseCon.status = 1; MouseCon.player = cnt;
					SetMouseRange(0, 20, 639, 179);
					if (Crew[MouseCon.player].CMenu)
						ActivateMenu(MouseCon.player, 0);
				}
	// error if no mouse?
}

BYTE DragBoxLarge(void)
{
	const int sensiv = 5;
	if (!Inside(MouseCon.x1 - MouseCon.x2, -sensiv, sensiv)) return 1;
	if (!Inside(MouseCon.y1 - MouseCon.y2, -sensiv, sensiv)) return 1;
	return 0;
}

void MouseControl(void)
{
	MANTYPE *onman = NULL;
	int plr = MouseCon.player;
	int mcom;

	if (BSA.Plr[plr].Eliminate) { MouseCon.status = 0; return; }

	Crew[plr].DropCount = 0;

	switch (MouseCon.cursor)
	{
	case 0: // Walk cross hair -----------------------------------------------
		MouseCon.x1 = MouseX();
		MouseCon.y1 = MouseY();
		MouseCon.but1 = Mousebut();
		switch (MouseCon.but1)
		{
		case 0:
			if (!Tick3) onman = OnWhichClonk(plr, MouseCon.x1, MouseCon.y1 - 20 + BackVPos);
			if (onman && (onman != Crew[plr].Cursor)) // Switch to selector
			{
				CursorTo(plr, onman);
				MouseCon.cursor = 5;
			}
			if (MouseCon.y1 >= 180) // Switch to menu
				if (BSA.RuleSet > R_EASY)
				{
					MouseCon.cursor = 1;
					// The original version teleported the mouse around and set limits to where the mouse can move here.
					// It is not necessary and won't work properly on modern systems so it was deleted.
					ActivateMenu(plr, 1);
				}
			break;
		case 1: // Switch to Click/DragBox
			MouseCon.cursor = 2;
			MouseCon.x2 = MouseCon.x1; MouseCon.y2 = MouseCon.y1;
			break;
		case 2: ThrowClick(); break;
		case 4: JumpClick(); break;
		}
		break;
	case 1: // Menu arrow -----------------------------------------------------
		MouseCon.x1 = MouseX();
		MouseCon.y1 = MouseY();
		MouseCon.but1 = Mousebut();
		switch (MouseCon.but1)
		{
		case 0:
			if (MouseCon.y1 < 180) // Switch menu off
				ActivateMenu(plr, 0); // Includes MouseFromMenu
			break;
		case 1: // Menu click to delay
			MouseCon.cursor = 3;
			break;
		}
		break;
	case 2: // DragBox -------------------------------------------------------
		MouseCon.x2 = MouseX();
		MouseCon.y2 = MouseY();
		MouseCon.but2 = Mousebut();
		switch (MouseCon.but2)
		{
		case 0: // DragBox released
			if (DragBoxLarge()) DragBoxSelection();
			else TargetClick();
			MouseCon.cursor = 0;
			MouseCon.x1 = MouseCon.x2; MouseCon.y1 = MouseCon.y2;
			break;
		case 3: // Two-Button-Jump
			if (UserPref.TwoButtonJump) JumpClick();
			break;
		}
		break;
	case 3: // Button click -----------------------------------------------------
		if (Mousebut() == 0)
		{
			MouseCon.cursor = 1;
			for (mcom = 0; mcom < 6; mcom++)
				if (Inside(MouseCon.x1 - (Crew[plr].StBX + 28 + 13 * (mcom % 3)), 0, 11))
					if (Inside(MouseCon.y1 - (180 + 1 + 9 * (mcom / 3)), 0, 7))
						break;
			if (mcom < 6) GameConExecMenu(3 + mcom, plr);
		}
		break;
	case 4: // Click release delay to cursor 0 -----------------------------------------------------
		if (Mousebut() == 0) MouseCon.cursor = 0;
		break;
	case 5: // Selector ------------------------------------------------------
		MouseCon.x1 = MouseX();
		MouseCon.y1 = MouseY();
		MouseCon.but1 = Mousebut();
		switch (MouseCon.but1)
		{
		case 0:
			onman = OnWhichClonk(plr, MouseCon.x1, MouseCon.y1 - 20 + BackVPos);
			if (!onman) // Switch to cross
			{
				MouseCon.cursor = 4;
				CursorAdjust(plr);
			}
			else
				if (Crew[plr].Cursor != onman) CursorTo(plr, onman);
			break;
		case 1:
			MouseCon.cursor = 0;
			CursorSelectsSingleCon(plr);
			break;
		case 2: ThrowClick(); break;
		case 4: JumpClick(); break;
		}
	}
}

//------------------------ Module Main Function ------------------------------

extern BYTE RoundMenuAbort(void);
extern void ClearGameSounds(void);
extern void NoGameTimeDelay(void);

BYTE GameControl(char inc, char exinc) // Returns abort round
{
	BYTE plr = 100, com = 100, bcon;
	int cnt, cnt2;
	BYTE sat, abort;

	if (inc)
	{
		for (cnt = 0; cnt < MaxGamePlr; cnt++) if (BSA.Plr[cnt].Col > -1)
			if (Inside(BSA.Plr[cnt].Con, 0, 2))
				for (cnt2 = 0; cnt2 < 10; cnt2++)
					if (inc == Config.KCom[BSA.Plr[cnt].Con][cnt2])
					{
						plr = cnt; com = cnt2;
					}

		if (inc == 27)
		{
			ClearGameSounds();
			abort = RoundMenuAbort();
			InitMouseCon();
			NoGameTimeDelay();
			return abort;
		}
	}

	switch (exinc)
	{
	case 63: Config.GameSpeed = Min(Config.GameSpeed + 1, 10); TimeOut(); break;
	case 62: Config.GameSpeed = Max(Config.GameSpeed - 1, 1);  TimeOut(); break;
		//case 67: PPage(2); getch(); break;
		//case 68: PPage(3); getch(); break;
	}

	if ((com == 100) || (plr == 100)) return 0; // No Player Com

	// Eliminated or not present: No control
	if (BSA.Plr[plr].Eliminate || !BSA.Plr[plr].Info) return 0;

	// KCom-Event Handling
	EventCall(100 + com);
	KComFlash(com);

	// Menu active?
	if (Crew[plr].CMenu != CMNOMENU) { GameConExecMenu(com, plr); return 0; }

	// No, regular control
	sat = 0;
	switch (com)
	{
	case 0:
		CursorPrev(plr);
		if (Crew[plr].TogMode == 0) Crew[plr].TogMode = 1;
		break;
	case 1:
		if (Crew[plr].TogMode == 0) // Toggle all after movm
		{
			if (Crew[plr].SelectAllCount > 100) SelectAllCon(plr);
			else if (Crew[plr].ConCnt < Crew[plr].ManCnt)
			{
				ToggleAllCon(plr); sat = 1;
			}
		}
		else
		{ // Toggle single after cursor set or toggle
			CursorTogglesSingleCon(plr);
			Crew[plr].TogMode = 2;
		}
		break;
	case 2:
		CursorNext(plr);
		if (Crew[plr].TogMode == 0) Crew[plr].TogMode = 1;
		break;
	case 3: case 4: case 5: case 6: case 7: case 8: // Clonk BASE Cons
		bcon = com; if (Inside(com, 6, 8)) bcon = com - 6;

		if (Crew[plr].TogMode == 1) // Movm con directly after cursor set only
			CursorSelectsSingleCon(plr);

		if (!(Crew[plr].TogMode == 1)) // Not in cursor single selection
			CheckVehicSelectsCon(plr);

		if (Crew[plr].TogMode == 2) // Cursor adjust after single toggle
			CursorAdjust(plr);

		ClonkControl(plr, bcon, -1);

		Crew[plr].TogMode = 0;
		break;
	case 9:
		if (BSA.RuleSet == 0) break;
		ClonksCeaseAction(plr);
		if ((Crew[plr].Cursor) && (Crew[plr].Cursor->mi->rank > 0))
		{
			switch (Crew[plr].Cursor->carr)
			{
			case LINECON:
				AbandonLine(Crew[plr].Cursor);
				ActivateMenu(plr, CMMAIN);
				break;
			default:
				ActivateMenu(plr, CMMAIN); break;
			}
			break;
		}
		ActivateMenu(plr, CMMAIN); // Menu even with no clonk
		break;
	}

	// SuccessiveAllToggle-Count (for select-all)
	if (sat) Crew[plr].SelectAllCount = Min(Crew[plr].SelectAllCount + 60, 200);
	else Crew[plr].SelectAllCount = 0;

	return 0;
}

void ExecGameCon(void) // Every Tick1
{
	int cnt;
	// Dead-Cursor check
	if (!Tick50)
		for (cnt = 0; cnt < 3; cnt++)
			if (Crew[cnt].Cursor) if (Crew[cnt].Cursor->act == MADEAD)
				CursorAdjust(cnt);
	// Mouse control
	if (MouseCon.status) MouseControl();
}