/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RW\D CLONK Battle Sequence PXS/Rock System Module

#include <stdlib.h>

#include <standard.h>
#include <vga4.h>
#include <std_gfx.h>

#include "clonk_bs.h"

#include "bsexcsys.h"
#include "bsmansys.h"
#include "bsgfxsys.h"
#include "bsvhcsys.h"
#include "bsstrsys.h"
#include "bsweasys.h"

#define VHDBallCrash  back[28]

//-------------------------- Main Global Externals ---------------------------

extern BSATYPE BSA;

extern char OSTR[500];

//-------------------- PXS Global Game Object Variables ------------------------

ROCKTYPE *FirstRock;

//-------------------------- (PXS System Variables) -----------------------------

PXSTYPE *PXSChunk[PXSMaxChunk];
PXSTYPE PXSCChunk1[PXSChunkSize];

PXSTYPE *MPXSCPtr;
int MPXSCChunk;

BYTE PXSSmokeOn;
BYTE PXSWtrOut;

//----------------------------- (PXS Control) -------------------------------------

void InitPXS(BYTE smokeon, BYTE wtrout)
  {
  int cnt;
  for (cnt=1; cnt<PXSMaxChunk; cnt++) PXSChunk[cnt]=NULL;
  PXSChunk[0]=PXSCChunk1;
  for (cnt=0; cnt<PXSChunkSize; cnt++) PXSChunk[0][cnt].type=PXSINACTIVE;

  MPXSCPtr=NULL; MPXSCChunk=-1;

  PXSSmokeOn=smokeon; PXSWtrOut=wtrout;
  }

void DeInitPXS(void)
  {
  int cnt;
  for (cnt=1; cnt<PXSMaxChunk; cnt++)
    if (PXSChunk[cnt])
      { delete [] PXSChunk[cnt]; PXSChunk[cnt]=0; }
  }

/*void ChunkMsg(void)
  {
  int cnt;
  OSTR[0]='=';
  for (cnt=0; cnt<PXSMaxChunk; cnt++) OSTR[cnt+1]=(PXSChunk[cnt]!=0) ? 'A' : '_';
  OSTR[PXSMaxChunk+1]=0;
  SystemMsg(OSTR);
  }*/

void PXSOut(BYTE type, int x, int y, int p)
  {
  int cnt,cnt2;
  PXSTYPE *pxp;

  if (!PXSSmokeOn && (type==PXSSMOKE)) return;
  if (LowFPS && (type==PXSSMOKE)) if (Rnd4()) return;

  for (cnt=0; cnt<PXSMaxChunk; cnt++)
    {
    if (!PXSChunk[cnt]) // New chunk
      {
      if (!(PXSChunk[cnt]=new PXSTYPE[PXSChunkSize]))
	{ /*SystemMsg("Insuf mem for new PXS chunk.");*/ LowMem=1; return; }
      for (cnt2=0,pxp=PXSChunk[cnt]; cnt2<PXSChunkSize; cnt2++,pxp++) pxp->type=PXSINACTIVE;
      //ChunkMsg();
      }                         // Check extng. chunk
    for (cnt2=0,pxp=PXSChunk[cnt]; cnt2<PXSChunkSize; cnt2++,pxp++)
      if (!pxp->type)
	{
	pxp->type=type; pxp->x=BoundBy(x,0,319); pxp->y=y; pxp->p=p;
	MPXSCPtr=pxp; MPXSCChunk=cnt;
	return;
	}
    }
  //SystemMsg("all PXS chunks full.");
  }

void MassPXSOut(BYTE type, int x, int y, int p)
  {
  // CPtr not set  or  CChunk not set  or  Chunk not extistant?
  if (!MPXSCPtr || (MPXSCChunk==-1) || (!PXSChunk[MPXSCChunk]))
    { PXSOut(type,x,y,p); return; }
  // Advanced CPtr beyond chunk size or next PXS not free?
  MPXSCPtr++;
  if ((MPXSCPtr>=PXSChunk[MPXSCChunk]+PXSChunkSize) || (MPXSCPtr->type))
    { PXSOut(type,x,y,p); return; }
  // Set PXS
  MPXSCPtr->type=type; MPXSCPtr->x=x; MPXSCPtr->y=y; MPXSCPtr->p=p;
  //LPage(CPGE); SOut("MASS",0,25,CRed,0); LPage(BackPage);
  }

inline BYTE PXSPixFree(int x, int y) // water is solid
  {
  return (!Inside(GBackPix(x,y),CSolid2L,CSolid2H));
  }

inline BYTE PXSPixFree2(int x, int y) // water is not solid
  {
  return (!Inside(GBackPix(x,y),CSolid2L,CSolidH));
  }

BYTE AcidReaction(int tx, int ty) // Returns 1 if acid has reacted
  {
  int cnt;
  BYTE bpix;

  if (random(2) || (ty>BackGrSize-7)) return 0; // No reaction

  for (cnt=0; cnt<2; cnt++) // Check own pixel and the one below
    {
    bpix=GBackPix(tx,ty+cnt);
    if (Inside(bpix,CEarth1,CWaterT)) break; // Earth,Ashes,Sand,Snow,Water

    if (ApplyHeat2Back(tx,ty+cnt,0)) return 1;
    }

  if (cnt==2) return 0;

  DrawBackPix(tx,ty+cnt);
  PXSOut(PXSSMOKE,tx,ty,random(20)+20);
  ReleasePXS(tx,ty);

  return 1;
  }

extern BYTE StructBurn;

extern BYTE Water2BurningStruct(int tx, int ty);

