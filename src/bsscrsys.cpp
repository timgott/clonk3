/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design --- CLONK --- Battle Sequence Scripting System Module ------

#include <stdio.h>
#include <stdlib.h>

#include "standard.h"
#include "std_gfx.h"
#include "stdfile.h"

#include "clonk_bs.h"
#include "clonk_sc.h"

#include "bsexcsys.h"
#include "bsgfxsys.h"
#include "bsmansys.h"
#include "bspxssys.h"

//----------------------------- Local Definitions ------------------------------

#define ET_UNDEF -1
#define ET_FRPOS  0
#define ET_EVENT  1
#define ET_DELAY  2
#define ET_IFCON  3

#define EA_UNDEF  -1
#define EA_CONTINU 0
#define EA_MESSAGE 1
#define EA_VARISET 2
#define EA_CALLFNC 3
#define EA_SUCCESS 4

//------------------------- Main Global Externals ----------------------------

extern char OSTR[500];
extern BSATYPE BSA;
extern SCRIPTINFO MissionScript;

extern VEHICTYPE *FirstVehic;
extern DWORD WipfResc;

extern int AnimalCount(int antype);

//-------------------- Global Scripting System Variables ---------------------

DWORD FramePos, FrameDelay;
FILE *ScrFile;
char ScrPrcLine[257];
int PrcEvtType;
DWORD PrcEvtValue;
char *PrcEvtAction;
BYTE MissionSuxS = 0;

//---------------------- MISSION Scripting Functions -------------------------

extern int CrewBaseXOverride;

int *BSAVarPtrByName(char *vname)
{
	if (SEqual(vname, "BackDepth")) return &BSA.BSize;
	if (SEqual(vname, "BackPhase")) return &BSA.BPhase;
	if (SEqual(vname, "BackPhaseLength")) return &BSA.BPLen;
	if (SEqual(vname, "BackCurveAmp")) return &BSA.BCrvAmp;
	if (SEqual(vname, "BackRandomAmp")) return &BSA.BRndAmp;
	if (SEqual(vname, "WaterLevel")) return &BSA.BWatLvl;
	if (SEqual(vname, "Volcano")) return &BSA.BVolcLvl;
	if (SEqual(vname, "Earthquake")) return &BSA.BQuakeLvl;
	if (SEqual(vname, "Canyon")) return &BSA.BCanyon;
	if (SEqual(vname, "BRockMode")) return &BSA.BRockMode;

	if (SEqual(vname, "Climate")) return &BSA.WClim;
	if (SEqual(vname, "Season")) return &BSA.WSeas;
	if (SEqual(vname, "Rain")) return &BSA.WRFMod;
	if (SEqual(vname, "Lightning")) return &BSA.WLPMod;
	if (SEqual(vname, "Comet")) return &BSA.WCmtLvl;
	if (SEqual(vname, "Environment")) return &BSA.WEnvr;
	if (SEqual(vname, "YearSpeed")) return &BSA.WYSpd;

	if (SEqual(vname, "Wipfs")) return &BSA.Wipfs;
	if (SEqual(vname, "Sharks")) return &BSA.Sharks;
	if (SEqual(vname, "Monsters")) return &BSA.Monsters;

	if (SEqual(vname, "Clonks")) return &BSA.Plr[0].Clonks;
	if (SEqual(vname, "Wealth")) return &BSA.Plr[0].Wealth;
	if (SEqual(vname, "Castle")) return &BSA.Plr[0].ReadyBase[0];
	if (SEqual(vname, "Elevator")) return &BSA.Plr[0].ReadyBase[1];
	if (SEqual(vname, "Windmill")) return &BSA.Plr[0].ReadyBase[2];
	if (SEqual(vname, "Pump")) return &BSA.Plr[0].ReadyBase[3];
	if (SEqual(vname, "Tower")) return &BSA.Plr[0].ReadyBase[4];
	if (SEqual(vname, "Lorry")) return &BSA.Plr[0].ReadyVhc[0];
	if (SEqual(vname, "Catapult")) return &BSA.Plr[0].ReadyVhc[1];
	if (SEqual(vname, "Sailboat")) return &BSA.Plr[0].ReadyVhc[2];
	if (SEqual(vname, "Crossbow")) return &BSA.Plr[0].ReadyVhc[3];
	if (SEqual(vname, "Balloon")) return &BSA.Plr[0].ReadyVhc[4];

	if (SEqual(vname, "StartXPos")) return &CrewBaseXOverride;

	return NULL;
}

