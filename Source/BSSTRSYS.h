/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

//---------------------------- BSSTRSYS Externals ----------------------------

extern STRUCTYPE *NewStruct(int type, int x, int y, int p, int con, int owner);
extern void DeleteStructs(void);
extern void ExecStructs(void);

extern void CheckStructIgnition(int tx, int ty);
extern void StructHitDamage(int tx, int ty, int dmg, int csdby);

extern BYTE StructIsConnected(STRUCTYPE *strc, int linetype);
extern STRUCTYPE *OnWhichStruct(int tx, int ty);
//extern int LocElevatorShaft(int tx);

extern BYTE ConstructionCheck(int type, int &ctx, int &cty);
extern void StructCompletion(STRUCTYPE *strc, BYTE byhand, int newown);

extern LINETYPE *NewLine(int type, int x1, int y1, STRUCTYPE *fstrc, STRUCTYPE *tstrc);
extern void DeleteLines(void);
extern void ExecLines(void);

extern BYTE LineCutEarthAt(int x1, int y1, int x2, int y2, int *retx, int *rety);
extern BYTE LineNotCarried(LINETYPE *lptr);

extern LINETYPE *PickUpOpenLine(int cx, int cy);

extern BYTE ConnectionExists(STRUCTYPE *str1, STRUCTYPE *str2, BYTE type);
extern BYTE NeedsNewElevatorShaft(STRUCTYPE *tstrc);


extern STRUCTYPE *FirstStruct;
extern LINETYPE *FirstLine;
