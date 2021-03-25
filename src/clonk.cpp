/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  CLONK 3.4 Radikal  by Matthes Bender

//           Coding start (all new code):     Juli 5th,  1995
//             MeleeBetaTest Release 0.4: February 22nd, 1996
//                      Beta Release 0.5:      May 20th, 1996
//   CLONK 3.0 Registered Player Release:   August 10th, 1996
// CLONK 3.1 Licenced Light-Full Version:   August 27th, 1996
//  CLONK 3.2 Non-Shareware English Demo:   October      1996
//  CLONK 3.3 Extended Shareware Release:   October 31st,1996
//  CLONK 3.4 English/International Shareware Release: March 10th,1997
//  CLONK 3.5 Non-Shareware English Retail Version: April, x 1997

const char *PrgInfoLine = "RedWolf Design CLONK 3.5 Radikal";

//------------------------------ Include Headers --------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <inttypes.h>

#include "standard.h"
#include "vga4.h"
#include "std_gfx.h"
#include "stdfile.h"

#include "svi20ext.h"

#include "clonk.h"
#include "clonk_sc.h"

#include "RandomWrapper.h" // implementiert random()
#include "UserInput.h"

#include "SDLmain.h"

//--------------------------- Extern Program Parts ---------------------------

extern void Intro(BYTE hires);
extern void EndTextMessage(void);
extern BYTE FIntegCheck(void);

extern BYTE MenuPagesLoaded;
extern char RunningPath[257];

extern void DrawTitleBack(BYTE tpge, BYTE design);
extern BYTE LoadMenuPages(void);
extern int DrawSurfacePreview(int dummy);
extern int SoundSetup(int dummy);
//extern BYTE Registered(void);
extern BYTE LoadConfigSystem(void);
extern BYTE SaveConfigSystem(void);
extern int PlayerImport(int dummy);
extern void DefaultConfig(void);
extern int EditKeyboardControls(int dummy);
//extern int Registration(int dummy);
extern void DefaultUserPref(void);
//extern void InitKLockCheck(void);

extern BYTE InitSVIMouse(void);

//------------------ Extern Game Initialization & ExecRound ------------------

extern void Randomize3(void);
extern void Randomize4(void);

extern BYTE ExecRound(void);

extern BYTE InitBSGraphics(void);
extern void DeInitBSGraphics(void);

extern void PrepareBSARockType(void);

//----------------- Main Global Game & System Setting Variables ---------------

CONFIG Config;
USERPREF UserPref;
BSATYPE BSA;

PLAYERINFO *FirstPlayer = NULL;

PLAYERRANK *FirstPlrRank = NULL;

SCRIPTINFO *FirstScript = NULL;
SCRIPTINFO MissionScript;
int MissionScriptNum;

const BYTE MainPage = 0, MenuPage = 1, FacePage = 3;

char OSTR[500];

PLAYERINFO *CCrewListPlr = NULL; // sucks but has to be
char ImportFileName[46] = "FILENAME.PCX";
char DatImportName[46] = "\\CLONK.DAT";

const int DefFaceNum = 8;

BYTE InRound = 0, FirstTimeRun, SBDetected;

//---------------------------- Local Definitions -----------------------------

#define LID_CAPS 0
#define LID_PLRS 1
#define LID_FACE 2
#define LID_SCRS 3

//------------------------------- Name Data ---------------------------------

const char *RankName[11] = {"Clonk", "Ensign", "Lieutenant", "Captain", "Major", "Lieutenant Colonel", "Colonel", "Brigade General", "Major General", "Lieutenant General", "General"};

const char *CGModeName[4] = {"Gold mine", "Monsterkill", "Wipf rescue", "Settlement"};

const char *CGModeDesc[4] = {"Your objective is to mine all the gold there is. Only the forces of nature stand in your way.",
							 "Your objective in this mode is to eliminate all the monsters. Well-functioning traps are to be made up by you.",
							 "In this mode, as many wipfs as possible are to be rescued and taken to your base.",
							 "There is no set goal in this mode, you may simply play as long as you wish. (Hit 'surrender' to end the round)."};

const char *EliminationName[3] = {"Kill the Captain", "Eliminate crew", "Capture the flag"};

const char *HomeBaseName[2] = {"Is the balloon", "Is the castle "};

const char *SModeName[3] = {"Mission", "Cooperative", "Melee"};
const char *RuleSetName[3] = {"Easy", "Medium", "Radical"};
const BYTE RuleSetCol[3] = {CDGreen, CDBlue, CDRed};

const char *SoundStatusName[4] = {"Deactivated", "using CT-VOICE", "using DSP/Timer", "using SDL"};

const char *ConTypeName[4] = {"Left  ", "Center", "Right ", "Mouse "};

const int MaxDefQuote = 13;

const char *DefQuoteSource[MaxDefQuote] = {"This was cool!", // MaxLen!
										   "CLONK is a pretty neat game.",
										   "It was a tough battle...",
										   "Victorious, as usual.",
										   "The last one becomes shark food.",
										   "Yabbadabbadoo!",
										   "Ein Flintstone kommt selten allein.",
										   "Wer den Wipf nicht ehrt, ist des Goldes nicht wert",
										   "Wieder in die Monstergrube geschleudert worden?",
										   "One for all, all against one.",
										   "And who's gonna clean up later?",
										   "Wer anderen eine Grube gr�bt, f�llt selbst hinein!",
										   "Try this!"};

extern char *GetClonkName(void);
extern BYTE InitCNameFile(void);
extern void DeInitCNameFile(void);

//------------------------ Extern Image File Handling -----------------------------

extern BYTE LoadAGC2PageV1(char *fname, BYTE tpge, int tx, int ty, WORD *iwdt, WORD *ihgt, BYTE *palptr, BYTE *scram);
extern BYTE SaveAGCPageV1(char *fname, BYTE fpge, int fx, int fy, WORD wdt, WORD hgt, BYTE *palptr, BYTE *scram);
extern char *AGCError(BYTE errnum);
extern BYTE LoadPCX(char *fname, BYTE tpge, int tx, int ty, COLOR *tpal);
extern char *LoadPCXError(BYTE errn);

//-------------------------- Player & Clonk Handling -----------------------------

int BSAPlrNum(void)
{
	int rval = 0;
	if (BSA.Plr[0].Col > -1)
		rval++;
	if (BSA.Plr[1].Col > -1)
		rval++;
	if (BSA.Plr[2].Col > -1)
		rval++;
	return rval;
}

PLAYERINFO *PNum2Plr(int pnum)
{
	PLAYERINFO *cplr = NULL;
	if (pnum > -1)
		for (cplr = FirstPlayer; (pnum > 0) && cplr; cplr = cplr->next, pnum--)
			;
	return cplr;
}

int Plr2PNum(PLAYERINFO *plr)
{
	int pnum = 0;
	PLAYERINFO *cplr;
	for (cplr = FirstPlayer; cplr && cplr != plr; cplr = cplr->next, pnum++)
		;
	if (!cplr)
		pnum = -1;
	return pnum;
}

MANINFO *MNum2Man(int mnum)
{
	MANINFO *man = NULL;
	if (!CCrewListPlr)
	{
		Message("mnum2man: null ccrewlistplr");
		return NULL;
	}
	for (man = CCrewListPlr->crew; (mnum > 0) && man; man = man->next, mnum--)
		;
	return man; // Uses global designation of current player
}

DWORD PlayerRankScore(int crnk)
{
	DWORD nrsc = 0L, addto = 100L;

	for (crnk; crnk > 0; crnk--)
	{
		nrsc += addto;
		addto += 100L;
	}

	return nrsc;
}

int ClonkRankExp(int crnk)
{
	int nrsc = 0, addto = 1000;

	for (crnk; crnk > 0; crnk--)
	{
		nrsc += addto;
		addto += 200;
	}

	return nrsc;
}

const int DeadLostLimit = 500; // Necessary exp to get reanimation chance

//------------------------ Player Rank Name Handling --------------------------

BYTE NewPlrRank(const char *name)
{
	PLAYERRANK *nrnk, *last;
	if (!(nrnk = new PLAYERRANK))
		return 0;
	SCopy(name, nrnk->name, MaxPlrRankName);
	nrnk->next = NULL;
	for (last = FirstPlrRank; last && last->next; last = last->next)
		;
	if (last)
		last->next = nrnk;
	else
		FirstPlrRank = nrnk;
	return 1;
}

void DefaultPlrRanks(void)
{
	const int DefPlrRankNum = 11;
	const char *defplrrank[DefPlrRankNum] = {"Novice", "Beginner", "Esquire", "Lord", "Baron", "Viscount",
									   "Count", "Earl", "Duke", "Dude", "Meister"};
	int cnt;
	for (cnt = 0; cnt < DefPlrRankNum; cnt++)
		NewPlrRank(defplrrank[cnt]);
}

void ClearPlrRanks(void)
{
	PLAYERRANK *crnk, *next;
	for (crnk = FirstPlrRank; crnk; crnk = next)
	{
		next = crnk->next;
		delete crnk;
	}
	FirstPlrRank = NULL;
}

const char *PlrRankName(int num)
{
	PLAYERRANK *crnk;
	for (crnk = FirstPlrRank; crnk && (num > 0); crnk = crnk->next, num--)
		;
	if (crnk)
		return crnk->name;
	return "[Rangnamen-Fehler]";
}

int PlrRankCount(void)
{
	PLAYERRANK *crnk;
	int rval;
	for (crnk = FirstPlrRank, rval = 0; crnk; crnk = crnk->next, rval++)
		;
	return rval;
}

//----------------------------- CrewList Handling -----------------------------

MANINFO *NewDefaultMan2List(MANINFO **crew)
{
	MANINFO *ncap;

	if (!crew)
	{
		Message("newman2list: null ptr");
		return NULL;
	}

	if (!(ncap = new MANINFO))
		return NULL;

	ncap->name[0] = 0;
	ncap->rank = 0;
	ncap->exp = 0;
	ncap->rnds = 0;
	ncap->rean = 0;
	ncap->dead = 0;

	ncap->next = *crew;
	*crew = ncap;

	return ncap;
}

MANINFO *NewMan2List(MANINFO **crew, char *name, int rank, int exp, int rnds, int rean)
{
	MANINFO *ncap;

	if (!(ncap = NewDefaultMan2List(crew)))
		return NULL;

	SCopy(name, ncap->name, MaxClonkName);
	ncap->rank = rank;
	ncap->exp = exp;
	ncap->rnds = rnds;
	ncap->rean = rean;
	ncap->dead = 0;

	return ncap;
}

void DeleteCrewList(MANINFO **crew)
{
	MANINFO *ccap, *ncap = NULL;

	if (!crew)
	{
		Message("deletecrewlist: null ptr");
		return;
	}

	for (ccap = *crew; ccap; ccap = ncap)
	{
		ncap = ccap->next;
		delete ccap;
	}
	*crew = NULL;
}

void DrawCrewListCell(int num, int tx, int ty, BYTE sel)
{
	MANINFO *centr = MNum2Man(num);
	LPage(MainPage);
	DBox(tx, ty, tx + 99, ty + 19, sel ? CGray5 : CWhite);
	if (centr)
	{
		DFrame(tx + 2, ty + 2, tx + 12, ty + 12, CGray2);
		SVIGCopy(MenuPage, 0 + 11 * centr->rank, 32, SVIPage, tx + 1, ty + 1, 11, 11, centr->dead);
		SOut(RankName[centr->rank], tx + 14, ty + 1, !centr->dead ? CGray1 : CGray2);
		SOutS(centr->name, tx + 14, ty + 7, !centr->dead ? CDGreen : CGray2, CGray4);
		SOut("Exp", tx + 98, ty + 1, !centr->dead ? CGray1 : CGray3, -1, 2);
		sprintf(OSTR, "%d", centr->exp);
		SOut(OSTR, tx + 98, ty + 7, !centr->dead ? CGray1 : CGray3, -1, 2);
		sprintf(OSTR, "Rnd:%d Wbl:%d", centr->rnds, centr->rean);
		SOut(OSTR, tx + 1, ty + 14, !centr->dead ? CGray1 : CGray3);
	}
}

int CrewListCount(PLAYERINFO *plr)
{
	int num;
	MANINFO *ccap;
	for (ccap = plr->crew, num = 0; ccap; ccap = ccap->next, num++)
		;
	return num;
}

void SortCrewList(MANINFO *crew)
{
	BYTE change;
	MANINFO *cmn, tmndat1, tmndat2, *tmnptr1, *tmnptr2, *tmnnxt1, *tmnnxt2;
	do
	{
		change = 0;
		for (cmn = crew; cmn && cmn->next; cmn = cmn->next)
			if (cmn->exp < cmn->next->exp)
			{
				tmnptr1 = cmn;
				tmnptr2 = cmn->next;
				tmndat1 = *tmnptr1;
				tmndat2 = *tmnptr2;

				tmnnxt1 = tmndat1.next;
				tmnnxt2 = tmndat2.next;
				tmndat1.next = tmnnxt2;
				tmndat2.next = tmnnxt1;

				*tmnptr1 = tmndat2;
				*tmnptr2 = tmndat1;

				change = 1;
			}
	} while (change);
}

void RemoveDeadFromList(PLAYERINFO *plr)
{
	MANINFO *cmn = plr->crew, *prev = NULL, *nxt;
	while (cmn)
	{
		if (cmn->dead && (cmn->exp < DeadLostLimit))
		{
			nxt = cmn->next;
			if (prev)
				prev->next = nxt;
			else
				plr->crew = nxt;
			delete cmn;
			cmn = nxt;
		}
		else
		{
			prev = cmn;
			cmn = cmn->next;
		}
	}
}

void DegradeEmrPromos(PLAYERINFO *plr)
{
	MANINFO *cmn;
	for (cmn = plr->crew; cmn; cmn = cmn->next)
		if (cmn->exp < ClonkRankExp(cmn->rank))
			cmn->rank--;
}

void CheckCrewLists(void)
{
	PLAYERINFO *plr;
	for (plr = FirstPlayer; plr; plr = plr->next)
	{
		DegradeEmrPromos(plr);
		RemoveDeadFromList(plr);
		SortCrewList(plr->crew);
	}
}

void AdjustCrew2Round(int plr)
{
	int cnum, addcl;
	MANINFO *cmn; // Safety
	if (!BSA.Plr[plr].Info)
	{
		Message("adjustcrew2round: null bsa.player[");
		return;
	}
	for (cmn = BSA.Plr[plr].Info->crew, cnum = 0; cmn; cmn = cmn->next)
		if (!cmn->dead)
			cnum++;
	for (addcl = BSA.Plr[plr].Clonks - cnum; addcl > 0; addcl--)
		NewMan2List(&(BSA.Plr[plr].Info->crew), GetClonkName(), 0, 0, 0, 0);
	SortCrewList(BSA.Plr[plr].Info->crew);
}

void PrepareTCrew4Round(int plr)
{
	int cnt;
	MANINFO *cmn; // Safety
	if (!BSA.Plr[plr].Info)
	{
		Message("preptcrew4round: null bsa.player[");
		return;
	}

	NewMan2List(&(BSA.Plr[plr].TCrew), GetClonkName(), 1, ClonkRankExp(1), 0, 0);
	for (cnt = 1; cnt < BSA.Plr[plr].Clonks; cnt++)
		NewMan2List(&(BSA.Plr[plr].TCrew), GetClonkName(), 0, ClonkRankExp(0), 0, 0);
	SortCrewList(BSA.Plr[plr].TCrew);
}

void ClearTCrews(void)
{
	int cnt;
	for (cnt = 0; cnt < MaxGamePlr; cnt++)
		DeleteCrewList(&(BSA.Plr[cnt].TCrew));
}

