/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  CLONK Miner's  Mutual type/def/constant header

#include "clonk.h"

//---------------------- RoundEXC Type Definitions ---------------------------

const int RockTypeNum=28;
const int RockOrderNum=RckOrderNum;

const int CFlashTime=100;

const int MagicNum=9;

//--------------------------------- Men --------------------------------------

typedef struct MANTYPE { MANINFO *mi;
			 int type; BYTE act, onf, con;
			 int x, y, tx, xdir, ydir, phase, strn, del, carr, carrp;
			 void *tptr;
			 MANTYPE *next;
		       };

typedef struct ROCKPROD { int Store[RockOrderNum],PrDel[RockOrderNum]; };

typedef struct CREWTYPE { MANTYPE *FirstMan,*Cursor;
			  MANINFO *HiRankCap;

			  BYTE TogMode;
			  int AveX,AveY;     // for controlled
			  int ManCnt,ConCnt; // for all out (not dead, not home)

			  int DontPushDelay;
			  int SelectAllCount,DropCount,DontPickUp;
			  int MsWalkDClick;
			  BYTE EDig; int EDigMass;

			  int CaFlX,CaFlY;

			  int CMenu,CMenuData;
			  BYTE RedrStB;
			  int StBX,StBWdt;
			  int ConFlash,CursorFlash;

			  // Home base & general data
			  int Wealth,StrBank;
			  int RockStorage[RockTypeNum];
			  BYTE Hostile[3];
			  int BaseX;

			  ROCKPROD RckProd;
			};

#define MNNOMAN  -1
#define MNMAN     0
#define MNSHARK   1
#define MNWIPF    2
#define MNMONSTER 3

#define MAWALK   0
#define MAFIGHT  1
#define MATHROW  2
#define MAFLY    3
#define MASWIM   4
#define MADIG    5
#define MABRIDGE 6
#define MABUILD  7
#define MAPUSH   8
#define MADEAD   9 // No ave/count
#define MAHOME  10 // No draw/count (no sprite)

const int ManActNum=11;

//-------------------------- CapMenu Types ----------------------------------

#define CMNOMENU    0
#define CMMAIN      1
#define CMRCKORDER  2
#define CMVHCORDER  3
#define CMHOSTILITY 4
#define CMBRIDGE    5
#define CMBUILD     6
#define CMSRCPIPE   7
#define CMSURRENDER 8
#define CMCOMMAND   9
#define CMMAGIC    10

//-------------------------------- Rocks -------------------------------------

typedef struct ROCKTYPE { int x, y, type, ydir, xdir, phase, thrby;
			  BYTE act;
			  ROCKTYPE *next;
			};

#define NOROCK    -1
#define ROCK       0
#define GOLD       1
#define FLINT      2
#define TFLINT     3
#define FBOMB      4
#define LIQROCK    5
#define LIQGRAN    6
#define LIQEARTH   7
#define STEEL      8
#define MONSTEGG   9
#define ZAPN      10
#define ZUPN      11
#define LOAM      12
#define BARREL    13
#define WATBARREL 14
#define ACDBARREL 15
#define OILBARREL 16
#define CONKIT    17
#define LINECON   18
#define ARROW     19
#define FARROW    20
#define BARROW    21
#define PLANT1    22
#define PLANT2    23
#define PLANT3    24
#define FLAG      25
#define COMET     26
#define ROCKPXS   27

#define RAFLY  0
#define RAROLL 1
#define RADEAD 2

const int RockValue[RockTypeNum]={1,5,5,4,15,1,8,0,10, 4, 2,2,2,
				   2, 2,10,7, 15,5,
				  10,20,25,
				   1, 2, 3, 10, 0, 0};

const int RockOrder[RockOrderNum]={FLINT,TFLINT,FBOMB,
				   LOAM,LIQROCK,LIQGRAN,
				   STEEL,CONKIT,LINECON,
				   BARREL,WATBARREL,OILBARREL,
				   ARROW,FARROW,BARROW};

const int RockOrderRad[RockOrderNum]={0,0,1,
				      0,0,1,
				      1,0,1,
				      0,0,1,
				      1,1,1};

//------------------------------ Vehicles ------------------------------------

typedef struct VEHICTYPE { int x, y, p, type, data, data2, data3, owner;
			   BYTE back[30];
			   void *tptr;
			   VEHICTYPE *next;
			 };
