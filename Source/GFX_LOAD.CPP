/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

#include <stdio.h>

#include <standard.h>
#include <vga4.h>
#include <std_gfx.h>
#include <stdfile.h>

//-----------------------------------------------------------------------------
// RedWolf Design  AGC-Loader/Saver  by M.Bender    [LOADS TO VGA4PAGE-PAGES]
//------------------------------------------------------------------------------

typedef struct AGCHEADER { char id[3];
			   BYTE ver[2];
			   WORD wdt,hgt;
			   BYTE cpal[768];
			 };

char *AGCError(BYTE errnum)
  {
  switch (errnum)
    {
    case 0: return "Kein Fehler";
    case 1: return "Falsche AGC-Version";
    case 2: return "Datei nicht gefunden oder nicht genug Speicher";
    case 3: return "Datei-Lesefehler";
    case 4: return "Datei ist keine AGC";
    case 5: return "Kann Datei nicht erstellen oder nicht genug Speicher";
    case 6: return "Datei-Schreibfehler";
    case 7: return "Nicht genug Speicher";
    default: return "Unbekannter Fehlercode";
    }
  }

/*char *AGCError(BYTE errnum)
  {
  switch (errnum)
    {
    case 0: return "No error";
    case 1: return "Incorrect AGC version";
    case 2: return "File not found or insufficient memory";
    case 3: return "File loading error";
    case 4: return "Not an AGC file";
    case 5: return "Cannot create file or insufficient memory";
    case 6: return "File saving error";
    case 7: return "Insufficient memory";
    default: return "Undefined error code";
    }
  }*/

BYTE LoadAGC2PageV1(char *fname, BYTE tpge, int tx, int ty, WORD *iwdt, WORD *ihgt, BYTE *palptr, BYTE *scram)
  { // File Version 1.0  Loader Version May 96
  AGCHEADER head;
  BYTE *fbuf,sfbuf,*scrptr=scram;
  WORD xcnt,ycnt;
  if (InitBFI(fname)) return 2;
  if (GetBFI(&head,sizeof(AGCHEADER))) return 3;
  if (!SEqual(head.id,"AGC")) { DeInitBFI(); return 4; }
  if (head.ver[0]*10+head.ver[1]>10) { DeInitBFI(); return 1; }
  LPage(tpge);
  for (ycnt=0; ycnt<head.hgt; ycnt++) for (xcnt=0; xcnt<head.wdt; xcnt++)
    {
    if (GetBFI(&sfbuf)) return 3;
    if (scrptr && *scrptr) { SPixF(tx+xcnt,ty+ycnt,sfbuf ^ *scrptr); scrptr++; if (!(*scrptr)) scrptr=scram; }
    else SPixF(tx+xcnt,ty+ycnt,sfbuf);
    }
  DeInitBFI();
  if (palptr) MemCopy(head.cpal,palptr,768); else SetDAC(0,256,head.cpal);
  if (iwdt) *iwdt=head.wdt; if (ihgt) *ihgt=head.hgt;
  return 0;
  }

BYTE SaveAGCPageV1(char *fname, BYTE fpge, int fx, int fy, WORD wdt, WORD hgt, BYTE *palptr, BYTE *scram)
  { // File Version 1.0  Loader Version May 96
  AGCHEADER head;
  BYTE sbuf,*scrptr=scram;
  WORD xcnt,ycnt;
  SCopy("AGC",head.id,3); head.ver[0]=1; head.ver[1]=0;
  head.wdt=wdt; head.hgt=hgt;
  if (palptr) MemCopy(palptr,head.cpal,768); else GetDAC(0,256,head.cpal);
  if (InitBFO(fname)) return 5;
  if (PutBFO(&head,sizeof(AGCHEADER))) return 6;
  LPage(fpge);
  for (ycnt=0; ycnt<hgt; ycnt++) for (xcnt=0; xcnt<wdt; xcnt++)
    {
    if (scrptr && *scrptr) { sbuf=GPixF(fx+xcnt,fy+ycnt) ^ *scrptr; scrptr++; if (!(*scrptr)) scrptr=scram; }
    else sbuf=GPixF(fx+xcnt,fy+ycnt);
    if (PutBFO(&sbuf)) return 6;
    }
  DeInitBFO();
  return 0;
  }

//------------------------------------------------------------------------------
// LoadPCX 1.0 Dec 1994  RedWolf Design
//         1.1 Feb 1996                              [LOADS TO VGA4PGE-PAGES]
//         1.2 May 1996
//------------------------------------------------------------------------------

typedef struct PCXHEADTYPE { BYTE manf,vers,encd,bppx;
			     WORD imx,imy,wdt,hgt,hres,vres;
			     BYTE egapal[48];
			     BYTE resv,plns;
			     WORD bpln,palt;
			     WORD scrw,scrh;
			     BYTE rest[54];
			   };

char *LoadPCXError(BYTE errn)
  {
  switch (errn)
    {
    case 0: return "Kein Fehler";
    case 1: return "Datei nicht gefunden";
    case 2: return "Datei-Lesefehler";
    case 3: return "Datei ist kein PCX";
    case 4: return "Bild ist zu gross (maximal 320x200)";
    case 5: return "Nicht genug Speicher";
    default: return "Unbekannter Fehlercode";
    }
  }

BYTE LoadPCX(char *fname, BYTE tpge, int tx, int ty, COLOR *tpal)
  {
  PCXHEADTYPE head;
  COLOR cpal[256];
  DWRD cnt,iwdt,ihgt;
  WORD rlen,itx=0,ity=0;
  BYTE fbuf,bfir;
  bfir=InitBFI(fname); if (bfir==1) return 5; if (bfir==2) return 1;
  if (GetBFI(&head,sizeof(PCXHEADTYPE))) return 2;
  iwdt=head.wdt+1; ihgt=head.hgt+1;
  if (head.manf!=10) { DeInitBFI(); return 3; }
  if ((iwdt>320) || (ihgt>200)) { DeInitBFI(); return 4; }
  LPage(tpge);
  cnt=0;
  while (cnt<iwdt*ihgt)
    {
    if (GetBFI(&fbuf)) return 2;
    if (Inside(fbuf,192,255))
      {
      rlen=fbuf-192;
      if (GetBFI(&fbuf)) return 2;
      while (rlen>0)
	{
	SPixF(tx+itx,ty+ity,fbuf);
	itx++; if (itx>=iwdt) { itx=0; ity++; }
	rlen--; cnt++;
	}
      }
    else
      {
      SPixF(tx+itx,ty+ity,fbuf);
      itx++; if (itx>=iwdt) { itx=0; ity++; }
      cnt++;
      }
    }
  if (GetBFI(&fbuf)) return 2; // 0CH CPAL ID
  if (GetBFI(&cpal,768)) return 2;
  for (cnt=0; cnt<768; cnt++) cpal[cnt/3].RGB[cnt%3]>>=2;
  if (tpal) MemCopy((BYTE*)cpal,(BYTE*)tpal,768);
  else SetDAC(0,256,cpal);
  DeInitBFI();
  return 0;
  }