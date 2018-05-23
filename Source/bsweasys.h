/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

//---------------------------- BSWEASYS Externals ----------------------------

extern void InitWeather(int climate, int season);
extern void ExecWeather(void);

extern void InitGround(void);
extern void ExecGround(int *scrto);

extern BYTE PlantTree(BYTE allgrown);

extern void Explode(int exx, int exy, int exrad, int causedby);


extern WEATHERTYPE Weather;
extern GROUNDTYPE Ground;