/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design GROUP

#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>

#include <standard.h>
#include <stdfile.h>



typedef struct GROUPHEAD { char head[26]; int entrynum; };

typedef struct INDEXENTRY { char fname[13]; DWRD size,offset; };





int NumOFiles(char *fdef)
  {
  int rval=0;
  if (!InitFileSearch(fdef)) return 0;
  while (SearchNextFile(NULL)) rval++;
  return rval;
  }

BYTE WriteIndeces(char *fdef)
  {
  int fhnd;
  INDEXENTRY CEntry;
  DWRD coffs=0L;
  cprintf("Creating index...\n\r");
  if (!InitFileSearch(fdef)) return 0;
  while (SearchNextFile(CEntry.fname))
    {
    CEntry.offset=coffs;
    if ((fhnd=open(CEntry.fname,O_RDONLY))==-1)
      { cprintf("Couldn't access file '%s'.\n\rMake sure GROUP is started from the same directory as the infiles.\n\r",CEntry.fname); DeInitBFO(); return 0; }
    CEntry.size=filelength(fhnd);
    close(fhnd);
    cprintf("%s %lu Bytes at %lu\n\r",CEntry.fname,CEntry.size,CEntry.offset);
    coffs+=CEntry.size;
    if (PutBFO(&CEntry,sizeof(INDEXENTRY)))
      { cprintf("Cannot write to outfile.\n\r"); return 0; }
    }
  return 1;
  }

BYTE AddFiles2Outfile(char *fdef)
  {
  char fname[13];
  BYTE buf;
  int fcnt=0; DWRD mcnt=0L;
  if (!InitFileSearch(fdef)) return 0;
  while (SearchNextFile(fname))
    {
    fcnt++;
    if (InitBFI(fname)) return 0;
    while (!GetBFI(&buf) && !PutBFO(&buf)) mcnt++;
    DeInitBFI();
    }
  cprintf("(%d files, %lu Bytes) ",fcnt,mcnt);
  return 1;
  }

void main(int argc, char *argv[])
  {
  GROUPHEAD GHead;

  cprintf("RedWolf Design GROUP\n\r");

  if (argc!=3) { cprintf("Usage: Group INFILES OUTFILE\n\r"); return; }

  cprintf("Counting files '%s'... ",argv[1]);
  SCopy("RedWolf Design GROUP FILE",GHead.head,25);
  GHead.entrynum=NumOFiles(argv[1]);
  cprintf("%d found\n\r",GHead.entrynum);

  if (GHead.entrynum==0) { cprintf("No files found. Nothing to do.\n\r"); return; }

  cprintf("Creating '%s', writing header...\n\r",argv[2]);

  if (InitBFO(argv[2]))
    { cprintf("Cannot create outfile.\n\r"); return; }

  if (PutBFO(&GHead,sizeof(GROUPHEAD)))
    { cprintf("Cannot write to outfile.\n\r"); return; }

  if (!WriteIndeces(argv[1])) return;

  cprintf("Adding files... ");

  if (!AddFiles2Outfile(argv[1]))
    { cprintf("Error!\n\r"); DeInitBFO(); return; }

  cprintf("Done\n\r");

  DeInitBFO();
  }