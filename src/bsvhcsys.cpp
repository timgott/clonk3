/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design CLONK Battle Sequence Vehicle Control System

#include "standard.h"
#include <stdlib.h>
#include <stdio.h>

#include "clonk_bs.h"

#include "bsexcsys.h"
#include "bsmansys.h"
#include "bsgfxsys.h"
#include "bspxssys.h"
#include "bsstrsys.h"
#include "bsweasys.h"
#include "bsvhcsys.h"

#include "RandomWrapper.h"

extern MANTYPE *NewAnimal(MANTYPE **firstman, BYTE type, int x, int y, int tx, BYTE act, int phase, int strn, int ydir, int xdir, int carr, void *tptr);

//-------------------------- Main Global Externals ---------------------------

extern BSATYPE BSA;
extern char OSTR[500];
extern void EventCall(int evnum);

//-------------------- VHC Global Game Object Variables ------------------------

VEHICTYPE *FirstVehic;

//---------------------------- Local Definitions -----------------------------

#define VHPELEVS   0
#define VHPBOATL 100
#define VHPBOATR 200
#define VHPBALLD   0
#define VHPBALLU  80

#define VHDCatPhase   back[0]
#define VHDCatLoad    back[1]
#define VHDCatFPower  back[2]

#define VHDXBowPhase  back[0]
#define VHDXBowLoadA  back[1]
#define VHDXBowDir    back[2]
#define VHDXBowDirCh  back[3]
#define VHDXBowJustL  back[4]
#define VHDXBowLoadF  back[5]
#define VHDXBowLoadB  back[6]

#define VHDAddToWait  back[25]
#define VHDToCastle   back[26]
#define VHDFirstDown  back[27]
#define VHDBallCrash  back[28]
#define VHDLApprCorr  back[29]

#define VHDOnWater    back[29]

#define VHDWipfLoad   back[29]

#define VHDDrilling   data3

//--------------------------- Vehicle data usage -----------------------------
//
//           p(hase)     data        data2       data3           back[]
//
// LORRY*    h.kin.enrgy erg2mvm buf vhc slope   (vhc dir)       rock load; 29:VHD
// ELEVATOR  up/dn mvm   top y       target y    drilling        pixback
// BALLOON   up/dn mvm   target x    target y    horiz.slide.mvm 0-9:pixback; 25-29:VHD
// CATAPULT* h.kin.enrgy erg2mvm buf vhc slope   vhc dir         0-2:VHD
// SAILBOAT* h.kin.enrgy erg2mvm buf (vhc slope) (vhc dir)       29:VHD
// SAILBOAT  lf/rt mvm	 erg2mvm buf                             pixback; 29:VHD
// CROSSBOW* h.kin.enrgy erg2mvm buf vhc slope   vhc dir         0-6:VHD
//
// * RVehic

// RVehic-dir: 0 Right 1 Left

//-------------------------- (Vehicle Control) -------------------------------

//---------------------------- Back Operation --------------------------------

inline void GrabBackPix(int tx, int ty, BYTE *bufptr, int vmd)
  {
  *bufptr=GBackPix(tx,ty);
  SBackPix(tx,ty,CVhcST-IsSkyBack(*bufptr)+2*vmd);
  }

void GrabBack(VEHICTYPE *vhc, int vmd) // vmd: -1/0/+1
  {
  int cnt;
  BYTE *bckmemptr=vhc->back;
  switch (vhc->type)
    {
    case VHELEVATOR:
      for (cnt=0; cnt<12; cnt++)
	{
	//GrabBackPix(vhc->x+2+cnt,vhc->y+2,bckmemptr++,vmd);
	GrabBackPix(vhc->x+2+cnt,vhc->y+12,bckmemptr++,vmd);
	}
      break;
    case VHBALLOON:
      for (cnt=0; cnt<10; cnt++)
	GrabBackPix(vhc->x+2+cnt,vhc->y+19,bckmemptr++,vmd);
      break;
    case VHSAILBOAT:
      if (vhc->VHDOnWater) // OnWater only
	for (cnt=0; cnt<10; cnt++)
	  GrabBackPix(vhc->x+1+cnt,vhc->y+8,bckmemptr++,vmd);
    }
  }

inline void DropBackPix(int tx, int ty, BYTE *bufptr)
  {
  if (Inside(GBackPix(tx,ty),CVhcL,CVhcH))
    SBackPix(tx,ty,*bufptr);
  }

void DropBack(VEHICTYPE *vhc)
  {
  int cnt;
  BYTE *bckmemptr=vhc->back;
  switch (vhc->type)
    {
    case VHELEVATOR:
      for (cnt=0; cnt<12; cnt++)
	{
	//DropBackPix(vhc->x+2+cnt,vhc->y+2,bckmemptr++);
	DropBackPix(vhc->x+2+cnt,vhc->y+12,bckmemptr++);
	}
      break;
    case VHBALLOON:
      for (cnt=0; cnt<10; cnt++)
	DropBackPix(vhc->x+2+cnt,vhc->y+19,bckmemptr++);
      break;
    case VHSAILBOAT:
      //if (!vhc->VHDOnWater) SystemError("Boat dropping back on land");
      for (cnt=0; cnt<10; cnt++)
	DropBackPix(vhc->x+1+cnt,vhc->y+8,bckmemptr++);
    }
  }

//---------------------------- Cross Checking --------------------------------

