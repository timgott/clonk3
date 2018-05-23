/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// CLONK Main Auxiliary

// Misc GFX Support, Sound Setup, Registration, Config & UP, KeyboardDef

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <bios.h>

#include <standard.h>
#include <vga4.h>
#include <std_gfx.h>
#include <stdfile.h>

#include <ct-voice.h>
#include <dsp-dtss.h>

#include "svi20ext.h"

#include "clonk.h"

#include "bp.h"

#define CAPSLOCK 64
#define NUMLOCK  32

//---------------------------- Main externals -------------------------------

const BYTE MainPage=0,MenuPage=1,FacePage=3;
const int DefFaceNum=8;

BYTE MenuPagesLoaded=0;

char RunningPath[257];


extern CONFIG   Config;
extern USERPREF UserPref;
extern BSATYPE  BSA;

extern PLAYERINFO *FirstPlayer;
extern PLAYERRANK *FirstPlrRank;

extern char OSTR[500];
extern BYTE InRound,FirstTimeRun,SBDetected;

extern char *SoundStatusName[3];

extern void ClearBSAPlrPtrs(void);
extern MANINFO *NewDefaultMan2List(MANINFO **crew);
extern PLAYERINFO *NewDefaultPlr2List(void);
extern void SortCrewList(MANINFO *crew);
extern BYTE NewPlrRank(char *name);
extern int SortPlayerList(int dummy);
extern void DeInitPlayerList(void);
extern void ClearPlrRanks(void);
extern int PlrListCount(void);
extern int CrewListCount(PLAYERINFO *plr);
extern void DefaultPlrRanks(void);
extern int PlrRankCount(void);

char *ConPlrName[3]={"Left Player","Center Player","Right Player"};
char *ConConName[10]={"Select Cursor left","Select Toggle","Select Cursor right",
		     "Throw","Jump","Dig",
		     "Clonks left","Clonks Stop","Clonks right",
		     "Activate menu"};

//------------------------ Extern Image File Handling -----------------------------

extern BYTE LoadAGC2PageV1(char *fname, BYTE tpge, int tx, int ty, WORD *iwdt, WORD *ihgt, BYTE *palptr, BYTE *scram);
extern BYTE SaveAGCPageV1(char *fname, BYTE fpge, int fx, int fy, WORD wdt, WORD hgt, BYTE *palptr, BYTE *scram);
extern char *AGCError(BYTE errnum);
extern BYTE LoadPCX(char *fname, BYTE tpge, int tx, int ty, COLOR *tpal);
extern char *LoadPCXError(BYTE errn);

//------------------------- Keyboard Technicals ------------------------------

unsigned far *BIOSKBFPtr;

void KBLockCheck(void)
  {
  union REGS regs;
  int kbstatus;
  kbstatus=(bioskey(2));
  if (kbstatus & CAPSLOCK) // CapsLock activated
    {
    *BIOSKBFPtr &= ~CAPSLOCK;
    regs.h.ah=1;
    int86(0x16,&regs,&regs);
    }
  if (!(kbstatus & NUMLOCK)) // NumLock deactivated
    {
    *BIOSKBFPtr |= NUMLOCK;
    regs.h.ah=1;
    int86(0x16,&regs,&regs);
    }
  }

void InitKLockCheck(void)
  {
  BIOSKBFPtr=(unsigned far *) MK_FP(0x40,0x17);
  }

//------------------------------ Misc GFX Support ----------------------------

void DrawTitleBack(BYTE tpge, BYTE design)
  {
  int cnt,cnt2;
  for (cnt=0; cnt<10; cnt++)
    for (cnt2=0; cnt2<4; cnt2++)
      B4Move(MenuPage,0,0,tpge,20*cnt2,20*cnt,20,20);
  B4Move(MenuPage,20,0,tpge,22+22*design,0,35,20);
  B4Move(MenuPage,50,20,tpge,52+22*design,20,5,5);
  }

void ResetPlayerFaces(void)
  {
  PLAYERINFO *cplr;
  for (cplr=FirstPlayer; cplr; cplr=cplr->next) cplr->face=0;
  }