void MoveLiquid(PXSTYPE *pxp, BYTE liqtype) // 0:Water 1:Acid 2:Oil 3:Lava
  {
  int tx,ty;

  tx=pxp->x; ty=pxp->y+1;

  // Water affected by wind
  if (liqtype==0) if (Tick2) if (pxp->y<130)
    {
    tx+=Weather.wind/23;
    if (tx>319) if ((ty<Ground.levell) && (ty<Ground.levelr)) tx=0;
		else { pxp->type=PXSINACTIVE; return; }
    if (tx<0)   if ((ty<Ground.levelr) && (ty<Ground.levell)) tx=319;
		else { pxp->type=PXSINACTIVE; return; }
    }

  if ((ty<0) || (ty>=BackGrSize)) { pxp->type=PXSINACTIVE; return; }

  if (PXSPixFree(tx,ty)) { pxp->x=tx; pxp->y=ty; return; } // Free Move

  // Hit ground

  if (StructBurn && (liqtype==0)) // Water -> burning structs
    if (Water2BurningStruct(pxp->x,pxp->y))
      pxp->type=PXSINACTIVE;

  if (liqtype==1) // Acid hits ground
    if (AcidReaction(pxp->x,pxp->y))
      { pxp->type=PXSINACTIVE; return; }

  int cnt; // stinkin old code!!
  BYTE drp,chkl,chkr;
  drp=0; chkl=chkr=1;
  for (cnt=1; (chkl || chkr) && !drp; cnt++) // replace pxp->x with cdx!!!
    {
    if (chkl) if (pxp->x-cnt<0)
      if (PXSWtrOut || (pxp->y<Ground.levell))
	{ pxp->type=PXSINACTIVE; return; }
      else chkl=0;
    if (chkr) if (pxp->x+cnt>319)
      if (PXSWtrOut || (pxp->y<Ground.levelr))
	{ pxp->type=PXSINACTIVE; return; }
      else chkr=0;
    if (chkl)
      {
      if (!PXSPixFree(pxp->x-cnt,pxp->y)) chkl=0;
      if (PXSPixFree(pxp->x-cnt,ty)) { pxp->x-=cnt; pxp->y=ty; drp=1; }
      }
    if (chkr && !drp)
      {
      if (!PXSPixFree(pxp->x+cnt,pxp->y)) chkr=0;
      if (PXSPixFree(pxp->x+cnt,ty)) { pxp->x+=cnt; pxp->y=ty; drp=1; }
      }
    }           // ^^^optimize!!!

  if (drp) return;

  // Liquid is dead

  BYTE cbpix,deadcol;

  cbpix=GBackPix(pxp->x,pxp->y);

  switch (liqtype) // Contact reaction
    {
    case 0: // Water
      if (Inside(cbpix,CLavaS1,CLavaT3))
	{
	SBackPix(pxp->x,pxp->y,CAshes1+random(3));
	pxp->type=PXSSMOKE; if (!Tick10) if (!random(10)) GSTP=SNDPSHSH;
	return;
	}
      if (Inside(GBackPix(pxp->x,pxp->y+1),CLavaS1,CLavaT3))
	{ SBackPix(pxp->x,pxp->y+1,CAshes1+random(3)); pxp->type=PXSSMOKE; return; }
      break;
    case 2: // Oil
      if (Inside(cbpix,CLavaS1,CLavaT3) || Inside(GBackPix(pxp->x,pxp->y+1),CLavaS1,CLavaT3))
	{ pxp->type=PXSBURNOIL; return; }
      break;
    case 3: // Lava
      if (ApplyHeat2Back(pxp->x,pxp->y,1) || ApplyHeat2Back(pxp->x,pxp->y+1,1)
	|| ApplyHeat2Back(pxp->x-1,pxp->y,1) || ApplyHeat2Back(pxp->x+1,pxp->y+1,1))
	  {
	  pxp->type=PXSINACTIVE;
	  if (!Tick10) if (!random(10)) GSTP=SNDPSHSH;
	  return;
	  }
      break;
    }

  if (Inside(cbpix,CLiqL,CLiqH)) // Dead in liquid
    {
    pxp->y--; // -> wave back
    if (!PXSPixFree(pxp->x,pxp->y)) // hit ceiling -> shift left/right
      {
      if ((pxp->x>0)   && PXSPixFree(pxp->x-1,pxp->y)) { pxp->x--; return; }
      if ((pxp->x<319) && PXSPixFree(pxp->x+1,pxp->y)) { pxp->x++; return; }
      }
    return;
    }

  if (cbpix<CGranite1) // Dead on sky, tunnel, vehic (replace)
    {
    deadcol=IsSkyBack(cbpix) ? CLiqL+2*liqtype : CLiqL+2*liqtype+1;
    if (liqtype==3) deadcol+=2*random(3);
    SBackPix(pxp->x,pxp->y,deadcol);
    pxp->type=PXSINACTIVE;
    return;
    }

  // Dead in other solid (no replace)
  pxp->type=PXSINACTIVE;
  }