VEHICTYPE *OnWhichVehic(int tx, int ty, VEHICTYPE *notthis)
  {
  VEHICTYPE *cvhc=FirstVehic;
  while (cvhc)
    {                 // vehic size??
    if (Inside(tx-cvhc->x,0,16) && Inside(ty-cvhc->y,0,20))
      if (!notthis || (cvhc!=notthis))
	return cvhc;
    cvhc=cvhc->next;
    }
  return NULL;
  }

int VehicleLoc(int tx, int ty) // Returns left border of vehic, if inside any
  {                            // With elevators, inside shaft is enough
  VEHICTYPE *cvhc;
  for (cvhc=FirstVehic; cvhc; cvhc=cvhc->next)
    if (cvhc->type!=VHNOVEHIC)
      if (Inside(tx-cvhc->x,0,11+4*(cvhc->type==VHBALLOON)))
	if (((cvhc->type==VHELEVATOR) && (ty>cvhc->data)) || Inside(ty-cvhc->y,0,25))
	  return cvhc->x+2*(cvhc->type!=VHSAILBOAT);
  return -1;
  }

//---------------------------- Data Handling ---------------------------------

VEHICTYPE *NewVehic(int type, int x, int y, int p, int data, int data2, int data3, int owner, void *tptr)
  {
  VEHICTYPE *nvehic,*cvhc;
  if (!(nvehic=new VEHICTYPE)) { LowMem=1; return NULL; }
  nvehic->x=x; nvehic->y=y; nvehic->p=p;
  nvehic->type=type; nvehic->owner=owner;
  nvehic->data=data; nvehic->data2=data2; nvehic->data3=data3;
  nvehic->tptr=tptr;
  if ((type==VHLORRY) || (type==VHCATAPULT) || (type==VHSAILBOAT))
    {			// Lorries,Cats,Boat to the end of line
    if (FirstVehic)
      { cvhc=FirstVehic; while (cvhc->next) cvhc=cvhc->next; cvhc->next=nvehic; }
    else
      FirstVehic=nvehic;  // -> sort in the following order
    nvehic->next=NULL;    // balloon,elevator,boat,catapult,lorry
    }			    //   C        C      B/A    A       A   current
  else                      //   B        B      B/A    A       A   should-be
    {
    nvehic->next=FirstVehic;
    FirstVehic=nvehic;
    }
  ZeroMem(nvehic->back,30); GrabBack(nvehic,0);
  return nvehic;
  }

void DeleteVehic(VEHICTYPE *tvhc)
  {
  VEHICTYPE *cvhc=FirstVehic,*prev=NULL;
  if (!tvhc) return;
  if (tvhc->type==VHLORRY) // or cat or boat
    ClearMenTPtr(tvhc);
  while (cvhc)
    {
    if (cvhc->next==tvhc) { prev=cvhc; break; }
    cvhc=cvhc->next;
    }
  if (tvhc==FirstVehic) FirstVehic=tvhc->next;
  if (prev) prev->next=tvhc->next;
  delete tvhc;
  }

void DeleteVehics(void)
  {
  VEHICTYPE *temptr;
  while (FirstVehic) { temptr=FirstVehic->next; delete FirstVehic; FirstVehic=temptr; }
  }

//------------------------------ (Movement) ----------------------------------

//------------------------------- Elevator -----------------------------------

BYTE ElevatorMovmOK(VEHICTYPE *vhc, int dir, BYTE drill)
  {
  int cy,cnt,solidcnt=0,snowcnt=0;
  cy=vhc->y+1+12*(dir==+1);
  if (!drill)
    {
    for (cnt=2; cnt<14; cnt++) // Snow scraping...
      if (Inside(GBackPix(vhc->x+cnt,cy),CSolidL,CSolidH))
	if (Inside(GBackPix(vhc->x+cnt,cy),CSnowS,CSnowT) && (snowcnt<3))
	  {
	  //ReleasePXS(vhc->x+cnt,cy);
	  DrawBackPix(vhc->x+cnt,cy);
	  snowcnt++;
	  }
	else solidcnt++;
    }
  else
    {
    if (vhc->y>=BackGrSize-5-13) return 0;
    for (cnt=2; cnt<14; cnt++)
      if (Inside(GBackPix(vhc->x+cnt,cy),CGranite1,CGold2))
	solidcnt++;
    }
  if (solidcnt<2) return 1;
  return 0;
  }