BYTE LoadMissionPreInitBSA(void)
{
	FILE *fhnd;
	int *bsavarptr;
	int eqspos;
	if (MissionScript.idnum < 0) return 0;
	//if (!(fhnd=fopen(MissionScript.fname,"r"))) return 0;
	if (!(fhnd = fopen("MISSIONS.SCR", "r"))) return 0;
	sprintf(OSTR, "[NewFile=%s]", MissionScript.fname);
	if (!LocateInFile(fhnd, OSTR)) { fclose(fhnd); return 0; }
	if (!LocateInFile(fhnd, "[PreInit]")) { fclose(fhnd); return 0; }
	AdvanceFileLine(fhnd);
	do
	{
		ReadFileLine(fhnd, OSTR, 256);
		bsavarptr = BSAVarPtrByName(OSTR);
		eqspos = SCharPos('=', OSTR);
		if (bsavarptr && (eqspos > -1))
			*bsavarptr = strtol(OSTR + eqspos + 1, NULL, 10);
	} while (bsavarptr && (eqspos > -1));
	fclose(fhnd);
	return 1;
}

void CloseScriptFile(void)
{
	if (!ScrFile) return;
	fclose(ScrFile);
	ScrFile = NULL;
}

BYTE SetPrcEvent(void)
{
	PrcEvtType = ET_UNDEF;
	if (SEqual(ScrPrcLine, "FP")) PrcEvtType = ET_FRPOS;
	if (SEqual(ScrPrcLine, "EV")) PrcEvtType = ET_EVENT;
	if (SEqual(ScrPrcLine, "DL")) PrcEvtType = ET_DELAY;
	if (SEqual(ScrPrcLine, "IF")) PrcEvtType = ET_IFCON;
	if (PrcEvtType == ET_UNDEF) return 0;
	PrcEvtValue = strtol(ScrPrcLine + 2, &PrcEvtAction, 10);
	while (*PrcEvtAction == ' ') PrcEvtAction++;
	return 1;
}

BYTE ReadScriptProcess(void)
{
	if (!ScrFile) return 0;
	if (!ReadFileLine(ScrFile, ScrPrcLine, 256))
	{
		CloseScriptFile(); RoundError("Error in script file (Proc)"); return 0;
	}
	if (!SetPrcEvent())
	{
		CloseScriptFile(); RoundError("Error in script (SetEvt)"); return 0;
	}
	return 1;
}

BYTE OpenScriptFile(void)
{
	if (MissionScript.idnum < 0)
	{
		RoundError("Not a valid script"); return 0;
	}
	//if (!(ScrFile=fopen(MissionScript.fname,"r")))
	if (!(ScrFile = fopen("MISSIONS.SCR", "r")))
	{
		RoundError("Script file not found,"); RoundError("in MISSIONS.SCR:"); RoundError(MissionScript.fname); return 0;
	}
	sprintf(OSTR, "[NewFile=%s]", MissionScript.fname);
	if (!LocateInFile(ScrFile, OSTR) || !LocateInFile(ScrFile, "[RoundScript]"))
	{
		CloseScriptFile(); RoundError("Error in script file (Init)"); return 0;
	}
	AdvanceFileLine(ScrFile);
	return ReadScriptProcess();
}

int GetEvtActionType(char *acstr)
{
	if (SEqual(acstr, "Continu")) return EA_CONTINU;
	if (SEqual(acstr, "Message")) return EA_MESSAGE;
	if (SEqual(acstr, "VariSet")) return EA_VARISET;
	if (SEqual(acstr, "CallFnc")) return EA_CALLFNC;
	if (SEqual(acstr, "Success")) return EA_SUCCESS;
	return EA_UNDEF;
}

void EvtActMessage(char *msg)
{
	GameMessage(msg, 160, 70, CWhite, NULL);
}

extern void KeyConDisplay(int stat);

void EvtActCallFnc(char *fnptr)
{
	if (SEqual(fnptr, "KeyConDisplay"))
	{
		KeyConDisplay(strtol(fnptr + 14, NULL, 10)); return;
	}
	if (SEqual(fnptr, "PlaySound"))
	{
		GSTP = strtol(fnptr + 10, NULL, 10); return;
	}
	RoundError("Error in script (CallFnc)");
}

void EvtActVariSet(char *vptr)
{
	int *bsavarptr;
	int eqspos;
	bsavarptr = BSAVarPtrByName(vptr);
	eqspos = SCharPos('=', vptr);
	if (bsavarptr && (eqspos > -1))
		*bsavarptr = strtol(vptr + eqspos + 1, NULL, 10);
	else
		RoundError("Error in Script (VariSet)");
}

void ExecEvtAction(void)
{
	FrameDelay = 0;
	switch (GetEvtActionType(PrcEvtAction))
	{
	case EA_CONTINU: break;
	case EA_MESSAGE: EvtActMessage(PrcEvtAction + 8); break;
	case EA_VARISET: EvtActVariSet(PrcEvtAction + 8); break;
	case EA_CALLFNC: EvtActCallFnc(PrcEvtAction + 8); break;
	case EA_SUCCESS: CloseScriptFile(); MissionSuxS = 1; break;
	default: CloseScriptFile(); RoundError("Error in Script (Exec)"); break;
	}
	PrcEvtType = ET_UNDEF;
	ReadScriptProcess();
}