void MoveSnow(PXSTYPE *pxp)
  {
  int tx,ty;
  BYTE cbpix;
  tx=pxp->x; if (Tick2) if (pxp->y<130) tx+=Weather.wind/20;
  ty=pxp->y+1;
  if (tx>318) { if (Inside(GBackPix(  1,pxp->y),CSky1,CSky2)) pxp->x=1;   else pxp->type=PXSINACTIVE; return; }
  if (tx<1)   { if (Inside(GBackPix(318,pxp->y),CSky1,CSky2)) pxp->x=318; else pxp->type=PXSINACTIVE; return; }
  if ((ty<0) || (ty>=BackGrSize)) { pxp->type=PXSINACTIVE; return; }
  if (PXSPixFree(tx,ty)) { pxp->x=tx; pxp->y=ty; return; } // Free Move

  if (PXSPixFree(tx-1,ty)) { pxp->x=tx-1; pxp->y=ty; return; } // Horizontal
  if (PXSPixFree(tx+1,ty)) { pxp->x=tx+1; pxp->y=ty; return; } // slide

  // Is dead

  if (Inside(GBackPix(pxp->x,pxp->y+1),CLavaS1,CLavaT3)) { pxp->type=PXSWATER; return; }

  cbpix=GBackPix(pxp->x,pxp->y);
  if (cbpix<CGranite1) // Dead on sky, tunnel, vehic (replace)
    { SBackPix(pxp->x,pxp->y,IsSkyBack(cbpix) ? CSnowS : CSnowT); pxp->type=PXSINACTIVE; return; }
  pxp->type=PXSINACTIVE; // Lost dead on any other
  }

void MoveSand(PXSTYPE *pxp)
  {
  int cnt,tx,ty;
  BYTE cbpix;
  tx=pxp->x; ty=pxp->y+1;
  if ((tx>318) || (tx<1) || (ty<0) || (ty>=BackGrSize)) { pxp->type=PXSINACTIVE; return; }
  if (PXSPixFree2(tx,ty)) { pxp->x=tx; pxp->y=ty; return; } // Free Move

  if (PXSPixFree2(tx-1,ty)) { pxp->x=tx-1; pxp->y=ty; return; } // Horizontal
  if (PXSPixFree2(tx+1,ty)) { pxp->x=tx+1; pxp->y=ty; return; } // slide

  cbpix=GBackPix(pxp->x,pxp->y);
  if ((cbpix<CGranite1) || Inside(cbpix,CLiqL,CLiqH)) // Dead on sky/tunnel/vehic/liquid (replace)
    {
    if (pxp->p==0) SBackPix(pxp->x,pxp->y,IsSkyBack(cbpix) ? CSandS : CSandT);
    if (pxp->p==1) SBackPix(pxp->x,pxp->y,CEarth1+1+random(4));
    pxp->type=PXSINACTIVE;
    return;
    }
  if (Inside(cbpix,CGroundL,CGroundH)) // Dead in granite, rock, earth, etc.
    {				       // -> Shift up, max. 7
    if (pxp->y>10)
      for (cnt=0; cnt<7; cnt++)
	if (!Inside(GBackPix(pxp->x,pxp->y-cnt),CSolidL,CSolidH))
	  {
	  pxp->y--;
	  return;
	  }
    }
  pxp->type=PXSINACTIVE; // Lost dead on any other (what could that be?)
  }

void MoveConcrete(PXSTYPE *pxp)
  {
  int tx,ty;

  tx=pxp->x; ty=pxp->y+1;

  if ((tx>318) || (tx<1) || (ty<0) || (ty>=BackGrSize)) { pxp->type=PXSINACTIVE; return; }
  if (PXSPixFree2(tx,ty)) { pxp->x=tx; pxp->y=ty; return; } // Free Move

  int cnt; // stinkin old code!!
  BYTE drp,chkl,chkr;
  drp=0; chkl=chkr=1;
  for (cnt=1; (chkl || chkr) && !drp; cnt++) // replace pxp->x with cdx!!!
    {
    if (chkl) if (pxp->x-cnt<0)   { pxp->type=PXSINACTIVE; return; }
    if (chkr) if (pxp->x+cnt>319) { pxp->type=PXSINACTIVE; return; }
    if (chkl)
      {
      if (!PXSPixFree2(pxp->x-cnt,pxp->y)) chkl=0;
      if (PXSPixFree2(pxp->x-cnt,ty)) { pxp->x-=cnt; pxp->y=ty; drp=1; }
      }
    if (chkr && !drp)
      {
      if (!PXSPixFree2(pxp->x+cnt,pxp->y)) chkr=0;
      if (PXSPixFree2(pxp->x+cnt,ty)) { pxp->x+=cnt; pxp->y=ty; drp=1; }
      }
    }           // ^^^optimize!!!

  if (drp) return;

  BYTE cbpix;

  cbpix=GBackPix(pxp->x,pxp->y);
  if (Inside(cbpix,CGranite1,CRock2))    // Dead in rock
    {
    pxp->y--; // -> wave back
    if (!PXSPixFree2(pxp->x,pxp->y)) // hit ceiling -> shift left/right
      {
      if ((pxp->x>0)   && PXSPixFree2(pxp->x-1,pxp->y)) { pxp->x--; return; }
      if ((pxp->x<319) && PXSPixFree2(pxp->x+1,pxp->y)) { pxp->x++; return; }
      }
    return;
    }

  if (!Inside(cbpix,CGroundL,CGroundH)) // Dead not on ground (replace)
    {
    SBackPix(pxp->x,pxp->y,CRock1+Rnd3()+1-3*pxp->p);
    pxp->type=PXSINACTIVE;
    return;
    }

  // Dead on ground (no replace)
  pxp->type=PXSINACTIVE;
  }

void MoveSmoke(PXSTYPE *pxp)
  {
  int tx,ty;
  int scx1,scx2;
  pxp->p--; if (pxp->p<0) { pxp->type=PXSINACTIVE; return; }
  tx=pxp->x; ty=pxp->y-1;
  if (Tick2) { if (pxp->y<130) tx+=Weather.wind/18; } else tx+=Tick3-1;
  if ((tx<1) || (tx>318) || (ty<1) || (ty>=BackGrSize)) { pxp->type=PXSINACTIVE; return; }
  if (PXSPixFree2(tx,ty)) { pxp->x=tx; pxp->y=ty; return; } // Free move
  scx1=scx2=pxp->x;
  while ((scx1>1) || (scx2<318)) // Horizontal slide check
    {
    if (scx1>1)
      {
      scx1--;
      if (!PXSPixFree2(scx1,pxp->y)) scx1=0;
      else if (PXSPixFree2(scx1,ty)) { tx=scx1; scx1=0; }
      }
    if (scx2<318)
      {
      scx2++;
      if (!PXSPixFree2(scx2,pxp->y)) scx2=319;
      else if (PXSPixFree2(scx2,ty)) { tx=scx2; scx2=319; }
      }
    }
  if (PXSPixFree2(tx,ty)) { pxp->x=tx; pxp->y=ty; }
  }

