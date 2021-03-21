/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design CLONK Miner's BattleSequenceRoundExecSystem Module

#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <SDL.h>

#include "standard.h"
#include "vga4.h"
#include "std_gfx.h"

#include "ct-voice.h"
#include "dsp-dtss.h"

#include "clonk_bs.h"
#include "clonk_sc.h"

#include "bsmansys.h"
#include "bsgfxsys.h"
#include "bspxssys.h"
#include "bsvhcsys.h"
#include "bsstrsys.h"
#include "bsweasys.h"
#include "bsscrsys.h"

#include "RandomWrapper.h"
#include "SDLSound.h"
#include "UserInput.h"
//------------------------- Main Global Externals ----------------------------

extern char OSTR[500];

extern USERPREF UserPref;
extern BSATYPE BSA;
extern CONFIG Config;

extern void InitProcessB(const char *msg, BYTE page = 0);

extern void DrawClock(BYTE tpge);
extern void KeyConDisplay(int stat);

extern int SaveMBx;

//-------------------- RoundEXC Global System Variables ---------------------------

long BackGrSize;

BYTE Tick2, Tick3, Tick5, Tick10, Tick20, Tick50;
BYTE Sec1, Sec5, Sec10, Sec20, Sec60;

BYTE LowMem;

DWORD RockCnt, PXSCnt, StructCnt, VehicCnt, LineCnt, ManCnt[3], AnimalCnt;
DWORD GoldCnt, OilCnt, WipfResc;

WORD RunSec, StartSec, LRunSec;

//---------------------- (Extern Man & Animal Control) ------------------------------

extern MANTYPE *NewMan(MANTYPE **firstman, MANINFO *mi, BYTE con, BYTE type, int x, int y, int tx, BYTE act, int phase, int strn, int ydir, int xdir, int carr, void *tptr);
extern void InitCrewData(BYTE plr, BYTE hostile);
extern void MoveCrew(BYTE plr);
extern STRUCTYPE *InitReadyBase(BYTE plr);
extern BYTE InitAnimals(void);
extern void MoveAnimals(void);
extern void DeleteMen(MANTYPE *firstman);
extern void ExecCrossChecking(void);
extern BYTE GameControl(char inc, char exinc);
extern void ExecGameCon(void);
extern void PromotionCheck(void);
extern MANTYPE *HighRankMan(BYTE plr);
extern MANINFO *HighRankManInfo(BYTE plr);
extern int CrewBaseXOverride;
extern void InitMouseCon(void);
extern void DeInitMouseCon(void);

//-------------------------- Fast Random Functions ---------------------------

const int FRndRes = 500;

int FRndBuf3[FRndRes];
BYTE FRndBuf4[FRndRes];
int FRndPtr3 = 0, FRndPtr4 = 0;

void Randomize3(void)
{
	int cnt;
	for (cnt = 0; cnt < FRndRes; cnt++) FRndBuf3[cnt] = random(3) - 1;
}

int Rnd3(void)
{
	FRndPtr3++; if (FRndPtr3 == FRndRes) FRndPtr3 = 0;
	return FRndBuf3[FRndPtr3];
}

void Randomize4(void)
{
	int cnt;
	for (cnt = 0; cnt < FRndRes; cnt++) FRndBuf4[cnt] = random(4);
}

BYTE Rnd4(void)
{
	FRndPtr4++; if (FRndPtr4 == FRndRes) FRndPtr4 = 0;
	return FRndBuf4[FRndPtr4];
}

//-------------------------- Round Error Handling ----------------------------

char RoundErrorLine[RoundErrNum][31];

void InitRoundError(void)
{
	int cnt;
	for (cnt = 0; cnt < RoundErrNum; cnt++)
		RoundErrorLine[cnt][0] = 0;
}

void RoundError(char *txt)
{
	int pos;
	for (pos = 0; pos < RoundErrNum; pos++)
	{
		if (!RoundErrorLine[pos][0]) break;
		if (SEqual(RoundErrorLine[pos], txt)) return;
	}
	if (pos < RoundErrNum)
		SCopy(txt, RoundErrorLine[pos], 30);
}

//---------------------------- Game DAC Control ------------------------------

int GameDACFPos, GameDACNum;
BYTE GameDAC[768];

void GameSetDAC(int fpos, int num, void *buf)
{
	GameDACFPos = fpos; GameDACNum = num;
	MemCopy((BYTE*)buf, GameDAC, num * 3);
}

//--------------------------- Game Sound Control -----------------------------

