/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RW\D CLONK Weather & Ground Control System Module

#include <stdlib.h>
#include <math.h>

#include <standard.h>
#include <vga4.h>

#include "clonk_bs.h"

#include "bsexcsys.h"
#include "bsmansys.h"
#include "bsgfxsys.h"
#include "bspxssys.h"
#include "bsstrsys.h"

extern BYTE GameSoundPlaying();
extern void GameSetDAC(int fpos, int num, void *buf);

//------------------------- Main Global Externals ----------------------------

extern BSATYPE BSA;
extern void EventCall(int evnum);

//-------------------- WEA Global Game Object Variables ------------------------

WEATHERTYPE Weather;
GROUNDTYPE Ground;

int CometSPhase,CometSTarget,CometSCsdBy;
int QuakeTarget;

//-------------------- (Tree Control -> somewhere else?) --------------------

BYTE PlantTree(BYTE allgrown)
  {
  int cnt,tx,ty,type;
  BYTE xfreestr;
  STRUCTYPE *cstrc;
  type=0; if (Weather.climate>200) { type=1; if (random(800)<Weather.climate-200) type=2; }
  for (cnt=0; cnt<5; cnt++) // Five tries if necessary
    {
    tx=random(76)*4;

    xfreestr=1;
    for (cstrc=FirstStruct; cstrc; cstrc=cstrc->next)
      if (cstrc->type>=STCACTUS) // Other tree no-double
	{ if (cstrc->x==tx) xfreestr=0; }
      else // No tree before building
	{ if (Inside(tx-cstrc->x,-16,+20)) xfreestr=0; }

    if (xfreestr)
      {
      ty=0; while (!Inside(GBackPix(tx+7,ty+16),CGroundL,CGroundH) && !Inside(GBackPix(tx+7,ty+16),CSandS,CSandT)) ty++;
      if (!Inside(GBackPix(tx+7,ty+16-7),CSolid2L,CSolid2H))
	if (Inside(GBackPix(tx+7,ty+16+4),CGroundL,CGroundH))
	  if (!NewStruct(STCACTUS+type,tx,ty,0,1000*allgrown,-1)) return 0;
	  else return 1;
      }
    }

  return 0;
  }

void NewPlant(void)
  {
  int tx,ty;
  tx=random(300)+10-2; ty=random(BackGrSize-20);
  while ((ty<BackGrSize-10) && !(Inside(GBackPix(tx,ty+4),CEarth1,CAshes2) && !Inside(GBackPix(tx,ty+1),CGranite1,CGold2)) ) ty++;
  if (ty<BackGrSize-10)
    NewRock(tx,ty,RADEAD,PLANT1,0,0,0,-1);
  }

//--------------------------- (Weather Control) ------------------------------

BYTE LaunchLightN(int type, int fx, int fy, int phase2)
  {
  int cnt;
  for (cnt=0; cnt<MaxLightN; cnt++)
    if (Weather.LightN[cnt].type==-1)
      {
      Weather.LightN[cnt].type=type;
      Weather.LightN[cnt].stat=0;
      Weather.LightN[cnt].phase=0;
      Weather.LightN[cnt].phase2=phase2;
      Weather.LightN[cnt].lx[0]=fx;
      Weather.LightN[cnt].ly[0]=fy;
      return 1;
      }
  return 0;
  }

void TakeOutBackstrokes(LIGHTNING *clgt)
  {
  int cnt,cnt2,jout;
  for (cnt=1; cnt<clgt->phase; cnt++)
    {
    for (jout=1; cnt+jout<clgt->phase+1; jout++)
      if (clgt->lx[cnt]==clgt->lx[cnt+jout])
	if (clgt->ly[cnt]==clgt->ly[cnt+jout])
	  break;
    if (cnt+jout<clgt->phase+1)
      {
      for (cnt2=cnt; cnt2<clgt->phase+1-jout; cnt2++)
	{
	clgt->lx[cnt2]=clgt->lx[cnt2+jout];
	clgt->ly[cnt2]=clgt->ly[cnt2+jout];
	}
      clgt->phase-=jout;
      }
    }
  }

