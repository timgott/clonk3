/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

//---------------------------- BSVHCSYS Externals ----------------------------

extern VEHICTYPE *NewVehic(int type, int x, int y, int p, int data, int data2, int data3, int owner, void *tptr);
extern void DeleteVehics(void);

extern void MoveVehic(VEHICTYPE *vhc);
extern void ExecVehics(void);
extern void RemoveVehic(VEHICTYPE *cvhc);

extern void MoveElevator(VEHICTYPE *vhc);

extern VEHICTYPE *OnWhichVehic(int tx, int ty, VEHICTYPE *notthis);
extern BYTE ConVehic(VEHICTYPE *vhc, BYTE plr);
extern int VehicleLoc(int tx, int ty);

extern VEHICTYPE *FirstVehic;