void MoveElevator(VEHICTYPE *vhc)
  {                                // data:  top-y    p:wait-count
  int dir,cnt;                     // data2: ty	        0    move
  BYTE cbyown;                     // data3: drill      1-20 wait
  MANTYPE *cman;

  // Drilling
  if (vhc->VHDDrilling)
    {
    if (ElevatorMovmOK(vhc,+1,1))
      {
      for (cnt=2; cnt<14; cnt++)
	{
	if (Inside(GBackPix(vhc->x+cnt,vhc->y+13),CEGroundL,CEGroundH))
	  SBackPix(vhc->x+cnt,vhc->y+13,CTunnel1+random(5));
	ReleasePXS(vhc->x+cnt,vhc->y+13-1);
	ReleasePXS(vhc->x+cnt-1,vhc->y+13);
	ReleasePXS(vhc->x+cnt+1,vhc->y+13);
	}
      DropBack(vhc); vhc->y+=1; GrabBack(vhc,0);
      // drilling sound
      }
    else
      {
      vhc->VHDDrilling=0;
      vhc->p=1; vhc->data2=vhc->data;
      }
    return;
    }

  // Wait phasing
  if (Inside(vhc->p,1,20)) if (!Tick3) { vhc->p++; if (vhc->p>20) vhc->p=0; }

  // Standard movement
  if (vhc->p==0)
    {
    dir=-1; if (vhc->data2>vhc->y) dir=+1;
    if (ElevatorMovmOK(vhc,dir,0) && (vhc->y!=vhc->data2)) // Move if free
      {                                                    // and not there
      DropBack(vhc); vhc->y+=dir; GrabBack(vhc,0);
      }
    else // Start waiting and set new target
      {
      vhc->p=1;
      if (dir==+1) vhc->data2=vhc->data; else vhc->data2=BackGrSize;
      }
    }

  // Elevator2WaitingClonk Control           // safety, remove
  if (vhc->owner<0) { RoundError("safety:elevator w\out owner"); return; }
  cbyown=0;
  if ((vhc->y%10)==0)
    for (cnt=0; (cnt<3) && !cbyown; cnt++)
      if (NotHostile(vhc->owner,cnt))
	for (cman=Crew[cnt].FirstMan; cman; cman=cman->next)
	  if (cman->act==MAWALK) if (cman->con)
	    if (cman->x==cman->tx)
	      if (Inside(cman->x+4-(vhc->x+8),-12,+12))
		{
		if (!Inside(vhc->y-cman->y,-20,+20))
		  vhc->data2=Max(cman->y-3,vhc->data);
		if (cnt==vhc->owner) cbyown=1; // Owner has priority
		}
  }

void ConElevator(VEHICTYPE *vhc)
  {
  if (vhc->VHDDrilling)
    {
    vhc->VHDDrilling=0;
    GameMessage("stop shaft|drilling",vhc->x+8,vhc->y-8,CWhite,NULL);
    vhc->p=VHPELEVS;
    }

  if (vhc->p==VHPELEVS) // Moving -> stop (and turn around)
    {
    if (vhc->data2==vhc->data) vhc->data2=BackGrSize;
    else vhc->data2=vhc->data;
    vhc->p++;
    }
  else // Waiting -> move (to old target)
    vhc->p=VHPELEVS;
  }

//------------------------------- Balloon ------------------------------------

void BlowUpBalloon(VEHICTYPE *tvhc)
  {
  RemoveVehic(tvhc);
  tvhc->VHDAddToWait=20;
  if (tvhc->VHDBallCrash!=1) // Heavy blow up
    {
    if (tvhc->y>-10) Explode(tvhc->x+8,tvhc->y+8,16,-1);
    tvhc->VHDAddToWait=60;
    }
  tvhc->x=0; tvhc->y=-20; tvhc->p=81;
  tvhc->tptr=NULL;
  tvhc->VHDBallCrash=0;
  // Balloon disintegrate in to pieces?!
  }

BYTE MoveBalloon(VEHICTYPE *vhc)
  {
  BYTE mvm=0,chkl,chkr;
  int ctco,cnt;
  DropBack(vhc);

  // Vertical movement
  if (!Tick2)                    // No Balloon <-> Vehic collision
    {
    ctco=vhc->y-1; if (vhc->p==0) ctco=vhc->y+1; if (vhc->VHDBallCrash) ctco=vhc->y+2;

    if (!Inside(GBackPix(vhc->x+8,ctco+19),CSolid2L,CSolidH)) // Upwards even through water, downwards only bottom through water
      if (!((vhc->p==0) || vhc->VHDBallCrash) || !Inside(GBackPix(vhc->x+8,ctco+16),CSolid2L,CSolid2H))
	{ vhc->y=ctco; mvm=1; }
    }
  else mvm=1;
  // Check for horizontal sliding
  if (vhc->p==80)
    if (Inside(GBackPix(vhc->x+8,vhc->y+18),CSolid2L,CSolid2H))
      vhc->data3=5;
  // Horizontal movement
  ctco=0;
  if (vhc->data3==0) // Horizontal movement by wind
    {
    ctco=BoundBy(Weather.wind/15,-1,+1);
    if (!Tick2) if (Abs(Weather.wind)<30) ctco=0; // no wind cancel
    if (vhc->VHDLApprCorr) ctco=0; // landing approach cancel
    }
  else // Horizontal sliding
    {
    for (cnt=1,chkl=chkr=1; (cnt<320) && (chkl || chkr); cnt++)
      {
      if (chkl)
	{
	if ((vhc->x+8-cnt<1) || Inside(GBackPix(vhc->x+8-cnt,vhc->y+19),CSolid2L,CSolid2H)) chkl=0;
	else if (!Inside(GBackPix(vhc->x+8-cnt,vhc->y+18),CSolid2L,CSolid2H)) { ctco=-1; break; }
	}
      if (chkr)
	{
	if ((vhc->x+8+cnt>302) || Inside(GBackPix(vhc->x+8+cnt,vhc->y+19),CSolid2L,CSolid2H)) chkr=0;
	else if (!Inside(GBackPix(vhc->x+8+cnt,vhc->y+18),CSolid2L,CSolid2H)) { ctco=+1; break; }
	}
      }
    }
  if (vhc->p==0) if (!vhc->VHDBallCrash) // Target (castle) landing control
    if (vhc->data>-1)
      if (vhc->y<vhc->data2)
	if (Abs(vhc->data-vhc->x)>vhc->data2-vhc->y) // Inside approach corridor
	  { ctco=Sign(vhc->data-vhc->x); vhc->VHDLApprCorr=1; }
  if (!Inside(vhc->x+ctco,1,302)) ctco=0; // border cancel
  // Exec horizontal movement
  if (ctco)
    if (!Inside(GBackPix(vhc->x+ctco+8,vhc->y+19),CSolid2L,CSolid2H))
      { vhc->x+=ctco; mvm=2; }
  if (mvm!=2) ctco=0;
  // End horiz. slide phase
  if (vhc->data3>0) vhc->data3--;
  GrabBack(vhc,ctco);
  return mvm;
  }

