/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

//--------------------------- BSPXSSYS Externals -----------------------------

extern void InitPXS(BYTE smokeon, BYTE wtrout);
extern void DeInitPXS(void);
extern void ExecPXS(void);

extern void PXSOut(BYTE type, int x, int y, int p);
extern void MassPXSOut(BYTE type, int x, int y, int p);

extern BYTE PXSPixFree(int x, int y);

extern BYTE NewRock(int x, int y, int act, int type, int xdir, int ydir, int phase, int thrby);
extern void DeleteRocks(void);
extern void MoveRocks(void);
extern void HomeRock(int rtype, int rphase, int plr);

extern void PourPXS(BYTE type, int tx, int ty, int phase, int amt, int rad);

extern ROCKTYPE *FirstRock;