void MoveZap(PXSTYPE *pxp)
  {
  int tx,ty,cnt;
  pxp->p--; if (pxp->p<1) pxp->type=PXSINACTIVE;
  tx=BoundBy(pxp->x+random(3)-1,0,319);
  ty=BoundBy(pxp->y+random(3)-1,0,BackGrSize-5);
  if (!Tick3 && !Rnd3())
    CheckClonkSting(pxp->x,pxp->y);
  if (PXSPixFree(tx,ty)) { pxp->x=tx; pxp->y=ty; return; } // Free move
  if (!PXSPixFree(pxp->x,pxp->y)) pxp->type=PXSINACTIVE;
  }

void MoveBubble(PXSTYPE *pxp)
  {
  if (Rnd3()) pxp->y--;
  pxp->x+=Rnd3();
  if (!Inside(pxp->y,0,BackGrSize-1) || !Inside(pxp->x,0,319))
    { pxp->type=PXSINACTIVE; return; }
  if (!Inside(GBackPix(pxp->x,pxp->y),CWaterS,CWaterT))
    { pxp->type=PXSINACTIVE; return; }
  }

void MoveBurnOil(PXSTYPE *pxp)
  {
  // Extinguish if water/oil above and left/right
  if (pxp->y>0)
    if (Inside(GBackPix(pxp->x,pxp->y-1),CWaterS,COilT))
      {
      SBackPix(pxp->x,pxp->y,IsSkyBack(GBackPix(pxp->x,pxp->y)) ? COilS : COilT);
      pxp->type=PXSINACTIVE;
      ApplyHeat2Back(pxp->x,pxp->y-1,0);
      ReleasePXS(pxp->x,pxp->y); // avoid oil walls
      return;
      }
  // Ignite left
  if (pxp->x>0) ApplyHeat2Back(pxp->x-1,pxp->y,0);
  // Ignite right
  if (pxp->x<319) ApplyHeat2Back(pxp->x+1,pxp->y,0);
  // Fall if free below
  if (PXSPixFree(pxp->x,pxp->y+1))
    { ReleasePXS(pxp->x,pxp->y-1); ReleasePXS(pxp->x-1,pxp->y); ReleasePXS(pxp->x+1,pxp->y); pxp->y++; }
  // Burn phase
  pxp->p--;
  if (pxp->p<0) // Die and ignite bottom
    {
    pxp->type=PXSINACTIVE;
    ReleasePXS(pxp->x,pxp->y-1); ReleasePXS(pxp->x-1,pxp->y); ReleasePXS(pxp->x+1,pxp->y);
    ApplyHeat2Back(pxp->x,pxp->y+1,0);
    }
  if (!Tick3  && !Rnd3()) PXSOut(PXSSMOKE,pxp->x,pxp->y-1,20+random(30));
  if (!Tick20 && !Rnd3()) CheckManIgnition(pxp->x,pxp->y);
  if (!Tick50 && !Rnd3()) CheckStructIgnition(pxp->x,pxp->y);
  }

void MoveLava(PXSTYPE *pxp)
  {
  MoveLiquid(pxp,3);
  if (!Tick3) if (pxp->type==PXSLAVA) if (!Rnd4())
    PXSOut(PXSSMOKE,pxp->x,pxp->y,20+random(30));
  if (!Ground.volcano.act) pxp->type=PXSINACTIVE;
  }

void ExecSpark(PXSTYPE *pxp)
  {
  ApplyHeat2Back(pxp->x,pxp->y,0);
  pxp->type=PXSINACTIVE;
  }

void ExecLiqDrain(PXSTYPE *pxp)
  {
  int cnt,cnt2,trnsf;
  BYTE pout;

  if (!Inside(GBackPix(pxp->x,pxp->y),CLiqL,CLiqH))
    { pxp->type=PXSINACTIVE; return; } // Death (ceased)

  for (cnt2=1; cnt2>=0; cnt2--)
    for (cnt=-1; cnt<=+1; cnt++) if ((cnt2==1) || (cnt!=0))
      if (Inside(pxp->x+cnt,0,319))
	if (PXSPixFree(pxp->x+cnt,pxp->y+cnt2))
	  {
	  trnsf=ExtractLiquid(pxp->x,pxp->y);
	  if (trnsf==-1) { RoundError("safety: trnsf should be liq"); return; }
	  PXSOut(LiqRelClassM[trnsf],pxp->x+cnt,pxp->y+cnt2,0);
	  return;
	  }

  pxp->type=PXSINACTIVE; // Death (clogged)
  }