void SetBalloonLaunch(VEHICTYPE *vhc)
  {
  int tx,ty,cnt;
  STRUCTYPE *cstrc,*lcast=NULL;
  MANTYPE *tman=(MANTYPE*)vhc->tptr;
  vhc->tptr=NULL; // (cancel cap-call balloon for next time)
  vhc->data=vhc->data2=-1;
  vhc->VHDFirstDown=1;
  vhc->VHDToCastle=0;
  // Safety
  if (vhc->owner<0) { RoundError("safety: balloon w/out owner"); vhc->type=VHNOVEHIC; return; }
  // Check for castle
  if (!tman) // Not called to clonk
    for (cstrc=FirstStruct; cstrc; cstrc=cstrc->next)
      if ((cstrc->type==STCASTLE) && (cstrc->owner==vhc->owner) && (cstrc->con>=1000))
	lcast=cstrc;
  if (lcast)
    {
    tx=lcast->x+8; ty=lcast->y-20;
    vhc->data=tx; vhc->data2=ty;
    vhc->x=BoundBy(tx-ty*Weather.wind/60,0,303);
    vhc->VHDToCastle=1;
    return;
    }
  // Else, check for call to clonk on surface
  if (tman && (tman->act<MADEAD))
    {
    tx=tman->x-4; ty=tman->y-11;
    for (cnt=ty+11; cnt>0; cnt--) if (Inside(GBackPix(tx+4,cnt),CSolid2L,CSolid2H)) break;
    if (cnt==0)
      {
      vhc->data=tx; vhc->data2=ty;
      vhc->x=BoundBy(tx-ty*Weather.wind/60,0,303);
      return;
      }
    }
  // Else, target land to plr pos (land on land if possible)
  if (FindSurface(1,Crew[vhc->owner].BaseX,20,&tx,&ty))
    {
    vhc->data=tx-8+random(11)-5; vhc->data2=ty-20;
    vhc->x=BoundBy(tx-ty*Weather.wind/60,0,303);
    return;
    }
  // Else, random
  tx=random(300)+10-8;
  ty=0; while (!Inside(GBackPix(tx,ty),CSolid2L,CSolid2H)) ty++; ty-=20;
  vhc->data=tx; vhc->data2=ty;
  vhc->x=BoundBy(tx-ty*Weather.wind/60,0,303);
  }

void SetBLaunchNoDouble(VEHICTYPE *vhc)
  {
  VEHICTYPE *cvhc;
  int cnt,evshift;
  if (vhc->x+8>160) evshift=-16;
  else evshift=+16;
  for (cnt=0; cnt<3; cnt++)
    for (cvhc=FirstVehic; cvhc; cvhc=cvhc->next)
      if ((cvhc->type==VHBALLOON) && (cvhc!=vhc))
	if (Inside(vhc->x-cvhc->x,-16,+16) && Inside(vhc->y-cvhc->y,-10,+10))
	  vhc->x+=evshift;
  }

/*BYTE GetPushOutDir(VEHICTYPE *vhc) // 1 left  2 right
  {
  int cnt,cy,cy2;

  for (cnt=0; cnt<20; cnt++)
    {
    if (vhc->x+1-cnt<5) return 2;
    if (vhc->x+14+cnt>314) return 1;
    cy=cy2=vhc->y+19;
    while (!Inside(GBackPix(vhc->x+1-cnt,cy),CSolidL,CSolid2H)) cy++;
    while (!Inside(GBackPix(vhc->x+14+cnt,cy2),CSolidL,CSolid2H)) cy2++;
    while (Inside(GBackPix(vhc->x+1-cnt,cy-1),CSolidL,CSolid2H)) cy--;
    while (Inside(GBackPix(vhc->x+14+cnt,cy2-1),CSolidL,CSolid2H)) cy2--;
    if (cy>cy2) return 1;
    if (cy2>cy) return 2;
    }

  if (vhc->x<vhc->data) return 1;
  if (vhc->y>vhc->data) return 2;

  return random(2)+1;
  }*/

void BalloonUnloadToCastle(VEHICTYPE *vhc) // Called at 1 above touch-down
  {
  STRUCTYPE *tstrc;
  MANTYPE *cman;
  ROCKTYPE *crck;
  VEHICTYPE *cvhc;
  int cnt;

  tstrc=OnWhichStruct(vhc->x-4,vhc->y+27);
  if (tstrc && (tstrc->type==STCASTLE))
    {
    // Unload men
    for (cnt=0; cnt<3; cnt++)
      if (NotHostile(vhc->owner,cnt))
	for (cman=Crew[cnt].FirstMan; cman; cman=cman->next)
	  if (cman->act==MAWALK && (cman->x==cman->tx))
	    if ((cman->y==vhc->y+10) && Inside(cman->x+4-(vhc->x+8),-6,+6))
	      cman->tx=tstrc->x+1;
    // Unload Rocks
    for (crck=FirstRock; crck; crck=crck->next)
      if (crck->act==RADEAD)
	if (crck->y==vhc->y+15) if (Inside(crck->x+2-vhc->x,2,13))
	  { crck->x=tstrc->x+8+random(11)-5; crck->y=tstrc->y+8; }
    // Unload Vehicles
    for (cvhc=FirstVehic; cvhc; cvhc=cvhc->next)
      if ((cvhc->type==VHLORRY) || (cvhc->type==VHSAILBOAT) || (cvhc->type==VHCATAPULT) || (cvhc->type==VHCROSSBOW))
	if ((cvhc->y==vhc->y+10) && Inside(cvhc->x-(vhc->x+2),-7,+7) )
	  {
	  cvhc->x=tstrc->x+10-6; cvhc->y=tstrc->y+2;
	  cvhc->p=45-90*random(2); if (cvhc->x<30) cvhc->p=45; if (cvhc->x>290) cvhc->p=-45;
	  }
    }
  }

