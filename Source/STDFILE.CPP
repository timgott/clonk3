/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design STANDARD FILE ACCESS Library

// Version: 1.0 May 1996 (DOSFileAccess,BFIO,PathFunctions,FileSearch)


#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include "standard.h"

//------------------------- Path & General Functions --------------------------------

void EnforceExtension(char *fname, char *xtn) // Period is to be part of xtn
  {
  while (*fname && (*fname!=46)) fname++;
  SCopy(xtn,fname,4);
  }

void AddExtension(char *fname, char *xtn)
  {
  char *sptr=fname; BYTE perds=0;
  while (*sptr) { if (*sptr=='.') perds++; sptr++; }
  if (!perds) EnforceExtension(fname,xtn);
  }

void GetRunningPath(char *pth) // Returned path never ends on a backslash
  {
  union REGS Regs;
  struct SREGS SRegs;
  char tstr[256];
  BYTE cdrv;
  Regs.h.ah=0x19;
  intdos(&Regs,&Regs);
  cdrv=Regs.h.al;
  Regs.h.ah=0x47;
  Regs.h.dl=cdrv+1;
  SRegs.ds=FP_SEG(tstr);
  Regs.x.si=FP_OFF(tstr);
  intdosx(&Regs,&Regs,&SRegs);
  if (Regs.x.cflag) { pth[0]=0; return; }
  if (tstr[0]) sprintf(pth,"%c:\\%s",cdrv+65,tstr);
  else sprintf(pth,"%c:",cdrv+65);
  }

BYTE DeleteFile(char *fname)
  {
  union REGS Regs;
  struct SREGS SRegs;
  Regs.h.ah=0x41;
  SRegs.ds=FP_SEG(fname);
  Regs.x.dx=FP_OFF(fname);
  intdosx(&Regs,&Regs,&SRegs);
  if (Regs.x.cflag)
    {
    if (Regs.x.ax==2) return 1;
    if (Regs.x.ax==5) return 2;
    }
  return 0;
  } // 0 NoError  1 NotFound  2 AccessDenied

BYTE FloppyProtected(int drive)
  {
  union REGS regs;
  struct SREGS sregs;
  BYTE *buf;
  int cnt;
  if (drive>1) return 2; // floppies only
  if (!(buf=new BYTE [512])) return 3; // allocate buffer
  for (cnt=0; cnt<6; cnt++) // read sector (6 attempts)
    {
    regs.x.ax=0x0201;      // read sector 1
    regs.x.cx=0x0001;      // cylinder 0, sector 1
    regs.x.dx=drive;       // head 0 and drive
    regs.x.bx=FP_OFF(buf); // buffer address
    sregs.es=FP_SEG(buf);
    int86x(0x13,&regs,&regs,&sregs);
    if (regs.x.ax==0x0001) break; // reading successfull
    }
  if (cnt==6) { delete [] buf; return 2; } // reading failed
  regs.x.ax=0x0301; // write sector
  regs.x.cx=0x0001;
  regs.x.dx=drive;
  regs.x.bx=FP_OFF(buf);
  sregs.es=FP_SEG(buf);
  int86x (0x13,&regs,&regs,&sregs);
  delete [] buf;
  if (regs.h.ah==0) return 1; // writing successfull
  return 0;
  } // 0 Protected  1 NotProtected  2 Error  3 InsufMem

BYTE PathProtected(char *path)
  {
  int drive=-1;
  if ((path[0]==65) || (path[0]==61)) drive=0;
  if ((path[0]==66) || (path[0]==62)) drive=1;
  if (drive!=-1) return (!(FloppyProtected(drive)==1));
  return 0;
  }

//--------------------------- DOS File Access --------------------------------

BYTE DOSOpen(char *fname, int *fhnd)
  {
  if (_dos_open(fname,_A_NORMAL,fhnd)==0) return 1;
  return 0;
  }