void ManInfoEdit(MANINFO *man)
{
	WINDOW *cwin;

	cwin = NewWindow("Clonk-Info", PLAIN, 1, -1, -1, 200, 80, 1, 0, XRVCLOSE, "Contents");

	NewObject("RankPic", PICTURE, 0, 0, 5, 15, 11, 11, MenuPage, 0, 0, 0 + 11 * man->rank, 32, 0, 0, 0, NULL, NULL);

	sprintf(OSTR, "Rank: �%c%s  �%cExperience: �%c%d", CGray1, RankName[man->rank], CGray2, CGray1, man->exp);
	TextObject(OSTR, 5, 30, CGray2);
	if (man->rank < 10)
		sprintf(OSTR, "(Promotion to %s at %d)", RankName[man->rank + 1], ClonkRankExp(man->rank + 1));
	else
		sprintf(OSTR, "(No more promotions)");
	TextObject(OSTR, 5, 36, CGray2);
	sprintf(OSTR, "Rounds (survived): �%c%d", CGray1, man->rnds);
	TextObject(OSTR, 5, 48, CGray2);
	sprintf(OSTR, "Reanimated: �%c%d times", CGray1, man->rean);
	TextObject(OSTR, 5, 54, CGray2);

	if (man->dead)
	{
		sprintf(OSTR, "%s is dead", man->name);
		TextObject(OSTR, 5, 66, CGray2);
	}

	NewObject("OK", BUTTON, 1, 0, 150, 65, 40, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

	NewObject("Name", TEXTBOX, 1, 0, 20, 11 + 6, MaxClonkName * 4 + 4, 9, 1, XRVCLOSE, 0, MaxClonkName, SLen(man->name), 0, 0, 0, man->name, NULL);

	if (InitErrorCheck())
		CloseWindow(cwin);
}

int ReanCost(MANINFO *man)
{
	return (Max(ClonkRankExp(man->rank), DeadLostLimit) / 30);
}

int ExecManRean(int mnum)
{
	MANINFO *man = MNum2Man(mnum);
	CCrewListPlr->scorepot -= ReanCost(man);
	man->dead = 0;
	man->rean++;
	return 0;
}

void AutoReanMan(PLAYERINFO *cplr, MANINFO *cman)
{
	if (cplr->scorepot >= ReanCost(cman))
	{
		cplr->scorepot -= ReanCost(cman);
		cman->dead = 0;
		cman->rean++;
	}
}

void ReanimateMan(int mnum)
{
	MANINFO *man = MNum2Man(mnum);

	if (!man)
	{
		Message("reanimateman: null man");
		return;
	}

	if (!man->dead)
	{
		sprintf(OSTR, "%s is alive and does not|need to be reanimated.", man->name);
		Message(OSTR);
		return;
	}
	if (CCrewListPlr->scorepot < ReanCost(man))
	{
		sprintf(OSTR, "Player %s needs at least %d storage score points,|to reanimate %s.", CCrewListPlr->name, ReanCost(man), man->name);
		Message(OSTR, "Insufficient storage points");
		return;
	}
	sprintf(OSTR, "Reanimate %s|for %d score points?", man->name, ReanCost(man));
	ConfirmedCall(OSTR, 0, &ExecManRean, mnum, "Reanimate Clonks");
}

int CrewList(int pnum)
{
	WINDOW *cwin;
	OBJECT *objptr1, *objptr2, *objptr3;
	PLAYERINFO *plr = PNum2Plr(pnum);
	if (!plr)
	{
		Message("crewlist: null player");
		return 0;
	} // Safety
	CCrewListPlr = plr;
	cwin = NewWindow("Permanent crew", STANDARD, 1, -1, 37, 268, 144, 1, 0, XRVCLOSE, "Contents");
	if (plr->name[0])
	{
		sprintf(OSTR, "Player: %s", plr->name);
		TextObject(OSTR, 268 / 2, 10, CGray2, -1, 1);
	}
	NewObject("CrewPic", PICTURE, 0, 0, 6, 4, 20, 10, MenuPage, 0, 0, 175, 25, 0, 0, 0, NULL, NULL);
	NewButton("Help", 0, 220, 118, 40, 9, XRVHELP, 0);
	objptr3 = NewButton("Reanimate", 0, 220, 107, 40, 9, 0, 0);
	objptr2 = NewButton("Info", 0, 220, 96, 40, 9, 0, 0);

	NewFlagBox("Automatic|Reanimat.", 220, 40, &plr->autorean);

	NewObject("Storage", VALBOX, 0, 0, 220, 16 + 6, 40, 7, 1, 0, 1, 0, 0, 0, 1, 0, NULL, (int *)&plr->scorepot);
	objptr1 = NewObject("CrewList", LISTBOX, 1, 0, 5, 16, 202, 122, LID_CAPS, 2, 6, 100, 20, 0, CrewListCount(plr), (CrewListCount(plr) > 0) ? 0 : -1, NULL, NULL);
	NewButton("OK", 0, 220, 129, 40, 9, XRVCLOSE, 0);
	NewULink(LBCACT, NULL, objptr2, objptr1, NULL, 0);
	NewULink(LBCACT, NULL, objptr3, objptr1, NULL, 1);
	if (InitErrorCheck())
		CloseWindow(cwin);
	return 0;
}

void CrewListCellAction(int num, int aid)
{
	MANINFO *centr = MNum2Man(num);
	if (centr)
		switch (aid)
		{
		case 0:
			ManInfoEdit(centr);
			break;
		case 1:
			ReanimateMan(num);
			break;
		}
}

//-------------------------- Face List Handling ------------------------------

void BlankDeFace(int fnum)
{
	int tx, ty, cnt;
	LPage(FacePage);
	tx = 40 * (fnum % 8);
	ty = 50 * (fnum / 8);
	DBox(tx, ty, tx + 39, ty + 49, CBlack);
	TOutS("No|Import|yet", tx + 20, ty + 5, CGray5, CGray1, 1);
}

int ColDif(COLOR col1, COLOR col2)
{
	int rval = 0;
	rval += abs(col1.b.red - col2.b.red);
	rval += abs(col1.b.green - col2.b.green);
	rval += abs(col1.b.blue - col2.b.blue);
	return rval;
}

void AdaptImage(BYTE page, int tx, int ty, int wdt, int hgt, COLOR *fpal, COLOR *tpal)
{
	BYTE newcol[256]; // slow!!!
	int cnt, cnt2;
	for (cnt = 0; cnt < 256; cnt++)
	{
		newcol[cnt] = 0;
		for (cnt2 = 16; cnt2 < 256; cnt2++) // Allow transparent black or not?
			if (ColDif(tpal[cnt2], fpal[cnt]) < ColDif(tpal[newcol[cnt]], fpal[cnt]))
				newcol[cnt] = cnt2;
	}
	LPage(page);
	for (cnt = 0; cnt < hgt; cnt++)
		for (cnt2 = 0; cnt2 < wdt; cnt2++)
			SPixF(tx + cnt2, ty + cnt, newcol[GPixF(tx + cnt2, ty + cnt)]);
}

int TestImportPCX(int tface)
{
	COLOR cpal[256], tpal[256];
	BYTE pcxrv;
	int tx, ty;

	LPage(MainPage);
	TOutS("Import", 60, 105, CWhite, CGray1, 1);

	tx = 40 * (tface % 8);
	ty = 50 * (tface / 8);
	GetDAC(0, 256, tpal);
	AddExtension(ImportFileName, ".PCX");
	Viewport(tx, ty, tx + 39, ty + 49);
	pcxrv = LoadPCX(ImportFileName, FacePage, tx, ty, cpal);
	if (!pcxrv)
		AdaptImage(FacePage, tx, ty, 40, 50, cpal, tpal);
	NoViewport();

	if (pcxrv)
	{
		sprintf(OSTR, "Error importing|%s:|�%c%s", ImportFileName, CDRed, LoadPCXError(pcxrv));
		Message(OSTR, "Error help not translated");
	}

	return 0;
}

int TakeOverImportFace(int dummy)
{
	int ffwdt, ffhgt;
	BYTE saverv;
	if (PathProtected(RunningPath))
	{
		Message("Error while saving|player portrait file FACES.DAT:|Drive write protected");
		return dummy;
	}
	ffwdt = 40 * Min(Config.FaceNum + 1, 8);
	ffhgt = 50 * ((Config.FaceNum + 1 - 1) / 8 + 1);
	saverv = SaveAGCPageV1("FACES.DAT", FacePage, 0, 0, ffwdt, ffhgt, NULL, NULL);
	if (!saverv)
	{
		Config.FaceNum++;
		Config.FaceFile = 1;
		AdjustLBCellNum(LID_FACE, Config.FaceNum);
	}
	else
	{
		sprintf(OSTR, "Error while saving|player portrait file FACES.DAT:|�%c%s", CDRed, AGCError(saverv));
		Message(OSTR);
	}
	return dummy;
}

int ImportFace(int dummy)
{
	WINDOW *cwin;
	OBJECT *tipic, *fobj, *tiback;

	if (Config.FaceNum >= 16)
	{
		Message("A maximum of 16 player|portraits can be stored.");
		return dummy;
	}

	BlankDeFace(Config.FaceNum);

	cwin = NewWindow("Portrait-PCX-Import", STANDARD, 1, -1, -1, 250, 95, 1, 0, XRVCLOSE, "Contents");

	tipic = NewObject("Import", PICTURE, 0, 0, 5, 15 + 6, 40, 50, FacePage, 1, 8, 0, 0, 0, 0, 0, NULL, &Config.FaceNum),
	tiback = NewObject("TIBack", FRAME, 0, 0, 5, 15 + 6, 40, 50, 0, 0, 0, -1, CBlue, 0, 0, 0, NULL, NULL);

	NewObject("[PCX-INFO]", TEXT, 0, 0, 50, 35, 50, 60, 0, 2, 0, CGray1, -1, 0, 0, 0,
			  "PCX graphic file of size 40x50 pixels with 256 colors can be imported into the game. Color 0 will be transparent.", NULL);

	NewButton("Help", 0, 180, 80, 45, 9, XRVHELP, 1);

	NewObject("Cancel", BUTTON, 1, 0, 130, 80, 45, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

	NewULink(EXCREDR, NULL,
			 NewObject("Take over", BUTTON, 1, 0, 80, 80, 45, 9, XRVCLOSE, 1, 0, 0, 0, 0, 0, 0, NULL, NULL),
			 NULL, &TakeOverImportFace, 0);

	fobj = NewObject("Import", BUTTON, 1, 0, 30, 80, 45, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL),
	NewULink(EXCREDR, NULL, fobj, tipic, NULL, 0);
	NewULink(EXCREDR, NULL, fobj, tiback, &TestImportPCX, Config.FaceNum);
	fobj = NewObject("File name", TEXTBOX, 1, 0, 50, 15 + 6, 182, 9, 1, 0, 0, 45, SLen(ImportFileName), 0, 0, 0, ImportFileName, NULL),
	NewULink(EXCREDR, NULL, fobj, tipic, NULL, 0);
	NewULink(EXCREDR, NULL, fobj, tiback, &TestImportPCX, Config.FaceNum);

	if (InitErrorCheck())
		CloseWindow(cwin);
	return dummy;
}

void DeleteFace(int num)
{
	PLAYERINFO *cplr;
	int cnt;
	BYTE saverv;
	int ffwdt, ffhgt;
	if (num < DefFaceNum)
	{
		Message("This portrait can|not be deleted.");
		return;
	}
	for (cplr = FirstPlayer; cplr; cplr = cplr->next)
		if (cplr->face == num)
			cplr->face = 0;
	for (cnt = num; cnt < Config.FaceNum - 1; cnt++)
	{
		LPage(FacePage);
		DBox(40 * (cnt % 8), 50 * (cnt / 8), 40 * (cnt % 8) + 39, 50 * (cnt / 8) + 49, CBlack);
		SVIGCopy(FacePage, 40 * ((cnt + 1) % 8), 50 * ((cnt + 1) / 8), FacePage, 40 * (cnt % 8), 50 * (cnt / 8), 40, 50, 0);
		for (cplr = FirstPlayer; cplr; cplr = cplr->next)
			if (cplr->face == cnt + 1)
				cplr->face = cnt;
	}
	Config.FaceNum--;
	AdjustLBCellNum(LID_FACE, Config.FaceNum);

	if (PathProtected(RunningPath))
	{
		Message("Error while saving|player portrait file FACES.DAT:|Drive write protected");
		return;
	}

	ffwdt = 40 * Min(Config.FaceNum + 1, 8);
	ffhgt = 50 * ((Config.FaceNum + 1 - 1) / 8 + 1);
	saverv = SaveAGCPageV1("FACES.DAT", FacePage, 0, 0, ffwdt, ffhgt, NULL, NULL);
	if (saverv)
	{
		sprintf(OSTR, "Error while saving|player portrait file FACES.DAT:|�%c%s", CDRed, AGCError(saverv));
		Message(OSTR);
		Config.FaceFile = 0;
	}
}

void DrawFaceListCell(int num, int tx, int ty, BYTE sel)
{
	LPage(MainPage);
	DBox(tx, ty, tx + 39, ty + 49, sel ? CBlue : CWhite);
	if (Inside(num, 0, Config.FaceNum - 1))
		SVIGCopy(FacePage, 40 * (num % 8), 50 * (num / 8), MainPage, tx, ty, 40, 50, 0);
}

BYTE FaceListCellAction(int num, int aid)
{
	if (Inside(num, 0, Config.FaceNum - 1))
		switch (aid)
		{
		case 0:
			return XRVCLOSE;
		case 1:
			DeleteFace(num);
			break;
		}
	return 0;
}

//----------------------------- Player List Handling -----------------------------

int PlrListCount(void)
{
	int rval = 0;
	PLAYERINFO *cplr;
	for (cplr = FirstPlayer; cplr; cplr = cplr->next, rval++)
		;
	return rval;
}

int PlrsInSession(int rval)
{
	PLAYERINFO *cplr;
	rval = 0;
	for (cplr = FirstPlayer; cplr; cplr = cplr->next)
		if (cplr->inses)
			rval++;
	return rval;
}

PLAYERINFO *NewDefaultPlr2List(void)
{
	PLAYERINFO *nplr;
	int cnt;

	if (!(nplr = new PLAYERINFO))
		return NULL;

	nplr->name[0] = 0;
	nplr->nick[0] = 0;
	SCopy("I'm a new player.", nplr->quote, MaxQuoteLen);
	nplr->rank = 0;
	for (cnt = 0; cnt < 3; cnt++)
	{
		nplr->rnds[cnt] = 0;
		nplr->won[cnt] = 0;
		nplr->score[cnt] = 0;
	}
	nplr->scorepot = 0L;
	nplr->playsec = 0L;
	nplr->face = 0;
	nplr->pfcol = 0;
	nplr->pfcon = 2;
	nplr->inses = 1;
	nplr->inrnd = 0;
	nplr->pfgpos = 0;

	nplr->crew = NULL;

	nplr->next = FirstPlayer;
	FirstPlayer = nplr;

	return nplr;
}

void DeInitPlayerList(void)
{
	PLAYERINFO *cplr, *nplr = NULL;
	for (cplr = FirstPlayer; cplr; cplr = nplr)
	{
		DeleteCrewList(&cplr->crew);
		nplr = cplr->next;
		delete cplr;
	}
	FirstPlayer = NULL;
}

float PlrAverage(PLAYERINFO *plr, int smode)
{
	if (plr->rnds[smode] == 0)
		return 0.0;
	return plr->score[smode] / plr->rnds[smode];
}

DWORD PlrTotalScore(PLAYERINFO *plr)
{
	return plr->score[0] + plr->score[1] + plr->score[2];
}

int PlrTotalRounds(PLAYERINFO *plr)
{
	return plr->rnds[0] + plr->rnds[1] + plr->rnds[2];
}

int PlrTotalWon(PLAYERINFO *plr)
{
	return plr->won[0] + plr->won[1] + plr->won[2];
}

float PlrTotalAverage(PLAYERINFO *plr)
{
	if (PlrTotalRounds(plr) == 0)
		return 0.0;
	return PlrTotalScore(plr) / PlrTotalRounds(plr);
}

void DrawPlrListCell(int num, int tx, int ty, BYTE sel)
{
	char *sortname[4] = {"Overall-Stats", "Mission-Stats", "Cooperative-Stats", "Melee-Stats"};
	PLAYERINFO *cplr = PNum2Plr(num);
	int spbn = Config.SortPlrBy % 4;
	LPage(MainPage);
	DBox(tx, ty, tx + 123, ty + 51, sel ? CGray5 : CWhite);
	if (!cplr)
		return;
	if (cplr->inses)
		SVIGCopy(MenuPage, 240, 50, MainPage, tx + 1, ty + 1, 40, 50, 0);
	SVIGCopy(FacePage, 40 * (cplr->face % 8), 50 * (cplr->face / 8), MainPage, tx + 1, ty + 1, 40, 50, !cplr->inses);
	SVIGCopy(MenuPage, 0 + 9 * cplr->pfcol, 20, MainPage, tx + 40, ty + 39, 9, 12, !cplr->inses);
	SVIGCopy(MenuPage, 72 + 12 * cplr->pfcon, 20, MainPage, tx + 50, ty + 39, 12, 12, !cplr->inses);
	SOut(PlrRankName(cplr->rank), tx + 42, ty + 1, cplr->inses ? CGray1 : CGray2);
	SOutS(cplr->name, tx + 42, ty + 7, cplr->inses ? CDBlue : CGray2, CGray4);
	if (Config.ShowQuotes)
	{
		sprintf(OSTR, "%c%s%c", '"', cplr->quote, '"');
		FTOut(OSTR, tx + 42, ty + 14, 19, 7, 0, cplr->inses ? CGray1 : CGray3, -1, 0);
	}
	else
	{
		SOut(sortname[spbn], tx + 42, ty + 14, CGray3);
		sprintf(OSTR, "Score: %" PRIu32, (spbn == 0) ? PlrTotalScore(cplr) : cplr->score[spbn - 1]);
		SOut(OSTR, tx + 42, ty + 20, cplr->inses ? CGray1 : CGray3);
		sprintf(OSTR, "Rounds: %d", (spbn == 0) ? PlrTotalRounds(cplr) : cplr->rnds[spbn - 1]);
		SOut(OSTR, tx + 42, ty + 26, cplr->inses ? CGray1 : CGray3);
		sprintf(OSTR, "Average: %.1f", (spbn == 0) ? PlrTotalAverage(cplr) : PlrAverage(cplr, spbn - 1));
		SOut(OSTR, tx + 42, ty + 32, cplr->inses ? CGray1 : CGray3);
	}
}

int SortPlayerList(int dummy) // Makes any stored pnum invalid!
{
	BYTE swtch, mfd;
	PLAYERINFO *cplr, tplrdat1, tplrdat2, *tplrptr1, *tplrptr2, *tplrnxt1, *tplrnxt2;
	do
	{
		mfd = 0;
		for (cplr = FirstPlayer; cplr && cplr->next; cplr = cplr->next)
		{
			swtch = 0;
			switch (Config.SortPlrBy)
			{
			case 0: // Total Score
				if (PlrTotalScore(cplr) < PlrTotalScore(cplr->next))
					swtch = 1;
				break;
			case 1:
			case 2:
			case 3: // Mission/Cooperate/Melee Score
				if (cplr->score[Config.SortPlrBy - 1] < cplr->next->score[Config.SortPlrBy - 1])
					swtch = 1;
				break;
			case 4: // Total Average
				if (PlrTotalAverage(cplr) < PlrTotalAverage(cplr->next))
					swtch = 1;
				break;
			case 5:
			case 6:
			case 7: // Mission/Cooperate/Melee Average
				if (PlrAverage(cplr, Config.SortPlrBy - 5) < PlrAverage(cplr->next, Config.SortPlrBy - 5))
					swtch = 1;
				break;
			}
			if (swtch)
			{
				tplrptr1 = cplr;
				tplrptr2 = cplr->next;
				tplrdat1 = *tplrptr1;
				tplrdat2 = *tplrptr2;

				tplrnxt1 = tplrdat1.next;
				tplrnxt2 = tplrdat2.next;
				tplrdat1.next = tplrnxt2;
				tplrdat2.next = tplrnxt1;

				*tplrptr1 = tplrdat2;
				*tplrptr2 = tplrdat1;

				mfd = 1;
			}
		}
	} while (mfd);

	return dummy;
}

int DeletePlrFromList(int pnum)
{
	PLAYERINFO *plr = PNum2Plr(pnum);
	PLAYERINFO *cplr, *prev = NULL;
	if (plr)
	{
		for (cplr = FirstPlayer; cplr; cplr = cplr->next)
			if (cplr->next == plr)
				prev = cplr;

		if (prev)
			prev->next = plr->next;
		else
			FirstPlayer = plr->next;

		DeleteCrewList(&plr->crew);
		delete plr;

		AdjustLBCellNum(LID_PLRS, PlrListCount());
	}
	return 0;
}

int PlrEmptyNameCheck(int pnum) // Called on close of PlayerInfEdit
{
	PLAYERINFO *cplr = PNum2Plr(pnum);
	if (cplr->name[0] == 0)
		SCopy("Player with no name", cplr->name);
	return 0;
}

int SetPlrColor(int pnum)
{
	WINDOW *cwin;
	PLAYERINFO *plr = PNum2Plr(pnum);
	if (!plr)
		return 0;
	cwin = NewWindow("Player color", STANDARD, 1, -1, -1, 200, 43, 1, 0, XRVCLOSE, NULL);
	NewObject("ColSelect", SELECTBOX, 1, 0, 20, 11, 160, 16, MenuPage, 9, 12, 0, 7, 0, 20, 0, NULL, &plr->pfcol);
	NewObject("OK", BUTTON, 1, 0, 80, 30, 40, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	if (InitErrorCheck())
		CloseWindow(cwin);
	return 0;
}

int SetPlayerFace(int pnum)
{
	WINDOW *cwin;
	PLAYERINFO *plr = PNum2Plr(pnum);
	OBJECT *dfobj;
	if (!plr)
		return 0;
	cwin = NewWindow("Player portrait", STANDARD, 1, -1, 45, 225, 122, 1, 0, XRVCLOSE, NULL);
	if (plr->name[0])
	{
		sprintf(OSTR, "for %s", plr->name);
		TextObject(OSTR, 225 / 2, 10, CGray2, -1, 1);
	}
	dfobj = NewButton("Delete", 0, 180, 98, 40, 9, 0, 0);
	NewULink(EXCREDR, NULL,
			 NewButton("Import", 0, 180, 87, 40, 9, 0, 0),
			 NULL, &ImportFace, 0);
	NewButton("Help", 0, 180, 76, 40, 9, XRVHELP, 0);
	NewULink(LBCACT, NULL, dfobj,
			 NewObject("FaceList", LISTBOX, 1, 0, 5, 16, 162, 102, LID_FACE, 4, 2, 40, 50, 0, Config.FaceNum, plr->face, NULL, &plr->face),
			 NULL, 1);
	NewObject("Select", BUTTON, 1, 0, 180, 109, 40, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	if (InitErrorCheck())
		CloseWindow(cwin);
	return 0;
}

int CapNameEmptyCheck(int pnum)
{
	PLAYERINFO *plr = PNum2Plr(pnum);
	if (plr)
		if (plr->crew)
			if (!plr->crew->name[0])
				SCopy(GetClonkName(), plr->crew->name, MaxClonkName);
	return 0;
}

void PlayerInfoEdit(PLAYERINFO *plr)
{
	WINDOW *cwin;
	OBJECT *optr;
	int pnum = Plr2PNum(plr);
	DWORD playmin;
	int ftx, fty;

	cwin = NewWindow("Player-Info", CARDBOX, 1, -1, 40, 250, 140, 1, 3, XRVCLOSE, "Contents");

	NewULink(EXCREDR, NULL,
			 NewObject("PlrPic", ICON, 1, 0, 5, 5, 44, 54, FacePage, 8, 0, 0, 0, 40, 50, 0, NULL, &plr->face),
			 NULL, &SetPlayerFace, pnum);

	NewButton("OK", 0, 205, 127, 40, 9, XRVCLOSE, 0);
	NewButton("Help", 0, 205, 116, 40, 9, XRVHELP, 0);

	NewObject("Last quote:", TEXT, 0, 0, 90, 106, 26, 4, 0, 2, 1, CGray2, -1, 0, 0, 0, plr->quote, NULL);

	// Preferences frame
	ftx = 5;
	fty = 100;
	NewObject("Preferences", FRAME, 0, 0, ftx, fty, 82, 35, 1, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);
	NewULink(EXCREDR, NULL,
			 NewObject("Control", SELECTBOX, 1, 0, ftx + 27, fty + 9, 52, 16, MenuPage, 12, 12, 0, 3, 72, 20, 1, NULL, &plr->pfcon),
			 ITextObject(ConTypeName, &plr->pfcon, ftx + 27, fty + 9 + 17, CGray2, CGray5),
			 NULL, 0);
	NewULink(EXCREDR, NULL,
			 NewObject("Color", ICON, 1, 0, ftx + 3, fty + 9, 19, 16, MenuPage, 8, 1, 0, 20, 9, 12, 0, NULL, &plr->pfcol),
			 NULL, &SetPlrColor, pnum);

	sprintf(OSTR, "Rank: %s", PlrRankName(plr->rank));
	TextObject(OSTR, 52, 23, CGray1);
	sprintf(OSTR, "Total score: %" PRIu32, PlrTotalScore(plr));
	TextObject(OSTR, 52, 29, CGray1);

	playmin = plr->playsec / 60;
	sprintf(OSTR, "Total playing time: %" PRIu32 ":%02" PRIu32 ":%02" PRIu32, playmin / 60, playmin % 60, plr->playsec % 60);
	TextObject(OSTR, 52, 92, CGray1);

	NewObject("Statistics", FRAME, 0, 0, 52, 40, 192, 49, 1, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);
	TextObject(" Mission  Single   Melee   Total", 110, 47, CDRed);
	sprintf(OSTR, "        Score  %6" PRIu32 "  %6" PRIu32 "  %6" PRIu32 "  %6" PRIu32, plr->score[0], plr->score[1], plr->score[2], PlrTotalScore(plr));
	TextObject(OSTR, 57, 55, CGray1);
	sprintf(OSTR, " Total rounds    %4d    %4d    %4d    %4d", plr->rnds[0], plr->rnds[1], plr->rnds[2], PlrTotalRounds(plr));
	TextObject(OSTR, 57, 61, CGray2);
	sprintf(OSTR, "          Won    %4d    %4d    %4d    %4d", plr->won[0], plr->won[1], plr->won[2], PlrTotalWon(plr));
	TextObject(OSTR, 57, 67, CDRed);
	sprintf(OSTR, "         Lost    %4d    %4d    %4d    %4d", plr->rnds[0] - plr->won[0], plr->rnds[1] - plr->won[1], plr->rnds[2] - plr->won[2], PlrTotalRounds(plr) - PlrTotalWon(plr));
	TextObject(OSTR, 57, 73, CDBlue);
	sprintf(OSTR, "      Average  %6.1f  %6.1f  %6.1f  %6.1f", PlrAverage(plr, 0), PlrAverage(plr, 1), PlrAverage(plr, 2), PlrTotalAverage(plr));
	TextObject(OSTR, 57, 79, CGray1);
	NewObject("RoundBack", FRAME, 0, 0, 114, 54, 27, 31, 0, 0, 0, -1, CWhite, -1, 0, 0, NULL, NULL);
	NewObject("RoundBack", FRAME, 0, 0, 146, 54, 27, 31, 0, 0, 0, -1, CWhite, -1, 0, 0, NULL, NULL);
	NewObject("RoundBack", FRAME, 0, 0, 178, 54, 27, 31, 0, 0, 0, -1, CWhite, -1, 0, 0, NULL, NULL);
	NewObject("RoundBack", FRAME, 0, 0, 210, 54, 27, 31, 0, 0, 0, -1, CWhite, -1, 0, 0, NULL, NULL);

	NewObject("CrewPic", PICTURE, 0, 0, 5, 63, 20, 10, MenuPage, 0, 0, 175, 25, 0, 0, 0, NULL, NULL);
	optr = NewObject("Crew", BUTTON, 1, 0, 5, 74, 44, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	NewULink(EXCREDR, NULL, optr, NULL, &CrewList, pnum);

	NewObject("Name", TEXTBOX, 1, 0, 52, 11, 84, 10, 1, XRVCLOSE, 0, MaxPlayerName, Min(SLen(plr->name), MaxPlayerName - 1), 0, 0, 0, plr->name, NULL),

		NewULink(CLOSEXC, cwin, NULL, NULL, &SortPlayerList, 0); // Called second!!!
	NewULink(CLOSEXC, cwin, NULL, NULL, &PlrEmptyNameCheck, pnum);

	if (InitErrorCheck())
		CloseWindow(cwin);
}

int ImportPlayers2List(int dummy)
{
	LineInput("Path and file name of CLONK.DAT to be loaded:", DatImportName, 45, &PlayerImport, 0, "Player import");
	Message("With this option you can import|player data from the CLONK.DAT file of a|different CLONK game.", "Player import");
	return dummy;
}

int NewPlayer2List(int dummy)
{
	PLAYERINFO *nplr;
	if (!(nplr = NewDefaultPlr2List()))
	{
		Message("Out of memory!|No new players.");
		return dummy;
	}
	NewMan2List(&nplr->crew, GetClonkName(), 1, 1000, 0, 0);
	PlayerInfoEdit(nplr);
	AdjustLBCellNum(LID_PLRS, PlrListCount());
	return dummy;
}

void PlrListCellAction(int num, int aid)
{
	PLAYERINFO *cplr = PNum2Plr(num);
	if (cplr)
		switch (aid)
		{
		case 0:
			PlayerInfoEdit(cplr);
			break;
		case 1:
			Toggle(cplr->inses);
			break;
		case 2:
			sprintf(OSTR, "Are you sure you want to|delete player %s?", cplr->name);
			ConfirmedCall(OSTR, 0, &DeletePlrFromList, num);
			break;
		}
}

//------------------------ Scenario List Handling ----------------------------

BYTE ScriptIDNumExists(int idnum)
{
	SCRIPTINFO *scr;
	for (scr = FirstScript; scr; scr = scr->next)
		if (scr->idnum == idnum)
			return 1;
	return 0;
}

BYTE ReadFile2NewScriptInfo(FILE *fhnd) //char *fname)
{
	SCRIPTINFO *nscr;
	//FILE *fhnd;
	BYTE fokay = 1;
	int fnlen;

	if (!(nscr = new SCRIPTINFO))
		return 0;
	//if (!(fhnd=fopen(fname,"r"))) { delete nscr; return 0; }
	//SCopy(fname,nscr->fname,29);

	fnlen = ReadFileUntil(fhnd, nscr->fname, ']', 13);
	nscr->fname[fnlen] = 0;
	if (fnlen < 5)
		fokay = 0;

	if (!ReadFileInfoLine(fhnd, "Head=", OSTR))
		fokay = 0;
	if (!SEqual(OSTR, "Clonk3Script"))
		fokay = 0;
	if (!ReadFileInfoLine(fhnd, "Title=", nscr->title, ScrTitleLen))
		fokay = 0;
	if (!ReadFileInfoLine(fhnd, "Desc=", nscr->desc, ScrDescLen))
		fokay = 0;
	if (!ReadFileInfoLine(fhnd, "IDNum=", OSTR))
		fokay = 0;
	nscr->idnum = strtol(OSTR, NULL, 10);
	if (!Inside(nscr->idnum, 1, 999) || ScriptIDNumExists(nscr->idnum))
		fokay = 0;
	if (!ReadFileInfoLine(fhnd, "RuleSet=", OSTR))
		fokay = 0;
	nscr->ruleset = strtol(OSTR, NULL, 10);
	if (!Inside(nscr->ruleset, 0, 2))
		fokay = 0;

	//fclose(fhnd);
	if (!fokay)
	{
		delete nscr;
		return 0;
	}

	nscr->next = FirstScript;
	FirstScript = nscr;
	return 1;
}

int ScriptListCount(void)
{
	int rval;
	SCRIPTINFO *cscr;
	for (cscr = FirstScript, rval = 0; cscr; cscr = cscr->next, rval++)
		;
	return rval;
}

void SortScriptList(void)
{
	BYTE change;
	SCRIPTINFO *scr, scrdat1, scrdat2, *scrptr1, *scrptr2, *tmnnxt1, *tmnnxt2;
	do
	{
		change = 0;
		for (scr = FirstScript; scr && scr->next; scr = scr->next)
			if (scr->idnum > scr->next->idnum)
			{
				scrptr1 = scr;
				scrptr2 = scr->next;
				scrdat1 = *scrptr1;
				scrdat2 = *scrptr2;
				tmnnxt1 = scrdat1.next;
				tmnnxt2 = scrdat2.next;
				scrdat1.next = tmnnxt2;
				scrdat2.next = tmnnxt1;
				*scrptr1 = scrdat2;
				*scrptr2 = scrdat1;
				change = 1;
			}
	} while (change);
}

int InitScriptList(void)
{
	FILE *fhnd;
	//char fname[13];
	int rnum = 0;

	//InitFileSearch("MISSION\\*.SCR");  Multi-file search
	//while (SearchNextFile(fname))
	//  {
	//  sprintf(OSTR,"MISSION\\%s",fname);
	//  if (ReadFile2NewScriptInfo(OSTR)) rnum++;
	//  }

	if ((fhnd = fopen("MISSIONS.SCR", "rt")) != 0)
	{
		while (LocateInFile(fhnd, "[NewFile=", 0))
			if (ReadFile2NewScriptInfo(fhnd))
				rnum++;
		fclose(fhnd);
	}

	SortScriptList();
	MissionScriptNum = 0;
	if (rnum == 0)
		MissionScriptNum = -1;
	MissionScript.idnum = -1;
	if (BSA.NextMission == 0)
		if (FirstScript)
			BSA.NextMission = FirstScript->idnum;
	return rnum;
}

void DeInitScriptList(void)
{
	SCRIPTINFO *cscr, *nscr = NULL;
	for (cscr = FirstScript; cscr; cscr = nscr)
	{
		nscr = cscr->next;
		delete cscr;
	}
	FirstScript = NULL;
}

void DrawScriptListCell(int num, int tx, int ty, BYTE sel)
{
	SCRIPTINFO *scr = NULL;
	BYTE hide;
	if (num > -1)
		for (scr = FirstScript; (num > 0) && scr; scr = scr->next, num--)
			;
	DBox(tx, ty, tx + 171, ty + 13, sel ? CGray5 : CWhite);
	if (scr)
	{
		hide = (scr->idnum > BSA.NextMission); // PAM
		sprintf(OSTR, "%d) %s", scr->idnum, scr->title);
		SOutS(OSTR, tx + 1, ty + 1, hide ? CGray3 : CDGreen, CGray4);
		SOut(scr->desc, tx + 1, ty + 8, hide ? CGray3 : CGray1);
		SOutS(RuleSetName[scr->ruleset], tx + 170, ty + 1, hide ? CGray3 : RuleSetCol[scr->ruleset], CGray4, 2);
	}
}

BYTE ScriptListCellAction(int num, int aid)
{
	SCRIPTINFO *scr = NULL;
	if (num > -1)
		for (scr = FirstScript; (num > 0) && scr; scr = scr->next, num--)
			;
	if (scr)
		switch (aid)
		{
		case 0:
			return 1; // XRV Start round
		default:
			Message("undef. aid on scrlistcellact");
		}
	return 0;
}

//---------------------- BSA Setting & Variable Handling ---------------------

void ClearBSAPlrPtrs(void)
{
	int cnt;
	for (cnt = 0; cnt < MaxGamePlr; cnt++)
	{
		BSA.Plr[cnt].Info = NULL;
		BSA.Plr[cnt].TCrew = NULL;
		BSA.Plr[cnt].Col = -1;
	}
}

int RealismValue(int dummy)
{
	REALISM *rlm = &(BSA.Realism);
	DWORD total = 0, rval = 0;
	total += 10;
	rval += rlm->StrcSnow * 10;
	total += 25;
	rval += rlm->StrcBurn * 25;
	total += 25;
	rval += rlm->StrcEnrg * 25;
	total += 50;
	rval += !rlm->EmrPromo * 50;
	total += 25;
	rval += !rlm->WtrOut * 25;
	total += 10;
	rval += rlm->RckOut * 10;
	total += 20;
	rval += !rlm->CstHome * 20;
	dummy++;
	return 100L * rval / total;
}

int DifficultyValue(int dummy)
{
	DWORD total = 0, rval = 0, gmfact;
	total += 30;
	rval += (BSA.RuleSet + 1) * 10;
	total += 10;
	rval += !BSA.AllowSurrender * 10;

	total += 20;
	rval += 20 * (6 - BSA.Plr[0].Clonks) / 5; // assumes all plrs equal...
	total += 40;
	rval += 40 * (200 - BSA.Plr[0].Wealth) / 200;
	total += 15;
	rval += !BSA.Plr[0].ReadyBase[0] * 15;
	total += 10;
	rval += !BSA.Plr[0].ReadyBase[1] * 10;
	total += 10;
	rval += !BSA.Plr[0].ReadyBase[2] * 10;
	total += 5;
	rval += !BSA.Plr[0].ReadyBase[3] * 5;
	total += 5;
	rval += !BSA.Plr[0].ReadyBase[4] * 5;
	total += 3;
	rval += !BSA.Plr[0].ReadyBase[0] * 3;
	total += 5;
	rval += !BSA.Plr[0].ReadyBase[1] * 5;
	total += 5;
	rval += !BSA.Plr[0].ReadyBase[2] * 5;
	total += 5;
	rval += !BSA.Plr[0].ReadyBase[3] * 5;
	total += 15;
	rval += !BSA.Plr[0].ReadyBase[4] * 15;

	gmfact = 1;
	if ((BSA.SMode == S_COOPERATE) && (BSA.CoopGMode == C_WIPFRESCUE))
		gmfact = 5;
	total += gmfact * 15;
	rval += gmfact * BSA.Wipfs * 1.5;
	total += 30;
	rval += BSA.Sharks * 3.0;
	gmfact = 1;
	if ((BSA.SMode == S_COOPERATE) && (BSA.CoopGMode == C_MONSTERKILL))
		gmfact = 3;
	total += gmfact * 50;
	rval += gmfact * BSA.Monsters * 5.0;

	gmfact = 1;
	if ((BSA.SMode == S_COOPERATE) && (BSA.CoopGMode == C_GOLDMINE))
		gmfact = 3;
	total += gmfact * 19;
	rval += gmfact * (BSA.BSize - 160) / 10;
	total += 15;
	rval += 15 * BSA.BCrvAmp / 100;
	total += 15;
	rval += 15 * BSA.BRndAmp / 100;
	total += 20;
	rval += 20 * BSA.BWatLvl / 100;
	total += 15;
	rval += 15 * BSA.BVolcLvl / 100;
	total += 10;
	rval += 10 * BSA.BQuakeLvl / 100;
	total += 10;
	rval += 10 * BSA.BCanyon;

	total += 15;
	rval += 15 * Abs(BSA.WClim - 500) / 500;
	total += 15;
	rval += 15 * BSA.WYSpd / 100;
	total += 20;
	rval += 20 * (BSA.WRFMod + 50) / 100;
	total += 20;
	rval += 20 * (BSA.WLPMod + 50) / 100;
	total += 15;
	rval += 15 * BSA.WCmtLvl / 100;
	total += 15;
	rval += 15 * (BSA.WEnvr == 1);

	total += 200;
	rval += 200 * (20 - BSA.FlintRadius) / 10;
	total += 200;
	rval += 200 * (5 - BSA.RckProdSpeed) / 4;

	dummy++;
	return 100L * rval / total;
}

void RndInitBackground(void)
{
	int rsper = (BSA.RuleSet + 1) * 100 / 3;
	// Absolute random default
	BSA.BCrvAmp = 0;
	BSA.BRndAmp = rsper;
	BSA.BCanyon = 0;

	if (!random(2))
	{
		BSA.BCrvAmp = random(rsper);
		BSA.BRndAmp = 20 + random(80);
		BSA.BPhase = random(100);
		BSA.BPLen = random(100);
		if (BSA.RuleSet > R_EASY)
			if (!random(10))
				BSA.BCanyon = 1;
	}

	BSA.BWatLvl = 0;
	BSA.BVolcLvl = 0;
	BSA.BQuakeLvl = 0;
	if (BSA.RuleSet > R_EASY)
	{
		if (!random(6))
			BSA.BVolcLvl = random(101);
		if (!random(12))
			BSA.BQuakeLvl = random(101);
	}
	if (!random(3) || BSA.Sharks)
		BSA.BWatLvl = random(Max(BSA.BCrvAmp, BSA.BRndAmp));

	// Game mode adjust
	if (BSA.SMode == S_MELEE)
	{
		if (!random(3))
		{
			BSA.BRndAmp = 20;
			BSA.BCrvAmp = 40 + random(BSA.RuleSet * 30);
			if (BSAPlrNum() == 2) // Hill/Valley
			{
				BSA.BPhase = 70;
				BSA.BPLen = 53;
			}
			if (BSAPlrNum() == 3) // Triple Hill/Valley
			{
				BSA.BPhase = 75;
				BSA.BPLen = 100;
			}
			BSA.BWatLvl = random(100);
			if (!random(2))
				BSA.BWatLvl = 70 + random(30);
		}
	}
}

void RndInitAnimals(void)
{
	if (BSA.RuleSet == R_EASY)
		BSA.Sharks = 0;
	else
		BSA.Sharks = random(4 + 3 * BSA.RuleSet);
	BSA.Wipfs = 0 + (!random(3)) * random(4 + 3 * BSA.RuleSet);
	BSA.Monsters = 0 + (!random(3)) * random(2 + 2 * BSA.RuleSet);
}

void RndInitWeather(void)
{
	if (BSA.RuleSet == R_EASY)
	{
		BSA.WClim = 300 + 600 * random(2);
		BSA.WSeas = 750 + 1000 * random(2);
		BSA.WYSpd = 0;
	}
	else
	{
		BSA.WClim = random(1000);
		BSA.WSeas = random(2000);
		BSA.WYSpd = random(101);
	}
	BSA.WRFMod = random(100) - 50;
	BSA.WLPMod = random(2) * (random(60) - 50);
	if ((BSA.RuleSet == R_EASY) || !random(3))
	{
		if (!random(2))
			BSA.WRFMod = -50;
		if (!random(2))
			BSA.WLPMod = -50;
	}
	BSA.WCmtLvl = random(2) * random(10 + 20 * BSA.RuleSet);
	BSA.WEnvr = 0;
	if (BSA.RuleSet > R_EASY)
		if (!random(10))
			BSA.WEnvr = 1 + random(2);
}

void DefBSAPlayer(int plr, BYTE newinit)
{
	int cnt;
	// First time init
	if (newinit)
	{
		BSA.Plr[plr].Info = NULL;
		BSA.Plr[plr].TCrew = NULL;
		BSA.Plr[plr].Col = -1;
		BSA.Plr[plr].Con = 0;
	}
	// Round defaults
	BSA.Plr[plr].Clonks = 1;
	BSA.Plr[plr].Wealth = 100;
	for (cnt = 0; cnt < 5; cnt++)
	{
		BSA.Plr[plr].ReadyBase[cnt] = BSA.Plr[plr].ReadyVhc[cnt] = 0;
	}
	BSA.Plr[plr].Eliminate = 0;
	BSA.Plr[plr].ScoreGain = 0;
}

void DefaultBSA(BYTE newinit)
{
	const int rpsmdf[RckOrderNum] = {5, 5, 0, 5, 0, 0, 0, 3, 3, 3, 0, 0, 1, 0, 0};
	const int rptdef[RckOrderNum] = {60, 40, 180, 20, 50, 120, 160, 120, 80, 60, 70, 80, 60, 120, 120};
	int cnt;
	// Game - First time init
	if (newinit)
	{
		BSA.SMode = S_MISSION;
		BSA.RuleSet = R_EASY;
		BSA.Round = 0;
		BSA.PlrsInRound = 0;
		BSA.PCrew = 0;
		BSA.TMode = 0;
		BSA.NextMission = 0;
	}
	// Game - Round defaults
	for (cnt = 0; cnt < RckOrderNum; cnt++)
	{
		BSA.RckProdStart[cnt] = rpsmdf[cnt]; // <- always setting for MISSION
		BSA.RckProdTime[cnt] = rptdef[cnt];	 //    COOP/MELEE setting in AdjBSA2SMd
	}
	BSA.RckProdSpeed = 1;
	BSA.AllowSurrender = 1;
	BSA.CoopGMode = C_GOLDMINE;
	BSA.GPlrElm = 0;
	// Players
	for (cnt = 0; cnt < MaxGamePlr; cnt++)
		DefBSAPlayer(cnt, newinit);
	// Animals
	BSA.AnmRandom = 0;
	BSA.Wipfs = 0;
	BSA.Sharks = 0;
	BSA.Monsters = 0;
	// Landscape
	BSA.BckRandom = 0;
	BSA.BSize = 165;
	BSA.BPhase = 50, BSA.BPLen = 50;
	BSA.BCrvAmp = 30, BSA.BRndAmp = 30;
	BSA.BWatLvl = 0;
	BSA.BVolcLvl = 0, BSA.BQuakeLvl = 0;
	BSA.BCanyon = 0;
	for (cnt = 0; cnt < 20; cnt++)
		BSA.BRockType[cnt] = 0; // ROCK
	BSA.BRockMode = 0;
	// Weather
	BSA.WeaRandom = 0;
	BSA.WClim = 500, BSA.WSeas = 1000, BSA.WYSpd = 10;
	BSA.WRFMod = -50, BSA.WLPMod = -50;
	BSA.WCmtLvl = 0;
	BSA.WEnvr = 0;
	// Special
	BSA.FlintRadius = 12;
	// Realism
	BSA.Realism.StrcSnow = 0;
	BSA.Realism.StrcBurn = 1;
	BSA.Realism.StrcEnrg = 1;
	BSA.Realism.WtrOut = 0;
	BSA.Realism.RckOut = 0;
	BSA.Realism.EmrPromo = 1;
	BSA.Realism.CstHome = 1;
	BSA.Realism.RealVal = RealismValue(0);
	// Difficulty
	BSA.DiffVal = DifficultyValue(0);
}

int AdjustBSA2SMode(int dummy)
{
	const int rpsdef[RckOrderNum] = {5, 5, 1, 8, 5, 2, 3, 5, 8, 10, 4, 3, 3, 1, 1};
	int cnt;
	switch (BSA.SMode)
	{
	case S_MISSION:
		BSA.RckProdSpeed = 0;
		break;
	case S_COOPERATE:
		for (cnt = 0; cnt < RckOrderNum; cnt++)
			BSA.RckProdStart[cnt] = rpsdef[cnt];
		if (BSA.RckProdSpeed == 0)
			BSA.RckProdSpeed = 1;
		BSA.GPlrElm = 1;
		BSA.AllowSurrender = 1;
		break;
	case S_MELEE:
		for (cnt = 0; cnt < RckOrderNum; cnt++)
			BSA.RckProdStart[cnt] = rpsdef[cnt];
		if (BSA.RckProdSpeed == 0)
			BSA.RckProdSpeed = 1;
		break;
	}
	return dummy;
}

void DefRealByRuleSet(void)
{
	BSA.Realism.StrcSnow = 0;
	BSA.Realism.StrcBurn = (BSA.RuleSet > R_EASY);
	BSA.Realism.StrcEnrg = (BSA.RuleSet > R_ADVANCED);
	BSA.Realism.WtrOut = (BSA.RuleSet < R_RADICAL);
	BSA.Realism.RckOut = (BSA.RuleSet > R_EASY);
	BSA.Realism.EmrPromo = 1;
	BSA.Realism.CstHome = (BSA.RuleSet < R_RADICAL);
	BSA.Realism.RealVal = RealismValue(0);
}

int AdjustBSA2RuleSet(int setdef) // setdef (set to default choices)
{								  // if switching rule sets
	int cnt;
	switch (BSA.RuleSet)
	{
	case R_EASY:
		// Unchooseable (required)
		BSA.Plr[0].Wealth = 0;
		for (cnt = 1; cnt < 5; cnt++)
			BSA.Plr[0].ReadyBase[cnt] = 0;
		for (cnt = 2; cnt < 5; cnt++)
			BSA.Plr[0].ReadyVhc[cnt] = 0;
		BSA.BckRandom = BSA.WeaRandom = 1;
		BSA.Sharks = 0;
		if (setdef)
		{
			// Chooseable (default only)
			BSA.CoopGMode = BoundBy(BSA.CoopGMode, 0, 2);
			BSA.Plr[0].ReadyBase[0] = 1;
		}
		break;
	case R_ADVANCED:
		// Unchooseable (required)
		BSA.Plr[0].ReadyBase[2] = 0;
		BSA.Plr[0].ReadyBase[3] = 0;
		for (cnt = 3; cnt < 5; cnt++)
			BSA.Plr[0].ReadyVhc[cnt] = 0;
		if (setdef)
		{
			// Chooseable (default only)
			BSA.Plr[0].Wealth = 20;
		}
		break;
	case R_RADICAL:
		if (setdef)
		{
			// Chooseable (default only)
			BSA.Plr[0].Wealth = 50;
			BSA.Plr[0].ReadyVhc[4] = 1;
		}
		break;
	}
	DefRealByRuleSet();
	return 0;
}

/*void AdjustBSA4Sharks(void)
  {
  BSA.WClim=Min(BSA.WClim,600); BSA.WSeas=750; BSA.WYSpd=0; BSA.WEnvr=0;
  BSA.Realism.WtrOut=0;
  // make sure lscape has water
  }*/

void AdjustBSA2CoopGMode(void)
{
	switch (BSA.CoopGMode)
	{
	case C_GOLDMINE:							  // Adjust climate for gold presence
		BSA.WClim = BoundBy(BSA.WClim, 350, 999); // make sure!!!
		break;
	case C_MONSTERKILL: // Adjust animal presence
		if (BSA.Monsters == 0)
			BSA.Monsters = 1 + random(2 + 2 * BSA.RuleSet);
		break;
	case C_WIPFRESCUE:
		if (BSA.Wipfs == 0)
			BSA.Wipfs = 3 + random(5 + 5 * BSA.RuleSet);
		break;
	}
}

extern BYTE LoadMissionPreInitBSA(void);

BYTE SetBSA4Mission(void)
{
	SCRIPTINFO *scr = NULL;
	int num = MissionScriptNum;
	BYTE rokay = 0;
	DefaultBSA(0);
	if (num > -1)
		for (scr = FirstScript; (num > 0) && scr; scr = scr->next, num--)
			;
	if (scr)
	{
		MissionScript = *scr;
		if (MissionScript.idnum > BSA.NextMission)
			Message("This mission is not yet available to you.", "Mission not yet available");
		else //PAM
		{
			BSA.RuleSet = MissionScript.ruleset;
			if (LoadMissionPreInitBSA())
			{
				DefRealByRuleSet();
				BSA.DiffVal = DifficultyValue(0);
				rokay = 1;
			}
			else
			{
				sprintf(OSTR, "Error reading mission file|�%c%s", CDRed, MissionScript.fname);
				Message(OSTR, "Program file error");
			}
		}
	}
	return rokay;
}

//--------------- Major SVI Menu Set Functions & Menu Sub Routines ----------------

int CheckMissionPassword(int dummy)
{ // Hardcoded stuff
	int jumpto = -1;
	Capitalize(OSTR);
	if (SEqualL(OSTR, "WIPFEMONSTERSCHNEE"))
		jumpto = 13;
	if (SEqualL(OSTR, "VULKANBEBENAUSBRUCH"))
		jumpto = 23;
	if (SEqualL(OSTR, "DASWARHARTEARBEIT"))
		jumpto = 30;
	if (jumpto > -1)
		BSA.NextMission = Max(BSA.NextMission, jumpto);
	else
		Message("Not a valid mission password", "Mission passwords");
	return dummy;
}

int EnterMissionPassword(int dummy)
{
	OSTR[0] = 0;
	LineInput("Enter mission password (F1 for help)", OSTR, 47, &CheckMissionPassword, 0, "Mission passwords");
	return dummy;
}

int SetSortBy(int dummy)
{
	WINDOW *cwin;
	cwin = NewWindow("Sort roster:", STANDARD, 1, -1, -1, 100, 100, 1, 0, XRVCLOSE, "Contents");
	NewObject("SortSelector", SELECTOR, 1, 0, 10, 15, 80, 8 * 8 + 1, 0, 0, 0, 8, 0, 0, 0, 0, "Total|Mission|Cooperative|Melee|Total Average|Mission Average|Coop. Average|Melee Average", &Config.SortPlrBy);
	NewButton("OK", 0, 30, 87, 40, 9, XRVCLOSE, 0);
	NewULink(CLOSEXC, cwin, NULL, NULL, &SortPlayerList, 0);
	if (InitErrorCheck())
		CloseWindow(cwin);
	return dummy;
}

int SetGameMode(int dummy)
{
	OBJECT *optr, *optr2;
	WINDOW *cwin;

	cwin = NewWindow("Cooperative Mode", STANDARD, 1, -1, -1, 220, 104, 1, 0, XRVCLOSE, "Contents");

	optr = NewObject("CoopModeSelect", SELECTBOX, 1, 0, 10, 11, 200, 34, MenuPage, 35, 30, 0, (BSA.RuleSet > R_EASY) ? 3 : 2, 0, 43, 0, NULL, &BSA.CoopGMode);

	NewULink(EXCREDR, NULL, optr, ITextObject(CGModeName, &BSA.CoopGMode, 12, 49, CGray1), NULL, 0);
	NewULink(EXCREDR, NULL, optr, NewObject("GModeDesc", TEXT, 0, 0, 12, 57, 49, 5, 0, 2, 0, CGray2, -1, 0, 0, 0, ((char *)CGModeDesc), &BSA.CoopGMode), NULL, 0);
	NewULink(EXCREDR, NULL, optr, NewObject("TextBack", FRAME, 0, 0, 10, 47, 200, 41, 0, 0, 0, -1, CGray5, 0, 0, 0, NULL, NULL), NULL, 0);

	NewObject("OK", BUTTON, 1, 0, 90, 91, 40, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

	if (InitErrorCheck())
		CloseWindow(cwin);
	return dummy;
}

int OpenCheatMenu(int dummy)
{
	WINDOW *cwin;
	/*if (!Registered())
	  { Message("There are some fun extra options|in the cheat menu. However, this is|available to registered players only.","Cheat-Menu"); return dummy; }*/

	cwin = NewWindow("Cheat-Menu", STANDARD, 1, -1, -1, 200, 100, 1, 0, XRVCLOSE, "Contents");

	TextObject("Armageddon:", 130, 14, CGray2);
	NewFlagBox("Comet storm", 130, 24, &BSA.CometHail);

	NewObject("Production speed", VALBOXB, 1, 0, 15, 40, 20, 9, 1, 'X', 0, 1, 3, 1, 0, 0, NULL, &BSA.RckProdSpeed);

	NewObject("Flintstone radius", VALBOXB, 1, 0, 15, 20, 20, 9, 1, 0, 0, 10, 20, 1, 0, 0, NULL, &BSA.FlintRadius);

	NewButton("OK", 0, 75, 88, 50, 9, XRVCLOSE, 1);

	if (InitErrorCheck())
		CloseWindow(cwin);
	return dummy;
}

int OpenUserPrefWindow(int ingame)
{
	WINDOW *cwin;
	if (ingame)
		cwin = NewWindow("Special", CARDBOX, 1, 5, 33, 310, 140, 6, 6, XRVCLOSE, "Contents");
	else
		DrawWindow(cwin = NewWindow("Special", CARDBOX, 0, 5, 35, 310, 150, 6, 6, 2, "Contents"));
	if (ingame)
		NewButton("Close", 0, 220, 125, 40, 9, XRVCLOSE, 0);
	if (!ingame)
		NewULink(EXCREDR, NULL,
				 NewButton("Cheat-Menu", 0, 5, 135, 60, 9, 0, 0),
				 NULL, &OpenCheatMenu, 0);
	NewObject("Game speed", VALBOXB, 1, 0, 195, 100, 20, 9, 1, 0, 0, 1, 10, 1, 0, 0, NULL, &Config.GameSpeed);
	TextObject("(F5/F6 in game)", 195, 111, CGray2);
	TextObject("Graphics", 195, 5, CGray2);
	NewObject("Lines in front|of structs", FLAGBOX, 1, 0, 195, 15, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &UserPref.LineBeforeStrc);
	TextObject("Game", 100, 5, CGray2);
	NewObject("Uncontrolled|Clonks halt", FLAGBOX, 1, 0, 100, 28, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &UserPref.CeaseAction);
	NewObject("Messages", FLAGBOX, 1, 0, 100, 15, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &UserPref.GameMsgOn);
	/*TextObject("Maussteuerung",5,95,CGray2);
	NewFlagBox("Zwei-Button-Springen",5,105,&UserPref.TwoButtonJump);
	sprintf(OSTR,"Angeschlossene Maus|hat %d Buttons.",MouseType);
	TextObject(OSTR,5,117,CGray2);*/
	TextObject("Dig double click", 5, 55, CGray2);
	NewObject("Slow            Fast", HSCROLL, 1, 0, 5, 65 + 6, 80, 5, 1, 0, 0, 0, 100, 1, 10, 0, NULL, &UserPref.DoubleDigSpeed);
	TextObject("Detail", 5, 5, CGray2);
	NewObject("Smoke", FLAGBOX, 1, 0, 5, 15, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &UserPref.SmokeOn);
	NewButton("Help", 0, 263, 135 - 10 * ingame, 40, 9, XRVHELP, 1);
	if (InitErrorCheck())
		CloseWindow(cwin);
	return 0;
}

void CreatePlayerInfoBox(BYTE cplr, int tx, int ty)
{
	if ((BSA.Plr[cplr].Col == -1) || (!BSA.Plr[cplr].Info))
		return;

	NewObject("Player", FRAME, 0, 0, tx, ty, 97, 60, 1, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);
	NewObject("PlrPic", PICTURE, 0, 0, tx + 2, ty + 2, 40, 50, FacePage, 0, 8, 0, 0, 0, 0, 0, NULL, &(BSA.Plr[cplr].Info->face));

	CTextObject(BSA.Plr[cplr].Info->name, tx + 2, ty + 53, CGray1);

	NewObject("Score", VALBOX, 0, 0, tx + 43, ty + 2 + 6, 40, 7, 1, 0, 1, 0, 0, 0, 1, 0, NULL, (int *)&(BSA.Plr[cplr].Info->score[BSA.SMode]));

	NewULink(EXCREDR, NULL,
			 NewObject("Crew", BUTTON, 1, !BSA.PCrew, tx + 43, ty + 16, 52, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL),
			 NULL, &CrewList, Plr2PNum(BSA.Plr[cplr].Info));

	NewObject("Con", PICTURE, 0, 0, tx + 43, ty + 26, 12, 12, MenuPage, 0, 4, 72, 20, 0, 0, 0, NULL, &BSA.Plr[cplr].Con);

	NewObject("Color", PICTURE, 0, 0, tx + 85, ty + 3, 9, 12, MenuPage, 0, 8, 0, 20, 0, 0, 0, NULL, &BSA.Plr[cplr].Col);
}

BYTE InitCSBMenuPages(void)
{
	int cnt;
	BYTE hd;
	OBJECT *objptr, *objptr2;
	WINDOW *winptr, *realismwin = 0;

	// Special Page (User Prefs) ------------------------------------------------

	OpenUserPrefWindow(0);

	// Realism Page ------------------------------------------------------------
	if (BSA.SMode > S_MISSION)
		if (BSA.RuleSet > R_EASY)
		{
			if (BSA.RuleSet < R_RADICAL)
				hd = 1;
			else
				hd = 0;

			realismwin = NewWindow("Realism", CARDBOX, 0, 5, 35, 310, 150, 5, 6, 2, "Contents");
			DrawWindow(realismwin);

			NewObject("(StrcSnow)", PICTURE, 0, hd, 5, 5, 30, 20, MenuPage, 0, 0, 0, 175, 0, 0, 0, NULL, NULL);
			NewObject("(StrcBurn)", PICTURE, 0, 0, 5, 28, 30, 20, MenuPage, 0, 0, 30, 175, 0, 0, 0, NULL, NULL);
			NewObject("(StrcEnrg)", PICTURE, 0, hd, 5, 51, 30, 20, MenuPage, 0, 0, 60, 175, 0, 0, 0, NULL, NULL);
			NewObject("(EmrPromo)", PICTURE, 0, 0, 100, 5, 30, 20, MenuPage, 0, 0, 90, 175, 0, 0, 0, NULL, NULL);
			NewObject("(WtrOut)", PICTURE, 0, 0, 100, 28, 30, 20, MenuPage, 0, 0, 120, 175, 0, 0, 0, NULL, NULL);
			NewObject("(RckOut)", PICTURE, 0, (BSA.RuleSet == R_RADICAL), 100, 51, 30, 20, MenuPage, 0, 0, 150, 175, 0, 0, 0, NULL, NULL);

			NewULink(EXCREDR, NULL,
					 NewObject("Home base", SELECTBOX, 1, hd, 195, 11, 64, 22, MenuPage, 30, 20, 0, 1, 90, 155, 1, NULL, &BSA.Realism.CstHome),
					 ITextObject(HomeBaseName, &BSA.Realism.CstHome, 195, 34, CGray1, CGray5), NULL, 0);

			NewObject("Rocks|out of|bounds", FLAGBOX, 1, (BSA.RuleSet == R_RADICAL), 131, 51, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.Realism.RckOut);
			NewObject("Screen limit|drains", FLAGBOX, 1, 0, 131, 28, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.Realism.WtrOut);
			NewObject("Promotion|(Exception)", FLAGBOX, 1, 0, 131, 5, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.Realism.EmrPromo);
			NewObject("Structures|need energy", FLAGBOX, 1, hd, 36, 51, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.Realism.StrcEnrg);
			NewObject("Structures|burn", FLAGBOX, 1, 0, 36, 28, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.Realism.StrcBurn);
			NewObject("Structures|snow in", FLAGBOX, 1, hd, 36, 5, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.Realism.StrcSnow);

			NewButton("Help", HIDEOFF, 268, 135, 35, 9, XRVHELP, 1); // HILFE wieder hinzugef�gt
		}

	// Weather Page ------------------------------------------------------------
	if (BSA.SMode > S_MISSION)
		if (BSA.RuleSet > R_EASY)
		{
			hd = BSA.WeaRandom;

			DrawWindow(NewWindow("Weather", CARDBOX, 0, 5, 35, 310, 150, 4, 6, 2, "Contents"));

			NewObject("Climate", PICTURE, 0, hd, 5, 5, 300, 35, MenuPage, 2, 0, 0, 100, 0, 0, 0, NULL, NULL);
			NewObject("Season", PICTURE, 0, hd, 5, 55, 20, 20, MenuPage, 1, 0, 0, 135, 0, 0, 0, NULL, NULL);
			NewObject("(Summer)", PICTURE, 0, hd, 27, 55, 20, 20, MenuPage, 0, 0, 20, 135, 0, 0, 0, NULL, NULL);
			NewObject("(Fall)", PICTURE, 0, hd, 49, 55, 20, 20, MenuPage, 0, 0, 40, 135, 0, 0, 0, NULL, NULL);
			NewObject("(Winter)", PICTURE, 0, hd, 71, 55, 20, 20, MenuPage, 0, 0, 60, 135, 0, 0, 0, NULL, NULL);
			NewObject("(Speed)", PICTURE, 0, hd, 93, 55, 20, 20, MenuPage, 0, 0, 280, 135, 0, 0, 0, NULL, NULL);
			NewObject("Rain/Snow", PICTURE, 0, hd, 125, 55, 50, 20, MenuPage, 1, 0, 80, 135, 0, 0, 0, NULL, NULL);
			NewObject("Thunderstorm", PICTURE, 0, hd, 180, 55, 50, 20, MenuPage, 1, 0, 130, 135, 0, 0, 0, NULL, NULL);
			NewObject("Comets", PICTURE, 0, hd, 235, 55, 20, 20, MenuPage, 1, 0, 180, 135, 0, 0, 0, NULL, NULL);

			NewULink(TOGHID, NULL,
					 NewObject("Random|weather", FLAGBOX, 1, HIDENEVER, 180, 133, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.WeaRandom),
					 NULL, NULL, 0);

			NewObject("Environment", SELECTBOX, 1, hd, 5, 90, 96, 22, MenuPage, 30, 20, 0, 2, 0, 155, 1, NULL, &BSA.WEnvr);

			NewObject("Comet Bar", HSCROLL, 1, hd, 235, 76, 20, 5, 0, 0, 0, 0, 100, 1, 8, 0, NULL, &BSA.WCmtLvl);
			NewObject("RFMod Bar", HSCROLL, 1, hd, 180, 76, 50, 5, 0, 0, 0, -50, +50, 1, 10, 0, NULL, &BSA.WLPMod);
			NewObject("RFMod Bar", HSCROLL, 1, hd, 125, 76, 50, 5, 0, 0, 0, -50, +50, 1, 10, 0, NULL, &BSA.WRFMod);
			NewObject("YSpeedBar", HSCROLL, 1, hd, 93, 76, 20, 5, 0, 0, 0, 0, 100, 1, 8, 0, NULL, &BSA.WYSpd);
			NewObject("Season Bar", HSCROLL, 1, hd, 5, 76, 86, 5, 0, 0, 0, 0, 1999, 1, 70, 0, NULL, &BSA.WSeas);
			NewObject("Climate Bar", HSCROLL, 1, hd, 5, 41, 300, 5, 0, 0, 0, 0, 999, 1, 20, 0, NULL, &BSA.WClim);

			NewButton("Help", HIDENEVER, 268, 135, 35, 9, XRVHELP, 1);
		}

	// Landscape Page ----------------------------------------------------------
	if (BSA.SMode > S_MISSION)
		if (BSA.RuleSet > R_EASY)
		{
			hd = BSA.BckRandom;

			winptr = NewWindow("Landscape", CARDBOX, 0, 5, 35, 310, 150, 3, 6, 2, "Contents");
			DrawWindow(winptr);

			NewObject("Depth", PICTURE, 0, HIDENEVER, 5, 11, 20, 60, MenuPage, 1, 0, 300, 100, 0, 0, 0, NULL, NULL);
			NewObject("Volc.", PICTURE, 0, hd, 40, 11, 20, 20, MenuPage, 1, 0, 300, 160, 0, 0, 0, NULL, NULL);
			NewObject("Quake", PICTURE, 0, hd, 65, 11, 20, 20, MenuPage, 1, 0, 300, 180, 0, 0, 0, NULL, NULL);

			NewULink(TOGHID, NULL,
					 NewObject("Random|landscape", FLAGBOX, 1, HIDENEVER, 180, 133, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.BckRandom),
					 NULL, NULL, 0);

			objptr = NewObject("Landscape", PICTURE, 0, hd, 5, 90, 80, 50, MenuPage, 1, 0, 240, 0, 0, 0, 0, NULL, NULL);

			NewULink(EXCREDR, NULL, NewObject("Canyon", FLAGBOX, 1, hd, 150, 84, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.BCanyon), objptr, &DrawSurfacePreview, 0);

			NewULink(EXCREDR, NULL, NewObject("Water level", HSCROLL, 1, hd, 95, 138, 50, 5, 1, 0, 0, 0, 100, 1, 10, 0, NULL, &BSA.BWatLvl), objptr, &DrawSurfacePreview, 0);
			NewULink(EXCREDR, NULL, NewObject("Random", HSCROLL, 1, hd, 95, 126, 50, 5, 1, 0, 0, 0, 100, 1, 10, 0, NULL, &BSA.BRndAmp), objptr, &DrawSurfacePreview, 0);
			NewULink(EXCREDR, NULL, NewObject("Phase", HSCROLL, 1, hd, 95, 114, 50, 5, 1, 0, 0, 0, 100, 1, 10, 0, NULL, &BSA.BPhase), objptr, &DrawSurfacePreview, 0);
			NewULink(EXCREDR, NULL, NewObject("Period", HSCROLL, 1, hd, 95, 102, 50, 5, 1, 0, 0, 0, 100, 1, 10, 0, NULL, &BSA.BPLen), objptr, &DrawSurfacePreview, 0);
			NewULink(EXCREDR, NULL, NewObject("Amplitude", HSCROLL, 1, hd, 95, 90, 50, 5, 1, 0, 0, 0, 100, 1, 15, 0, NULL, &BSA.BCrvAmp), objptr, &DrawSurfacePreview, 0);

			NewObject("QuakeL Bar", HSCROLL, 1, hd, 65, 32, 20, 5, 0, 0, 0, 0, 100, 1, 10, 0, NULL, &BSA.BQuakeLvl);
			NewObject("VolcLv Bar", HSCROLL, 1, hd, 40, 32, 20, 5, 0, 0, 0, 0, 100, 1, 10, 0, NULL, &BSA.BVolcLvl);
			NewObject("BackGr Bar", VSCROLL, 1, HIDENEVER, 26, 31, 5, 40, 0, 0, 0, 165, 350, 1, 15, 0, NULL, &BSA.BSize);

			DrawSurfacePreview(0);

			NewButton("Help", HIDENEVER, 268, 135, 35, 9, XRVHELP, 1);
		}
	// Animal Page -------------------------------------------------------------
	if (BSA.SMode > S_MISSION)
	{
		hd = BSA.AnmRandom;

		DrawWindow(NewWindow("Animals", CARDBOX, 0, 5, 35, 310, 150, 2, 6, 2, "Contents"));

		NewObject("wipfpic", PICTURE, 0, hd, 5, 5, 30, 20, MenuPage, 0, 0, 30, 80, 0, 0, 0, NULL, NULL);
		NewObject("monspic", PICTURE, 0, hd, 5, 30, 30, 20, MenuPage, 0, 0, 60, 80, 0, 0, 0, NULL, NULL);
		if (BSA.RuleSet > R_EASY)
			NewObject("sharkpic", PICTURE, 0, hd, 5, 55, 30, 20, MenuPage, 0, 0, 0, 80, 0, 0, 0, NULL, NULL);

		NewULink(TOGHID, NULL,
				 NewObject("Random|animals", FLAGBOX, 1, HIDENEVER, 180, 133, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.AnmRandom),
				 NULL, NULL, 0);

		if (BSA.RuleSet > R_EASY)
			NewObject("Sharks", VALBOXB, 1, hd, 36, 66, 20, 9, 1, 0, 0, 0, 10, 1, 0, 0, NULL, &BSA.Sharks);
		NewObject("Monsters", VALBOXB, 1, hd, 36, 41, 20, 9, 1, 0, 0, 0, 10, 1, 0, 0, NULL, &BSA.Monsters);
		NewObject("Wipfs", VALBOXB, 1, hd, 36, 16, 20, 9, 1, 0, 0, 0, 10, 1, 0, 0, NULL, &BSA.Wipfs);

		NewButton("Help", HIDENEVER, 268, 135, 35, 9, XRVHELP, 1);
	}
	// Game Page ---------------------------------------------------------------
	if (BSA.SMode > S_MISSION)
	{
		winptr = NewWindow("Game", CARDBOX, 0, 5, 35, 310, 150, 1, 6, 2, "Contents");

		// Starting Conditions
		NewObject("Starting conditions", FRAME, 0, 0, 160, 75, 144, 70, 1, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);
		for (cnt = 4; cnt >= 0; cnt--)
		{
			hd = 0;
			if (((BSA.RuleSet < R_ADVANCED) && (cnt > 1)) || ((BSA.RuleSet < R_RADICAL) && (cnt > 2)))
				hd = 1;
			NewObject("Vehicles", PICTURE, 0, hd, 225 + 11 * cnt, 110 + 6, 10, 10, MenuPage, (cnt == 0), 0, 230, 50 + 10 * cnt, 0, 0, 0, NULL, NULL);
			NewObject("ReadyVhc", FLAGBOX, 1, hd, 225 + 11 * cnt + 1, 110 + 6 + 11, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.Plr[0].ReadyVhc[cnt]);
		}
		for (cnt = 4; cnt >= 0; cnt--)
		{
			hd = 0;
			if ((BSA.RuleSet == R_EASY) && (cnt > 0))
				hd = 1;
			if ((BSA.RuleSet == R_ADVANCED) && Inside(cnt, 2, 3))
				hd = 1;
			NewObject("Structures", PICTURE, 0, hd, 163 + 11 * cnt, 110 + 6, 10, 10, MenuPage, (cnt == 0), 0, 230, 10 * cnt, 0, 0, 0, NULL, NULL);
			NewObject("ReadyBase", FLAGBOX, 1, hd, 163 + 11 * cnt + 1, 110 + 6 + 11, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.Plr[0].ReadyBase[cnt]);
		}

		TextObject("Home base", 163, 137, CGray2);
		ITextObject(HomeBaseName, &BSA.Realism.CstHome, 163 + 40, 137, CGray2);

		NewObject("WealthPic", PICTURE, 0, (BSA.RuleSet == R_EASY), 230, 78, 20, 15, MenuPage, 0, 0, 135 + 20, 20, 0, 0, 0, NULL, NULL);
		NewObject("Gold", VALBOXB, 1, (BSA.RuleSet == R_EASY), 230 + 21, 78 + 6, 20, 9, 1, 0, 0, 0, 200, 10, 0, 0, NULL, &BSA.Plr[0].Wealth);
		NewObject("ClonkPic", PICTURE, 0, 0, 163, 78, 20, 15, MenuPage, 0, 0, 135, 20, 0, 0, 0, NULL, NULL);
		NewObject("Clonks", VALBOXB, 1, 0, 163 + 21, 78 + 6, 20, 9, 1, 0, 0, 1, 6, 1, 0, 0, NULL, &BSA.Plr[0].Clonks);

		// Game Setup
		NewObject((BSA.SMode == S_MELEE) ? "Melee" : "Cooperative", FRAME, 0, 0, 5, 75, 148, 70, 1, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);

		NewObject("Rule set:", PICTURE, 0, 0, 8, 78 + 6, 50, 15, FacePage, 1, 3, 0, 100 + 35, 0, 0, 0, NULL, &BSA.RuleSet);

		switch (BSA.SMode)
		{
		case S_MELEE:
			NewObject("Allow|surrender", FLAGBOX, 1, (BSA.RuleSet == R_EASY), 8, 131, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.AllowSurrender);
			objptr = NewObject("Player Elimination:", SELECTBOX, 1, 0, 8, 100 + 6, 66, 16, MenuPage, 15, 12, 0, 2, 120, 73, 1, NULL, &BSA.GPlrElm);
			NewULink(EXCREDR, NULL, objptr, ITextObject(EliminationName, &BSA.GPlrElm, 8, 123, CGray2), NULL, 0);
			NewULink(EXCREDR, NULL, objptr, CTextObject("                ", 8, 123, CGray2, CGray5), NULL, 0);
			break;
		case S_COOPERATE:
			objptr = NewObject("Mode:", ICON, 1, (BSA.SMode != S_COOPERATE), 13, 100 + 6, 41, 36, MenuPage, 10, 1, 0, 43, 35, 30, 0, NULL, &BSA.CoopGMode);
			NewULink(EXCREDR, NULL, objptr, objptr, &SetGameMode, 0);
			ITextObject(CGModeName, &BSA.CoopGMode, 62, 78, CGray1);
			NewObject("GModeDesc", TEXT, 0, HIDEOFF, 62, 84, 22, 10, 0, 2, 0, CGray3, -1, 0, 0, 0, ((char *)CGModeDesc), &BSA.CoopGMode);
			break;
		}

		// Players
		for (cnt = MaxGamePlr - 1; cnt >= 0; cnt--)
			CreatePlayerInfoBox(cnt, 5 + 101 * cnt, 8);
	}
	else
	{
		winptr = NewWindow("Mission", CARDBOX, 0, 5, 35, 310, 150, 1, 6, 2, "Contents");

		for (cnt = 0; cnt < MaxGamePlr; cnt++)
			if (BSA.Plr[cnt].Info)
				CreatePlayerInfoBox(cnt, 205, 14);

		NewButton("Help", HIDENEVER, 268, 130, 35, 9, XRVHELP, 1);

		NewULink(EXCREDR, NULL,
				 NewButton("Password", HIDENEVER, 205, 130, 58, 9, 0, 1),
				 NULL, &EnterMissionPassword, 0);

		NewObject("MissionList", LISTBOX, 1, 0, 10, 10, 174, 128, LID_SCRS, 1, 9, 172, 14, BoundBy(MissionScriptNum - 4, 0, ScriptListCount() - 9), ScriptListCount(), MissionScriptNum, NULL, &MissionScriptNum);
	}

	DrawWindow(winptr);

	// MainBack Page -----------------------------------------------------------
	NewWindow("MainBack", NOBACK, 0, 0, 189, 320, 20, 0, 0, 2, "Contents");

	NewObject("Round:", VALBOX, 0, 0, 84 + 25, 1, 13, 7, 1, 0, 0, 0, 1000, 1, 2, 0, NULL, &BSA.Round);

	if (BSA.SMode > S_MISSION) // or visible & update with script selection...
	{
		NewULink(OBJFUNC, realismwin, NULL,
				 NewObject("Realism:", VALBOX, 0, 0, 84 + 86, 1, 17, 7, 1, '%', 0, 0, 100, 1, 2, 0, NULL, &BSA.Realism.RealVal),
				 &RealismValue, 0);
		NewULink(OBJFUNCA, NULL, NULL,
				 NewObject("Level:", VALBOX, 0, 0, 84 + 135, 1, 17, 7, 1, '%', 0, 0, 100, 1, 2, 0, NULL, &BSA.DiffVal),
				 &DifficultyValue, 0);
	}

	NewButton(Config.LangType ? "Bye" : "End session", 0, 245, 0, 70, 9, 2, 0);
	NewButton(Config.LangType ? "Action Go!" : "Start round", 0, 6, 0, 70, 9, 1, 0);

	if (InitErrorCheck())
	{
		ClearSVI();
		return 0;
	}

	return 1;
}

BYTE InitRosterMenuPages(void)
{
	WINDOW *winptr;
	OBJECT *optr1, *optr2, *optr3, *optr4, *optr5, *optr6, *optrl;
	int ftx, fty;

	// Options Window -----------------------------------------------------------
	winptr = NewWindow("Options", CARDBOX, 0, 5, 23, 310, 162, 3, 6, 2, "Contents");
	DrawWindow(winptr);

	ftx = 100;
	fty = 10;
	NewObject("Control", FRAME, 0, 0, ftx, fty, 85, 35, 1, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);
	NewObject("KeyPic", PICTURE, 0, 0, ftx + 45, fty + 5, 36, 12, MenuPage, 0, 0, 72, 20, 0, 0, 0, NULL, NULL);
	NewULink(EXCREDR, NULL,
			 NewButton("Redefine Keyboard", 0, ftx + 5, fty + 21, 75, 9, 0, 0),
			 NULL, &EditKeyboardControls, 0);

	NewFlagBox("SVGA Title", 10, 110, &Config.HiResTitle);
	//TextObject("(see help first)",10+11,110+6,CGray1);

	NewFlagBox("No quotes on", 10, 90, &UserPref.NoQuotes);
	TextObject("promotions", 10 + 11, 90 + 6, CGray1);

	NewFlagBox("Save CLONK.DAT after", 10, 70, &Config.SaveDatAlways);
	TextObject("every round (for safety)", 10 + 11, 70 + 6, CGray1);

	NewFlagBox("alternative|button text", 10, 50, &Config.LangType);

	ftx = 10;
	fty = 10;
	NewObject("Sound", FRAME, 0, 0, ftx, fty, 85, 35, 1, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);
	NewObject("SoundPic", PICTURE, 0, 0, ftx + 68, fty + 5, 17, 16, MenuPage, 0, 0, 208, 84, 0, 0, 0, NULL, NULL);
	TextObject("Sound effects:", ftx + 5, fty + 5, CGray1);
	ITextObject(SoundStatusName, &Config.Sound, ftx + 5, fty + 13, CDRed);
	NewULink(EXCREDR, NULL,
			 NewButton("Sound Setup", 0, ftx + 5, fty + 21, 60, 9, 0, 0),
			 NULL, &SoundSetup, 0);

	NewButton("Help", 0, 273, 147, 30, 9, XRVHELP, 1);

	// Roster Window -----------------------------------------------------------
	winptr = NewWindow("Roster", CARDBOX, 0, 5, 23, 310, 162, 2, 6, 2, "Contents");
	DrawWindow(winptr);

	TextObject("Player:", 265, 89, CGray2);
	optr1 = NewObject("Import", BUTTON, 1, 0, 265, 140, 40, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	optr2 = NewObject("Delete", BUTTON, 1, 0, 265, 129, 40, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	optr3 = NewObject("New", BUTTON, 1, 0, 265, 118, 40, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	optr5 = NewObject("Particip.", BUTTON, 1, 0, 265, 107, 40, 9, 0, 1, 0, 0, 0, 0, 0, 0, NULL, NULL),
	optr4 = NewObject("Info/Edit", BUTTON, 1, 0, 265, 96, 40, 9, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

	TextObject("Roster:", 265, 45, CGray2);
	NewULink(EXCREDR, NULL,
			 NewButton("Sort by", 0, 265, 52, 40, 9, 0, 1),
			 NULL, &SetSortBy, 0);

	optr6 = NewFlagBox("Show|quotes", 265, 63, &Config.ShowQuotes);

	optrl = NewObject("PlayerRoster", LISTBOX, 1, 0, 2, 2, 250, 158, LID_PLRS, 2, 3, 124, 52, 0, PlrListCount(), (PlrListCount() > 0) ? 0 : -1, NULL, NULL);

	NewULink(EXCREDR, NULL, optr1, optrl, &ImportPlayers2List, 0);
	NewULink(LBCACT, NULL, optr2, optrl, NULL, 2);
	NewULink(EXCREDR, NULL, optr3, NULL, &NewPlayer2List, 0);
	NewULink(LBCACT, NULL, optr4, optrl, NULL, 0);
	NewULink(LBCACT, NULL, optr5, optrl, NULL, 1);
	NewULink(EXCREDR, NULL, optr6, optrl, NULL, 0);

	NewButton("Help", 0, 265, 151, 40, 9, XRVHELP, 1);

	// Game Window -------------------------------------------------------------
	winptr = NewWindow("Session", CARDBOX, 0, 5, 23, 310, 162, 1, 6, 2, "Contents");

	// Game Options Frame
	NewObject("Options", FRAME, 0, 0, 224, 12, 76, 90, 3, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);

	NewObject("Game Position", SELECTOR, ACC, HIDEOFF, 227, 82, 70, 2 * 8 + 1, 1, 0, 0, 2, 0, 0, 0, 0, "random|by control", &UserPref.PlrPosByCon);

	NewObject("Tournament mode", SELECTOR, ACC, HIDEOFF, 227, 40, 70, 2 * 8 + 1, 1, 0, 0, 2, 0, 0, 0, 0, "Rotation|Challenge", &BSA.TMode);
	TextObject("With more than|3 Melee players", 262, 58, CGray2, -1, 1);

	NewObject("CrewPic", PICTURE, 0, 0, 227, 16, 20, 10, MenuPage, 0, 0, 175, 25, 0, 0, 0, NULL, NULL);
	NewObject("Permanent|crew", FLAGBOX, 1, 0, 250, 16, 10, 10, 1, 0, 0, 0, 0, 0, 0, 0, NULL, &BSA.PCrew);

	// Rule Set Frame
	NewObject("Rule set", FRAME, 0, 0, 10, 64, 204, 27, 1, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);
	NewULink(EXCREDR, NULL,
			 NewObject("Rule set", SELECTBOX, 1, 0, 13, 67, 199, 21, FacePage, 50, 15, 0, 2, 0, 100 + 35, 0, NULL, &BSA.RuleSet),
			 NULL, &AdjustBSA2RuleSet, 1);

	// Type of Session Frame
	NewObject("Session type", FRAME, 0, 0, 10, 12, 204, 43, 2, 0, 0, CGray3, -1, CGray5, 0, 0, NULL, NULL);
	NewULink(EXCREDR, NULL,
			 NewObject("SessionMode", SELECTBOX, 1, 0, 13, 15, 199, 37, FacePage, 65, 35, 0, 2, 0, 100, 0, NULL, &BSA.SMode),
			 NULL, &AdjustBSA2SMode, 0);

	CTextObject((char*)PrgInfoLine, 3, 154, CGray4);
	/*if (Registered())
	  sprintf(OSTR,"�%cThis CLONK game is registered for �%c%s",CGray3,CGray2,Config.RegName);
	  else
	  SCopy("This is an unregistered shareware version!",OSTR);
	  TextObject(OSTR,3,154,CRed);*/

	NewButton("Help", 0, 273, 147, 30, 9, XRVHELP, 1);

	DrawWindow(winptr);

	// MainBack Page -----------------------------------------------------------
	winptr = NewWindow("MainBack", NOBACK, 0, 0, 189, 320, 20, 0, 0, 2, "Contents");

	NewObject(Config.LangType ? "See ya" : "Quit game", BUTTON, 1, 0, 245, 0, 70, 9, 2, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
	NewObject(Config.LangType ? "Here we go" : "Start session", BUTTON, 1, 0, 6, 0, 70, 9, 1, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

	NewULink(OBJFUNCA, NULL, NULL,
			 NewObject("Participants:", VALBOX, 0, 0, 84 + 53, 1, 9, 7, 1, 0, 0, 0, 1000, 1, 2, 0, NULL, &BSA.PlrsInRound),
			 &PlrsInSession, 0);

	if (InitErrorCheck())
	{
		ClearSVI();
		return 0;
	}
	return 1;
}

BYTE InitEndPrPage(DWORD modify)
{
	int cnt, ctx, cty, winwdt;

	winwdt = 95 * BSAPlrNum() + 10;

	sprintf(OSTR, "Round %d:", BSA.Round);
	NewWindow(OSTR, STANDARD, 0, -1, 25, winwdt, 160, 1, 0, XRVCLOSE, "Contents");

	switch (BSA.SMode)
	{
	case S_MISSION:
		SCopy(MissionScript.title, OSTR, 23);
		break;
	case S_COOPERATE:
		sprintf(OSTR, "%s %" PRIu32, CGModeName[BSA.CoopGMode], modify);
		break;
	case S_MELEE:
		sprintf(OSTR, "%s %" PRIu32, EliminationName[BSA.GPlrElm], modify);
		break;
	}
	TextObject(OSTR, winwdt / 2, 10, CGray2, -1, 1);

	ctx = 10;
	for (cnt = 0; cnt < MaxGamePlr; cnt++)
	{
		if (BSA.Plr[cnt].Col > -1)
			if (BSA.Plr[cnt].Info) // latter one safety
			{
				cty = 18 + 10 * (BSA.Plr[cnt].Eliminate != 0);

				NewObject("PlrPic", PICTURE, 0, 0, ctx + 22, cty + 3, 40, 50, FacePage, 0, 8, 0, 0, 0, 0, 0, NULL, &(BSA.Plr[cnt].Info->face));

				CTextObject((char*)PlrRankName(BSA.Plr[cnt].Info->rank), ctx + 43, cty + 55, CGray2, -1, 1);
				CTextObject(BSA.Plr[cnt].Info->name, ctx + 43, cty + 61, CGray1, -1, 1);

				sprintf(OSTR, "%" PRIu32 " Before", BSA.Plr[cnt].Info->score[BSA.SMode] - modify * (BSA.Plr[cnt].ScoreGain + 100 * (!BSA.Plr[cnt].Eliminate)) / 100);
				TextObject(OSTR, ctx + 42, cty + 72, CGray2, -1, 1);

				if (BSA.Plr[cnt].ScoreGain > 0)
				{
					sprintf(OSTR, "+%5d Round", modify * BSA.Plr[cnt].ScoreGain / 100);
					TextObject(OSTR, ctx + 42, cty + 80, CGray1, -1, 1);
				}
				TextObject("+00000 Round", ctx + 42, cty + 80, CGray4, -1, 1);

				if (!BSA.Plr[cnt].Eliminate)
				{
					sprintf(OSTR, "+  %3d Bonus", modify);
					TextObject(OSTR, ctx + 42, cty + 86, CDRed, -1, 1);
				}
				TextObject("+00000 Bonus", ctx + 42, cty + 86, CGray4, -1, 1);

				NewObject("SumLine", FRAME, 0, 0, ctx + 15, cty + 92, 55, 1, 0, 0, 0, CGray2, -1, 0, 0, 0, NULL, NULL);
				NewObject("SumLine", FRAME, 0, 0, ctx + 15, cty + 100, 55, 1, 0, 0, 0, CGray1, -1, 0, 0, 0, NULL, NULL);

				sprintf(OSTR, "%" PRIu32 " Total", BSA.Plr[cnt].Info->score[BSA.SMode]);
				TextObject(OSTR, ctx + 42, cty + 94, CGray1, -1, 1);

				NewObject("Frame", FRAME, 0, 0, ctx, cty, 85, 125 - 10 * (BSA.Plr[cnt].Eliminate != 0), 0, 0, 0, 64 + BSA.Plr[cnt].Col, CGray5, 0, 0, 0, NULL, NULL);

				ctx += 95;
			}
	}

	NewObject("OK", BUTTON, 1, 0, winwdt / 2 - 20, 147, 40, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

	if (InitErrorCheck())
	{
		ClearSVI();
		return 0;
	}
	return 1;
}

//--------------------------- SVI Extern Functions ----------------------------

void DrawListBoxCell(int lid, int num, int tx, int ty, BYTE sel)
{
	switch (lid)
	{
	case LID_CAPS:
		DrawCrewListCell(num, tx, ty, sel);
		break;
	case LID_PLRS:
		DrawPlrListCell(num, tx, ty, sel);
		break;
	case LID_FACE:
		DrawFaceListCell(num, tx, ty, sel);
		break;
	case LID_SCRS:
		DrawScriptListCell(num, tx, ty, sel);
		break;
	default:
		Message("System Error|Undefined list id on draw-lbc");
		break;
	}
}

BYTE ListBoxCellAction(int lid, int num, int aid)
{
	switch (lid)
	{
	case LID_CAPS:
		CrewListCellAction(num, aid);
		break;
	case LID_PLRS:
		PlrListCellAction(num, aid);
		break;
	case LID_FACE:
		return FaceListCellAction(num, aid);
	case LID_SCRS:
		return ScriptListCellAction(num, aid);
	default:
		Message("System Error|Undefined list id on lbc action");
		break;
	}
	return 0;
}

//---------------------------- Game Program Parts ------------------------------

int InRoundInfo(int dummy)
{
	WINDOW *cwin;

	cwin = NewWindow("Round-Info", CARDBOX, 1, -1, -1, 200, 70, 1, 3, XRVCLOSE, "Contents");

	sprintf(OSTR, " Rule set: �%c%s", CGray1, RuleSetName[BSA.RuleSet]);
	TextObject(OSTR, 70, 5, CGray2);
	sprintf(OSTR, "  Session: �%c%s", CGray1, SModeName[BSA.SMode]);
	TextObject(OSTR, 70, 12, CGray2);
	switch (BSA.SMode)
	{
	case S_MISSION:
		sprintf(OSTR, "  Mission: �%c%.20s", CGray1, MissionScript.title);
		break;
	case S_MELEE:
		sprintf(OSTR, "     Mode: �%c%s", CGray1, EliminationName[BSA.GPlrElm]);
		break;
	case S_COOPERATE:
		sprintf(OSTR, "     Mode: �%c%s", CGray1, CGModeName[BSA.CoopGMode]);
		break;
	}
	TextObject(OSTR, 70, 19, CGray2);

	NewObject("Round", VALBOX, 0, 0, 5 + 40, 5, 17, 7, 1, 0, 0, 0, 1000, 1, 3, 0, NULL, &BSA.Round);
	NewObject("Realism", VALBOX, 0, 0, 5 + 40, 13, 17, 7, 1, '%', 0, 0, 100, 1, 3, 0, NULL, &BSA.Realism.RealVal);
	NewObject("Level", VALBOX, 0, 0, 5 + 40, 21, 17, 7, 1, '%', 0, 0, 100, 1, 3, 0, NULL, &BSA.DiffVal);

	NewButton("OK", 0, 75, 57, 50, 9, XRVCLOSE, 1);

	if (InitErrorCheck())
		CloseWindow(cwin);

	return dummy;
}

BYTE RoundMenuAbort(void)
{
	int rval = 0;

	//InitSVIMouse();

	NewWindow("Runden-Menu", STANDARD, 1, -1, -1, 170, 80, 1, 0, XRVCLOSE, "Contents");

	NewButton("Help", 0, 35, 51, 100, 9, XRVHELP, 1);
	NewULink(EXCREDR, NULL,
			 NewButton("Special Options", 0, 35, 40, 100, 9, 0, 1),
			 NULL, &OpenUserPrefWindow, 1);
	NewULink(EXCREDR, NULL,
			 NewButton("Key definitions", 0, 35, 29, 100, 9, 0, 1),
			 NULL, &EditKeyboardControls, 0);
	NewULink(EXCREDR, NULL,
			 NewButton("Round-Info", 0, 35, 18, 100, 9, 0, 1),
			 NULL, &InRoundInfo, 0);
	NewButton("Continue", 0, 87, 67, 67, 9, 2, 0);
	NewButton("Abort round", 0, 16, 67, 67, 9, 1, 0);

	if (!InitErrorCheck())
		if (RunSVI() == 1)
			rval = 1;
	ClearSVI();

	return rval;
}

int DefaultQuote(int pnum)
{
	PLAYERINFO *plr = PNum2Plr(pnum);
	if (plr)
		if (!plr->quote[0])
			SCopy(DefQuoteSource[random(MaxDefQuote)], plr->quote, MaxQuoteLen);
	return 0;
}

void EnterNewPlrRank(void)
{
	static char nrank[MaxPlrRankName + 1]; // Instead of global variable
	nrank[0] = 0;
	LineInput("Please enter a new rank name:", nrank, MaxPlrRankName, NULL, 0);
	// SVI run called by LineInput, sole prime run assumed
	if (!nrank[0])
		sprintf(nrank, "Clonk Number %d", PlrRankCount() + 1);
	NewPlrRank(nrank);
}

void PlayerPromotionCheck(void)
{
	int plr;
	PLAYERINFO *cplr;

	for (plr = 0; plr < MaxGamePlr; plr++)
		if (BSA.Plr[plr].Info)
		{
			cplr = BSA.Plr[plr].Info;

			if (cplr->rank + 1 >= PlrRankCount())
				EnterNewPlrRank();

			if (cplr->rank < PlrRankCount())
				if (PlrTotalScore(cplr) >= PlayerRankScore(cplr->rank + 1))
				{
					cplr->rank++;

					NewWindow("Promotion!", PLAIN, 0, -1, -1, 120, 140, 1, 0, XRVCLOSE, "Contents");

					NewObject("Honors", PICTURE, 0, 0, 20, 15, 80, 65, MenuPage, 0, 0, 200, 135, 0, 0, 0, NULL, NULL);
					NewObject("PlrPic", PICTURE, 0, 0, 40, 15, 40, 50, FacePage, 0, 8, 0, 0, 0, 0, 0, NULL, &(cplr->face));
					NewObject("PicBack", FRAME, 0, 0, 30, 15, 60, 60, 0, 0, 0, -1, 64 + BSA.Plr[plr].Col, 0, 0, 0, NULL, NULL);

					CTextObject(cplr->name, 60, 85, CDBlue, -1, 1);
					CTextObject("becomes a", 60, 93, CGray2, -1, 1);
					CTextObject((char*)PlrRankName(cplr->rank), 60, 101, CDBlue, -1, 1);

					NewObject("Congrats!", BUTTON, 1, 0, 25, 127, 70, 9, XRVCLOSE, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

					if (!InitErrorCheck())
						RunSVI();
					ClearSVI();

					if (UserPref.NoQuotes)
						SCopy("No comment.", cplr->quote);
					else
					{
						cplr->quote[0] = 0;
						sprintf(OSTR, "%s, please comment on your promotion:", cplr->name);
						LineInput(OSTR, cplr->quote, MaxQuoteLen, &DefaultQuote, Plr2PNum(cplr), "comment");
					}
				}
		}
}

extern WORD RunSec;
extern DWORD WipfResc;

void RoundEndProcess(PLAYERINFO **winnerp)
{
	int plr;
	DWORD modify;
	MANINFO *cmn;

	if (!LoadMenuPages())
		return;

	// Calculate score modifier
	switch (BSA.SMode)
	{
	case S_MISSION:
		modify = BSA.DiffVal; // 0%-100%
		break;
	case S_COOPERATE:
		modify = BSA.DiffVal + BSA.Realism.RealVal; // 0%-200%
		if (BSA.CoopGMode == C_WIPFRESCUE)
			if (BSA.Wipfs == 0)
				Message("Error: Wipf-Rescue mode should|have had wipfs!");
			else
				modify = modify * WipfResc / BSA.Wipfs;
		break;
	case S_MELEE:
		modify = 100L; // 100%
		break;
	}

	// Automatics: Round counts, score gains, stats
	for (plr = 0; plr < MaxGamePlr; plr++)
		if (BSA.Plr[plr].Col > -1)
			if (BSA.Plr[plr].Info)
			{
				if (BSA.PCrew)
					for (cmn = BSA.Plr[plr].Info->crew; cmn; cmn = cmn->next)
					{
						if (!cmn->dead)
							cmn->rnds++; // Only survived rounds count
						if (BSA.Plr[plr].Info->autorean)
							if (cmn->dead)
								AutoReanMan(BSA.Plr[plr].Info, cmn);
					}

				if (!BSA.Plr[plr].Eliminate)
				{
					BSA.Plr[plr].Info->score[BSA.SMode] += 100L * modify / 100L;
					BSA.Plr[plr].Info->scorepot += 100L * modify / 100L;
					BSA.Plr[plr].Info->won[BSA.SMode]++;

					// Mission advance & passwords
					if (BSA.SMode == S_MISSION) // Called once only for max 1 plr in mission
					{
						switch (MissionScript.idnum) // (Hardcoded idnums)
						{
						case 12:
							Message("You have completed all EAYS missions!|Please copy the following pass word:|�!WIPFEMONSTERSCHNEE�i|With this pass word you can continue on this mission,|in case your score file gets lost. (F1 for more info)", "mission passwords");
							break;
						case 22:
							Message("You have completed all MEDIUM missions!|Please copy the following pass word:|�!VULKANBEBENAUSBRUCH�i|With this pass word you can|skip over all easy and medium missions.", "mission passwords");
							break;
						case 30:
							Message("You have completed all tutorial rounds!|Please copy the following pass word:|�!DASWARHARTEARBEIT�i", "mission passwords");
							break;
						}
						if (MissionScript.next)
							BSA.NextMission = Max(BSA.NextMission, MissionScript.next->idnum);
					}

					*winnerp = BSA.Plr[plr].Info; // Setting only makes sense in MELEE
				}

				BSA.Plr[plr].Info->rnds[BSA.SMode]++;
				BSA.Plr[plr].Info->score[BSA.SMode] += BSA.Plr[plr].ScoreGain * modify / 100L;
				BSA.Plr[plr].Info->scorepot += BSA.Plr[plr].ScoreGain * modify / 100L;
				BSA.Plr[plr].Info->playsec += RunSec;
			}

	// Round End Window
	DrawTitleBack(MainPage, 0);
	if (InitEndPrPage(modify))
	{
		RunSVI();
		ClearSVI();
	}

	// Misc
	PlayerPromotionCheck();

	BSA.Round++;
}

BYTE CustomSetBSA(void)
{
	int cnt;
	BYTE svir, retryset;

	if (!LoadMenuPages())
		return 0;

	DrawTitleBack(MainPage, 0);
	do
	{
		retryset = 0;
		if (!InitCSBMenuPages())
			return 0;
		svir = RunSVI();
		ClearSVI();
		if (svir != 1)
			return 0; // Session aborted

		// Prepare BSA 4 Round

		// MISSION: Default BSA & Load settings from script file
		if (BSA.SMode == S_MISSION)
			if (!SetBSA4Mission())
				retryset = 1;

	} while (retryset);

	// Prepare BSA 4 Round Cont.

	PrepareBSARockType();

	// Player starting condition equalization (not if diffr.cond. game)
	BSA.Plr[2].Clonks = BSA.Plr[1].Clonks = BSA.Plr[0].Clonks; // [^^^^ handicap]
	BSA.Plr[2].Wealth = BSA.Plr[1].Wealth = BSA.Plr[0].Wealth;
	for (cnt = 0; cnt < 5; cnt++)
	{
		BSA.Plr[1].ReadyBase[cnt] = BSA.Plr[2].ReadyBase[cnt] = BSA.Plr[0].ReadyBase[cnt];
		BSA.Plr[1].ReadyVhc[cnt] = BSA.Plr[2].ReadyVhc[cnt] = BSA.Plr[0].ReadyVhc[cnt];
	}

	// Randomize Randoms
	if (BSA.BckRandom)
		RndInitBackground();
	if (BSA.WeaRandom)
		RndInitWeather();
	if (BSA.AnmRandom)
		RndInitAnimals();

	// Adjust 2 CoopGMode
	if (BSA.SMode == S_COOPERATE)
		AdjustBSA2CoopGMode();

	// Player/Crew adjust
	for (cnt = 0; cnt < MaxGamePlr; cnt++)
		if (BSA.Plr[cnt].Col > -1)
		{
			BSA.Plr[cnt].Eliminate = 0;
			BSA.Plr[cnt].ScoreGain = 0;
			if (BSA.PCrew)
				AdjustCrew2Round(cnt); // insufficient mem check & abort!!!
			else
				PrepareTCrew4Round(cnt);
		}

	return 1;
}

BYTE MainMenus(void) // Returns  0 Exit  1 GmeSession
{
	BYTE svir;

	if (!LoadMenuPages())
		return 0;

	DrawTitleBack(MainPage, 1);

	if (FirstTimeRun)
	{
		OpenPowerHelp("welcome");
		RunSVI();
	}

	if (PlrListCount() < 1)
	{
		NewPlayer2List(0);
		RunSVI();
	}

	if (FirstTimeRun)
		if (SBDetected)
		{
			SoundSetup(0);
			RunSVI();
		}
		else
			ConfirmedCall("Is a sound blaster compatible|sound card installed on this system?", 2, &SoundSetup, 0);

	if (!InitRosterMenuPages())
		return 0;

	svir = RunSVI();

	ClearSVI();

	FirstTimeRun = 0;

	if (svir == 1)
		return 1;
	return 0;
}

extern char RoundErrorLine[RoundErrNum][31];

void RoundErrorMessage(void)
{
	int cnt;
	if (!RoundErrorLine[0][0])
		return;

	NewWindow("RoundErrors", STANDARD, 1, -1, -1, 130, 35 + RoundErrNum * 6, 0, 0, XRVCLOSE, "Contents");

	TextObject("The following errors have occured|in the course of the last round:", 65, 3, CDRed, -1, 1);

	for (cnt = 0; cnt < RoundErrNum; cnt++)
		if (RoundErrorLine[cnt][0])
			TextObject(RoundErrorLine[cnt], 5, 17 + 6 * cnt, CGray1);

	NewObject("ListBack", FRAME, 0, 0, 4, 16, 122, 3 + RoundErrNum * 6, 0, 0, 0, -1, CGray5, 0, 0, 0, NULL, NULL);

	NewButton("OK", 0, 45, 35 + RoundErrNum * 6 - 13, 40, 9, XRVCLOSE, 1);

	if (!InitErrorCheck())
		RunSVI();
	ClearSVI();
}

//----------------------------- Main Program ---------------------------------

BYTE ExecRoundOkay(void)
{
	/*if (!Registered() && (BSA.RuleSet>R_EASY) && (BSA.SMode!=S_MISSION))
	  {
	  ConfirmedCall("In the unregistered version, you can play|by the easy rule set (einfach) only.|Enter registration password now?",2,&Registration,0,"Registration");
	  if (!Registered()) return 0;
	  }*/

	switch (BSA.SMode)
	{
	case S_MISSION:
		if (MissionScript.idnum < 0)
		{
			Message("No mission has been selected!", NULL);
			return 0;
		}
		break;
	case S_MELEE:
		if (BSA.RuleSet == R_EASY)
			if (BSA.GPlrElm == 2)
				if (!BSA.Plr[0].ReadyBase[0])
				{
					Message("In the EASY rule set, capture the flag|can not be played without|a castle home base.", "round starting conditions");
					return 0;
				}
		break;
	case S_COOPERATE:
		if (BSA.RuleSet == R_EASY)
			switch (BSA.CoopGMode)
			{
			case C_GOLDMINE:
				if (!BSA.Plr[0].ReadyBase[0])
				{
					Message("In the EASY rule set, gold mine|can not be played without|a castle home base.", "round starting conditions");
					return 0;
				}
				break;
			case C_WIPFRESCUE:
				if (!BSA.Plr[0].ReadyBase[0] || !BSA.Plr[0].ReadyVhc[0])
				{
					Message("Int the EASY rule set, you need|a castle home base and a lorry|if you want to rescue wipfs.", "round starting conditions");
					return 0;
				}
				break;
			}
		break;
	}
	return 1;
}

BYTE SessionStartOkay(void)
{
	if (PlrsInSession(0) < 1)
	{
		Message("Please mark all players who want|to take part in the new session.", NULL);
		return 0;
	}
	switch (BSA.SMode)
	{
	case S_MISSION:
		if (PlrsInSession(0) > 1)
		{
			Message("The tutorial rounds can only be played|by one player at a time.", NULL);
			return 0;
		}
		break;
	case S_COOPERATE:
		if (!Inside(PlrsInSession(0), 1, 3))
		{
			Message("In cooperative mode, a maximum of|three players per round can participate.", NULL);
			return 0;
		}
		break;
	case S_MELEE:
		if (PlrsInSession(0) < 2)
		{
			Message("A single player may not|play in melee mode.", NULL);
			return 0;
		}
		break;
	}
	return 1;
}

void GetPlayers2BSA(void)
{
	int pcon, ppos, plrc, pin, cnt;
	PLAYERINFO *cplr;
	BYTE pused;

	ClearBSAPlrPtrs();

	// Roster-higher player gets col/con/gpos-preference priority

	for (cplr = FirstPlayer, pin = 0; cplr && (pin < MaxGamePlr); cplr = cplr->next)
		if (cplr->inrnd)
		{

			// Choose Con
			pcon = cplr->pfcon;
			for (cnt = 0, pused = 0; cnt < MaxGamePlr; cnt++)
				if (BSA.Plr[cnt].Col > -1)
					if (BSA.Plr[cnt].Con == pcon)
						pused = 1;
			if (pused)
				pcon = 0;
			do
			{
				for (cnt = 0, pused = 0; cnt < MaxGamePlr; cnt++)
					if (BSA.Plr[cnt].Col > -1)
						if (BSA.Plr[cnt].Con == pcon)
							pused = 1;
				if (pused)
					pcon++;
			} while (pused);
			// Set BSA-Pos & GPos & Con                         // Mouse should
			if (UserPref.PlrPosByCon) // By actual round pcon   // have lowest
			{
				ppos = pcon;
				if (ppos > 2)
					ppos = 2;
			}	 // gpos placement
			else // By random                                   // priority
				ppos = random(MaxGamePlr);
			while (BSA.Plr[ppos].Col > -1)
				++ppos %= MaxGamePlr;
			BSA.Plr[ppos].GPos = ppos; // that's double
			BSA.Plr[ppos].Con = pcon;
			// Set Col
			plrc = cplr->pfcol;
			do
			{
				for (cnt = 0, pused = 0; cnt < MaxGamePlr; cnt++)
					if (BSA.Plr[cnt].Col == plrc)
						pused = 1;
				if (pused)
					++plrc %= 8;
			} while (pused);
			BSA.Plr[ppos].Col = plrc;
			// Set Info
			BSA.Plr[ppos].Info = cplr;

			pin++;
		}
}

void InitPlayersInRnd(PLAYERINFO **oldestp)
{					  // Will try to get up to 3 players in session for inrnd
	PLAYERINFO *cplr; // and will reset all others
	int cnt;
	*oldestp = NULL;
	for (cplr = FirstPlayer, cnt = 3; cplr; cplr = cplr->next)
	{
		cplr->inrnd = 0;
		if (cplr->inses && (cnt > 0))
		{
			cplr->inrnd = 1;
			cnt--;
			if (!(*oldestp))
				*oldestp = cplr;
		}
	}
}

void NewPlayersInRnd(PLAYERINFO **oldestp, PLAYERINFO *winnerp)
{
	PLAYERINFO *cplr;
	BYTE exok, cnt;
	switch (BSA.TMode)
	{
	case 0: // Rotation: oldest out, new in
		cplr = (*oldestp)->next;
		(*oldestp)->inrnd = 0;
		*oldestp = NULL;
		for (exok = 0; !exok; cplr = cplr->next)
		{
			if (!cplr)
				cplr = FirstPlayer;
			if (!(*oldestp))
				if (cplr->inses && cplr->inrnd)
					*oldestp = cplr;
			if (cplr->inses && !cplr->inrnd)
			{
				cplr->inrnd = 1;
				exok = 1;
			}
		}
		break;
	case 1: // Challenge: winner stays, others out
		for (cplr = (*oldestp)->next, exok = 0; !exok; cplr = cplr->next)
		{
			if (!cplr)
				cplr = FirstPlayer;
			if (cplr->inses && !cplr->inrnd)
			{
				*oldestp = cplr;
				exok = 1;
			}
		}
		for (cplr = FirstPlayer; cplr; cplr = cplr->next)
			if (cplr != winnerp)
				cplr->inrnd = 0;
		for (cplr = *oldestp, cnt = 2; cnt > 0; cplr = cplr->next)
		{
			if (!cplr)
				cplr = FirstPlayer;
			if (cplr->inses && !cplr->inrnd)
			{
				cplr->inrnd = 1;
				cnt--;
			}
		}
		break;
	}
}

void GameSession(void)
{
	BYTE aborted;
	PLAYERINFO *oldestp, *winnerp;

	if (!SessionStartOkay())
		return;

	AdjustBSA2RuleSet(0); // Settings required by rule set...

	InitPlayersInRnd(&oldestp);
	GetPlayers2BSA();

	BSA.Round = 1;

	while (CustomSetBSA())
		if (ExecRoundOkay())
		{
			if (!oldestp)
				Message("System Error: oldestp should|never be null!");

			Config.RoundStartCount++;

			InRound = 1;
			//------------------------------------------
			aborted = ExecRound();
			//------------------------------------------
			InRound = 0;

			ClearTCrews();

			MenuPagesLoaded = 0;
			LPage(MainPage);
			PPage(MainPage);
			InitSVIMouse();

			if (aborted == 2)
				Message("Insufficient memory|can't start the round with these settings", "insufficient memory for round");
			RoundErrorMessage();

			if (!aborted)
			{
				RoundEndProcess(&winnerp);
				Config.RoundEndCount++;
			}

			CheckCrewLists();

			if (Config.SaveDatAlways)
				SaveConfigSystem();

			// If MELEE tournament, get new players
			if (PlrsInSession(0) > 3)
				if (!aborted)
					NewPlayersInRnd(&oldestp, winnerp);
			GetPlayers2BSA();

			/*if (!Registered())
		  if (Config.RoundEndCount>150)
		  if (!random(2))
		  Message("You've been playing Clonk for quite a while now!|You should definitely register your copy of the game.");*/
		}

	if (BSA.SMode == S_MISSION)
		DefaultBSA(0); // Reset from extremes

	SortPlayerList(0);

	ClearBSAPlrPtrs();

	/*if (!Registered())
	  if (Config.RoundEndCount>70)
	  if (!random(4))
	  Message("And don't forget: Clonk is Shareware.|If you like the game you should register.");*/
}

void MainGameLoop(void)
{
	BYTE exok = 0;

	// Init names and scripts
	if (!InitCNameFile())
	{
		InitMsg("Missing name file!", CGray4);
		WaitForInput();
	}

	InitMsgOpen("Searching script files... ");
	sprintf(OSTR, "%d found", InitScriptList());
	InitMsg(OSTR);

	// Run
	do
	{
		switch (MainMenus())
		{
		case 0:
			exok = 1;
			break;
		case 1:
			GameSession();
			break;
		}
	} while (!exok);

	// Deinit names ans scripts
	DeInitScriptList();
	DeInitCNameFile();
}

void MainPrg(void)
{
	BYTE pfail;
	InitMsgOpen("Init system... ");
	randomize();
	Randomize3();
	Randomize4();
	GetRunningPath(RunningPath);
	//InitKLockCheck();
	InitMsg("Done");

	pfail = 0;

	// File integrity
	InitMsgOpen("File Integrity check... ");
	if (FIntegCheck())
		InitMsg("Ok");
	else
		pfail = 1;

	if (!pfail)
	{
		// Init/Config
		FirstTimeRun = 0;
		InitMsgOpen("Configuration check... ");
		if (!LoadConfigSystem())
		{
			InitMsg("New Game", CGray4);
			FirstTimeRun = 1;
			DefaultConfig();
			DefaultUserPref();
			DefaultBSA(1);
			AdjustBSA2RuleSet(1);
		}
		else
			InitMsg("Done");

		// Intro
		InitMsg("Running Intro...");
		Intro(Config.HiResTitle);
		ResetInitMsg();

		InitMsgOpen("Init BS graphics... ");
		if (InitBSGraphics())
		{
			InitMsg("Done");
			MainGameLoop();
		}
		else
			pfail = 1;

		// DeInit/Config
		SaveConfigSystem();
		DeInitPlayerList();
		ClearPlrRanks();
	}

	if (pfail) // In case of failure, still continuing InitMsgs
	{
		InitMsg("Failure", CGray4);
		InitMsg("Program aborted", CGray5);
		WaitForInput();
	}

	DeInitBSGraphics();
}

//------------------------------ void main(void) ----------------------------

int printUsage()
{
	printf("Usage: clonk [-scale FACTOR] OR\n       clonk [--s FACTOR]\n");
	return 2;
}

int main(int argc, char *argv[])
{
	int scale = 4;
	UpscaleInterpolationType interpolationType = INTERPOLATION_SCALE2X;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--scale") == 0 || strcmp(argv[i], "/scale") == 0 || strcmp(argv[i], "-scale") == 0)
		{
			if (i + 1 < argc)
			{
				scale = SDL_atoi(argv[i + 1]);
			}
		}
		if (strcmp(argv[i], "--interpolation") == 0 || strcmp(argv[i], "/interpolation") == 0 || strcmp(argv[i], "-interpolation") == 0)
		{
			if (i + 1 < argc) {
				if (SDL_strcasecmp(argv[i+1], "none") == 0)
					interpolationType = INTERPOLATION_NONE;
				else if (SDL_strcasecmp(argv[i+1], "scale2x") == 0)
					interpolationType = INTERPOLATION_SCALE2X;
				else if (SDL_strcasecmp(argv[i+1], "scale4x") == 0)
					interpolationType = INTERPOLATION_SCALE4X;
			}
		}
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "/help") == 0) {
			printf("%s [--scale NUMBER] [--interpolation none|scale2x|scale4x]\n", argv[0]);
			return 0;
		}
	}

	if (!InitSDL(scale, interpolationType))
	{
		CloseSDL();
		printf("SDL Initialization failure");
		return 0;
	}

	if (InitSVI(MainPage, "CLONK3.HLP"))
	{
		InitMsg(PrgInfoLine, CRed);
		InitMsg("");
		SVISetExternFunctions(&DrawListBoxCell, &ListBoxCellAction);
		MainPrg();
		CloseSVI();
		CloseSDL();
		EndTextMessage();
		return 0;
	}
	else
	{
		printf("RW\\D SVI Initialization failure\n\r");
		return 1;
	}
}