void CheckLightNConnection(LIGHTNING *clgt)
  {
  const int crange=30;

  STRUCTYPE *cstrc;
  int lphs=clgt->phase;

  for (cstrc=FirstStruct; cstrc; cstrc=cstrc->next)
    if ((cstrc->type==STMAGIC) && (cstrc->con>=1000))
      if (Inside(clgt->lx[lphs]-(cstrc->x+7),-crange,+crange))
	if (Inside(clgt->ly[lphs]-(cstrc->y-9),-crange,+crange))
	  {
	  clgt->stat=1;
	  clgt->lx[lphs]=cstrc->x+7;
	  clgt->ly[lphs]=cstrc->y-9;
	  clgt->phase2=0;
	  TakeOutBackstrokes(clgt);
	  cstrc->energy=BoundBy(cstrc->energy+300,0,10000);

	  if (Crew[cstrc->owner].CMenu==CMMAGIC) Crew[cstrc->owner].RedrStB=1;

	  return;
	  }
  }

extern void BlowPXS(int amt, int type, int phase, int fx, int fy, int lvl);

void MoveLightN(void)
  {
  int cnt,cnt2,bstroke,lphs;
  int acx,acy,aca,acr;
  LIGHTNING *lptr;

  for (cnt=0; cnt<MaxLightN; cnt++)
    if (Weather.LightN[cnt].type>-1)
      {
      lptr=&(Weather.LightN[cnt]);
      lphs=lptr->phase;

      if (lptr->stat==0) // Regular movement ---------------------------------
	switch (lptr->type)
	  {
	  case 0: // Thunderstorm lightning - - - - - - - - - - - - - - - - - - -
	    // Struck ground
	    if (Inside(GBackPix(lptr->lx[lphs],lptr->ly[lphs]),CSolidL,CSolid2H))
	      {
	      while (Inside(GBackPix(lptr->lx[lphs],lptr->ly[lphs]),CSolidL,CSolid2H)) lptr->ly[lphs]--;
	      CheckManHit(lptr->lx[lphs],lptr->ly[lphs],10);
	      StructHitDamage(lptr->lx[lphs],lptr->ly[lphs]-5,5,-1);
	      lptr->type=-1;
	      }
	    // Else movement
	    else
	      {
	      if (!random(8) && (lphs>=2)) // Backstroke
		{
		lphs++;
		bstroke=2+BoundBy(random(lphs-2),0,3);
		for (cnt2=1; (cnt2<=bstroke) && (lptr->phase<LightNLength-1) && (lphs-cnt2>0); cnt2++)
		  {
		  lptr->phase++;
		  lptr->lx[lptr->phase]=lptr->lx[lphs-cnt2];
		  lptr->ly[lptr->phase]=lptr->ly[lphs-cnt2];
		  }
		lphs=lptr->phase;
		}
	      else // Regular advance
		{
		lptr->phase++; lphs=lptr->phase;
		lptr->lx[lphs]=BoundBy(lptr->lx[lphs-1]+random(40)-19,0,319);
		lptr->ly[lphs]=BoundBy(lptr->ly[lphs-1]+random(5)+5,0,BackGrSize-5);
		}
	      // Lightning gets lost (too long)
	      if (lphs>=LightNLength-1) lptr->type=-1;
	      // Check for connection
	      if (lptr->type>-1) if (lphs>4)
		CheckLightNConnection(lptr);
	      }
	    break;
	  case 1: // Magic lightning - - - - - - - - - - - - - - - - - - - - -
	    // Struck ground (connect)
	    if (Inside(GBackPix(lptr->lx[lphs],lptr->ly[lphs]),CSolidL,CSolid2H))
	      {
	      while (Inside(GBackPix(lptr->lx[lphs],lptr->ly[lphs]),CSolidL,CSolid2H)) lptr->ly[lphs]--;
	      CheckManHit(lptr->lx[lphs],lptr->ly[lphs],10);
	      StructHitDamage(lptr->lx[lphs],lptr->ly[lphs]-5,5,-1);
	      lptr->stat=1; lptr->phase2=0;
	      }
	    // Else movement
	    else
	      {
	      // Calculate arc
	      acr=Max(Abs(lptr->lx[0]-lptr->phase2)/2,50);
	      acx=lptr->lx[0]-acr*Sign(lptr->lx[0]-lptr->phase2);
	      acy=lptr->ly[0];
	      if (lptr->lx[0]>lptr->phase2) aca=90-(180*lptr->phase/20);
	      else aca=-90+(180*lptr->phase/20);
	      // Advance lightning
	      lptr->phase++; lphs=lptr->phase;
	      lptr->lx[lphs]=BoundBy(acx+sin(M_PI*aca/180.0)*acr+random(10)-5,0,319);
	      lptr->ly[lphs]=acy-cos(M_PI*aca/180.0)*(acr/2)+random(10)-5;
	      // If arc completed, move like regular lightning
	      if (lphs>25) lptr->type=0;
	      // Lightning gets lost (too long)
	      if (lphs>=LightNLength-1) lptr->type=-1;
	      // Check for connection
	      //if (lptr->type>-1) if (lphs>10)
	      //	CheckLightNConnection(lptr);
	      }
	    break;
	  }
      else // Connected movement -----------------------------------------------
	{
	lptr->phase2++;
	for (cnt2=1; cnt2<lphs; cnt2++)
	  {
	  lptr->lx[cnt2]+=random(3)-1;
	  lptr->ly[cnt2]+=random(3)-1;
	  }
	if (!Tick5)
	  {
	  GSTP=SNDBRZZ;
	  BlowPXS(1,PXSSPARK,0,lptr->lx[lphs],lptr->ly[lphs],4);
	  PXSOut(PXSSMOKE,lptr->lx[lphs],lptr->ly[lphs],5+random(10));
	  }

	// Connected phase over
	if (lptr->phase2>40) lptr->type=-1;
	}
      }
  }

