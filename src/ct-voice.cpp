/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design CT-VOICE Support System

// Version 1.0 May 1995 for CLONK A.P.E.
//         1.1 May 1996 for CLONK 3 Radikal

// based on GAME GURU Source

// I N C L U D E S ///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#include "standard.h"
#include "stdfile.h"

// G L O B A L S  //////////// CT-VOICE Variables ////////////////////////////

char *CTVoicePtr, *CTVoiceRealDataPtr;
char *data_ptr;
unsigned CTVoiceStatus;

// F U N C T I O N S /////////////////////////////////////////////////////////

/*unsigned CTVoiceGetVersion(void)
  {
  unsigned version;
  asm {
	  mov bx,0          // function 0 get version number
	  call CTVoicePtr   // call the driver
	  mov version,ax    // store in version variable
	  }
  // Correct output: "%X.0%X",((version>>8) & 0x00ff),(version&0x00ff)
  return version;
  }*/

int InitCTVoice(void)
{
	/*int status;
	asm{
		mov bx,3          // function 3 initialize the driver
		call CTVoicePtr   // call the driver
		mov status,ax     // store in version variable
	}
	if (status == 0) return 0; // NoError
	return (4 + status); // 4+1:IncorrDrvVersion 4+2:I/OError 4+3:DMAIntError*/
	return 4+1;
}

void DeInitCTVoice(void)
{
	/*asm{
		mov bx,9          // function 9 terminate the driver
		call CTVoicePtr   // call the driver
	}
		//_dos_freemem(FP_SEG(CTVoicePtr));
	delete[] CTVoiceRealDataPtr;*/
}

void CTVoiceSetBase(unsigned port)
{
	/*asm{
		mov bx,1          // function 1 set port address
		mov ax,port       // move the port number into ax
		call CTVoicePtr   // call the driver
	}*/
}

void CTVoiceSpeaker(unsigned on)
{
	/*asm{
		mov bx,4          // function 4 turn speaker on or off
		mov ax,on         // move the on/off flag into ax
		call CTVoicePtr   // call the driver
	}*/
}

void CTVoicePlay(BYTE *buf)
{
	/*unsigned segm, offm;
	segm = FP_SEG(buf);
	offm = FP_OFF(buf) + *((WORD*)(buf + 20)); // Headers offset
	asm{
		mov bx,6         // function 6 play a VOC file
		mov ax,segm      // can only mov a register into segment so we need this
		mov es,ax        // es gets the segment
		mov di,offm      // di gets offset
		call CTVoicePtr  // call the driver
	}*/
}

void CTVoiceStop(void)
{
	/*asm{
		mov bx,8          // function 8 stop a sound
		call CTVoicePtr   // call the driver
	}*/
}

/*void CTVoicePause(void)
  {
  asm {
	  mov bx,10         // function 10 pause a sound
	  call CTVoicePtr   // call the driver
	  }
  }

void CTVoiceContinue(void)
  {
  asm {
	  mov bx,11         // function 11 continue play
	  call CTVoicePtr   // call the driver
	  }
  }

void CTVoiceBreak(void) // Break a sound loop
  {
  asm {
	  mov bx,12         // function 12 break loop
	  call CTVoicePtr   // call the driver
	  }
  }*/

void CTVoiceSetInt(unsigned dma)
{
	/*
	asm{
		mov bx,2          // function 2 set DMA interupt number
		mov ax,dma        // move the dma number into ax
		call CTVoicePtr   // call the driver
	}*/
}

void CTVoiceSetStatusAdr(char *status)
{/*
	unsigned segm, offm;
	segm = FP_SEG(status);
	offm = FP_OFF(status);
	asm{
		mov bx,5         // function 5 set status varible address
		mov es,segm      // es gets the segment
		mov di,offm      // di gets offset
		call CTVoicePtr  // call the driver
	}*/
}