typedef struct GAMESOUND {
	int id;
	long age;
	BYTE *buf;
};

const int MaxGameSound = 15;

GAMESOUND GameSound[MaxGameSound];


BYTE GameSoundOn;

int GSTP, LGSP;

void InitGameSounds(void)
{
	int cnt;
	for (cnt = 0; cnt < MaxGameSound; cnt++)
	{
		GameSound[cnt].id = -1;
		GameSound[cnt].age = 0L;
		GameSound[cnt].buf = NULL;
	}
	GSTP = -1; LGSP = -1;
}

BYTE GameSoundPlaying(void)
{
	if (GameSoundOn)
		switch (Config.Sound)
		{
		case 1: return CTVSoundCheck();
		case 2: return DSPSoundCheck();
		case 3: return SDLSoundCheck();
		}
	return 0;
}

void InitGameSound(void)
{
	int errcode;
	GameSoundOn = 0;
	switch (Config.Sound)
	{
	case 1:
		errcode = InitCTVSound(Config.SoundPath, Config.SoundPort, Config.SoundIRQ);
		if (errcode)
		{
			RoundError("CT-Voice Sound Init error:");
			RoundError(CTVErrMsg(errcode));
		}
		else
			GameSoundOn = 1;
		break;
	case 2:
		if (!InitDSPSound(&Config.SoundPort))
			RoundError("DSP/Timer Sound Init error");
		else
			GameSoundOn = 1;
		break;
	case 3:
		if (!SDLInitSound())
		{
			char msg[200];
			sprintf(msg, "SDL Sound Init error: %s", SDL_GetError());
			RoundError(msg);
		}
		else
			GameSoundOn = 1;
	}
	InitGameSounds();
}

/*void GameSoundStatusMsg(void)
  {
  int cnt;
  for (cnt=0; cnt<MaxGameSound; cnt++)
	if (GameSound[cnt].id>-1)
	  sprintf(OSTR+3*cnt,"%02d ",GameSound[cnt].id);
	else
	  sprintf(OSTR+3*cnt,"-- ");

  LPage(CPGE);
  SOut(OSTR,0,6,CWhite,CBlack);
  }*/

BYTE LoadGameSound(int id, int sndnum) // Returns VOC loading errors
{
	char filename[50];
	int loaderr;
	sprintf(filename, "C3RSOUND.GRP|CLSM%02d.VOC", id);
	//sprintf(filename,"SOUND\\CLSM%02d.VOC",id);
	loaderr = LoadVOC(filename, &(GameSound[sndnum].buf));
	if (loaderr == 0)
	{
		GameSound[sndnum].id = id;
		GameSound[sndnum].age = 0L;
		//GameSoundStatusMsg();
	}
	if (loaderr > 1)
	{
		sprintf(OSTR, "Sound file error %d", loaderr);
		RoundError(OSTR);
	}
	if (loaderr == 1) LowMem = 1;
	return loaderr;
}

BYTE KickOutOldGameSound(void) // Returns 0 if no sounds loaded
{
	int cnt, oldest = -1;

	for (cnt = 0; cnt < MaxGameSound; cnt++)
		if (GameSound[cnt].id > -1)
			if ((oldest == -1) || (GameSound[cnt].age > GameSound[oldest].age))
				oldest = cnt;

	if (oldest == -1) return 0;
	// better: StopSound
	//if ((GameSound[oldest].id == LGSP) && GameSoundPlaying()) return 1;

	SDLClearSound();

	GameSound[oldest].id = -1;
	GameSound[oldest].age = 0L;
	DestroyVOC(&(GameSound[oldest].buf));

	//GameSoundStatusMsg();

	return 1;
}

void ClearGameSounds(void)
{
	//while (GameSoundPlaying());
	SDLClearSound();
	while (KickOutOldGameSound());
}

void DeInitGameSound(void)
{
	if (GameSoundOn)
	{
		ClearGameSounds();
		switch (Config.Sound)
		{
		case 1: DeInitCTVSound(); break;
		case 2: DeInitDSPSound(); break;
		case 3: SDLCloseSound(); break;
		}
	}
}

void SoundAging(void)
{
	int cnt;
	for (cnt = 0; cnt < MaxGameSound; cnt++)
		if (GameSound[cnt].id > -1)
			GameSound[cnt].age++;
}

void StartPlayGameSound(int sndnum)
{
	switch (Config.Sound)
	{
	case 1: CTVPlaySound(GameSound[sndnum].buf); break;
	case 2: DSPPlaySound(GameSound[sndnum].buf); break;
	case 3: SDLPlaySound(GameSound[sndnum].buf); break;
	}
}