void ExecPXS(void)
  {
  int cnt,cnt2,cnt3;
  PXSTYPE *pxp;
  BYTE chkempty,zapsout=0;
  PXSCnt=0;
  for (cnt=0; cnt<PXSMaxChunk; cnt++)
    if (PXSChunk[cnt])
      {
      chkempty=1;
      for (cnt2=0,pxp=PXSChunk[cnt]; cnt2<PXSChunkSize; cnt2++,pxp++)
	if (pxp->type)
	  {
	  switch (pxp->type)
	    {
	    case PXSWATER: MoveLiquid(pxp,0); break;
	    case PXSSMOKE: MoveSmoke(pxp); break;
	    case PXSMASSWATER: for (cnt3=0; (cnt3<5) && pxp->type; cnt3++) MoveLiquid(pxp,0); break;
	    case PXSSNOW: if (Rnd3()) MoveSnow(pxp); break;
	    case PXSZAP: MoveZap(pxp); zapsout=1; break;
	    case PXSSAND: if (Rnd3()) MoveSand(pxp);  break;
	    case PXSBUBBLE: MoveBubble(pxp); break;
	    case PXSOIL: MoveLiquid(pxp,2); break;
	    case PXSMASSOIL: for (cnt3=0; (cnt3<4) && pxp->type; cnt3++) MoveLiquid(pxp,2); break;
	    case PXSCONCRETE: MoveConcrete(pxp); break;
	    case PXSBURNOIL: MoveBurnOil(pxp); break;
	    case PXSLAVA: MoveLava(pxp); break;
	    case PXSMASSLAVA: for (cnt3=0; (cnt3<4) && pxp->type; cnt3++) MoveLava(pxp); break;
	    case PXSACID: MoveLiquid(pxp,1); break;
	    case PXSMASSACID: for (cnt3=0; (cnt3<4) && pxp->type; cnt3++) MoveLiquid(pxp,1); break;
	    case PXSSPARK: ExecSpark(pxp); break;
	    case PXSLIQDRAIN: ExecLiqDrain(pxp); break;
	    }
	  chkempty=0;
	  PXSCnt++;
	  }
      if (chkempty && (cnt>0)) // Delete chunk
	{
	delete [] PXSChunk[cnt]; PXSChunk[cnt]=0;
	//ChunkMsg();
	}
      }
  if (zapsout && !Tick3) if (!random(100)) GSTP=SNDZAP;
  }

extern long BackVPos; // does this have to be?

void DrawSinglePXS(PXSTYPE *pxp)
  {
  const BYTE PXSCol[PXSTypeNum]={0,CWaterS,0,CWaterS,CSnowS,CZap,CSandS,43,COilS,COilS,0,63,CLavaS1,CLavaS1,CAcidS,CAcidS,CLightning,226};

  int cnt3;
  BYTE bcol;

  if (pxp->type==PXSINACTIVE) return;

  if (Inside(pxp->y-BackVPos,0,159))
    switch (pxp->type)
      {
      case PXSSMOKE:
	bcol=CSmoke+pxp->p/10;
	for (cnt3=0; cnt3<4; cnt3++)
	  SPixF(pxp->x+Rnd3(),pxp->y-BackVPos+Rnd3()-1+20,bcol+random(2));
	break;
      case PXSBURNOIL:
	if (!Rnd3()) DrawFireSpot(pxp->x,pxp->y);
	break;
      case PXSLAVA: case PXSMASSLAVA:
	SPixA(pxp->x,pxp->y-BackVPos+20,CLavaS1+2*(Rnd3()+1));
	break;
      case PXSSAND:
	SPixA(pxp->x,pxp->y-BackVPos+20,pxp->p ? CEarth1+1+random(4) : CSandS);
	break;
      case PXSCONCRETE:
	SPixA(pxp->x,pxp->y-BackVPos+20,pxp->p ? CGranite1 : CRock1+1);
	break;
      case PXSLIQDRAIN: break;
	default:
	SPixA(pxp->x,pxp->y-BackVPos+20,PXSCol[pxp->type]);
	break;
      }
  }

void DrawPXS(void)
  {
  int cnt,cnt2;
  PXSTYPE *pxp;
  for (cnt=0; cnt<PXSMaxChunk; cnt++)
    if (PXSChunk[cnt])
      for (cnt2=0,pxp=PXSChunk[cnt]; cnt2<PXSChunkSize; cnt2++,pxp++)
	DrawSinglePXS(pxp);
  }

//--------------------------- (Special FX Control) ---------------------------

void ZapNest(int znx, int zny, BYTE iszup)
  {
  int cnt,tx,ty;
  for (cnt=0; cnt<75-50*iszup; cnt++)
    {
    tx=BoundBy(znx+random(11)-5,0,319); ty=BoundBy(zny+random(11)-5,0,BackGrSize-5);
    if (PXSPixFree(tx,ty)) PXSOut(PXSZAP,tx,ty,500+random(200));
    }
  GSTP=SNDZAP;
  }

void PourPXS(BYTE type, int tx, int ty, int phase, int amt, int rad)
  {
  int cnt;
  for (cnt=0; cnt<amt; cnt++)
    PXSOut(type,tx+random(2*rad)-rad,ty+random(2*rad)-rad,phase);
  }

//----------------------------- (Rock Control) -------------------------------

void HomeRock(int rtype, int rphase, int plr)
  {
  int cnt,wgain;
  BYTE notord=1;
  if ((rtype==NOROCK) || !Inside(plr,0,2)) return;
  if (rtype==FLAG) { BSA.Plr[rphase].Eliminate=1; Crew[rphase].RedrStB=1; }
  wgain=RockValue[rtype];
  if (Inside(rtype,ARROW,BARROW)) wgain=RockValue[rtype]*rphase/5;
  Crew[plr].Wealth=Min(Crew[plr].Wealth+wgain,999);
  if (Crew[plr].CMenu==CMRCKORDER) Crew[plr].RedrStB=1;

  if (Inside(rtype,WATBARREL,OILBARREL)) rtype=BARREL;
  for (cnt=0; cnt<RockOrderNum; cnt++)
    if (rtype==RockOrder[cnt]) // Orderable, regain RckProd
      {
      if (!Inside(rtype,ARROW,BARROW) || (rphase==5)) // Arrows full only
	if (Crew[plr].RckProd.Store[cnt]<15)
	  Crew[plr].RckProd.Store[cnt]++;
      notord=0;
      }
  if (notord) ScoreGain(plr,wgain);
  }