BYTE LoadMenuPages(void)
  {
  BYTE cpal[256*3];
  BYTE loadrv;

  if (MenuPagesLoaded) return 1;

  // Load Game Graphics
  loadrv=0;
  if (!Config.FaceFile)
    loadrv=LoadAGC2PageV1("C3RGRAFX.GRP|C3DEFACE.AGC",FacePage,0,0,NULL,NULL,cpal,0);
  if (!loadrv)
    loadrv=LoadAGC2PageV1("C3RGRAFX.GRP|C3MENU2.AGC",FacePage,0,100,NULL,NULL,cpal,0);
  if (!loadrv)
    loadrv=LoadAGC2PageV1("C3RGRAFX.GRP|C3MENU1.AGC",MenuPage,0,0,NULL,NULL,cpal,0);
  if (loadrv)
    {
    sprintf(OSTR,"Error while reading the|graphics file C3RGRAFX.GRP:|€%c%s|€%cProgram aborted",CDRed,AGCError(loadrv),CGray1);
    Message(OSTR,"program file error"); return 0;
    }
  DefCols2Pal(cpal);
  SetDAC(0,256,cpal); // Running colors are taken from C3MENU1.AGC
  MenuPagesLoaded=1;
  InitSVIGrayMap();

  // Load User Portraits (FACES.DAT)
  if (Config.FaceFile)
    {
    loadrv=LoadAGC2PageV1("FACES.DAT",FacePage,0,0,NULL,NULL,cpal,0);
    if (loadrv)   // could also check for correct size
      {
      sprintf(OSTR,"Error while reading the|player portrait FACES.DAT:|€%c%s",CDRed,AGCError(loadrv));
      Message(OSTR,"program file error");
      ResetPlayerFaces();
      Config.FaceFile=0; Config.FaceNum=DefFaceNum;
      // Unchecked attempt to restore default faces
      LoadAGC2PageV1("C3RGRAFX.GRP|C3DEFACE.AGC",FacePage,0,0,NULL,NULL,cpal,0);
      }
    }

  return 1;
  }

extern void DrawSurface(int addsize, void (*drawpixfunc)(int tx,int ty));

void DSPSetPix(int tx, int ty) // Recalculate 320x200-projection
  {
  if ((tx%4==0) && (ty%4==0)) SPixA(240+tx/4,0+ty/4,202+random(4));
  }

int DrawSurfacePreview(int dummy)
  {
  // for now, brutally blast pic to (current) page
  LPage(MenuPage);
  B4Move(MenuPage,60,50,MenuPage,60,0,20,50);
  DBox(240,33-15*BSA.BWatLvl/100,319,49,224);
  DrawSurface(0,&DSPSetPix);
  // DrawSurface uses global BSA
  return dummy;
  }

//--------------------------- Sound Setup Menus ------------------------------

WINDOW *SoundSetWin;

int DeactivateSound(int dummy)
  {
  Config.Sound=0;
  CloseWindow(SoundSetWin);
  return dummy;
  }

int ActivateCTVMethod(int dummy)
  {
  Config.Sound=1;
  CloseWindow(SoundSetWin);
  return dummy;
  }

int ActivateDSPMethod(int dummy)
  {
  Config.Sound=2;
  CloseWindow(SoundSetWin);
  return dummy;
  }

int InitTestDSPMethod(int dummy)
  {
  BYTE *sample;
  DWRD prcnt;

  Config.Sound=0;
  Config.SoundIRQ=Config.SoundTIRQ;
  Config.SoundPort=0x210+0x10*((Config.SoundTPort-210)/40);

  InitProcessB("DSP/Timer Sound test...");

  if (LoadVOC("C3RSOUND.GRP|CLSM00.VOC",&sample))
    { Message("Error while loading|sound file C3RSOUND.GRP","error help not translated"); return dummy; }

  if (!InitDSPSound(&Config.SoundPort))
    { Message("DSP/Timer-Method could|not be initialized","error help not translated"); DestroyVOC(&sample); return dummy; }

  Config.SoundTPort=210+10*((Config.SoundPort-0x210)/0x10);

  DSPPlaySound(sample);
  for (prcnt=0; DSPSoundCheck(); prcnt++) ProcessB(prcnt/5);
  DeInitDSPSound();
  DestroyVOC(&sample);

  ConfirmedCall("Sound okay?",0,&ActivateDSPMethod,0,NULL);
  return dummy;
  }

