/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf-D CLONK Header

const int MaxGamePlr=3;

const int MaxClonkName=15;
const int MaxPlayerName=20,MaxNickName=10;
const int MaxQuoteLen=50;
const int MaxPlrRankName=MaxPlayerName;

const int RckOrderNum=15;

const int RoundErrNum=8;

typedef struct CONFIG {
		      int FaceNum,FaceFile,ImportMode;
		      int SortPlrBy,ShowQuotes;
		      int GameSpeed;
		      int SaveDatAlways;
		      int LangType;

		      int HiResTitle;

		      int Sound;
		      int SoundTIRQ,SoundTPort;
		      WORD SoundIRQ,SoundPort;
		      char SoundPath[256];
		      int SoundDynamic;

		      char KCom[3][10];

		      DWRD RegVal;
		      BYTE RegName[30+1];

		      DWRD RoundStartCount,RoundEndCount;

		      BYTE futurebuf[100]; // new in 3.1
		      };

typedef struct PLAYERRANK { char name[MaxPlrRankName+1];
			    PLAYERRANK *next;
			  };

typedef struct REALISM {
		       int StrcSnow,StrcBurn,StrcEnrg;
		       int WtrOut,RckOut;
		       int EmrPromo;
		       int CstHome;
		       int RealVal;
		       };

typedef struct USERPREF {
			int SmokeOn;
			int GameMsgOn;
			int LineBeforeStrc;
			int PlrPosByCon;
			int CeaseAction;
			int DoubleDigSpeed;
			int NoQuotes;
			int TwoButtonJump; // new in 3.2

			BYTE futurebuf[48]; // new in 3.1
			};

typedef struct MANINFO { char name[MaxClonkName+1];
			 int rank,exp;
			 int rnds,rean;
			 BYTE dead;
			 BYTE futurebuf[10]; // new in 3.1
			 MANINFO *next;
		       };

typedef struct PLAYERINFO {
			  char name[MaxPlayerName+1],nick[MaxNickName+1];
			  char quote[MaxQuoteLen+1];
			  int rank;
			  int rnds[3],won[3]; DWRD score[3],scorepot;
			  DWRD playsec;
			  int face;
			  int pfcol,pfcon;
			  BYTE inses,inrnd;
			  int pfgpos; // new in 3.1
			  int autorean; // new in 3.2
			  BYTE futurebuf[46]; // new in 3.1
			  MANINFO *crew;
			  PLAYERINFO *next;
			  };

typedef struct BSAPLRTYPE { PLAYERINFO *Info;
			    MANINFO *TCrew;
			    int Col,Con;
			    int Clonks,Wealth;
			    int ReadyBase[5];
			    int ReadyVhc[5];
			    BYTE Eliminate;
			    WORD ScoreGain;
			    int GPos; // new in 3.2
			    BYTE futurebuf[48]; // new in 3.2
			  };

typedef struct BSATYPE {
		       // Session
		       int SMode;
		       int RuleSet;
		       int PCrew,TMode;
		       int AllowSurrender;
		       int PlrsInRound;
		       int NextMission;
		       // Game
		       int CoopGMode,GPlrElm;
		       int Round;
		       int DiffVal;
		       // Players
		       BSAPLRTYPE Plr[MaxGamePlr];
		       // Game Data
		       int RckProdStart[RckOrderNum],RckProdTime[RckOrderNum];
		       int RckProdSpeed;
		       // Animals
		       int AnmRandom;
		       int Wipfs,Sharks;
		       int Monsters;           // Sinus-Phase start:
		       // Landscape            // 0-100 = 0-2ã
		       int BSize;              // Sinus-Phase length:
		       int BPhase,BPLen;       // 0-100 = 1/4ã-4ã
		       int BCrvAmp,BRndAmp;    // Curve,Random 0-100%
		       int BWatLvl;            // Water Level 0-100
		       int BVolcLvl,BQuakeLvl; // Volcano/Quake Level 0-100
		       int BCanyon;
		       int BRockType[20];
		       int BRockMode;
		       int BckRandom;
		       // Weather
		       int WClim,WSeas,WYSpd;
		       int WRFMod,WLPMod;
		       int WCmtLvl;            // Comet level 0-100
		       int WEnvr;
		       int WeaRandom;
		       // Special
		       int FlintRadius;
		       // Realism
		       REALISM Realism;
		       // Special
		       int CometHail; // new in 3.2

		       BYTE futurebuf[98]; // new in 3.1
		       };

//----------------------------------------------------------------------------

// Session modes

#define S_MISSION   0  // (Tutorial)
#define S_COOPERATE 1  // (Single player)
#define S_MELEE     2  // (Tournament)

// Cooperate game modes

#define C_GOLDMINE    0
#define C_MONSTERKILL 1
#define C_WIPFRESCUE  2
#define C_SETTLE      3

// Rule sets

#define R_EASY     0
#define R_ADVANCED 1
#define R_RADICAL  2