void PlayGameSound(int id)
{
	int cnt, sndnum = -1;

	// DAC has priority
	if (GameDACFPos > -1) return;

	// Sound idle okay?
	if (GameSoundPlaying()) return;

	// Sound in bank?
	for (cnt = 0; cnt < MaxGameSound; cnt++)
		if (GameSound[cnt].id == id)
		{
			sndnum = cnt; break;
		}

	// No, load sound (kick out old one if necessary)
	if (sndnum == -1)
	{
		// Find bank spot to load into
		do
		{
			for (cnt = 0; cnt < MaxGameSound; cnt++)
				if (GameSound[cnt].id == -1)
				{
					sndnum = cnt; break;
				}
			if (sndnum == -1) KickOutOldGameSound();
		} while (sndnum == -1);

		// Load sound, repeated try while insuf mem & old sounds can be kicked
		while ((LoadGameSound(id, sndnum) == 2) && KickOutOldGameSound());

		if (GameSound[sndnum].id != id) sndnum = -1; // Loading failure
	}

	// Play sound
	if (sndnum > -1) StartPlayGameSound(sndnum);
}

//---------------------------- Screen Adjustment -----------------------------

void ScreenTo(int *scrto)
{
	int rval = 0, cnt, cnum = 0;
	for (cnt = 0; cnt < 3; cnt++)
		if (!BSA.Plr[cnt].Eliminate) if (Crew[cnt].FirstMan)
			if (Crew[cnt].TogMode != 1) // Regular CrewAveY
			{
				if (Crew[cnt].AveY != -1) { rval += Crew[cnt].AveY; cnum++; }
			}
			else // Toggling, use Cursor Y
			{
				if (Crew[cnt].Cursor) { rval += Crew[cnt].Cursor->y; cnum++; }
			}
	if (cnum) *scrto = (rval / cnum) - 75;
}

//------------------------- Clonk Name File Handling --------------------------

FILE *CNameFile;
char CNameBuf[MaxClonkName + 1];

BYTE InitCNameFile(void)
{
	if (!(CNameFile = fopen("NAMES.TXT", "r"))) return 0;
	return 1;
}

void DeInitCNameFile(void)
{
	if (CNameFile) fclose(CNameFile);
}

char *GetClonkName(void)
{
	int advn, cnt, rwind = 0;
	char fbuf;
	BYTE nameok = 0;
	if (CNameFile)
	{
		do
		{
			advn = random(10) + 3;
			while (advn > 0)
			{
				fbuf = fgetc(CNameFile);
				if (fbuf == EOF) { rewind(CNameFile); rwind++; }
				if (rwind > 2) { sprintf(CNameBuf, "Clonk %d", random(20) + 1); return CNameBuf; }
				if (!fbuf || (fbuf == 0x0A)) advn--;
			}
			for (cnt = 0; cnt < MaxClonkName; cnt++)
			{
				CNameBuf[cnt] = fgetc(CNameFile);
				if ((CNameBuf[cnt] == EOF) || (CNameBuf[cnt] == 0x0A) || !CNameBuf[cnt]) break;
			}
			CNameBuf[cnt] = 0;
			if (CNameBuf[0] && (CNameBuf[0] != '#') && (CNameBuf[0] != ' ')) nameok = 1;
		} while (!nameok);
	}
	else
		sprintf(CNameBuf, "Clonk %d", random(20) + 1);
	return CNameBuf;
}

//----------------------- Round Object Setup ----------------------------------

extern int BSAPlrNum(void);