int InitTestCTVMethod(int dummy)
  {
  BYTE *sample,errcode;
  DWRD prcnt;

  if (SEqual(Config.SoundPath,"NONE"))
    { Message("You need to specify path and filename|of the CT-VOICE.DRV driver that|came with your sound blaster card."); return dummy; }

  Config.Sound=0;
  Config.SoundIRQ=Config.SoundTIRQ;
  Config.SoundPort=0x210+0x10*((Config.SoundTPort-210)/10);

  InitProcessB("CT-VOICE Sound test...");

  if (LoadVOC("C3RSOUND.GRP|CLSM00.VOC",&sample))
    { Message("Error while loading|sound file C3RSOUND.GRP"); return dummy; }

  errcode=InitCTVSound(Config.SoundPath,Config.SoundPort,Config.SoundIRQ);

  if (errcode)
    {
    sprintf(OSTR,"€%c%s€%c|%s",CDRed,Config.SoundPath,CGray1,CTVErrMsg(errcode));
    Message(OSTR);
    DestroyVOC(&sample);
    return dummy;
    }

  CTVPlaySound(sample);
  for (prcnt=0; CTVSoundCheck(); prcnt++) ProcessB(prcnt/5);
  DeInitCTVSound();
  DestroyVOC(&sample);

  ConfirmedCall("Sound okay?",0,&ActivateCTVMethod,0,NULL);
  return dummy;
  }

int SoundSetup(int dummy)
  {
  SoundSetWin=NewWindow("Sound Setup",STANDARD,1, -1,-1,250,130, 2,4, 0,"Contents");

  NewObject("SoundPic",PICTURE,0,0, 200,15,17,16, MenuPage,0,0, 208,84,0,0,0, NULL,NULL);

  TextObject("Sound effects:",100,20,CGray1);
  ITextObject(SoundStatusName,&Config.Sound,100,28,CDRed);

  CTextObject("Initialize and test sound:",125,83,CGray1,-1,1);

  NewULink(EXCREDR,NULL,
    NewObject("Using CT-VOICE Method",BUTTON,1,0, 50,103,150,9, 0,1,0, 0,0,0,0,0, NULL,NULL),
      NULL,&InitTestCTVMethod,0);

  NewULink(EXCREDR,NULL,
    NewObject("Using DSP/Timer Method",BUTTON,1,0, 50,91,150,9, 0,1,0, 0,0,0,0,0, NULL,NULL),
      NULL,&InitTestDSPMethod,0);

  NewObject("Path to driver file",TEXTBOX,1,0, 20,60,204,9, 1,0,0, 50,SLen(Config.SoundPath),0,0,0, Config.SoundPath,NULL);
  CTextObject("CT-VOICE.DRV",100,54,CDRed);

//  NewButton("Help",0, 184,40,40,9, XRVHELP,0);

  NewObject("IRQ"         ,VALBOXB,1,0, 20,40,20,9, 1,0,0,   2,7,1,0,0, NULL,&Config.SoundTIRQ);
  NewObject("Port Address",VALBOXB,1,0, 20,20,20,9, 1,'h',0, 210,260,10,0,0, NULL,&Config.SoundTPort);

  NewULink(EXCREDR,NULL,
    NewObject("No Sound",BUTTON,1,0, 50,115,150,9, 0,1,0, 0,0,0,0,0, NULL,NULL),
      NULL,&DeactivateSound,0);

  if (InitErrorCheck()) CloseWindow(SoundSetWin);
  return dummy;
  }

//----------------------- Registration Handling & Menus ----------------------