BYTE LoadCTVoice(char *fname)
{
	/*int fhnd;
	unsigned bytes_read;
	DWORD bufsize;
	WORD segment, offset;
	BYTE *loadptr;
	if (DOS(fname, O_RDONLY, &fhnd) != 0) return 1;
	bufsize = filelength(fhnd) + 16;
	if (!(CTVoiceRealDataPtr = new BYTE[bufsize])) { _dos_close(fhnd); return 2; }
	segment = FP_SEG(CTVoiceRealDataPtr);
	CTVoicePtr = (char far*) MK_FP(segment + 1, 0);
	loadptr = CTVoicePtr;
	do
	{
		if (_dos_read(fhnd, loadptr, 0x4000, &bytes_read) != 0)
		{
			_dos_close(fhnd); delete[] CTVoiceRealDataPtr; return 3;
		}
		loadptr += bytes_read;
	} while (bytes_read == 0x4000);
	_dos_close(fhnd);
	if (!SEqual((BYTE*)CTVoicePtr + 3, "CT-VOICE")) // Always correct?
	{
		delete[] CTVoiceRealDataPtr; return 4;
	}
	return 0;*/
	return 2;
} // 0:NoError 1:FileNotFound 2:InsufMem 3:ReadError 4:NoCT-VOICE

BYTE LoadVOC(char *fname, BYTE **buf)
{
	BYTE errv;
	WORD size;

	if ((errv = InitBFI(fname)) != 0) return 10 + errv;

	size = BFIFileLength(); if (!Inside(size, 1, 50000)) { DeInitBFI(); return 2; }

	if (!(*buf = new BYTE[size])) { DeInitBFI(); return 1; }

	if ((errv = GetBFI(*buf, size)) != 0) { delete[] * buf; return 10 + errv; }

	if ((errv = DeInitBFI()) != 0) { delete[] * buf; return 10 + errv; }

	if (!SEqual((char*)*buf, "Creative")) { delete[] * buf; return 3; }

	return 0;

} // Returns 0:NoError 1:InsufMem 2:SizeError 3:NotVOC 10+n:BFI-Error

void DestroyVOC(BYTE **buf)
{
	delete[] * buf;
	*buf = NULL;
}

// I N T E R F A C E /////////////////////////////////////////////////////////

char *CTVErrMsg(int errcode)
{
	static char *ctverrmsg[8] = { "Kein Fehler aufgetreten",
				   "Datei nicht gefunden",
				   "Nicht genug Speicher",
				   "Fehler beim Lesen der Datei",
				   "Datei ist kein CT-VOICE Treiber",
				   "Falsche CT-VOICE Treiberversion",
				   "Treiber I/O-Fehler",
				   "Treiber DMA-Fehler"
	};
	static char unkerrmsg[50];

	if (Inside(errcode, 0, 7)) return ctverrmsg[errcode];
	sprintf(unkerrmsg, "Unbekannter Fehlercode %d", errcode);
	return unkerrmsg;
}

BYTE InitCTVSound(char *ctvname, unsigned port, unsigned irq)
{
	BYTE errs;
	errs = LoadCTVoice(ctvname); if (errs) return errs; // Returns 0,1-4
	errs = InitCTVoice();        if (errs) return errs; // Returns 0,5-7
	CTVoiceSetBase(port);
	CTVoiceSetInt(irq);
	CTVoiceSetStatusAdr((char*)&CTVoiceStatus);
	CTVoiceSpeaker(1);
	return 0;
} // returns ErrMsg code

void DeInitCTVSound(void)
{
	CTVoiceStop();
	CTVoiceSpeaker(0);
	DeInitCTVoice();
}

void CTVPlaySound(BYTE *vocbuf)
{
	if (vocbuf)
		CTVoicePlay(vocbuf);
}

BYTE CTVSoundCheck(void)
{
	return CTVoiceStatus;
}

// BLASTER AUTODETECTION /////////////////////////////////////////////////////


BYTE AutodetectBlaster(unsigned *port, unsigned *irq, char *path)
{
	char *cenv, *cptr;
	BYTE dok = 1;
	*port = 0x220; *irq = 5; // Own defaults
	SCopy("NONE", path);

	cenv = getenv("BLASTER");
	if (cenv)
	{
		cptr = cenv; while (*cptr && (*cptr != 'A')) cptr++;
		if (*cptr == 'A') *port = 0x210 + 0x10 * ((100 * (cptr[1] - 48) + 10 * (cptr[2] - 48) + cptr[3] - 48) - 210) / 10;
		else dok = 0;
		cptr = cenv; while (*cptr && (*cptr != 'I')) cptr++;
		if (*cptr == 'I') *irq = cptr[1] - 48;
		else dok = 0;
	}
	else dok = 0;

	cenv = getenv("SOUND");
	if (cenv && (SLen(cenv) < 220))
		sprintf(path, "%s\\DRV\\CT-VOICE.DRV", cenv);
	else
		dok = 0;

	return dok;
}
