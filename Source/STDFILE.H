/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design STANDARD FILE ACCESS Library Header

extern void EnforceExtension(char *fname, char *xtn); // xtn with period!
extern void AddExtension(char *fname, char *xtn);
extern void GetRunningPath(char *pth); // Returned path doesn't end on a '\'
extern BYTE DeleteFile(char *fname);
extern BYTE FloppyProtected(int drive);
extern BYTE PathProtected(char *path);

extern BYTE DOSOpen(char *fname, int *fhnd);
extern BYTE DOSCreate(char *fname, int *fhnd);
extern void DOSClose(int fhnd);
extern BYTE DOSRead(int fhnd, void *fbuf, WORD fbtr);
extern BYTE DOSWrite(int fhnd, void *fbuf, WORD fbtw);

extern BYTE InitBFI(char *fname, WORD bufsize=5000); // On any error, BFIO
extern BYTE DeInitBFI(void);			     // deinits by itself
extern BYTE GetBFI(void *vtbuf, WORD size=1);
extern DWRD BFIFileLength(void);                     // Return 0 if no error
extern BYTE InitBFO(char *fname, WORD bufsize=5000);
extern BYTE DeInitBFO(void);
extern BYTE PutBFO(void *vtbuf, WORD size=1);

extern BYTE InitFileSearch(char *path);   // Set search path
extern BYTE SearchNextFile(char *nameto); // Returns file name w/out path
extern BYTE GetFileInfo(char *fname, WORD *time, WORD *date, DWRD *size);

extern BYTE LocateInFile(FILE *file, BYTE *index, BYTE wrap=1);
extern BYTE ReadFileLine(FILE *fhnd, char *tobuf, int maxlen);
extern BYTE ReadFileInfoLine(FILE *fhnd, BYTE *info, BYTE *tbuf, int maxlen=256);
extern void AdvanceFileLine(FILE *fhnd);
extern DWRD ReadFileUntil(FILE *fhnd, char *tbuf, char smark, int maxlen);
