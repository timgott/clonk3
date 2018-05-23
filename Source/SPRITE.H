/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf-D  SPRITE  Header  M.Bender
// Version and date see library

//--------------------------- Type Defintions --------------------------------

typedef struct BSPRITE { BYTE spge, alias;
			 int sx, sy, sw, sh;
			 BYTE *smask,*cmask;
		       };

typedef struct ASPRITE { BSPRITE shift[4]; };

//----------------------------- Functions --------------------------------

extern void RWCopy(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE trns);
extern void RWCopyE(BYTE fpge, int fx, int fy, BYTE tpge, int tx, int ty, int wdt, int hgt, BYTE trns, float scalex, float scaley);

extern WORD MaskSize(int wdt, int hgt);
extern void CreateMask(BYTE *mbuf, BYTE spge, int sx, int sy, int wdt, int hgt);
extern BYTE TBMove(BYTE fpge, int frx, int fry, BYTE tpge, int tox, int toy, BYTE wdt, BYTE hgt);

extern void SetColorBase(BYTE newsbc, int cbx, int cby);

extern void CreateCSprite(BYTE pge, int sx, int sy, int sw, int sh);
extern void CSprite(BYTE fpge, int sx, int sy, BYTE tpge, int tx, int ty, BYTE sw, BYTE sh);

extern BYTE CreateBSprite(BSPRITE *bsptr, BYTE spge, int sx, int sy, int sw, int sh, BYTE clrd=0);
extern void CreateBSpriteAlias(BSPRITE *bsptr, BYTE spge, int sx, int sy, int sw, int sh, BSPRITE *bsalias);
extern void DestroyBSprite(BSPRITE *bsptr);
extern void BSprite(BSPRITE *bsptr, BYTE tpge, int tx, int ty);
extern void BSpriteY(BSPRITE *bsptr, BYTE tpge, int tx, int ty, int yoff, int nhgt);
extern void BSpriteC(BSPRITE *bsptr, BYTE tpge, int tx, int ty, BYTE col);

extern BYTE CreateASprite(ASPRITE *asptr, BYTE spge, int sx, int sy, int sw, int sh, BYTE cspr=0, BYTE clrd=0);
extern void CreateASpriteAlias(ASPRITE *asptr, BYTE spge, int sx, int sy, int sw, int sh, ASPRITE *asalias, BYTE cspr=0);
extern void DestroyASprite(ASPRITE *asptr);
extern void ASprite(ASPRITE *asptr, BYTE tpage, int tx, int ty);
extern void ASpriteC(ASPRITE *asptr, BYTE tpage, int tx, int ty, BYTE col);

extern BYTE SpriteInitError;