BYTE NewRock(int x, int y, int act, int type, int xdir, int ydir, int phase, int thrby)
  {
  ROCKTYPE *nrck;
  if (!(nrck=new ROCKTYPE)) { LowMem=1; return 0; }
  nrck->x=x; nrck->y=y;
  nrck->act=act; nrck->type=type;
  nrck->xdir=xdir; nrck->ydir=ydir;
  nrck->phase=phase;
  nrck->thrby=thrby;
  nrck->next=FirstRock;
  FirstRock=nrck;
  return 1;
  }

void DeleteRock(ROCKTYPE *trck)
  {
  ROCKTYPE *crck=FirstRock,*prev=NULL;
  if (!trck) return;
  ClearMenTPtr(trck);
  while (crck)
    {
    if (crck->next==trck) { prev=crck; break; }
    crck=crck->next;
    }
  if (trck==FirstRock) FirstRock=trck->next;
  if (prev) prev->next=trck->next;
  delete trck;
  }

void DeleteRocks(void)
  {
  ROCKTYPE *temptr;
  while (FirstRock) { temptr=FirstRock->next; delete FirstRock; FirstRock=temptr; }
  }

void FirstContact(ROCKTYPE *crck)
  {
  switch (crck->type)
    {
    case FLINT: case COMET:
      Explode(crck->x,crck->y,(crck->type==FLINT) ? BSA.FlintRadius : 10+random(5),crck->thrby);
      crck->type=NOROCK;
      break;
    case ROCKPXS:
      PXSOut(crck->phase,crck->x+1,crck->y+3,crck->thrby);
      crck->type=NOROCK;
      break;
    }
  }

void HitContact(ROCKTYPE *crck)
  {
  BYTE ltype;
  switch (crck->type)
    {
    case NOROCK:
      break;
    case FLINT: case COMET: case ROCKPXS:
      FirstContact(crck);
      break;
    case FBOMB:
      PourPXS(PXSBURNOIL,crck->x+1,crck->y+1,100+random(50),30,12);
      crck->type=NOROCK;
      GSTP=SNDFIRE;
      break;
    case MONSTEGG:
      crck->phase=10;
      crck->act=RAROLL; GSTP=SNDROCK;
      break;
    case ZAPN: case ZUPN:
      ZapNest(crck->x+1,crck->y+1,crck->type-ZAPN);
      crck->type=NOROCK;
      break;
    case LIQROCK: case LIQGRAN:
      PourPXS(PXSCONCRETE,crck->x+1,crck->y+1,(crck->type==LIQGRAN),30,3);
      crck->type=NOROCK;
      break;
    case LIQEARTH:
      PourPXS(PXSSAND,crck->x+1,crck->y+1,1,30,3);
      crck->type=NOROCK;
      break;
    case WATBARREL: case ACDBARREL: case OILBARREL:
      ltype=PXSWATER; if (crck->type==ACDBARREL) ltype=PXSACID; if (crck->type==OILBARREL) ltype=PXSOIL;
      PourPXS(ltype,crck->x+1,crck->y+1,0,15,2);
      crck->type=BARREL;
      break;
   case BARROW:
     Explode(crck->x+1,crck->y+1,5,crck->thrby);
     crck->type=NOROCK;
     break;
   default: // ROCK,GOLD,LOAM,TFLINT,STEEL,CONKIT,LINECON,PLANTs,ARROW,FARROW
     crck->act=RAROLL;
     if (Inside(crck->type,ARROW,FARROW)) GSTP=SNDFIGHT1;
     else if (crck->type<PLANT1) GSTP=SNDROCK;
     break;
   }
 }

extern void BlowUpBalloon(VEHICTYPE *tvhc);

extern MANTYPE *NewAnimal(MANTYPE **firstman, BYTE type, int x, int y, int tx, BYTE act, int phase, int strn, int ydir, int xdir, int carr, void *tptr);