BYTE DOSCreate(char *fname, int *fhnd)
  {
  if (_dos_creat(fname,_A_NORMAL,fhnd)==0) return 1;
  return 0;
  }

void DOSClose(int fhnd)
  {
  _dos_close(fhnd);
  }

BYTE DOSRead(int fhnd, void *fbuf, WORD fbtr)
  {
  WORD fbar;
  if ((_dos_read(fhnd,fbuf,fbtr,&fbar)==0) && (fbar==fbtr)) return 1;
  return 0;
  }

BYTE DOSWrite(int fhnd, void *fbuf, WORD fbtw)
  {
  WORD fbaw;
  if ((_dos_write(fhnd,fbuf,fbtw,&fbaw)==0) && (fbaw==fbtw)) return 1;
  return 0;
  }

//------------------------- GROUP file definitions ---------------------------

typedef struct GROUPHEAD { char head[26]; int entrynum; };
typedef struct INDEXENTRY { char fname[13]; DWRD size,offset; };

//-------------------- Buffered Binary File Input/Output ------------------------------

// Version 1.0  March 1996 (BFI)
//         1.1    May 1996 (BFO)
//         1.2   June 1996 (Multiple file access, GROUP file access)

//  1 Insufficient memory
//  2 File not found
//  3 File reading error
//  4 End of file
//  5 Cannot create file
//  6 File writing error
//  7 Buffer overload safety
//  8 BFI/O already active
//  9 BFI/O not active yet
// 10 Group file error
// 11 File not in group file

int BFIHandle=-1;
BYTE *BFIBuffer;
WORD BFIBufSize,BFIBufPtr,BFIBufLoad;
long BFICFLen;

BYTE DeInitBFI(void)
  {
  if (BFIHandle==-1) return 9;
  delete [] BFIBuffer;
  DOSClose(BFIHandle);
  BFIHandle=-1;
  return 0;
  }

BYTE InitBFI(char *fname, WORD bufsize=5000)
  {
  int fnpipe,cnt;
  long offread;
  GROUPHEAD GHead;
  INDEXENTRY CEntry,FEntry;
  char gfname[256];
  // Check for BFI activity
  if (BFIHandle!=-1) return 8;
  // Allocate buffer memory
  for (BFIBufSize=bufsize; (BFIBufSize>=100) && !(BFIBuffer=new BYTE [BFIBufSize]); BFIBufSize/=2);
  if (BFIBufSize<100) return 1;

  if ((fnpipe=SCharPos('|',fname))==-1) // Regular file
    {
    // Open file
    if (!DOSOpen(fname,&BFIHandle))
      { delete [] BFIBuffer; BFIHandle=-1; return 2; }
    // Set CFLen
    BFICFLen=filelength(BFIHandle);
    if (BFICFLen==-1) { DeInitBFI(); return 3; }
    }
  else // GROUP file
    {
    // Open group file
    SCopy(fname,gfname,fnpipe);
    if (!DOSOpen(gfname,&BFIHandle))
      { delete [] BFIBuffer; BFIHandle=-1; return 2; }
    // Read group file header
    if (!DOSRead(BFIHandle,&GHead,sizeof(GROUPHEAD))) { DeInitBFI(); return 3; }
    if (!SEqual(GHead.head,"RedWolf Design GROUP")) { DeInitBFI(); return 10; }
    // Search for correct index entry
    FEntry.size=0;
    for (cnt=0; cnt<GHead.entrynum; cnt++)
      {
      if (!DOSRead(BFIHandle,&CEntry,sizeof(INDEXENTRY))) { DeInitBFI(); return 3; }
      if (SEqual(CEntry.fname,fname+fnpipe+1)) FEntry=CEntry;
      }
    if (FEntry.size==0) { DeInitBFI(); return 11; }
    // Read to offset (using BFIBuffer)
    for (offread=FEntry.offset; offread>0; offread-=BFIBufSize)
      if (!DOSRead(BFIHandle,BFIBuffer,Min(offread,BFIBufSize)))
	{ DeInitBFI(); return 3; }
    // Set CFLen
    BFICFLen=FEntry.size;
    }

  // Reset reading pointers
  BFIBufLoad=0; BFIBufPtr=0;
  return 0;
  }