BYTE InitRoundObjects(void)
{
	static int rdyvhct[5] = { VHLORRY,VHCATAPULT,VHSAILBOAT,VHCROSSBOW,VHBALLOON };
	int cnt, cnt2, outx, outy, moutx;
	VEHICTYPE *plrbln = NULL;
	STRUCTYPE *plrcst;
	MANINFO *cman;
	void *hometptr;

	LPage(BackPage);

	for (cnt = 0; cnt < MaxGamePlr; cnt++) InitCrewData(cnt, (BSA.SMode == S_MELEE));

	InitPXS(UserPref.SmokeOn, BSA.Realism.WtrOut);

	for (cnt = 0; cnt < MaxGamePlr; cnt++) // Players
		if (BSA.Plr[cnt].Col >= 0) if (BSA.Plr[cnt].Info) // latter one is safety
		{
			// ReadyBase & ReadyBalloon
			plrcst = InitReadyBase(cnt);

			if (BSA.Plr[cnt].ReadyVhc[4])
				plrbln = NewVehic(VHBALLOON, 0, -20, 159, -1, -1, 0, cnt, NULL);

			if (BSA.Realism.CstHome) hometptr = plrcst; else hometptr = plrbln;

			// No-Base-Entry location
			if (!hometptr)
			{
				LPage(BackPage);
				if (!FindSurface(1, Crew[cnt].BaseX, 20, &outx, &outy))
				{
					outx = Crew[cnt].BaseX; for (outy = 0; !Inside(GBackPix(outx, outy), CSolidL, CSolid2H); outy++);
				}
				outx = BoundBy(outx, 0, 311);
			}
			else
				outx = outy = 0;

			// Init clonks
			for (cman = (BSA.PCrew ? BSA.Plr[cnt].Info->crew : BSA.Plr[cnt].TCrew), cnt2 = BSA.Plr[cnt].Clonks; cman && (cnt2 > 0); cman = cman->next)
				if (!cman->dead)
				{
					moutx = outx - 4 + random(21) - 10;
					NewMan(&Crew[cnt].FirstMan, cman, 0, MNMAN, moutx, outy - 9, moutx + 1 - 2 * (moutx > 160), hometptr ? MAHOME : MAWALK, 0, 100 + 5 * cman->rank, 0, 0, NOROCK, hometptr);
					cnt2--;
				}

			// Cursor & con set to hirank
			Crew[cnt].Cursor = HighRankMan(cnt);
			if (Crew[cnt].Cursor) Crew[cnt].Cursor->con = 1;

			Crew[cnt].HiRankCap = HighRankManInfo(cnt); // HiRankCap for KillCap

			if (BSA.GPlrElm == 2) // Flags out
				if (Crew[cnt].Cursor)
				{
					Crew[cnt].Cursor->carr = FLAG;
					Crew[cnt].Cursor->carrp = cnt;
				}
			if (BSA.Realism.CstHome && !BSA.Plr[cnt].ReadyBase[0] && (BSA.RuleSet > R_EASY))
			{                                                // Conkit out for
				NewRock(outx, outy - 8, RADEAD, CONKIT, 0, 0, 0, -1);     // own castle base
			}

			// Init other ReadyVehics
			for (cnt2 = 0; cnt2 < 4; cnt2++)
				if (BSA.Plr[cnt].ReadyVhc[cnt2])
					if (hometptr)
						NewVehic(rdyvhct[cnt2], -1, -1, 0, 0, 0, 0, cnt, hometptr);
					else
						NewVehic(rdyvhct[cnt2], BoundBy(outx - 8 + random(4), 0, 308), outy - 8, 0, 0, 0, 0, cnt, NULL);
		}

	InitAnimals();

	for (cnt = random(5 + Weather.climate / 50); cnt >= 0; cnt--) PlantTree(1);

	return !LowMem;
}

void DeInitRoundObjects(void)
{
	int cnt;
	for (cnt = 0; cnt < 3; cnt++) DeleteMen(Crew[cnt].FirstMan);
	DeleteMen(FirstAnimal);
	DeleteRocks();
	DeleteVehics();
	DeleteStructs();
	DeleteLines();
	DeInitPXS();
	CrewBaseXOverride = -1;
}

//-------------------------- Game Goal Operation -------------------------------

void PlayerEliminationCheck(void)
{
	int cnt;
	BYTE elimn;
	for (cnt = 0; cnt < 3; cnt++)
		if ((BSA.Plr[cnt].Col > -1) && !BSA.Plr[cnt].Eliminate)
		{
			elimn = 0;
			switch (BSA.GPlrElm)
			{
			case 0: // Kill the cap
				if (!Crew[cnt].HiRankCap) { RoundError("Safety: null gghirankcap"); return; }
				if (Crew[cnt].HiRankCap->dead) elimn = 1;
				break;
			case 1: // Kill crew
				if (Crew[cnt].ManCnt < 1) elimn = 1;
				break;
			case 2: // Capture the flag
			  // Flag check done by HomeRock()
				if (Crew[cnt].ManCnt < 1) elimn = 1;
				break;
			}
			if (elimn)
			{
				BSA.Plr[cnt].Eliminate = 1;
				Crew[cnt].RedrStB = 1;
				GSTP = SNDSURREND;
			}
		}
}