void Explode(int exx, int exy, int exrad, int causedby)
  {
  int cnt;
  BlastFree(exx,exy,exrad);
  for (cnt=0; cnt<exrad*3; cnt++) PXSOut(PXSSMOKE,exx+random(20)-10,exy+random(20)-10,random(30));
  CheckManHit(exx,exy,exrad);
  StructHitDamage(exx,exy,exrad,causedby);
  Weather.expln.act=2+exrad/2; Weather.expln.x=exx; Weather.expln.y=exy;
  GSTP=SNDEXPLODE;
  EventCall(200);
  }

void LaunchComet(int tx, int csdby)
  {
  if (tx>-1)
    NewRock(tx+random(60)-30,0,RAFLY,COMET,random(50)-25,50,0,csdby);
  else
    NewRock(random(240)+40,0,RAFLY,COMET,random(150)-74,50,0,csdby);
  }

void LaunchCometS(int tx, int csdby)
  {
  CometSPhase=5;
  CometSTarget=tx; CometSCsdBy=csdby;
  }

void LaunchFreeze(void)
  {
  Weather.mflvl=-1;
  Weather.mfact=150;
  }

void PrepSkyColor(BYTE type)
  {
  COLOR tpal[32];
  switch (type)
    {
    case 0: // Green
      SetColor(tpal,0,8,15,8); SetColor(tpal,31,26,63,30);
      break;
    case 1: // Dark Blue
      SetColor(tpal,0,0,0,3); SetColor(tpal,31,0,0,25);
      break;
    }
  FadeColor(tpal,0,31); SetDAC(CSky1,32,tpal); // Pre-Game (No Sound)
  }

void InitWeather(int clim, int seas)
  {
  int cnt;
  Weather.climate=clim; Weather.season=seas;
  Weather.wind=Weather.wind2=0;
  Weather.stdtemp=50-100*Weather.climate/1000;
  if (Weather.season<1000) Weather.stdtemp+=Weather.season/20-25;
  else Weather.stdtemp-=Weather.season/20-75;
  Weather.temp=BoundBy(Weather.stdtemp+random(31)-15,-50,50);
  Weather.lsttmp=Weather.temp; Weather.tchcnt=0;
  Weather.rfall=BoundBy(random(101)-50,-50,50*(Weather.climate>200));
  // Init Lightning
  Weather.thlprob=0;
  for (cnt=0; cnt<MaxLightN; cnt++) Weather.LightN[cnt].type=-1;
  // Init  Melt/Freeze
  Weather.mflvl=Weather.stdtemp/10;
  Weather.mfact=-1;
  // Init Explosion
  Weather.expln.act=-1;
  // Init Environment
  switch (BSA.WEnvr)
    {
    case 0: /*NORMAL*/ break;
    case 1: // Polluted
      PrepSkyColor(0);
      break;
    case 2: // Night
      PrepSkyColor(1);
      break;
    }
  // Init CometHail
  CometSPhase=CometSTarget=CometSCsdBy=0;
  }