extern BYTE LostCaptureFlag(BYTE plr);

void MoveBalloon1(VEHICTYPE *vhc) // Phase  0: Down   1- 79:           Delay
  {				  //       80: Up    81-159+AddToWait: Delay
  int cnt,cnt2;
  STRUCTYPE *tstrc;
  MANTYPE *tman;

  // Phase
  if (!Tick2) if (Inside(vhc->p,1,79) || Inside(vhc->p,81,159+30*vhc->VHDAddToWait)) vhc->p++;

  // ComeOkay?
  if (vhc->p==158)
    {
    if (LostCaptureFlag(vhc->owner)) vhc->p=81;
    if (BSA.Plr[vhc->owner].Eliminate) vhc->p=81;
    }

  // Turn around for touch down
  if (vhc->p>159+30*vhc->VHDAddToWait)
    {
    vhc->p=0; vhc->VHDAddToWait=0;
    if (vhc->y<-19) { SetBalloonLaunch(vhc); SetBLaunchNoDouble(vhc); }
    }

  // Enforce downward movement if crashing
  if (vhc->VHDBallCrash)
    {
    // Smokey
    if (vhc->VHDBallCrash==1) PXSOut(PXSSMOKE,vhc->x+12,vhc->y+4,5+random(20));
    else for (cnt=0; cnt<5; cnt++) PXSOut(PXSSMOKE,vhc->x+random(16),vhc->y+3+random(10),10+random(20));
    vhc->p=0;
    }

  // Downward movement
  if (vhc->p==0)
    {
    // Grab rock storage (on entry) must be at -16 can't be inside SetB.Launch
    if (!BSA.Realism.CstHome)
      if (vhc->y==-16)
	for (cnt=0; cnt<RockTypeNum; cnt++)      // no checking for no-owner
	  while (Crew[vhc->owner].RockStorage[cnt]>0)
	    {
	    NewRock(vhc->x+3+random(7),vhc->y+15,RADEAD,cnt,0,0,5*Inside(cnt,ARROW,BARROW),-1);
	    Crew[vhc->owner].RockStorage[cnt]--;
	    if (Crew[vhc->owner].CMenu==CMRCKORDER) Crew[vhc->owner].RedrStB=1;
	    }
    // Load drop check (castle approach ty-1)
    if (vhc->VHDToCastle)
      if (vhc->y==vhc->data2-1)
	BalloonUnloadToCastle(vhc);
    // Touch down
    if (!MoveBalloon(vhc))
      {
      vhc->p=1;
      vhc->VHDFirstDown=0;
      vhc->VHDLApprCorr=0; // Cancel landing corridor flag
      if (vhc->VHDToCastle) // Activate castle gate
	if (!BSA.Realism.CstHome)
	  {
	  tstrc=OnWhichStruct(vhc->x-4,vhc->y+27);
	  if (tstrc && (tstrc->type==STCASTLE))
	    if (tstrc->p==0) tstrc->p=1;
	  }
      if (vhc->VHDBallCrash) BlowUpBalloon(vhc);
      }
    }

  // Upward movement
  if (vhc->p==80)
    {
    if (!MoveBalloon(vhc)) vhc->p=150;
    if (vhc->y<-19) vhc->p=81; // Balloon out on top
    }

  // Check for immediate call to clonk
  if (!Tick50) if (vhc->tptr) if (!vhc->VHDAddToWait)
    {
    tman=(MANTYPE*)vhc->tptr;
    if (Abs(vhc->x-(tman->x-4))<Max(tman->y-11-vhc->y,0)) // In corridor?
      {
      vhc->data=tman->x-4; vhc->data2=tman->y-11;
      vhc->p=0;
      vhc->tptr=NULL;
      vhc->VHDToCastle=0;
      }
    }
  }

void ConBalloon(VEHICTYPE *vhc)
  {
  if (vhc->VHDFirstDown) return;
  if (Inside(vhc->p,VHPBALLD,VHPBALLU-1)) // Moving down/waiting -> move up
    { vhc->p=VHPBALLU; return; }
  if (Inside(vhc->p,VHPBALLU,159+30*vhc->VHDAddToWait)) // Moving up/waiting
    {                                                   // -> move down
    vhc->data=vhc->x; vhc->data2=BackGrSize;
    vhc->p=VHPBALLD;
    return;
    }   			   // Change from waiting for down ain't
  }				   // gonna happen

//-------------------------------- RVehics --------------------------------------

extern DWORD WipfResc;