/*BYTE Registered(void)
  {
  if (Config.RegVal==4111784121L) return 1;
  return 0;
  }

void ScrambleString(BYTE *cstr, BYTE sval)
  {
  for (cstr; *cstr; cstr++) *cstr=*cstr ^ sval;
  }

void ScrambleConfigRegName(void)
  {
  ScrambleString(Config.RegName,238);
  }

BYTE RRegName[31],RRegKey[31];

int CheckRegistration(int dummy)
  {
  DWRD rregval;
  if (RRegName[0] && RRegKey[0])
    {
    Capitalize(RRegName);
    rregval=strtoul(RRegKey,NULL,10);
    if (rregval==bp(RRegName,56442))
      {
      SCopy(RRegName,Config.RegName);
      Config.RegVal=4111784121L;          // ->msg how to unreg
      Message("Thank you very much for registering!|This copy of clonk is now registered to your name|and must not be distributed any further.");
      }
    else
      Message("Nice try. Player name and code do not match.");
    }
  return dummy;
  }

int Registration(int dummy)
  {
  NewWindow("CLONK 3 Radikal Registration",STANDARD,1, -1,-1,150,70, 1,0, XRVCLOSE, "Registration");

  RRegName[0]=0; RRegKey[0]=0;

//  NewButton("Help",0,   102,57,40,9, XRVHELP,1);
  NewButton("Cancel",0,  57,57,40,9, XRVCLOSE,1);
  NewULink(EXCREDR,NULL,
    NewButton("OK",0,       12,57,40,9, XRVCLOSE,1),
      NULL,&CheckRegistration,0);

  NewObject("Personal Code:",TEXTBOX,1,0, 15,40,123,9, 1,XRVTAB_O,0, 30,0,0,0,0, RRegKey,NULL);
  NewObject("Player Name:",     TEXTBOX,1,0, 15,20,123,9, 1,XRVTAB_O,0, 30,0,0,0,0, RRegName,NULL);

  return dummy;
  }*/

//-------------------------- LoadSave Config & UP ------------------------------

BYTE PlayerDouble(PLAYERINFO *plr)
  {
  PLAYERINFO *cplr;
  int cnt=0;
  for (cplr=FirstPlayer; cplr; cplr=cplr->next)
    if (SEqualL(cplr->name,plr->name)) cnt++;
  if (cnt>=2) return 1;
  return 0;
  }

extern void DeleteCrewList(MANINFO **crew);

void DeletePlayer(PLAYERINFO *plr)
  {
  PLAYERINFO *cplr,*prev=NULL;
  if (plr)
    {
    for (cplr=FirstPlayer; cplr; cplr=cplr->next)
      if (cplr->next==plr) prev=cplr;
    if (prev) prev->next=plr->next;
    else FirstPlayer=plr->next;
    DeleteCrewList(&plr->crew);
    delete plr;
    }
  }

BYTE LoadCfgSysProcess(char *fname, BYTE pimp)
  {
  int fver,plrcnt,crwcnt,rnkcnt;
  PLAYERINFO *cplr;
  MANINFO *cman;
  PLAYERRANK crnk;
  int cnt,cmove; BYTE *cptr;

  // Open File
  if (InitBFI(fname)) return 10;

  // Read DAT-Header "CLONK 3.X DAT-File"
  if (GetBFI(OSTR,18)) return 12;
  fver=OSTR[8]-'0'; OSTR[8]='x';
  if (!SEqual(OSTR,"CLONK 3.x DAT-File") || !Inside(fver,0,5)) return 11;

  if (pimp) // Read over cfg
    {
    cmove=sizeof(CONFIG)-110*(fver==0); // Config
    cmove+=sizeof(USERPREF)-50*(fver==0); // UserPref
    cmove+=sizeof(BSATYPE)-(100*(fver<1)+MaxGamePlr*50*(fver<2)); // BSA
    for (cnt=0; cnt<cmove; cnt++) if (GetBFI(OSTR)) return 12;
    }
  else // Real cfg load
    {

    // Read Config
    if (GetBFI(&Config,sizeof(CONFIG)-110*(fver==0))) return 12;
    if (fver<1) // RStCnt and REnCnt have been moved 10 bytes due to RegName extens.
      MemCopy(((BYTE*)&Config)+341,((BYTE*)&Config)+351,8);
    if (fver<2) // Swap KCom 1 and 2
      for (cnt=0; cnt<10; cnt++)
	{ cmove=Config.KCom[1][cnt]; Config.KCom[1][cnt]=Config.KCom[2][cnt]; Config.KCom[2][cnt]=cmove; }
    //ScrambleConfigRegName();

    // Read UserPref
    if (GetBFI(&UserPref,sizeof(USERPREF)-50*(fver==0))) return 12;
    if (fver<2)
      {
      UserPref.TwoButtonJump=(MouseType<3);
      }

    // Read BSA
    cmove=100*(fver<1)+MaxGamePlr*50*(fver<2); // To be not loaded
    if (GetBFI(&BSA,sizeof(BSATYPE)-cmove)) return 12;
    if (fver<2)
      {
      // Adjust futurebuf/vars added into BSAPLRTYPE
      cptr=((BYTE*)&BSA)+22; cmove=sizeof(BSATYPE)-22;
      for (cnt=0; cnt<MaxGamePlr; cnt++)
	{
	MemCopy(cptr+39,cptr+39+50,cmove-50);
	cptr+=sizeof(BSAPLRTYPE);
	cmove-=sizeof(BSAPLRTYPE);
	BSA.Plr[cnt].GPos=0;
	}
      // New variables
      BSA.CometHail=0;
      }
    ClearBSAPlrPtrs();

    }

  // Read Players
  if (GetBFI(&plrcnt,sizeof(int))) return 12;

  for (plrcnt; plrcnt>0; plrcnt--)
    {
    if (!(cplr=NewDefaultPlr2List())) return 13;
    if (GetBFI(cplr,sizeof(PLAYERINFO)-sizeof(PLAYERINFO*)-sizeof(MANINFO*)-50*(fver==0))) return 12;
    if (fver<1) // New variables added into futurebuf
      cplr->pfgpos=0;
    if (fver<2)
      cplr->autorean=0;

    if (GetBFI(&crwcnt,sizeof(int))) return 12;

    for (crwcnt; crwcnt>0; crwcnt--)
      {
      if (!(cman=NewDefaultMan2List(&cplr->crew))) return 13;
      if (GetBFI(cman,sizeof(MANINFO)-sizeof(MANINFO*)-10*(fver==0))) return 12;
      }

    if (pimp)
      {
      if (cplr->face>=DefFaceNum) cplr->face=0;
      if (PlayerDouble(cplr)) { DeletePlayer(cplr); cplr=NULL; }
      }

    if (cplr) SortCrewList(cplr->crew);
    }

  // Read Ranks
  if (GetBFI(&rnkcnt,sizeof(int))) return 12;

  if (pimp)
    {		// Other dat has more rank names, clear & load those
    if (rnkcnt>PlrRankCount()) ClearPlrRanks();
    else { DeInitBFI(); return fver; } // Else, avoid rank load
    }

  for (rnkcnt; rnkcnt>0; rnkcnt--)
    {
    if (GetBFI(&crnk,sizeof(PLAYERRANK)-sizeof(PLAYERRANK*))) return 12;
    if (!NewPlrRank(crnk.name)) return 13;
    }

  DeInitBFI();

  return fver;
  }