void WeatherControl(void) // Every Tick10 or Sec5
  {
  int wbf;
  // Season Control (none if yspeed is zero)
  if (!Tick50) if (BSA.WYSpd>10)
    {
    Weather.season+=BSA.WYSpd/10;
    if (Weather.season>1999) Weather.season=0;
    }
  // Temperature Control
  if (!Tick50)
    {          // Renew stdtemp
    Weather.stdtemp=50-100*Weather.climate/1000;
    if (Weather.season<1000) Weather.stdtemp+=Weather.season/20-25;
    else Weather.stdtemp-=Weather.season/20-75;
    }
  Weather.temp=BoundBy(BoundBy(Weather.temp+random(7)-3,Weather.stdtemp-15,Weather.stdtemp+15),-50,50);
  // Wind Control
  wbf=1; if (Inside(Weather.season,1000,1500)) wbf=2; // WindBoundaryFactor (Fall->Wind)
  Weather.wind=BoundBy(Weather.wind+random(9)-4,-30*wbf,30*wbf);
  Weather.wind2=BoundBy(Weather.wind2+Sign(Weather.wind-Weather.wind2),-20*wbf,20*wbf);
  // RFall Control
  Weather.rfall=BoundBy(Weather.rfall+random(15)-7,-50,50*(Weather.climate>200));
  // LProb Control
  if (Weather.thlprob>0) Weather.thlprob--;
  Weather.tchcnt++;
  if (Weather.tchcnt>10) // 100 frames
    { // rapid temp change
    if (Abs(Weather.temp-Weather.lsttmp)>10-BSA.WLPMod/10)
      Weather.thlprob=Min(Weather.thlprob+30,100);
    Weather.lsttmp=Weather.temp; Weather.tchcnt=0;
    }
  // Lightning launch control
  if (!Tick50)
    if (random(100)<Weather.thlprob+BSA.WLPMod) // New Flash
      { LaunchLightN(0,random(300)+10,-50,0); if (!random(2)) GSTP=SNDTHUNDER; }
  // Comet launch control
  if (Sec60)
    if (random(100)<BSA.WCmtLvl)
      LaunchComet(-1,-1);
  // Melt/Freeze launch control
  if (Sec20)
    if (Weather.mfact==-1)
      {
      Weather.mflvl=Weather.stdtemp/10;
      Weather.mfact=150;
      }
  // New tree growth in spring
  if (!Tick50)
    if (Inside(Weather.season,0,500))
      if ((Weather.season-BSA.WYSpd/10)/50!=Weather.season/50)
	if (!random(5))
	  PlantTree(0);
  // New plant control
  if (Sec20) if (BSA.RuleSet>R_EASY) if (BSA.SMode>S_MISSION)
    if (!random(3)) NewPlant();
  // CometHail Movement
  if (CometSPhase>0) if (!Tick20)
    {
    LaunchComet(CometSTarget,CometSCsdBy);
    CometSPhase--;
    }
  }

void MoveMeltFreeze(void)
  {
  int cnt,cline;
  BYTE freeze,cbpix;
  freeze=0; if (Weather.mflvl<=-1) freeze=1;
  if (freeze && Tick3) return;
  if (!freeze && Tick10) return;
  cline=150-Weather.mfact;
  for (cnt=0; cnt<320; cnt++)
    {
    cbpix=GBackPix(cnt,cline);
    if (freeze)
      {
      if (Inside(cbpix,CWaterS,CWaterT))
	SBackPix(cnt,cline,CSnowT-IsSkyBack(cbpix));
      }
    else
      if (Inside(cbpix,CSnowS,CSnowT))
	{
	SBackPix(cnt,cline,CWaterT-IsSkyBack(cbpix));
	ReleasePXS(cnt,cline);
	}
    }
  Weather.mfact--;
  }

void ExecWeather(void) // Every Tick1
  {
  BYTE rftype;
  // Weather control
  if (!Tick10 || Sec5) WeatherControl();
  // Rainfall
  if (Weather.rfall+BSA.WRFMod>0)
    if ((Weather.rfall+BSA.WRFMod>10) || !Rnd3())
      {
      rftype=PXSWATER; if (Weather.temp<-10) rftype=PXSSNOW;
      if (BSA.WEnvr==1) rftype=PXSACID;
      PXSOut(rftype,random(320),0,0);
      }
  // Lightning movement
  MoveLightN();
  // Explosion movement
  if (Weather.expln.act>-1) Weather.expln.act--;
  // Melt/Freeze movement
  if (Weather.mfact>-1) MoveMeltFreeze();

  // Special
  if (!Tick3) if (BSA.CometHail) if (!random(40)) LaunchComet(-1,-1);
  }