void HomeRVehic(VEHICTYPE *vhc, void *htptr, int homeat)
  {
  int cnt,oid;
  BYTE sellv;
  vhc->x=-1; // Means home
  vhc->y=vhc->p=vhc->data=vhc->data2=vhc->data3=0;
  vhc->tptr=htptr;
  sellv=1;
  // Lorry sells contents
  if (vhc->type==VHLORRY)
    {
    for (cnt=0; cnt<RockTypeNum; cnt++)
      {
      for (vhc->back[cnt]; vhc->back[cnt]>0; vhc->back[cnt]--)
	{ HomeRock(cnt,0,homeat); sellv=0; }
      if (vhc->VHDWipfLoad>0) sellv=0;
      Crew[homeat].Wealth=Min(Crew[homeat].Wealth+vhc->VHDWipfLoad*10,999);
      ScoreGain(homeat,vhc->VHDWipfLoad*10);
      WipfResc+=vhc->VHDWipfLoad;
      vhc->VHDWipfLoad=0;
      }
    if (BSA.Realism.CstHome) sellv=1;
    EventCall(202);
    }
  // Empty lorry or other vehic is resold (not in EASY)
  if (sellv && (BSA.RuleSet>0))
    {
    oid=1; if (vhc->type==VHCATAPULT) oid=2; if (vhc->type==VHSAILBOAT) oid=4; if (vhc->type==VHCROSSBOW) oid=3;
    vhc->type=VHNOVEHIC;
    Crew[homeat].Wealth=Min(Crew[homeat].Wealth+VhcOrderPrice[oid],999);
    }
  }

void MoveRVehic(VEHICTYPE *vhc) // Moves Lorry, Catapult, Sailboat, Crossbow
  {
  BYTE lgpix,rgpix,mvd;
  int cnt,cnt2,stnpix,ldata2,outdir;
  VEHICTYPE *cvhc;
  STRUCTYPE *tstrc;

  if (vhc->x<0) // Vehic is home
    {
    if (!vhc->tptr)
      {
      RoundError("safety: vhc home w\out tptr");
      return;
      }
    // Base Re-Entry
    if (BSA.Realism.CstHome) // Castle: immediate re-entry
      {
      tstrc=(STRUCTYPE*) vhc->tptr;
      if (tstrc->p==0) tstrc->p=1; // activate gate
      if (tstrc->p>=15)
	{
	outdir=1-2*random(2); if (tstrc->x<50) outdir=+1; if (tstrc->x>270) outdir=-1;
	vhc->x=tstrc->x+10-6+4*outdir; vhc->y=tstrc->y+2;
	vhc->p=45*outdir;
	vhc->data=vhc->data3=0; vhc->data2=1;
	vhc->tptr=NULL;
	}
      }
    else // Balloon: re-entry on balloon re-entry
      {
      cvhc=(VEHICTYPE*) vhc->tptr;
      if ((cvhc->p==0) && (cvhc->y==-16))
	{
	vhc->x=cvhc->x+2; vhc->y=cvhc->y+10;
	vhc->p=vhc->data=vhc->data2=vhc->data3=0;
	vhc->tptr=NULL;
	}
      }
    return;
    }

  // Shift up on vehic (other vehic has moved before)
  if (Inside(GBackPix(vhc->x+6,vhc->y+8),CVhcL,CVhcH)) vhc->y--;

  // Falling down
  for (cnt=0; !Inside(GBackPix(vhc->x+6,vhc->y+9),CSolidL,CSolidH) && (cnt<2); vhc->y++, cnt++);

  // Slope force left/right (vhc->p is horizontal kinetic energy)
  lgpix=Inside(GBackPix(vhc->x+5,vhc->y+9),CSolidL,CSolidH);
  rgpix=Inside(GBackPix(vhc->x+7,vhc->y+9),CSolidL,CSolidH);
  ldata2=vhc->data2;
  vhc->data2=1;
  if (!lgpix && rgpix) { vhc->p-=5; vhc->data2++; }
  if (lgpix && !rgpix) { vhc->p+=5; vhc->data2--; }
  if (ldata2!=vhc->data2) if (!random(6)) if (!Inside(vhc->p,-50,+50))
    if (vhc->type!=VHSAILBOAT) GSTP=SNDYANK;

  stnpix=150; // Default/Lorry
  if ((vhc->type==VHCATAPULT) || (vhc->type==VHCROSSBOW)) stnpix=55;
  if (vhc->type==VHSAILBOAT) stnpix=40;
  vhc->p=BoundBy(vhc->p,-stnpix,stnpix);

  // Hit left/right wall
  if (Inside(GBackPix(vhc->x+2,vhc->y),CSolidL,CSolidH)) vhc->p=Max(0,vhc->p);
  if (Inside(GBackPix(vhc->x+9,vhc->y),CSolidL,CSolidH)) vhc->p=Min(0,vhc->p);

  // Horizontal movement
  vhc->data+=vhc->p; mvd=0;
  if (vhc->data> 100) { vhc->x++; vhc->data=0; mvd=1; vhc->data3=0; }
  if (vhc->data<-100) { vhc->x--; vhc->data=0; mvd=1; vhc->data3=1; }
  vhc->p-=Sign(vhc->p);

  // Solid matter shift up if free above
  while (Inside(GBackPix(vhc->x+6,vhc->y+8),CSolidL,CSolidH) && !Inside(GBackPix(vhc->x+6,vhc->y-1),CSolidL,CSolidH)) vhc->y--;

  // OnVehic movement
  stnpix=GBackPix(vhc->x+6,vhc->y+9);
  if (Inside(stnpix,CVhcL,CVhcH))
    { stnpix-=CVhcL; stnpix>>=1; stnpix--; vhc->x+=stnpix; }

  // Border checking
  if (vhc->x<=5)   { vhc->x=5;   vhc->p=Max(0,vhc->p); }
  if (vhc->x>=303) { vhc->x=303; vhc->p=Min(0,vhc->p); }

  // Stop on other vehic (not boat)
  if (mvd && (vhc->type!=VHSAILBOAT))
    {
    cvhc=FirstVehic;
    while (cvhc)
      {
      if ((cvhc->type==VHBALLOON) || (cvhc->type==VHELEVATOR) || (cvhc->type==VHSAILBOAT))
	if (vhc->x-2*(cvhc->type!=VHSAILBOAT)==cvhc->x) if (cvhc!=vhc)
	  if (Inside(vhc->y-cvhc->y,-4,16))
	    vhc->p=0;
      cvhc=cvhc->next;
      }
    }

  // Homing check
  if (vhc->y<-8) if (!BSA.Realism.CstHome)
    {
    for (cvhc=FirstVehic; cvhc; cvhc=cvhc->next)
      if ((cvhc->type==VHBALLOON) && (cvhc->owner>=0))
	if (Inside(vhc->x+6-cvhc->x,0,16) && Inside(vhc->y+3-cvhc->y,0,20))
	  break;
    if (cvhc) HomeRVehic(vhc,cvhc,cvhc->owner);
    }

  }

