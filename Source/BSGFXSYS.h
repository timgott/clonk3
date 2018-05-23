/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

//--------------------------- BSGFXSYS Externals -----------------------------

extern BYTE CPGE;

extern BYTE InitBack(void);
extern void DeInitBack(void);
extern void TimeOut(void);
extern void DrawScreen(void);
extern void AdjustBackVPos(int *nvy);

extern void SystemMsg(char *msg);
extern void InitGameMessage();
extern void GameMessage(char *msg, int tx, int ty, int col, MANTYPE *tman);
extern void ClearGMsgTPtr(MANTYPE *tman);

extern BYTE GBackPix(long fx, long fy);
extern void SBackPix(long fx, long fy, BYTE col);
extern BYTE IsSkyBack(BYTE color);
extern void DrawBackPix(int tx, int ty);
extern void DrawSkyPix(int tx, int ty);
extern void DrawTunnelPix(int tx, int ty);

extern void ReleasePXS(int rrx, int rry);
extern BYTE ApplyHeat2Back(int tx, int ty, BYTE lavahot);
extern int  ExtractLiquid(int fx, int fy);
extern void MeltFree(int tx, int ty, int rad);
extern void ClearLineOfSnow(int tx, int ty, int wdt);

extern int  DigFree(int tx, int ty, int rad);
extern void BlastFree(int tx, int ty, int rad);

extern void DrawLoamChunk(int tx, int ty);
extern void DrawSteelChunk(int tx, int ty);

extern BYTE FindSurface(BYTE type, int atx, int wdt, int *tx, int *ty);
extern void RaiseUpTerrain(int tx, int ty, int wdt);

extern void DrawFireSpot(int tx, int ty);

extern BYTE LowFPS;