//---------------------------- (Ground Control) ------------------------------

void InitGround(void)
  {
  Ground.gconwait=0;
  // Init volcano
  Ground.volcano.act=0;
  Ground.volcano.glowtick=0;
  GetDAC(CLavaS1,6,&Ground.volcano.lfcol[0][0]);
  GetDAC(CAshes1,3,&Ground.volcano.ltcol[0][0]);
  GetDAC(CAshes1,3,&Ground.volcano.ltcol[3][0]);
  // Init Earthquake
  Ground.equake=0;
  QuakeTarget=-1;
  // Init GrLvl
  Ground.levell=Ground.levelr=0;
  // Matter Accounting
  Ground.MaAcRun=0;
  Ground.MaAcGold=Ground.MaAcOil=0;
  }

void LaunchMatterAccount(void)
  {
  Ground.MaAcRun=0;
  Ground.MaAcGold=Ground.MaAcOil=0;
  }

void MoveMatterAccount(void) // Every Tick1 if running
  {
  int cnt;
  BYTE bpix;

  if (Ground.MaAcRun>=BackGrSize-5) { Ground.MaAcRun=-1; return; }

  for (cnt=0; cnt<320; cnt+=2)
    {
    bpix=GBackPix(cnt,Ground.MaAcRun);
    if (Inside(bpix,CGold1,CGold2)) Ground.MaAcGold++;
    //if (Inside(bpix,COilS,COilT)) Ground.MaAcOil++;
    }

  Ground.MaAcRun+=2;
  }

void LaunchEQuake(int target)
  {
  Ground.equake=300+random(300);
  QuakeTarget=target;
  }

void EQuakeCollapse(void)
  {
  int cnt,cx,cy;
  cx=random(320); if (QuakeTarget>-1) cx=BoundBy(QuakeTarget+random(60)-30,0,319);
  cy=random(BackGrSize-30);
  for (cnt=random(15)+50*(!random(5)); cnt>0; cnt--)
    {
    while (!Inside(GBackPix(cx,cy),CGroundL,CGroundH)) { cy++; if (cy>BackGrSize-10) return; }
    while (!Inside(GBackPix(cx,cy+1),CSky1,CTunnel2)) { cy++; if (cy>BackGrSize-10) return; }
    if (Inside(GBackPix(cx,cy),CEarth1,CAshes2))
      { DrawBackPix(cx,cy); PXSOut(PXSSAND,cx,cy,1); ReleasePXS(cx,cy-1); ReleasePXS(cx-1,cy); ReleasePXS(cx+1,cy); }
    if (cnt%2) cx=Min(cx+1,319);
    else cx=BoundBy(cx+random(11)-5,0,319);
    cy-=7;
    }
  }

void MoveEQuake(int *scrto)
  {
  (*scrto)+=random(5)-2;
  Ground.equake--;
  if (!Tick3) if (!random(4)) EQuakeCollapse();
  if (Sec1) if (!random(3)) GSTP=SNDEQUAKE;
  }

void DrawVolcSpot(int tx, int ty, int size)
  {
  int cnt;
  for (cnt=0; cnt<size; cnt++)
    SBackPix(tx-size/2+cnt,ty,CLavaT1+2*random(3));
  }

void Lava2AshesLine(int ly)
  {
  int cnt;
  for (cnt=0; cnt<320; cnt++)
    if (Inside(GBackPix(cnt,ly),CLavaS1,CLavaT3))
      SBackPix(cnt,ly,CAshes1+random(3));
  }

void SetLavaCoolingColor(VOLCANO *volc, int prgs) // 0-100% of fading
  {
  int cnt,cnt2;
  for (cnt=0; cnt<6; cnt++)
    for (cnt2=0; cnt2<3; cnt2++)
      volc->lccol[cnt][cnt2]=volc->lfcol[cnt][cnt2]+((int)volc->ltcol[cnt][cnt2]-(int)volc->lfcol[cnt][cnt2])*prgs/100;
  }