char *LoadCfgStatus(BYTE code)
  {
  char *rval;
  switch (code)
    {
    case  0: rval="3.0 import"; break;
    case  1: rval="3.1 import"; break;
    case  2: rval="3.2 import"; break;
    case  3: rval="3.3 import"; break;
    case  4: rval="3.4 import"; break;
    case  5: rval="version 3.5"; break;
    case 10: rval="file not found"; break;
    case 11: rval="incorrect dat"; break;
    case 12: rval="file error"; break;
    case 13: rval="insufficient memory"; break;
    default: rval="undefined error"; break;
    }
  return rval;
  }

BYTE LoadConfigSystem(void)
  {
  BYTE lval;
  lval=LoadCfgSysProcess("CLONK.DAT",0);
  sprintf(OSTR,"%s... ",LoadCfgStatus(lval)); InitMsgOpen(OSTR);
  if (Inside(lval,0,9))
    { SortPlayerList(0); return 1; }
  else
    { DeInitPlayerList(); ClearPlrRanks(); }
  return 0;
  }

extern char DatImportName[46];
#define LID_PLRS 1

int PlayerImport(int dummy)
  {
  BYTE lval;
  lval=LoadCfgSysProcess(DatImportName,1);
  if (Inside(lval,0,9))
    {
    SortPlayerList(0);
    AdjustLBCellNum(LID_PLRS,PlrListCount());
    }
  else
    {
    sprintf(OSTR,"Error importing|%s:|%s",DatImportName,LoadCfgStatus(lval));
    Message(OSTR);
    }
  return dummy;
  }

