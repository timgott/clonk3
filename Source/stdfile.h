/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design STANDARD FILE ACCESS Library Header

void EnforceExtension(char *fname, char *xtn); // xtn with period!
void AddExtension(char *fname, char *xtn);
void GetRunningPath(char *pth); // Returned path doesn't end on a '\'
BYTE DeleteFile(char *fname);
BYTE FloppyProtected(int drive);
BYTE PathProtected(char *path);

BYTE DOSOpen(char *fname, FILE **stream, const char* mode);
void DOSClose(FILE* stream);
BYTE DOSRead(FILE* stream, void *fbuf, WORD fbtr);
BYTE DOSWrite(FILE* stream, void *fbuf, WORD fbtw);
BYTE FileExists(char *fname);

BYTE InitBFI(char *fname, WORD bufsize=5000); // On any error, BFIO
BYTE DeInitBFI(void);			     // deinits by itself
BYTE GetBFI(void *vtbuf, WORD size=1);
DWORD BFIFileLength(void);                     // Return 0 if no error
BYTE InitBFO(char *fname, WORD bufsize=5000);
BYTE DeInitBFO(void);
BYTE PutBFO(void *vtbuf, WORD size=1);

BYTE InitFileSearch(char *path);   // Set search path
BYTE SearchNextFile(char *nameto); // Returns file name w/out path
BYTE GetFileInfo(char *fname, WORD *time, WORD *date, DWORD *size);

BYTE LocateInFile(FILE *file, char *index, BYTE wrap=1);
BYTE ReadFileLine(FILE *fhnd, char *tobuf, int maxlen);
BYTE ReadFileInfoLine(FILE *fhnd, char *info, char *tbuf, int maxlen=256);
void AdvanceFileLine(FILE *fhnd);
DWORD ReadFileUntil(FILE *fhnd, char *tbuf, char smark, int maxlen);