BYTE GetBFI(void *vtbuf, WORD size=1)
  {
  WORD transfer;
  BYTE *tbuf=(BYTE*)vtbuf;
  if (BFIHandle==-1) return 9;
  while (size>0)
    if (BFIBufPtr<BFIBufLoad) // Transfer buffer
      {
      transfer=Min(BFIBufLoad-BFIBufPtr,size);
      MemCopy(BFIBuffer+BFIBufPtr,tbuf,transfer);
      BFIBufPtr+=transfer;
      tbuf+=transfer;
      size-=transfer;
      }
    else // Load to buffer
      {
      if (_dos_read(BFIHandle,BFIBuffer,BFIBufSize,&BFIBufLoad)!=0)
	{ DeInitBFI(); return 3; }
      if (BFIBufLoad==0)
	{ DeInitBFI(); return 4; }
      BFIBufPtr=0;
      }
  return 0;
  }

DWRD BFIFileLength(void)
  {
  return BFICFLen;
  }

int BFOHandle=-1;
BYTE *BFOBuffer;
WORD BFOBufSize,BFOBufLoad;

BYTE InitBFO(char *fname, WORD bufsize=5000)
  {
  if (BFOHandle!=-1) return 8;
  // Allocate buffer memory
  for (BFOBufSize=bufsize; (BFOBufSize>=100) && !(BFOBuffer=new BYTE [BFOBufSize]); BFOBufSize/=2);
  if (BFOBufSize<100) return 1;
  // Create file
  if (!DOSCreate(fname,&BFOHandle))
    { delete [] BFOBuffer; return 5; }
  // Reset reading pointers
  BFOBufLoad=0;
  return 0;
  }

BYTE DeInitBFO(void)
  {
  BYTE rval=0;
  if (BFOHandle==-1) return 9;
  if (!DOSWrite(BFOHandle,BFOBuffer,BFOBufLoad)) rval=6;
  DOSClose(BFOHandle);
  delete [] BFOBuffer;
  BFOHandle=-1;
  return rval;
  }

BYTE PutBFO(void *vtbuf, WORD size=1)
  {
  WORD transfer;
  BYTE *fbuf=(BYTE*) vtbuf;
  if (BFOHandle==-1) return 9;
  while (size>0)
    if (BFOBufLoad<BFOBufSize) // Transfer to buffer
      {
      transfer=Min(BFOBufSize-BFOBufLoad,size);
      MemCopy(fbuf,BFOBuffer+BFOBufLoad,transfer);
      BFOBufLoad+=transfer;
      fbuf+=transfer;
      size-=transfer;
      }
    else // Save buffer to disk
      {
      if (BFOBufLoad>BFOBufSize)
	{ BFOBufLoad=BFOBufSize; DeInitBFO(); return 7; }
      if (!DOSWrite(BFOHandle,BFOBuffer,BFOBufLoad))
	{ DeInitBFO(); return 6; }
      BFOBufLoad=0;
      }
  return 0;
  }

//---------------------------- File Search ----------------------------------

typedef struct DTAENTRY { BYTE reserved[21];
			  BYTE attr;
			  WORD time,date;
			  DWRD size;
			  char name[13];
			};

// Attribute byte:  1 ReadOnly  2 Hidden      4 System
//		    8 Label    16 Directory  32 Archive

DTAENTRY DTAEntry;
BYTE DTAEntryLoaded;

BYTE DTAFirstEntry(char *path, BYTE atr)
  {
  union REGS Regs;
  struct SREGS SRegs;
  Regs.h.ah=0x4E;
  Regs.x.cx=atr;
  SRegs.ds=FP_SEG(path);
  Regs.x.dx=FP_OFF(path);
  intdosx(&Regs,&Regs,&SRegs);
  return !Regs.x.cflag;
  }