BYTE IfConditionMet(int conn) // Undefined condition number yields no error
{
	int cnt;
	ROCKTYPE *crck;
	VEHICTYPE *cvhc;

	if (!Sec1) return 0;

	// 99: First Clonk carries something
	if (conn == 99)
		for (cnt = 0; cnt < 3; cnt++) if (Crew[cnt].FirstMan)
			if (Crew[cnt].FirstMan->carr != NOROCK)
				return 1;
	// 100-126: First Clonk carries rock type conn-100
	if (Inside(conn, 100, 100 + RockTypeNum - 1))
		for (cnt = 0; cnt < 3; cnt++) if (Crew[cnt].FirstMan)
			if (Crew[cnt].FirstMan->carr == conn - 100)
				return 1;
	// 150-179: Some players has more than or equal to conn-150 clonks
	if (Inside(conn, 150, 179))
		for (cnt = 0; cnt < 3; cnt++)
			if (Crew[cnt].ManCnt >= conn - 150) return 1;
	// 300-399: Some players wealth exceeds or equals conn-300
	if (Inside(conn, 300, 399))
		for (cnt = 0; cnt < 3; cnt++)
			if (Crew[cnt].Wealth >= conn - 300) return 1;
	// 400-409: First vehic/lorry contains conn-400 gold
	if (Inside(conn, 400, 409))
		if (FirstVehic && FirstVehic->type == VHLORRY)
			if (FirstVehic->back[GOLD] >= conn - 400) return 1;
	// Single cons
	switch (conn)
	{
	case 200: case 201: // Five/Fifteen rocks on right border
		for (crck = FirstRock, cnt = 0; crck; crck = crck->next)
			if (crck->x > 300) cnt++;
		if (cnt >= 5 + 10 * (conn == 201)) return 1;
		break;
	case 202: // First Clonk is pushing
		for (cnt = 0; cnt < 3; cnt++) if (Crew[cnt].FirstMan)
			if (Crew[cnt].FirstMan->act == MAPUSH)
				return 1;
		break;
	case 203: // Five wipfs have been saved
		if (WipfResc >= 5) return 1;
		break;
	case 204: // No monsters left
		if (AnimalCount(MNMONSTER) == 0) return 1;
		break;
	case 205: // Lorry on right border
		for (cvhc = FirstVehic; cvhc; cvhc = cvhc->next)
			if (cvhc->type == VHLORRY)
				if (cvhc->x > 300)
					return 1;
		break;
	}

	return 0;
}

//--------------------------- Scripting Process -------------------------------

void InitScript(void)
{
	FramePos = 0;
	FrameDelay = 0;
	ScrFile = NULL;
	PrcEvtType = ET_UNDEF;
	MissionSuxS = 0;
	if (BSA.SMode == S_MISSION) OpenScriptFile();
}

void ExecScript(void) // Every Tick1 in S_MISSION
{
	// Script Event Handling
	switch (PrcEvtType)
	{
	case ET_FRPOS: if (PrcEvtValue == FramePos) ExecEvtAction(); break;
	case ET_DELAY: if (PrcEvtValue == FrameDelay) ExecEvtAction(); break;
	case ET_IFCON: if (IfConditionMet(PrcEvtValue)) ExecEvtAction(); break;
	}
	// Frame Counter
	FramePos++; FrameDelay++;
}

void ExecEventCall(int evnum)
{
	if (PrcEvtType == ET_EVENT) if (PrcEvtValue == evnum) ExecEvtAction();
	//sprintf(OSTR,"event call: %d",evnum); SystemError(OSTR);
}

BYTE MissionSuccess(void)
{
	return MissionSuxS;
}

void DeInitScript(void)
{
	CloseScriptFile();
}

// Event Numbers:
// --------------
// 100-109: Player KCom (#-100) of any player
// 200    : Explosion
// 201    : Castle gate activated
// 202    : Lorry homed
// 203    : Cat/XBow fired
// 204    : Gold blasted free
// 205    : Elevator shaft drilling
// 206    : Tower up-transport
// 207    : Tower gate activated
// 208    : Construction activated
// 209    : Construction complete
// 210    : Connection made
// 211    : Pump pumps
// 212    : Castle produces oil barrel
// 213    : Oilpower active
// 250-252: Bridge: wall,diagonal,level
// 300-308: Menu (#-300) activated
// 400-426: Rock type conn-400 has been ordered

// Condition Numbers:
// ------------------
// 100-126: First Clonk carries rock type conn-100
// 099    : First Clonks carries something
// 150-179: Some players has more than or equal to conn-150 clonks
// 200/201: Five/Fifteen rocks on right border
// 202    : First Clonk is pushing
// 203    : Five wipfs have been saved
// 204    : No monsters left
// 205    : Lorry on right border
// 300-399: Some player's wealth exceeds or equals conn-300
// 400-409: First vehic/lorry contains conn-400 gold