extern void RockOutAccount(int *gold, int *oil);
extern int AnimalCount(int antype);

BYTE GameOverCheck(void) // Every Sec5, returns over+delay
{
	int cnt, plrin, racgold, racoil;
	BYTE over = 0;

	// Count players in game
	for (cnt = 0, plrin = 0; cnt < 3; cnt++)
		if ((BSA.Plr[cnt].Col > -1) && !BSA.Plr[cnt].Eliminate)
			plrin++;

	// Rock Account
	if ((BSA.SMode == S_COOPERATE) && (BSA.CoopGMode == C_GOLDMINE))
	{
		RockOutAccount(&racgold, &racoil);
		GoldCnt = racgold; OilCnt = racoil;
	}

	switch (BSA.SMode) // Game over if mission goal complete
	{
	case S_MISSION: // According to each mission script - - - - - - - - - - -
		if (MissionSuccess()) over = 1;
		break;
	case S_MELEE: // Game over if less than two competitors - - - - - - - - - -
		if (plrin < 2) over = 1;
		break;
	case S_COOPERATE: // Game over if goal fulfilled - - - - - - - - - - - - -
		switch (BSA.CoopGMode)
		{
		case C_GOLDMINE: // No resources left
			if (Ground.MaAcRun == -1)
				if ((Ground.MaAcGold < 50) && (racgold == 0)) over = 1;
			break;
		case C_MONSTERKILL: // Animals killed
			if (AnimalCount(MNMONSTER) == 0) over = 1;
			break;
		case C_WIPFRESCUE: // Wipfs rescued (percentage?!)
			if (AnimalCount(MNWIPF) == 0) over = 1;
			break;
		case C_SETTLE: // No limitations
			break;
		}
		if (over)
		{
			GameMessage("Mission completed!", 160, 70, CWhite, NULL); GSTP = SNDTRUMPET; over = 100;
		}
		break;
	}

	// Always game over if no player left
	if (plrin < 1) over = 1;

	return over;
}

//----------------------------- Round Messages ------------------------------

extern char *CGModeName[6];
extern char *EliminationName[3];
extern char *CoopSubName[3];
extern char *RuleSetName[3];
extern SCRIPTINFO MissionScript;

void GameModeMessage(void)
{
	switch (BSA.SMode)
	{
	case S_MISSION:
		sprintf(OSTR, "Mission %d: %s", MissionScript.idnum, MissionScript.title);
		break;
	case S_MELEE:
		sprintf(OSTR, "Melee %s: %s", RuleSetName[BSA.RuleSet], EliminationName[BSA.GPlrElm]);
		break;
	case S_COOPERATE:
		sprintf(OSTR, "%s cooperative: %s", RuleSetName[BSA.RuleSet], CGModeName[BSA.CoopGMode]);
		break;
	}
	GameMessage(OSTR, 160, 70, CWhite, NULL);
}

void RoundMessage(void)
{
	sprintf(OSTR, "Round %d", BSA.Round);
	GameMessage(OSTR, 160, 70, CWhite, NULL);
}

//---------------------------- Main Game Loop --------------------------------

void EventCall(int evnum)
{
	if (BSA.SMode == S_MISSION) ExecEventCall(evnum);
}

void NoGameTimeDelay(void)
{
	WORD secdel;
	RunSec = time(NULL) - StartSec;
	secdel = RunSec - LRunSec;
	StartSec += secdel;
}

//extern void KBLockCheck(void);