void MoveVolcano(void) // Every Tick1 if active
  {
  VOLCANO *volc=&Ground.volcano;
  int cnt,cnt2,brcnt=0;
  BYTE cbpix;
  BYTE glowbuf[6][3];
  // Aging
  volc->age--;
  // Normal volcano activity
  if (volc->age>=BackGrSize+300)
    {
    for (cnt=0; cnt<VBranchNum; cnt++)
      switch (volc->b[cnt].act)
	{
	case 1: // Branch moving
	  brcnt++;
	  if (Rnd3())
	    {
	    // Move Branch
	    if (!random(20)) volc->b[cnt].dir=random(3)-1;
	    if (volc->b[cnt].x<15)  volc->b[cnt].dir=+1;
	    if (volc->b[cnt].x>304) volc->b[cnt].dir=-1;
	    volc->b[cnt].y--;
	    volc->b[cnt].x=BoundBy(volc->b[cnt].x+volc->b[cnt].dir+Rnd3(),12,307);
	    // border hit turn round dir
	    DrawVolcSpot(volc->b[cnt].x,volc->b[cnt].y,volc->b[cnt].size);
	    // New Branch?
	    if (!random(10)) if (random(20)<volc->b[cnt].size)
	      for (cnt2=0; cnt2<VBranchNum; cnt2++)
		if (!volc->b[cnt2].act)
		  {
		  volc->b[cnt2].act=1;
		  volc->b[cnt2].x=volc->b[cnt].x;
		  volc->b[cnt2].y=volc->b[cnt].y;
		  volc->b[cnt2].dir=0; volc->b[cnt2].size=volc->b[cnt].size/3;
		  volc->b[cnt].size-=2;
		  break;
		  }
	    // Branch Death Check
	    if ((volc->b[cnt].y<0) || (volc->b[cnt].size<1))
	      { volc->b[cnt].act=0; break; } // dead (screen top or size)
	    cbpix=GBackPix(volc->b[cnt].x,volc->b[cnt].y-1);
	    if (cbpix<CSolid2L)
	      { volc->b[cnt].act=2; GSTP=SNDLAVA; break; } // sky free
	    if (Inside(cbpix,CWaterS,COilT))
	      { volc->b[cnt].act=2; break; } // other liquid dead (open?)
	    }
	  break;
	case 2: // Branch open
	  brcnt++;
	  if (!Tick3)
	    {
	    // Free Branch Adjustment
	    while (!Inside(GBackPix(volc->b[cnt].x,volc->b[cnt].y  ),CSolid2L,CSolid2H)) volc->b[cnt].y++;
	    while ( Inside(GBackPix(volc->b[cnt].x,volc->b[cnt].y-1),CLavaS1 ,CLavaT3 )) volc->b[cnt].y--;
	    if (Inside(GBackPix(volc->b[cnt].x,volc->b[cnt].y-1),CSolid2L,CSolidH)) volc->b[cnt].act=0;
	    // Lava Output
	    //PXSOut(PXSLAVA,volc->b[cnt].x+cnt2,volc->b[cnt].y-1,0);
	    BlowPXS(volc->b[cnt].size/(3+Rnd3()),PXSLAVA,0,volc->b[cnt].x+cnt2,volc->b[cnt].y-3,10);
	    }
	  break;
	}
    //if (brcnt==0) volc->age=BackGrSize;
    if (!Tick50) if (!random(10)) GSTP=SNDLAVA;
    }

  // Dying volcano activity
  if (volc->age==BackGrSize) GameSetDAC(CLavaS1,6,&volc->ltcol[0][0]);
  if (volc->age<BackGrSize)
    {
    if (volc->age<0) // All dead
      { volc->act=0; return; }
    Lava2AshesLine(volc->age);
    }

  if (!Tick5)
    if (Inside(volc->age-BackGrSize,0,200)) // Lava Cooling (Color)
      {
      SetLavaCoolingColor(volc,(200-(volc->age-BackGrSize))/2);
      if (volc->age<BackGrSize+100) // Need to set DAC, not done by glowing
	GameSetDAC(CLavaS1,6,&volc->lccol[0][0]);
      }

  if (!Tick10)
    if (volc->age>BackGrSize+100) // Glowing
      {
      volc->glowtick++; if (volc->glowtick>2) volc->glowtick=0;
      for (cnt=0; cnt<3; cnt++)
	MemCopy(&volc->lccol[2*cnt][0],&glowbuf[2*((cnt+volc->glowtick)%3)][0],6);
      GameSetDAC(CLavaS1,6,&glowbuf[0][0]);
      }            // other DAC's do make noise
  }