void MoveLorry(VEHICTYPE *vhc)
  {
  int cnt;
  MoveRVehic(vhc);
  // Wipf load out if in water
  if (!Tick5) if (vhc->VHDWipfLoad>0)
    if (Inside(GBackPix(vhc->x+6,vhc->y),CLiqL,CLiqH))
      {
      for (cnt=0; cnt<vhc->VHDWipfLoad; cnt++)                                                              // strn & carr(food) is not correct!!!
	NewAnimal(&FirstAnimal,MNWIPF,vhc->x+random(4),vhc->y-3+random(4),BoundBy(vhc->x+random(21)-10,0,311),MASWIM,0,100,0,40+random(20),0,NULL);
      vhc->VHDWipfLoad=0;
      }
  }

void MoveCatapult(VEHICTYPE *vhc)
  {
  int ydir,xdir;
  MoveRVehic(vhc);
  // Phase: 0 Empty  1-50 Charge  51-60 Fire
  if (vhc->VHDCatPhase)
    {
    vhc->VHDCatPhase++;
    vhc->VHDCatFPower++; // Firepower (0-50)
    if (vhc->VHDCatPhase==51) GSTP=SNDCATAPULT;
    if (vhc->VHDCatPhase==60)
      {
      xdir=15*(4-8*vhc->data3)*Max(vhc->VHDCatFPower,26)/50;
      ydir=(-40)*vhc->VHDCatFPower/50+random(10);
      if (vhc->data3) ydir-=10*(vhc->data2-1); else ydir+=10*(vhc->data2-1);
      NewRock(vhc->x+8-7*vhc->data3,vhc->y-5,RAFLY,vhc->VHDCatLoad,xdir,ydir,(vhc->VHDCatLoad==TFLINT) ? 50+random(20) : 0,vhc->owner);
      vhc->VHDCatPhase=0;
      EventCall(203);
      }
    }
  }

void MoveCrossbow(VEHICTYPE *vhc)
  {
  int xdir,ydir,atype,aphase;
  MoveRVehic(vhc);

  // Dir change (VHDXBowDirCh: 9 ArmUp 11 ArmDown)
  if (vhc->VHDXBowDirCh)
    {
    if (vhc->VHDXBowDirCh==9)
      if (vhc->VHDXBowDir>=25) vhc->VHDXBowDir-=15;
      else { ++vhc->data3%=2; vhc->VHDXBowDir=10; }
    if (vhc->VHDXBowDirCh==11)
      vhc->VHDXBowDir+=15;
    vhc->VHDXBowDirCh=0;
    }

  // Phase: 0 Waiting 1-10 Firing 11-30 Charging
  if (vhc->VHDXBowPhase)
    {
    if (vhc->VHDXBowPhase<9) vhc->VHDXBowPhase++; // Double firing speed
    vhc->VHDXBowPhase++;

    if (vhc->VHDXBowPhase==10)
      {
      if (vhc->VHDXBowLoadB>0) { atype=BARROW; vhc->VHDXBowLoadB--; }
      else if (vhc->VHDXBowLoadF>0) { atype=FARROW; vhc->VHDXBowLoadF--; }
	else if (vhc->VHDXBowLoadA>0) { atype=ARROW; vhc->VHDXBowLoadA--; }
	  else atype=ARROW;

      xdir=40*(vhc->VHDXBowDir)/70+random(11)-5;
      if (xdir==0) xdir=1-vhc->data3*2; // No vertical shoots
      if (Inside(xdir,-9,+9)) xdir=10*Sign(xdir);
      ydir=-50*(90-vhc->VHDXBowDir)/70+random(11)-5;
      if (vhc->data3) xdir*=-1;
      aphase=1;
      if (atype==FARROW) aphase=10+200;
      NewRock(vhc->x+8-7*vhc->data3,vhc->y-1,RAFLY,atype,xdir,ydir,aphase,vhc->owner);
      }

    if (vhc->VHDXBowPhase>30) vhc->VHDXBowPhase=0;
    }

  vhc->VHDXBowJustL=0;
  }

//---------------------------- Boat On Water ---------------------------------

