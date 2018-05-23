/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */


const int ScrTitleLen=30,ScrDescLen=40;

typedef struct SCRIPTINFO { int idnum;
			    char title[ScrTitleLen+1],desc[ScrDescLen+1];
			    char fname[30]; // Suited for "MISSION\*.SCR"
			    BYTE ruleset;
			    SCRIPTINFO *next;
			  };