/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

#include <stdio.h>
#include <conio.h>

int TextFileCheckSum(char *fname) // Checksums division remainder 12345
  {
  FILE *fhnd;
  int checksum=0;
  int fbuf;
  if (!(fhnd=fopen(fname,"rt"))) return -1;
  do
    {
    fbuf=fgetc(fhnd);
    if (fbuf!=EOF) { checksum+=fbuf; checksum%=12345; }
    }
  while (fbuf!=EOF);
  fclose(fhnd);
  return checksum;
  }


void main(int argc, char *argv[])
  {
  int csm;
  cprintf("RedWolf Design TextFile Checksum\n\r");

  if (argc!=2) { cprintf("Usage: TFCheck CHECKFILE\n\r"); return; }

  csm=TextFileCheckSum(argv[1]);

  if (csm==-1) cprintf("File not found.\n\r");
  else cprintf("'%s' CheckSum: %d\n\r",argv[1],csm);
  }