BYTE ExecRoundLoop(void) // Returns aborted
{
	BYTE gcs = 0, over = 0;
	char inc, exinc;
	int scrto = 0, cnt;

	StartSec = time(NULL); LRunSec = 0;
	TimeOut();

	GSTP = SNDINTRO;
	RoundMessage();

	do
	{
		// Scripting
		if (BSA.SMode == S_MISSION) ExecScript();

		// Screen Control
		Toggle(CPGE); DrawScreen(); PPage(CPGE);

		// Tick Control
		Tick2++;  if (Tick2 == 2)   Tick2 = 0;
		Tick3++;  if (Tick3 == 3)   Tick3 = 0;
		Tick5++;  if (Tick5 == 5)   Tick5 = 0;
		Tick10++; if (Tick10 == 10) Tick10 = 0;
		Tick20++; if (Tick20 == 20) Tick20 = 0;
		Tick50++; if (Tick50 == 50) Tick50 = 0;

		// Game Object Execution
		LPage(BackPage);
		ExecStructs();
		ExecVehics();
		ExecPXS();
		MoveCrew(0); MoveCrew(1); MoveCrew(2);
		MoveAnimals();
		MoveRocks();
		ExecLines();

		ExecWeather();
		ExecGround(&scrto);

		ExecGameCon();

		// Cross Checking
		ExecCrossChecking();

		// Screen adjustment
		if (!Tick3) ScreenTo(&scrto);
		AdjustBackVPos(&scrto);

		// Promotion/Experience/GameGoal Check
		if (Sec5)
		{
			PromotionCheck();
			if (!over) over = GameOverCheck();
		}
		else
			if (Sec1) PlayerEliminationCheck();
		if (over == 1) gcs = 1; if (over > 1) over--;

		// System measures
		if (LowMem)
		{
			RoundError("Temporarily out of memory");
			KickOutOldGameSound();
			LowMem = 0;
		}

		// Time Control
		Sec1 = Sec5 = Sec10 = Sec20 = Sec60 = 0;
		if (!Tick3)
		{
			RunSec = time(NULL) - StartSec;
			if (RunSec != LRunSec)
			{
				TimeOut();
				LRunSec = RunSec;
				Sec1 = 1;
				if (LRunSec % 5 == 0)  Sec5 = 1;  if (LRunSec % 10 == 0) Sec10 = 1;
				if (LRunSec % 20 == 0) Sec20 = 1; if (LRunSec % 60 == 0) Sec60 = 1;
				if (LRunSec == 2) GameModeMessage();
			}
		}

		// Sound Control
		if (GameSoundOn)
		{
			if (GSTP > -1)
			{
				PlayGameSound(GSTP);
				LGSP = GSTP; GSTP = -1;
			}
			if (Sec1) SoundAging();
		}
		// GameSetDAC
		if (GameDACFPos > -1) if (!GameSoundPlaying())
		{
			SetDAC(GameDACFPos, GameDACNum, GameDAC); GameDACFPos = -1;
		}

		// Player Input Game Control
		//if (!Tick5) KBLockCheck();
		/*if (kbhit())
		{
			inc = exinc = 0;
			inc = getch(); if (!inc) exinc = getch();
			if (GameControl(inc, exinc)) gcs = 2;
		}*/

		SDL_Event e;
		int inc = 0;
		int exinc = 0;
		
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_KEYDOWN:
				if (e.key.repeat) break;
				RemapKey(e.key.keysym.sym, &inc, &exinc);
				if (GameControl(inc, exinc)) gcs = 2;
				break;

			case SDL_QUIT:
				gcs = 2;
				break;
			}
		}

		// Game Speed
		if (Config.GameSpeed != 10) SDL_Delay(200 / Config.GameSpeed);
		//if (Config.GameSpeed != 10) SDL_Delay(100 / Config.GameSpeed);
	} while (!gcs);

	return (gcs == 2);
}

//------------------------- Module Main Function -----------------------------

BYTE ExecRound(void) // BSA & UserPref come in global
{		     // Returns 1 aborted, 2 insufficient memory
	int cnt;
	BYTE aborted;

	InitProcessB("Preparing round...");

	// SystemInit
	CPGE = 0; // Current page is SVI MainPage 0
	Tick2 = Tick3 = Tick5 = Tick10 = Tick20 = Tick50 = 0;
	Sec5 = Sec10 = Sec20 = Sec60 = 0;
	LowMem = 0;
	GameDACFPos = -1;
	InitRoundError();

	// SoundInit
	InitGameSound();

	// GameInit
	KeyConDisplay(0);
	InitMouseCon();
	InitGameMessage();
	if (BSA.SMode == S_MISSION) InitScript();

	FirstAnimal = NULL; FirstStruct = NULL; FirstVehic = NULL; FirstLine = NULL; FirstRock = NULL;

	RockCnt = PXSCnt = StructCnt = VehicCnt = LineCnt = ManCnt[0] = ManCnt[1] = ManCnt[2] = AnimalCnt = 0;
	WipfResc = 0;

	InitWeather(BSA.WClim, BSA.WSeas);
	InitGround();

	BackGrSize = BSA.BSize;
	SaveMBx = -1;

	if (!InitBack()) return 2;
	if (!InitRoundObjects()) { DeInitRoundObjects(); DeInitBack(); return 2; }

	DrawClock(0);

	aborted = ExecRoundLoop();

	//DeInit
	DeInitRoundObjects();
	DeInitBack();

	if (BSA.SMode == S_MISSION) DeInitScript();

	DeInitGameSound();

	return aborted;
}