BYTE SaveCfgSysProcess(void)
  {
  int plrcnt,crwcnt,rnkcnt;
  PLAYERINFO *cplr;
  MANINFO *cman;
  PLAYERRANK *crnk;

  if (PathProtected(RunningPath))
    { Message("Error while saving|config file CLONK.DAT:|Drive write protected"); return 0; }

  if (InitBFO("CLONK.DAT")) return 0;

  if (PutBFO("CLONK 3.5 DAT-File",18)) return 0;

  //ScrambleConfigRegName();

  if (PutBFO(&Config,sizeof(CONFIG))) return 0;
  if (PutBFO(&UserPref,sizeof(USERPREF))) return 0;
  if (PutBFO(&BSA,sizeof(BSATYPE))) return 0;

  //ScrambleConfigRegName();

  plrcnt=PlrListCount();
  if (PutBFO(&plrcnt,sizeof(int))) return 0;

  for (cplr=FirstPlayer; cplr; cplr=cplr->next)
    {
    if (PutBFO(cplr,sizeof(PLAYERINFO)-sizeof(PLAYERINFO*)-sizeof(MANINFO*))) return 0;
    crwcnt=CrewListCount(cplr);
    if (PutBFO(&crwcnt,sizeof(int))) return 0;
    for (cman=cplr->crew; cman; cman=cman->next)
      if (PutBFO(cman,sizeof(MANINFO)-sizeof(MANINFO*))) return 0;
    }

  rnkcnt=PlrRankCount();
  if (PutBFO(&rnkcnt,sizeof(int))) return 0;

  for (crnk=FirstPlrRank; crnk; crnk=crnk->next)
    if (PutBFO(crnk,sizeof(PLAYERRANK)-sizeof(PLAYERRANK*))) return 0;

  DeInitBFO();

  return 1;
  }

BYTE SaveConfigSystem(void)
  {
  if (!SaveCfgSysProcess())
    { Message("Error while saving|config file CLONK.DAT"); return 0; }
  return 1;
  }

void DefaultKComs(void)
  {
  char kcom[3][10]={'q','w','e','a','s','d','z','x','c','v',
		    'i','o','p','k','l',';',',','.','?','m',
		    '7','8','9','4','5','6','1','2','3','0'};
  MemCopy(kcom[0],Config.KCom[0],3*10);
  }

void DefaultConfig(void)
  {
  Config.FaceNum=DefFaceNum; Config.FaceFile=0; Config.ImportMode=0;
  Config.SortPlrBy=0; Config.ShowQuotes=0;
  Config.GameSpeed=7;
  Config.SaveDatAlways=0;
  Config.LangType=0;

  Config.HiResTitle=0;

  DefaultPlrRanks();

  SBDetected=0;
  Config.Sound=0;
  if (AutodetectBlaster(&Config.SoundPort,&Config.SoundIRQ,Config.SoundPath))
    {
    InitMsg("SoundBlaster Autodetected");
    SBDetected=1;
    }
  Config.SoundTIRQ=Config.SoundIRQ;
  Config.SoundTPort=210+10*((Config.SoundPort-0x210)/0x10);
  Config.SoundDynamic=0;

  DefaultKComs();

  Config.RegVal=0L;
  SCopy("NONE",Config.RegName);

  Config.RoundStartCount=Config.RoundEndCount=0;
  }

void DefaultUserPref(void)
  {
  UserPref.SmokeOn=1;
  UserPref.GameMsgOn=1;
  UserPref.LineBeforeStrc=0;
  UserPref.PlrPosByCon=1;
  UserPref.CeaseAction=1;
  UserPref.DoubleDigSpeed=30;
  UserPref.NoQuotes=0;
  }

//--------------------------- Keyboard Command Setting ------------------------

BYTE KeyCommandOkay(BYTE inc)
  {
  if (Inside(inc,33,126)) return 1;
  if (Inside(inc,129,154)) return 1;
  return 0;
  }

int KComUsed(char kcom)
  {
  int cnt,cnt2;
  for (cnt=0; cnt<3; cnt++)
    for (cnt2=0; cnt2<10; cnt2++)
      if (Config.KCom[cnt][cnt2]==kcom)
	return cnt*10+cnt2;
  return -1;
  }

void ReplaceEqualKCom(char kcom)
  {
  int cnt,cnt2;
  char nkcom;
  for (cnt=0; cnt<3; cnt++)
    for (cnt2=0; cnt2<10; cnt2++)
      if (Config.KCom[cnt][cnt2]==kcom)
	{
	for (nkcom=97; (KComUsed(nkcom)>-1); nkcom++);
	if (nkcom>122) for (nkcom=48; (KComUsed(nkcom)>-1); nkcom++);
	Config.KCom[cnt][cnt2]=nkcom;
	}
  }

int EditKeyboardControls(int inrnd);