void MoveRocks(void)
  {
  const int rolltox[4]={-1,1,-2,2};
  int cnt,ctco,stnpix;
  ROCKTYPE *crck=FirstRock,*rptr;
  VEHICTYPE *tvhc;
  BYTE hitgrnd,mvd;
  RockCnt=0;
  while (crck)
    {
    if (crck->type!=NOROCK)
      switch (crck->act)
	{
	case RAFLY: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	  // horizontal movement
	  ctco=crck->x+crck->xdir/10;
	  // out on side?
	  if (BSA.Realism.RckOut)
	    if (!Inside(ctco,4,312) && (Abs(crck->xdir)>10))
	      { crck->type=NOROCK; break; }
	  ctco=BoundBy(ctco,4,312);
	  while (crck->x!=ctco)
	    {
	    if (crck->x<ctco) crck->x++; else crck->x--;
	    if (Inside(GBackPix(crck->x-1,crck->y+1),CSolidL,CSolidH))
	      { ctco=crck->x; crck->xdir=0; crck->ydir=35; FirstContact(crck); }
	    if (Inside(GBackPix(crck->x+4,crck->y+1),CSolidL,CSolidH))
	      { ctco=crck->x; crck->xdir=0; crck->ydir=35; FirstContact(crck); }
	    }
	  // vertical movement
	  crck->ydir+=2;
	  ctco=BoundBy(crck->y+(crck->ydir-20)/10,-15,BackGrSize-5); // slow!
	  hitgrnd=0;
	  while ((crck->y!=ctco) && (crck->type!=NOROCK) && !hitgrnd)
	    {
	    if (crck->y<ctco) crck->y++; else crck->y--;
	    if (Inside(GBackPix(crck->x+1,crck->y-1),CSolidL,CSolidH))
	      { ctco=crck->y; crck->ydir=35; FirstContact(crck); }
	    hitgrnd=0;
	    if ((Inside(GBackPix(crck->x,crck->y+4),CSolidL,CSolidH)) || (Inside(GBackPix(crck->x+3,crck->y+4),CSolidL,CSolidH))) hitgrnd=1;

	    if (hitgrnd) HitContact(crck);

	    }
	  // Lorry/Balloon checking
	  if (crck->type!=NOROCK)
	    {
	    for (tvhc=FirstVehic; tvhc; tvhc=tvhc->next)
	      {
	      if (tvhc->type==VHLORRY) if (tvhc->x>=0)
		if (Inside(crck->x-tvhc->x,-1,9) && Inside(crck->y-tvhc->y,-1,2))
		  {
		  switch (crck->type) // Hit lorry
		    {
		    case FLINT: case TFLINT:
		      Explode(crck->x,crck->y,BSA.FlintRadius-2*(crck->type==TFLINT),crck->thrby);
		      break;
		    case ZAPN: case ZUPN:
		      ZapNest(crck->x+1,crck->y+1,crck->type-ZAPN);
		      break;
                    case ARROW: case FARROW: case BARROW: case FLAG: case ROCKPXS:
		      break;
		    case MONSTEGG:
		      if (!random(2))
			{
			NewAnimal(&FirstAnimal,MNMONSTER,crck->x-5,crck->y-10,crck->x-5+random(21)-10,MAWALK,0,100,0,0,-1,NULL);
			GSTP=SNDMONSTER;
			}
		      break;
		    default: // ROCK,GOLD,LOAM,CONKIT,LIQROCK,LINECON,PLANTs
		      if (!Inside(crck->type,PLANT1,PLANT3)) GSTP=SNDCLONK;
		      if (tvhc->back[crck->type]<255) tvhc->back[crck->type]++;
		      break;
		    }
		  crck->type=NOROCK;
		  break;
		  }
	      if (tvhc->type==VHBALLOON) if (Inside(crck->type,ARROW,BARROW))
		if (tvhc->y>-10)
		  if (Inside(crck->x-tvhc->x,4,12) && Inside(crck->y-tvhc->y,0,12))
		    {
		    switch (crck->type)
		      {
		      case ARROW: tvhc->VHDBallCrash=1; break;
		      case FARROW: tvhc->VHDBallCrash=2; break;
		      case BARROW: BlowUpBalloon(tvhc); break;
		      }
		    crck->type=NOROCK;
		    if (Inside(crck->thrby,0,2)) if (!NotHostile(tvhc->owner,crck->thrby))
		      ScoreGain(crck->thrby,25);
		    }
	      }
	    }
	  // Failsafe boundary
	  crck->x=BoundBy(crck->x,4,312);
	  break;
	case RAROLL: // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	  if (Inside(GBackPix(crck->x+2,crck->y+4),CSolidL,CSolidH))
	    {
	    for (cnt=0; cnt<4; cnt++)
	      if (Inside(crck->x+2+rolltox[cnt],4,312))
		if (!Inside(GBackPix(crck->x+2+rolltox[cnt],crck->y+4),CSolidL,CSolidH))
		  { crck->x+=rolltox[cnt]; break; }
	    if (cnt==4) crck->act=RADEAD;
	    }
	  else
	    crck->y++;
	  // Failsafe boundary
	  crck->x=BoundBy(crck->x,4,312);
	  if ((crck->x==4) || (crck->x==312)) crck->act=RADEAD;
	  break;
	case RADEAD: // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	  mvd=0;
	  // Shift up if on VehicSolid
	  if (Inside(GBackPix(crck->x+1,crck->y+3),CVhcL,CVhcH)) { crck->y--; mvd=1; }
	  // Shift down
	  if (!Inside(GBackPix(crck->x+1,crck->y+4),CSolidL,CSolidH)) { crck->y++; mvd=1; }
	  // OnVehic movement
	  stnpix=GBackPix(crck->x+1,crck->y+4);
	  if (Inside(stnpix,CVhcL,CVhcH))
	    { stnpix-=CVhcL; stnpix>>=1; stnpix--; crck->x+=stnpix; mvd=1; }
	  // Failsafe boundary
	  crck->x=BoundBy(crck->x,4,312);
	  // Homing check
	  if (crck->y<-3) if (!BSA.Realism.CstHome)
	    {
	    tvhc=OnWhichVehic(crck->x+1,crck->y+1,NULL);
	    if (!tvhc) RoundError("safety: Rock out w/out vehic");
	    else if (tvhc->owner<0) RoundError("safety: Rock out on vehic w/out owner");
	      else if (!( (crck->type==FLAG) && (crck->phase==tvhc->owner) ))
		{
		HomeRock(crck->type,crck->phase,tvhc->owner);
		crck->type=NOROCK;
		}
	    }
	  // Movement Reaction
	  if (mvd)
	    {
	    // Zup nest checking
	    if (crck->type==ZUPN) { ZapNest(crck->x+1,crck->y+1,1); crck->type=NOROCK; }
	    // Plant
	    if (Inside(crck->type,PLANT1,PLANT3))
	      {
	      crck->phase=0;
	      crck->type=BoundBy(PLANT1+crck->phase/10,PLANT1,PLANT3);
	      }
	    // Monster egg
	    if (crck->phase<10) crck->phase++;
	    }
	  // Barrels fill
	  if (!Tick20)
	    if (crck->type==BARREL)
	      for (cnt=0; cnt<15; cnt++)
		{
		stnpix=ExtractLiquid(crck->x+1,crck->y+2);
		if (Inside(stnpix,0,2)) crck->type=WATBARREL+stnpix;
		else break; // abort lava extraction, little lava is still extracted
		}

	  // Plant checking
	  if (!Tick50)
	    if (Inside(crck->type,PLANT1,PLANT3))
	      if (Inside(GBackPix(crck->x+1,crck->y+3),CWaterS,CWaterT)
	       && Inside(GBackPix(crck->x+1,crck->y+4),CEarth1,CAshes2)
		&& !Inside(GBackPix(crck->x+1,crck->y+0),CSolidL,CSolid2H))
		  { // Grow, conditions met
		  /*ExtractLiquid(crck->x+1,crck->y+3);
		  if (crck->phase<110-(crck->x%30)) crck->phase++;
		  if (Inside(GBackPix(crck->x+1,crck->y+0),CSky1,CSky2))
		    if (crck->phase<110-(crck->x%30)) crck->phase++; // Double if outside
		  crck->type=BoundBy(PLANT1+crck->phase/10,PLANT1,PLANT3);
		  // Reproduction/spread seed
		  if (Sec5) if (crck->phase>80) if (!random(3))
		    {                  // only if not too crowded already!!
		    crck->phase-=30;
		    NewRock(BoundBy(crck->x-5+10*random(2),4,312),crck->y-4,RADEAD,PLANT1,0,0,0,-1);
		    }
		  */
		  }
	      else
		  { // Else degenerate to standard minimum size (Tick50)
		  crck->phase+=Sign(5-crck->phase);
		  crck->type=BoundBy(PLANT1+crck->phase/10,PLANT1,PLANT3);
		  }
	  break;
	}
    // TFLINT phasing
    if ((crck->type==TFLINT) && (crck->phase>0))
      {
      crck->phase--; if (!Tick3) PXSOut(PXSSMOKE,crck->x+Rnd3(),crck->y,10+random(20));
      if (crck->phase<1) { Explode(crck->x,crck->y,BSA.FlintRadius-2,crck->thrby); crck->type=NOROCK; }
      }
    // FARROW phasing
    if ((crck->type==FARROW) && (crck->phase>=10))
      {
      crck->phase--; if (!Tick2) PXSOut(PXSSMOKE,crck->x+Rnd3(),crck->y,10+random(20));
      ApplyHeat2Back(crck->x+1,crck->y+1,0);
      if (!Tick20 && !Rnd3()) CheckManIgnition(crck->x+1,crck->y);
      if (!Tick50 && !Rnd3()) CheckStructIgnition(crck->x+1,crck->y);
      if (crck->phase<=10) crck->type=NOROCK;
      if (Inside(GBackPix(crck->x+1,crck->y+2),CWaterS,CWaterT))
	{ crck->phase=1; GSTP=SNDPSHSH; }
      }
    // MONSTEGG phasing
    if (crck->type==MONSTEGG)
      {
      if (crck->phase<10) if (crck->phase>3)
	{ if (Sec1) if (!random(20)) crck->phase++; }
      if (crck->phase==10)
	if (!Inside(GBackPix(crck->x+2,crck->y-1),CSolidL,CSolidH))
	  { crck->phase++; GSTP=SNDKNURPS; }
      if (crck->phase>10)
	{
	crck->phase++;
	if (crck->phase>49)
	  {
	  NewAnimal(&FirstAnimal,MNMONSTER,crck->x-5,crck->y-10,crck->x+10,MAWALK,0,100,1,0,-1,NULL);
	  crck->type=NOROCK;
	  }
	}
      }
    // Comet trail
    if (crck->type==COMET)
      for (cnt=0; cnt<5; cnt++)
	PXSOut(PXSSMOKE,crck->x+Rnd3(),crck->y-cnt*2,10+random(20));
    // ROCKPXS safety
    if (crck->type==ROCKPXS)
      if (crck->act!=RAFLY)
	{
	crck->type=NOROCK;
	RoundError("Safety: ROCKPXS should fly only");
	}
    // Rock destroyed?
    if (crck->type==NOROCK) { rptr=crck; crck=crck->next; DeleteRock(rptr); }
    else { crck=crck->next; RockCnt++; }
    }
  }