BYTE DTANextEntry(void)
  {
  union REGS Regs;
  Regs.h.ah=0x4F;
  intdos(&Regs,&Regs);
  return !Regs.x.cflag;
  }

void DTASet(DTAENTRY *dptr)
  {
  union REGS Regs;
  struct SREGS SRegs;
  Regs.h.ah=0x1A;
  SRegs.ds=FP_SEG(dptr);
  Regs.x.dx=FP_OFF(dptr);
  intdosx(&Regs,&Regs,&SRegs);
  }

BYTE InitFileSearch(char *path)
  {
  DTASet(&DTAEntry);
  DTAEntryLoaded=0;
  if (DTAFirstEntry(path,31))
    { DTAEntryLoaded=1; return 1; }
  return 0;
  }

BYTE SearchNextFile(char *nameto)
  {
  if (!DTAEntryLoaded)
    if (!DTANextEntry())
      { if (nameto) nameto[0]=0; return 0; }
  if (nameto) SCopy(DTAEntry.name,nameto,12);
  DTAEntryLoaded=0;
  return 1;
  }

BYTE GetFileInfo(char *fname, WORD *time, WORD *date, DWRD *size)
  {
  if (!InitFileSearch(fname)) return 0;
  // If successfull, file info will be loaded into global DTAEntry
  *time=DTAEntry.time;
  *date=DTAEntry.date;
  *size=DTAEntry.size;
  return 1;
  }

//---------------------------- Text File Handling ---------------------------

BYTE LocateInFile(FILE *file, BYTE *index, BYTE wrap=1)
  {
  int fchr,needok,idxcnt=0,loops=0;        // Locates file pointer to
  long lpos;                               // end of located string
  BYTE exok=0;
  needok=SLen(index);
  if (!file) return 0;
  do
    {
    fchr=fgetc(file);
    if (fchr==EOF)
      if (wrap) { rewind(file); fchr=fgetc(file); loops++; if (loops>=2) exok=2; }
      else return 0;
    if (fchr==index[idxcnt])
      { idxcnt++; if (idxcnt==needok) exok=1; }
    else
      idxcnt=0;
    }
  while (!exok);
  if (exok==1) return 1;
  return 0;
  }

BYTE ReadFileLine(FILE *fhnd, char *tobuf, int maxlen) // Reads mxlen string
  {			  			       // or till EOL to buf
  int cread;
  if (!fhnd) return 0;
  for (cread=0; cread<maxlen; cread++)
    {
    *tobuf=fgetc(fhnd);
    if (!(*tobuf) || (*tobuf==EOF) || (*tobuf==0x0A)) break;
    tobuf++;
    }
  *tobuf=0;
  if (cread==0) return 0;
  return 1;
  }

BYTE ReadFileInfoLine(FILE *fhnd, BYTE *info, BYTE *tbuf, int maxlen=256)
  {
  if (!LocateInFile(fhnd,info)) return 0;
  if (!ReadFileLine(fhnd,tbuf,maxlen)) return 0;
  return 1;
  }

void AdvanceFileLine(FILE *fhnd)
  {
  int cread,loops=0;
  if (!fhnd) return;
  do
    {
    cread=fgetc(fhnd);
    if (cread==EOF) { rewind(fhnd); loops++; }
    }
  while ((cread!=0x0A) && (loops<2));
  }

DWRD ReadFileUntil(FILE *fhnd, char *tbuf, char smark, int maxlen)
  {
  DWRD rcnt=0;
  int cread;
  while (maxlen>0)
    {
    cread=fgetc(fhnd);
    if ((cread==smark) || (cread==EOF)) break;
    *tbuf=cread; tbuf++;
    maxlen--;
    rcnt++;
    }
  *tbuf=0;
  return rcnt;
  }