int SetKeyboardControl(int conid)
  {
  WINDOW *cwin;
  int tplr,tcon,usedby;
  BYTE inc,exok=0;
  tplr=conid/10; tcon=conid%10;
  cwin=NewWindow("EditKeyCon",STANDARD,1, -1,-1,200,45, 0,0, 0,NULL);
  NewObject("IndexFrame",FRAME,0,0, 5+1+8*(tcon%3),5+6*(tcon/3),9,7, 0,0,0, CYellow,-1,0,0,0, NULL,NULL);
  NewObject("KeyConPic",PICTURE,0,0, 5,5,28,26, InRound?2:MenuPage,0,0, InRound?148:180,InRound?117:74,0,0,0, NULL,NULL);
  TextObject("Please enter a new|keyboard command for:",100,5,CGray2,-1,1);
  TextObject(ConPlrName[tplr],100,20,CGray1,-1,1);
  TextObject(ConConName[tcon],100,27,CGray1,-1,1);
  sprintf(OSTR,"Before: '%c'",Config.KCom[tplr][tcon]);
  TextObject(OSTR,100,36,CGray2,-1,1);
  if (InitErrorCheck()) { CloseWindow(cwin); return 0; }
  DrawWindow(cwin); // For GFX only
  OSTR[0]=0;
  do
    {
    while (!kbhit()) KBLockCheck();
    inc=getch(); if (!inc) getch();
    if (KeyCommandOkay(inc))
      {
      usedby=KComUsed(inc);
      if ((usedby>-1) && (usedby!=conid))
	{
	sprintf(OSTR,"Key '%c' has been in use|for €%c%s: %s€%c|(This command should be redefined)",inc,CDRed,ConPlrName[usedby/10],ConConName[usedby%10],CGray1);
	ReplaceEqualKCom(inc);
	}
      Config.KCom[tplr][tcon]=inc;
      exok=1;
      }
    if (inc==27) exok=1;
    }
  while (!exok);
  CloseWindow(cwin);
  EditKeyboardControls(0);
  if (OSTR[0]) Message(OSTR,NULL);
  return 0;
  }

int DefaultSetKComs(int dummy)
  {
  DefaultKComs();
  EditKeyboardControls(0);
  Message("Keyboard commands|set back to default");
  return dummy;
  }

int EditKeyboardControls(int dummy)
  {
  char ostr2[10];
  int cnt,cnt2,btx,bty;
  WINDOW *cwin;
  cwin=NewWindow("Redefine keyboard",STANDARD,1, -1,-1,280,90, 1,0, XRVCLOSE,"Contents");
  NewObject("KeyConPic",PICTURE,0,0, 214,3,28,26, InRound?2:MenuPage,0,0, InRound?148:180,InRound?117:74,0,0,0, NULL,NULL);
  TextObject("Left|player",45+48*0+21,16,CGray1,-1,1);
  TextObject("Center|player",45+48*1+21,16,CGray1,-1,1);
  TextObject("Right|player",45+48*2+21,16,CGray1,-1,1);
  TextObject("Select",42,32,CRed,-1,2);
  TextObject("Control",42,42,CDGreen,-1,2);
  TextObject("Control",42,52,CDGreen,-1,2);
  TextObject("Left   Toggle   Right ",186,32,CRed);
  TextObject("Throw  Jump     Dig   ",186,42,CDGreen);
  TextObject("Left   Stop     Right ",186,52,CDGreen);
  TextObject("Menu",186,62,CDBlue);
  for (cnt=2; cnt>=0; cnt--)
    for (cnt2=9; cnt2>=0; cnt2--)
      {
      btx=45+48*cnt+14*(cnt2%3); bty=30+10*(cnt2/3);
      sprintf(ostr2,"%c",Config.KCom[cnt][cnt2]);
      NewULink(CLWINEXC,cwin,
	NewButton(ostr2,0, btx,bty,13+6*(cnt2==9),9, 0,0),
	  NULL,&SetKeyboardControl,10*cnt+cnt2);
      }
  NewULink(CLWINEXC,cwin,
    NewButton("Default",0, 200,77,75,9, 0,1),
      NULL,&DefaultSetKComs,0);
  NewButton("OK",0, 115,77,50,9, XRVCLOSE,0);

  if (InitErrorCheck()) CloseWindow(cwin);

  return dummy;
  }

