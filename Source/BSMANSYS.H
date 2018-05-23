/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// BSMANSYS Header

extern CREWTYPE Crew[3];
extern MANTYPE *FirstAnimal;

extern void CheckManHit(int tx, int ty, int rad);
extern void CheckManIgnition(int tx, int ty);
extern void CheckClonkSting(int x, int y);

extern void LoseTPtr(MANTYPE *mptr);
extern void ClearMenTPtr(void *tptr);

extern BYTE NotHostile(int plr1, int plr2);
extern void ScoreGain(int plr, int score);
extern int HighRank(BYTE plr);
//extern MANINFO *HighRankMan(BYTE plr);