void BoatOnwMovm(VEHICTYPE *vhc)
  {
  int dir=0,cnt;
  BYTE cbyown;
  MANTYPE *cman;

  DropBack(vhc);
  // Adjust to water level
  if (!Inside(GBackPix(vhc->x+6,vhc->y+9),CSolidL,CSolid2H)) vhc->y++;
  if (Inside(GBackPix(vhc->x+6,vhc->y+8),CLiqL,CLiqH)) vhc->y--;

  // Phasing
  if ((vhc->p!=VHPBOATL) && (vhc->p!=VHPBOATR)) // Wait
    vhc->p++;
  else // Move
    {
    // Boat2WaitingCaptain Control           // safety, remove
    if (vhc->owner<0) { RoundError("safety: boat w\out owner"); return; }
    cbyown=0;
    if ((vhc->x%10)==0)
      for (cnt=0; (cnt<3) && !cbyown; cnt++)
	if (NotHostile(vhc->owner,cnt))
	  for (cman=Crew[cnt].FirstMan; cman; cman=cman->next)
	    if (cman->act==MAWALK) if (cman->con)
	      if (cman->x==cman->tx)
		if (Inside(cman->y+4-(vhc->y+5),-12,+12))
		  {
		  if (cman->x+8<vhc->x) vhc->p=VHPBOATL;
		  if (cman->x>vhc->x+16) vhc->p=VHPBOATR;
		  if (cnt==vhc->owner) cbyown=1; // Owner has priority
		  }
    // Execute movement
    dir=-1; if (vhc->p==VHPBOATR) dir=+1;
    if (Inside(vhc->x+dir,0,311) && !Inside(GBackPix(vhc->x+dir+6,vhc->y+9),CSolidL,CSolidH)
     && !Inside(GBackPix(vhc->x+11*(dir==+1),vhc->y+8),CSolidL,CSolidH))
      {
      vhc->data+=50;
      if (Sign(Weather.wind)==dir) vhc->data+=2.0*Abs(Weather.wind);
      vhc->data=Min(vhc->data,200);
      if (vhc->data>=100)
	{
	vhc->x+=dir;
	vhc->data-=100;
	}
      else dir=0; // dir indicates executed movement...
      }
    else
      { vhc->p++; if (vhc->p>VHPBOATR) vhc->p=0; }
    }

  // PushOut                           ...or wanted load movement
  //if (Inside(vhc->p,VHPBOATL+1,VHPBOATL+15)) dir=-1;
  //if (Inside(vhc->p,0,15)) dir=+1;

  // Return to OnLand
  if (Inside(GBackPix(vhc->x+6,vhc->y+9),CSolidL,CSolidH))
    { vhc->p=vhc->data=vhc->data2=vhc->data3=0; vhc->VHDOnWater=0; return; }

  GrabBack(vhc,dir);
  }

void MoveSailboat(VEHICTYPE *vhc)
  {
  if (vhc->VHDOnWater) // OnWater flag
    BoatOnwMovm(vhc);
  else
    {
    MoveRVehic(vhc);
    if (Inside(GBackPix(vhc->x+6,vhc->y+8),CLiqL,CLiqH)) // Watering...
      if (Inside(GBackPix(vhc->x+6,vhc->y+9),CLiqL,CLiqH) || Inside(GBackPix(vhc->x+6,vhc->y+7),CLiqL,CLiqH))
	{ vhc->VHDOnWater=1; vhc->p=16; vhc->data=0; GrabBack(vhc,0); ClearMenTPtr(vhc); }
    }
  }

void ConBoat(VEHICTYPE *vhc)
  {
  if (Inside(vhc->p,0,VHPBOATL-1)) // Waiting for left -> move left
    { vhc->p=VHPBOATL; return; }
  if (vhc->p==VHPBOATL) // Moving left -> stop (and turn)
    { vhc->p=VHPBOATL+16; return; }
  if (Inside(vhc->p,VHPBOATL+1,VHPBOATR-1)) // Waiting for right -> move right
    { vhc->p=VHPBOATR; return; }
  if (vhc->p==VHPBOATR) // Moving right -> stop (and turn)
    { vhc->p=16; return; }
  }

//------------------------------- General -------------------------------------

BYTE ConVehic(VEHICTYPE *vhc, BYTE plr)
  {
  if (!NotHostile(plr,vhc->owner)) return 0;
  switch (vhc->type)
    {
    case VHELEVATOR: ConElevator(vhc); return 1;
    case VHSAILBOAT: if (vhc->VHDOnWater) { ConBoat(vhc); return 1; } else return 0;
    case VHBALLOON:  ConBalloon(vhc);  return 1;
    default: return 0; // Uncontrolable vehic
    }
  }

void ExecVehics(void) // Every Tick1
  {
  VEHICTYPE *cvhc=FirstVehic;
  VEHICTYPE *tptr;
  VehicCnt=0;
  while (cvhc)
    {
    // Automobiles
    switch (cvhc->type)
      {
      case VHLORRY:    MoveLorry(cvhc);    break;
      case VHCATAPULT: MoveCatapult(cvhc); break;
      case VHCROSSBOW: MoveCrossbow(cvhc); break;
      case VHSAILBOAT: MoveSailboat(cvhc); break;
      case VHBALLOON:  MoveBalloon1(cvhc); break;
      }
    // Deletion of removed vehics
    if (cvhc->type==VHNOVEHIC) { tptr=cvhc; cvhc=cvhc->next; DeleteVehic(tptr); }
    else { cvhc=cvhc->next; VehicCnt++; }
    }
  }

void RemoveVehic(VEHICTYPE *cvhc)
  {
  DropBack(cvhc);
  cvhc->type=VHNOVEHIC;
  }