#define VHNOVEHIC -1
#define VHLORRY    0
#define VHELEVATOR 1
#define VHBALLOON  2
#define VHCATAPULT 3
#define VHSAILBOAT 4
#define VHCROSSBOW 5

const int VhcOrderNum=6;     // Clonk,Lorry,Cat,Boat,XBow,Balloon
const int VhcOrderPrice[VhcOrderNum]={10,10,15,15,20,20};

//----------------------------- Structures -----------------------------------

typedef struct STRUCTYPE { int type, x, y, p, con, energy, owner, liqstr, damage;
			   BYTE onf;
			   void *tptr;
			   STRUCTYPE *next;
			 };

#define STNOSTRUCT  -1
#define STWINDMILL   0
#define STWATERWHEEL 1
#define STOILPOWER   2
#define STELEVATOR   3
#define STPUMP       4
#define STCASTLE     5
#define STHOUSE      6
#define STTOWER      7
#define STMAGIC      8
#define STOTHER      9
#define STOTHER2    10
#define STCACTUS    11
#define STDECIDTREE 12
#define STCONIFTREE 13

const int StructTypeNum=14;

const int StructWdt[StructTypeNum]={16,16,16,16,16,20,16,16, 16,0,0, 16,16,16};
const int StructHgt[StructTypeNum]={20,12,12,20,12,12,12,20, 20,0,0, 20,20,20};
const int StructMaxDam[StructTypeNum]={20,20,30,20,20,50,30,85, 60,0,0, 5,5,5};

const int StructBuildNum=9;
const int StructBuild[StructBuildNum]={STCASTLE,STTOWER,STELEVATOR,
				       STWINDMILL,STOILPOWER,STPUMP,
				       STMAGIC,STOTHER,STOTHER};
const int StructBuildRS[StructBuildNum]={0,1,1, 2,2,2, 2,3,3};

//------------------------------- Lines --------------------------------------

const int LineLen=25;

typedef struct LINETYPE { int type, x[LineLen], y[LineLen], lsec;
			  BYTE mfd;
			  STRUCTYPE *fstrc,*tstrc;
			  LINETYPE *next;
			};

#define LNNOLINE -1
#define LNENERGY  0
#define LNSRCPIPE 1
#define LNTRGPIPE 2

//----------------------------- PXS Types ------------------------------------

typedef struct PXSTYPE { BYTE type; int x,y,p; };

#define PXSINACTIVE   0
#define PXSWATER      1
#define PXSSMOKE      2
#define PXSMASSWATER  3
#define PXSSNOW       4
#define PXSZAP        5
#define PXSSAND       6
#define PXSBUBBLE     7
#define PXSOIL        8
#define PXSMASSOIL    9
#define PXSCONCRETE  10
#define PXSBURNOIL   11
#define PXSLAVA      12
#define PXSMASSLAVA  13
#define PXSACID      14
#define PXSMASSACID  15
#define PXSSPARK     16

#define PXSLIQDRAIN  17

const int PXSTypeNum=18;

const int PXSChunkSize=300,PXSMaxChunk=15;

//---------------------------- Weather & Ground -----------------------------------

const int LightNLength=60;
const int MaxLightN=5;

typedef struct LIGHTNING { int type,stat,phase,phase2,lx[LightNLength],ly[LightNLength]; };

typedef struct EXPLTYPE { int act, x, y; };

typedef struct WEATHERTYPE { LIGHTNING LightN[MaxLightN];
			     EXPLTYPE expln;
			     DWORD climate,season;
			     int wind,wind2;
			     int stdtemp,temp,lsttmp,tchcnt;
			     int rfall;
			     int mflvl,mfact;
			     int thlprob;
			   };

const int VBranchNum=20;

typedef struct VBRANCH { BYTE act; int x,y,dir,size; };

typedef struct VOLCANO { BYTE act; int age;
			 VBRANCH b[VBranchNum];
			 BYTE glowtick;
			 BYTE lfcol[6][3],lccol[6][3],ltcol[6][3];
		       };

typedef struct GROUNDTYPE { VOLCANO volcano;
			    int equake;
			    int gconwait;
			    int levell,levelr;
			    int MaAcRun;
			    DWORD MaAcGold,MaAcOil;
			  };

