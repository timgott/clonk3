/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RW-D CT-VOICE Support Headers

extern BYTE LoadVOC(char *fname, BYTE **buf);
extern void DestroyVOC(BYTE **buf);

extern char *CTVErrMsg(int errcode);

extern BYTE InitCTVSound(char *ctvname, WORD port, WORD irq);
extern void DeInitCTVSound(void);
extern void CTVPlaySound(BYTE *vocbuf);
extern BYTE CTVSoundCheck(void);
extern void CTVoiceStop(void);

extern BYTE AutodetectBlaster(WORD *port, WORD *irq, char *path);
