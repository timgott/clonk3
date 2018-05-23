/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  SVI  STANDARD VISUAL INTERFACE Header (C) by M.Bender

// Version and date see library file

#include "svi20typ.h"

extern BYTE SVIPage;
extern WORD SVIInitErrors;

extern void ResetInitMsg(void);
extern void InitMsg(char *msg, BYTE col=CIMsg);
extern void InitMsgOpen(char *msg, BYTE col=CIMsg);

extern void SVISetExternFunctions(void(*dlbc)(int,int,int,int,BYTE),BYTE(*lbca)(int,int,int));

extern OBJECT *NewObject(char *title, BYTE type, BYTE acc, BYTE hid, int x, int y, int wdt, int hgt, BYTE bd1, BYTE bd2, BYTE bd3, int id1, int id2, int id3, int id4, int id5, char *bptr, int *dptr);
extern WINDOW *NewWindow(char *title, BYTE type, BYTE prior, int x, int y, int wdt, int hgt, int num, int outof, BYTE escxrv, char *hindex);
extern ULINK  *NewULink(BYTE type, WINDOW *fwin, OBJECT *fobj, OBJECT *tobj, int (*exfunc)(int), int expar);

extern OBJECT *TextObject(char *txt, int tx, int ty, int fcol, int bcol=-1, BYTE form=0);
extern OBJECT *CTextObject(char *txt, int tx, int ty, int fcol, int bcol=-1, BYTE form=0);
extern OBJECT *ITextObject(char **txt, int *index, int tx, int ty, int fcol, int bcol=-1, BYTE form=0);
extern OBJECT *NewButton(char *title, BYTE hide, int tx, int ty, int wdt, int hgt, BYTE xrval, BYTE noulr);
extern OBJECT *NewFlagBox(char *title, int tx, int ty, int *valptr, BYTE hide=0);

extern void DrawWindow(WINDOW *win);

extern void SVIGCopy(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE gray);

extern void CloseWindow(WINDOW *win, BYTE noredr=0);

extern BYTE RunSVI(void);
extern void ClearSVI(void);

BYTE InitSVI(int screenScale, BYTE svipge, char *helpfname);
extern void CloseSVI(void);

extern void InitSVIGrayMap(void);

extern void Message(char *txt, char *hindex=NULL);
extern void ConfirmedCall(char *txt, int ctype, int (*exfunc)(int), int expar, char *hindex=NULL);
extern void LineInput(char *msg, char *txt, int maxlen, int (*exfunc)(int)=NULL, int expar=0, char *hindex=NULL);

extern void DefColors(void);
extern void DefCols2Pal(BYTE *tpal);

extern BYTE InitErrorCheck(void);

extern void AdjustLBCellNum(int lid, int ncnum);

extern void InitProcessB(char *msg, BYTE page=SVIPage);
extern void ProcessB(int val);

extern void OpenPowerHelp(char *index);

extern BYTE MouseType;