void RockOutAccount(int *gold, int *oil)
  {
  ROCKTYPE *crck;
  VEHICTYPE *cvhc;
  MANTYPE *cman;
  int plr;

  *gold=0; *oil=0;

  for (crck=FirstRock; crck; crck=crck->next)
    {
    if (crck->type==GOLD) (*gold)++;
    if (crck->type==OILBARREL) (*oil)++;
    }

  for (cvhc=FirstVehic; cvhc; cvhc=cvhc->next)
    if (cvhc->type==VHLORRY)
      {
      (*gold)+=cvhc->back[GOLD];
      (*oil)+=cvhc->back[OILBARREL];
      }

  for (plr=0; plr<3; plr++)
    for (cman=Crew[plr].FirstMan; cman; cman=cman->next)
      if (cman->act<MADEAD)
	{
	if (cman->carr==GOLD) (*gold)++;
	if (cman->carr==OILBARREL) (*oil)++;
	}
  }

void BlowPXS(int amt, int type, int phase, int fx, int fy, int lvl)
  {
  int cnt;
  for (cnt=0; cnt<amt; cnt++)
    NewRock(fx+random(10)-5,fy+random(10)-5,RAFLY,ROCKPXS,random(lvl*3.0)-lvl*1.5,20-random(lvl*2.0),type,phase);
  }