void LaunchVolcano(int tx)
  {
  int cnt;
  Ground.volcano.act=1;
  Ground.volcano.age=1500;
  MemCopy(&Ground.volcano.lfcol[0][0],&Ground.volcano.lccol[0][0],18);
  for (cnt=0; cnt<VBranchNum; cnt++)
    {
    Ground.volcano.b[cnt].act=0;
    Ground.volcano.b[cnt].dir=0;
    }
  Ground.volcano.b[0].act=1;
  if (tx==-1) tx=20+random(280);
  Ground.volcano.b[0].x=tx;
  Ground.volcano.b[0].y=BackGrSize-4;
  Ground.volcano.b[0].size=20; // the deeper the bigger?
  LaunchEQuake(-1);
  }

void GroundControl(void) // Every Tick50
  {
  Ground.gconwait++;
  if (Ground.gconwait==15)
    {
    Ground.gconwait=0;
    // Volcano control
    if (!Ground.volcano.act)
      if (random(100)<BSA.BVolcLvl)
	if (!random(4)) LaunchVolcano(-1);
    // EQuake control
    if (!Ground.equake)
      if (random(100)<BSA.BQuakeLvl)
	if (!random(4)) LaunchEQuake(-1);
    // Matter Account Control
    if (!Sec20)
      if ((BSA.SMode==S_COOPERATE) && (BSA.CoopGMode==C_GOLDMINE))
	LaunchMatterAccount();
    }
  }

void AdjustGrLevel(void)
  {                                              // Excluding granite for border-castle
  for (Ground.levell=0; !Inside(GBackPix(0,Ground.levell),CRock1,CGroundH); Ground.levell++);
  Ground.levell=BoundBy(Ground.levell,0,130-60*BSA.BWatLvl/100);
  for (Ground.levelr=0; !Inside(GBackPix(319,Ground.levelr),CRock1,CGroundH); Ground.levelr++);
  Ground.levelr=BoundBy(Ground.levelr,0,130-60*BSA.BWatLvl/100);
  }

void ExecGround(int *scrto) // Every Tick1	scrto for Quake
  {
  // Ground Control
  if (!Tick50) GroundControl();
  // Volcano movement
  if (Ground.volcano.act) MoveVolcano();
  // EQuake movement
  if (Ground.equake) MoveEQuake(scrto);
  // Matter Accounting
  if (Ground.MaAcRun>-1) MoveMatterAccount();
  // Misc
  if (Sec5) AdjustGrLevel();
  }

//---------------------------- BSA BRockType ------------------------------------

void AddBRockType(int type, int amount, int *addptr)
  {
  for (amount; amount>0; amount--)
    if (*addptr<20)
      { BSA.BRockType[*addptr]=type; (*addptr)++; }
  }

void PrepareBSARockType(void)
  {
  int cnt,addbr=0;
  for (cnt=0; cnt<20; cnt++) BSA.BRockType[cnt]=ROCK;

  if (BSA.SMode==S_MISSION) // Mission, go by BRockMode table
    {
    switch (BSA.BRockMode)
      {
      case 0: // All Rocks
	break;
      case 1: // Some Flints, some Loam
	AddBRockType(FLINT,5,&addbr);
	AddBRockType(LOAM,5,&addbr);
	break;
      case 2: // Gold out
	AddBRockType(GOLD,10,&addbr);
	break;
      case 3: // Gold & Loam
	AddBRockType(GOLD,10,&addbr);
	AddBRockType(LOAM,5,&addbr);
	break;
      case 4: // More Flints, some loam
	AddBRockType(FLINT,10,&addbr);
	AddBRockType(LOAM,5,&addbr);
	break;
      case 5: // Gold, Flint, Loam
	AddBRockType(FLINT,10,&addbr);
	AddBRockType(LOAM,5,&addbr);
	AddBRockType(GOLD,7,&addbr);
	break;
      }
    }
  else // "Normal" game mixture
    {
    AddBRockType(FLINT,4,&addbr);
    AddBRockType(LOAM,5,&addbr);
    if (BSA.WClim>200) AddBRockType(GOLD,3,&addbr);
    if (BSA.RuleSet>R_EASY)
      {
      AddBRockType(PLANT1,2,&addbr);
      AddBRockType(ZAPN,1,&addbr);
      AddBRockType(ZUPN,1,&addbr);
      }
    }
  }