//------------------------- Game & System Constants --------------------------

// VGA4PAGE Page usage
const BYTE BackPage=3,GFXPage=2;

//------------------------- Color/Material Constants -------------------------

// Other
const int CSmoke=96,CZap=60,CLightning=88;
// Background
const BYTE CSky1=128,CSky2=159,CTunnel1=160,CTunnel2=164;
// Vehicle Mobile Solids
const BYTE CVhcLS=186,CVhcLT=187,CVhcSS=188,CVhcST=189,CVhcRS=190,CVhcRT=191;
// Extreme/Very Solids
const BYTE CGranite1=192,CGranite2=194,CRock1=195,CRock2=197,CGold1=198,CGold2=200;
// Solids
const BYTE CEarth1=201,CEarth2=205,CAshes1=206,CAshes2=208;
// Removeable Solids, solid to men (snow)
const BYTE CSandS=220,CSandT=221,CSnowS=222,CSnowT=223;
// Removeable Solids, solid to PXS (liquids)
const BYTE CWaterS=224,CWaterT=225,CAcidS=226,CAcidT=227,COilS=228,COilT=229;
const BYTE CLavaS1=230,CLavaT1=231,CLavaS2=232,CLavaT2=233,CLavaS3=234,CLavaT3=235;




// Class defines
const BYTE CVhcL=CVhcLS,CVhcH=CVhcRT;           // vehicle class
const BYTE CPXSL=CSandS,CPXSH=CLavaT3;          // PXS/removable class
const BYTE CLiqL=CWaterS,CLiqH=CLavaT3;         // liquid class
const BYTE CGroundL=CGranite1,CGroundH=CAshes2; // "earthy" ground solid
const BYTE CEGroundL=CEarth1,CEGroundH=CAshes2; // diggable ground solid

const BYTE CSolidL=CVhcL,CSolidH=CSnowT;       // to men
const BYTE CSolid2L=CGranite1,CSolid2H=CLiqH;  // to (some) PXS & balloon





const BYTE LiqRelClassM[6]={PXSMASSWATER,PXSMASSACID,PXSMASSOIL,PXSMASSLAVA,PXSMASSLAVA,PXSMASSLAVA};
const BYTE LiqRelClass[6]={PXSWATER,PXSACID,PXSOIL,PXSLAVA,PXSLAVA,PXSLAVA};


//----------------------------- SVI Color Constants ------------------------------

const BYTE CBlack=0,CGray1=1,CGray2=2,CGray3=3,CGray4=4,CGray5=5,CWhite=6;
const BYTE CDRed=7,CDGreen=8,CDBlue=9,CRed=10,CGreen=11,CBlue=12,CYellow=13;

//------------------------ Mouse Control Definitions -------------------------

typedef struct MOUSECON { BYTE status;
			  int player;
			  BYTE cursor;
			  BYTE but1,but2;
			  int x1,y1,x2,y2;
			};

//-------------------------------- Sound Defines -----------------------------

#define SNDCLONK    0
#define SNDEXPLODE  1
#define SNDROCK     2
#define SNDSPLASH   3
#define SNDSCREAM   4
#define SNDCATAPULT 5
#define SNDTHUNDER  6
#define SNDHURT1    7
#define SNDHURT2    8
#define SNDHURT3    9
#define SNDDEAD    10
#define SNDMONSTER 11
#define SNDWIPF    12
#define SNDYIPPIE  13
#define SNDZAP     14
#define SNDBREATH  15
#define SNDFIGHT1  16
#define SNDFIGHT2  17
#define SNDYANK    18
#define SNDMETAL   19
#define SNDPSHSH   20
#define SNDEQUAKE  21
#define SNDCHAP    22
#define SNDMUNCH   23
#define SNDTRUMPET 24
#define SNDCONSTR1 25
#define SNDCONSTR2 26
#define SNDCONNECT 27
#define SNDFIRE    28
#define SNDGATE    29
#define SNDDRILL   30
#define SNDCGATE   31
#define SNDDING    32
#define SNDCASH    33
#define SNDERROR   34
#define SNDXBOW    35
#define SNDLAVA    36
#define SNDCGATEC  37
#define SNDINTRO   38
#define SNDSURREND 39
#define SNDGRAB    40
#define SNDKNURPS  41
#define SNDBRZZ    42
