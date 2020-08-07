/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design STANDARD FILE ACCESS Library

// Version: 1.0 May 1996 (DOSFileAccess,BFIO,PathFunctions,FileSearch)


#include <stdio.h>
#include <stdlib.h>
#include <SDL_filesystem.h>
#include "standard.h"

//------------------------- Path & General Functions --------------------------------

void EnforceExtension(char *fname, char *xtn) // Period is to be part of xtn
{
	while (*fname && (*fname != 46)) fname++;
	SCopy(xtn, fname, 4);
}

void AddExtension(char *fname, char *xtn)
{
	char *sptr = fname; BYTE perds = 0;
	while (*sptr) { if (*sptr == '.') perds++; sptr++; }
	if (!perds) EnforceExtension(fname, xtn);
}

void GetRunningPath(char *pth) // Returned path never ends on a backslash // YOU'RE WRONG: Returned path ALWAYS ends on a backslash now. Also reduced length: Function body was 15 lines, now is 18. 8:]
{
	/*union REGS Regs; // I don't understand how this code works,
	struct SREGS SRegs;
	char tstr[256];
	BYTE cdrv;
	Regs.h.ah=0x19;
	intdos(&Regs,&Regs);
	cdrv=Regs.h.al;
	Regs.h.ah=0x47;
	Regs.h.dl=cdrv+1;
	SRegs.ds=FP_SEG(tstr); // but i leave it because it looks beautiful.
	Regs.x.si=FP_OFF(tstr);
	intdosx(&Regs,&Regs,&SRegs);
	if (Regs.x.cflag) { pth[0]=0; return; }
	if (tstr[0]) sprintf(pth,"%c:\\%s",cdrv+65,tstr);
	else sprintf(pth,"%c:",cdrv+65);*/
	char* path = SDL_GetBasePath(); // Portable and easier too understand but better than nothing.
	SDL_strlcpy(pth, path, sizeof(pth));
	SDL_free(path); // I hope this works.
}

BYTE DeleteFile(char *fname)
{
	return remove(fname);
} // 0 NoError  1 NotFound  2 AccessDenied

BYTE FloppyProtected(int drive)
{
	/*union REGS regs;
	struct SREGS sregs;
	BYTE *buf;
	int cnt;
	if (drive > 1) return 2; // floppies only
	if (!(buf = new BYTE[512])) return 3; // allocate buffer
	for (cnt = 0; cnt < 6; cnt++) // read sector (6 attempts)
	{
		regs.x.ax = 0x0201;      // read sector 1
		regs.x.cx = 0x0001;      // cylinder 0, sector 1
		regs.x.dx = drive;       // head 0 and drive
		regs.x.bx = FP_OFF(buf); // buffer address
		sregs.es = FP_SEG(buf);
		int86x(0x13, &regs, &regs, &sregs);
		if (regs.x.ax == 0x0001) break; // reading successfull
	}
	if (cnt == 6) { delete[] buf; return 2; } // reading failed
	regs.x.ax = 0x0301; // write sector
	regs.x.cx = 0x0001;
	regs.x.dx = drive;
	regs.x.bx = FP_OFF(buf);
	sregs.es = FP_SEG(buf);
	int86x(0x13, &regs, &regs, &sregs);
	delete[] buf;
	if (regs.h.ah == 0) return 1; // writing successfull
	return 0;*/
	return 2; // Gibts nicht mehr braucht man nicht mehr fehler punkt
} // 0 Protected  1 NotProtected  2 Error  3 InsufMem

BYTE PathProtected(char *path) // Was soll das eigentlich sein
{
	int drive = -1;
	if ((path[0] == 65) || (path[0] == 61)) drive = 0;
	if ((path[0] == 66) || (path[0] == 62)) drive = 1;
	if (drive != -1) return (!(FloppyProtected(drive) == 1));
	return 0;
}

//--------------------------- DOS File Access --------------------------------
// Nicht nur DOS inzwischen (Anm. d. Red.)

BYTE DOSOpen(char *fname, FILE **handle, const char* mode)
{
	//if (_dos_open(fname, _A_NORMAL, fhnd) == 0) return 1;
	*handle = fopen(fname, mode);
	if (*handle != 0) return 1;
	return 0;
}

void DOSClose(FILE *stream)
{
	fclose(stream);
}

BYTE DOSRead(FILE *stream, void *fbuf, WORD fbtr) // btr: bytes to read; bar: bytes actually read
{
	size_t bar = fread(fbuf, fbtr, 1, stream);
	if (bar == fbtr) return 1;
	return 0;
}

BYTE DOSWrite(FILE *stream, void *fbuf, WORD fbtw)
{
	if (fwrite(fbuf, 1, fbtw, stream) == fbtw) return 1;
	return 0;
}

long FileSize(FILE* file)
{
	long pos = ftell(file);
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, pos, SEEK_SET);
	return size;
}

//------------------------- GROUP file definitions ---------------------------

typedef struct GROUPHEAD { char head[26]; int16_t entrynum; };
typedef struct INDEXENTRY { char fname[13]; int32_t size; int32_t offset; };

//-------------------- Buffered Binary File Input/Output ------------------------------
// eigentlich voll nutzlos aber gut lass mal sag ich nur

// Version 1.0  March 1996 (BFI)
//         1.1    May 1996 (BFO)
//         1.2   June 1996 (Multiple file access, GROUP file access)

//  1 Insufficient memory
//  2 File not found
//  3 File reading error
//  4 End of file
//  5 Cannot create file
//  6 File writing error
//  7 Buffer overload safety
//  8 BFI/O already active
//  9 BFI/O not active yet
// 10 Group file error
// 11 File not in group file

FILE *BFIHandle = nullptr;
BYTE *BFIBuffer;
WORD BFIBufSize, BFIBufPtr, BFIBufLoad;
long BFICFLen;

BYTE DeInitBFI(void)
{
	if (BFIHandle == nullptr) return 9;
	delete[] BFIBuffer;
	DOSClose(BFIHandle);
	BFIHandle = nullptr;
	return 0;
}

BYTE InitBFI(char *fname, WORD bufsize = 5000)
{
	int fnpipe, cnt;
	long offread;
	GROUPHEAD GHead;
	INDEXENTRY CEntry, FEntry;
	char gfname[256];
	// Check for BFI activity
	if (BFIHandle != nullptr) return 8;
	// Allocate buffer memory
	for (BFIBufSize = bufsize; (BFIBufSize >= 100) && !(BFIBuffer = new BYTE[BFIBufSize]); BFIBufSize /= 2);
	if (BFIBufSize < 100) return 1;

	if ((fnpipe = SCharPos('|', fname)) == -1) // Regular file
	{
		// Open file
		if (!DOSOpen(fname, &BFIHandle, "rb"))
		{
			delete[] BFIBuffer; BFIHandle = nullptr; return 2;
		}
		// Set CFLen
		BFICFLen = FileSize(BFIHandle);
		if (BFICFLen == -1) { DeInitBFI(); return 3; }
	}
	else // GROUP file
	{
		// Open group file
		SCopy(fname, gfname, fnpipe);
		if (!DOSOpen(gfname, &BFIHandle, "rb"))
		{
			delete[] BFIBuffer; BFIHandle = nullptr; return 2;
		}

		// Read group file header
		if (!fread(&(GHead.head), sizeof((GHead.head)), 1, BFIHandle) ||
			!fread(&(GHead.entrynum), sizeof((GHead.entrynum)), 1, BFIHandle))
		{
			DeInitBFI(); return 3;
		}

		if (!SEqual(GHead.head, "RedWolf Design GROUP")) { DeInitBFI(); return 10; }
		// Search for correct index entry
		FEntry.size = 0;
		for (cnt = 0; cnt < GHead.entrynum; cnt++)
		{
			if (!fread(&(CEntry.fname), sizeof((CEntry.fname)), 1, BFIHandle) ||
				!fread(&(CEntry.size), sizeof((CEntry.size)), 1, BFIHandle) ||
				!fread(&(CEntry.offset), sizeof((CEntry.offset)), 1, BFIHandle))
			{
				DeInitBFI(); return 3;
			}
			if (SEqual(CEntry.fname, fname + fnpipe + 1)) FEntry = CEntry;
		}
		if (FEntry.size == 0) { DeInitBFI(); return 11; }
		// Read to offset (using BFIBuffer)
		for (offread = FEntry.offset; offread > 0; offread -= BFIBufSize)
			//if (!DOSRead(BFIHandle, BFIBuffer, Min(offread, BFIBufSize)))
			if (!fread(BFIBuffer, sizeof(BYTE), Min(offread, BFIBufSize), BFIHandle))
			{
				DeInitBFI(); return 3;
			}
		// Set CFLen
		BFICFLen = FEntry.size;
	}

	// Reset reading pointers
	BFIBufLoad = 0; BFIBufPtr = 0;
	return 0;
}

BYTE GetBFI(void *vtbuf, WORD size = 1)
{
	WORD transfer;
	BYTE *tbuf = (BYTE*)vtbuf;
	if (BFIHandle == nullptr) return 9;
	while (size > 0)
		if (BFIBufPtr < BFIBufLoad) // Transfer buffer
		{
			transfer = Min(BFIBufLoad - BFIBufPtr, size);
			MemCopy(BFIBuffer + BFIBufPtr, tbuf, transfer);
			BFIBufPtr += transfer;
			tbuf += transfer;
			size -= transfer;
		}
		else // Load to buffer
		{
			if ((BFIBufLoad = fread(BFIBuffer, 1, BFIBufSize, BFIHandle)) == 0 && !feof(BFIHandle))
			{
				//printf("GetBFI, BFIBufLoad = fread, error %d, eof %d\n", ferror(BFIHandle), feof(BFIHandle));
				DeInitBFI(); return 3;
			}
			if (BFIBufLoad == 0)
			{
				DeInitBFI(); return 4;
			}
			BFIBufPtr = 0;
		}
	return 0;
}

DWORD BFIFileLength(void)
{
	return BFICFLen;
}

FILE *BFOHandle = nullptr;
BYTE *BFOBuffer;
WORD BFOBufSize, BFOBufLoad;

BYTE InitBFO(char *fname, WORD bufsize = 5000)
{
	if (BFOHandle != nullptr) return 8;
	// Allocate buffer memory
	for (BFOBufSize = bufsize; (BFOBufSize >= 100) && !(BFOBuffer = new BYTE[BFOBufSize]); BFOBufSize /= 2);
	if (BFOBufSize < 100) return 1;
	// Create file
	if (!DOSOpen(fname, &BFOHandle, "wb"))
	{
		delete[] BFOBuffer; return 5;
	}
	// Reset reading pointers
	BFOBufLoad = 0;
	return 0;
}

BYTE DeInitBFO(void)
{
	BYTE rval = 0;
	if (BFOHandle == nullptr) return 9;
	if (!DOSWrite(BFOHandle, BFOBuffer, BFOBufLoad)) rval = 6;
	DOSClose(BFOHandle);
	delete[] BFOBuffer;
	BFOHandle = nullptr;
	return rval;
}

BYTE PutBFO(void *vtbuf, WORD size = 1)
{
	WORD transfer;
	BYTE *fbuf = (BYTE*)vtbuf;
	if (BFOHandle == nullptr) return 9;
	while (size > 0)
		if (BFOBufLoad < BFOBufSize) // Transfer to buffer
		{
			transfer = Min(BFOBufSize - BFOBufLoad, size);
			MemCopy(fbuf, BFOBuffer + BFOBufLoad, transfer);
			BFOBufLoad += transfer;
			fbuf += transfer;
			size -= transfer;
		}
		else // Save buffer to disk
		{
			if (BFOBufLoad > BFOBufSize)
			{
				BFOBufLoad = BFOBufSize; DeInitBFO(); return 7;
			}
			if (!DOSWrite(BFOHandle, BFOBuffer, BFOBufLoad))
			{
				DeInitBFO(); return 6;
			}
			BFOBufLoad = 0;
		}
	return 0;
}

//---------------------------- File Search ----------------------------------

typedef struct DTAENTRY {
	BYTE reserved[21];
	BYTE attr;
	WORD time, date;
	DWORD size;
	char name[13];
};

// Attribute byte:  1 ReadOnly  2 Hidden      4 System
//		    8 Label    16 Directory  32 Archive

DTAENTRY DTAEntry;
BYTE DTAEntryLoaded;
/* Ein anderes mal vielleicht...
BYTE DTAFirstEntry(char *path, BYTE atr)
{
	union REGS Regs;
	struct SREGS SRegs;
	Regs.h.ah = 0x4E;
	Regs.x.cx = atr;
	SRegs.ds = FP_SEG(path);
	Regs.x.dx = FP_OFF(path);
	intdosx(&Regs, &Regs, &SRegs);
	return !Regs.x.cflag;
}

BYTE DTANextEntry(void)
{
	union REGS Regs;
	Regs.h.ah = 0x4F;
	intdos(&Regs, &Regs);
	return !Regs.x.cflag;
}

void DTASet(DTAENTRY *dptr)
{
	union REGS Regs;
	struct SREGS SRegs;
	Regs.h.ah = 0x1A;
	SRegs.ds = FP_SEG(dptr);
	Regs.x.dx = FP_OFF(dptr);
	intdosx(&Regs, &Regs, &SRegs);
}

BYTE InitFileSearch(char *path)
{
	DTASet(&DTAEntry);
	DTAEntryLoaded = 0;
	if (DTAFirstEntry(path, 31))
	{
		DTAEntryLoaded = 1; return 1;
	}
	return 0;
}

BYTE SearchNextFile(char *nameto)
{
	if (!DTAEntryLoaded)
		if (!DTANextEntry())
		{
			if (nameto) nameto[0] = 0; return 0;
		}
	if (nameto) SCopy(DTAEntry.name, nameto, 12);
	DTAEntryLoaded = 0;
	return 1;
}

BYTE GetFileInfo(char *fname, WORD *time, WORD *date, DWORD *size)
{
	if (!InitFileSearch(fname)) return 0;
	// If successfull, file info will be loaded into global DTAEntry
	*time = DTAEntry.time;
	*date = DTAEntry.date;
	*size = DTAEntry.size;
	return 1;
}*/

//---------------------------- Text File Handling ---------------------------

BYTE LocateInFile(FILE *file, char *index, BYTE wrap = 1)
{
	int fchr, needok, idxcnt = 0, loops = 0;        // Locates file pointer to
	long lpos;                               // end of located string
	BYTE exok = 0;
	needok = SLen(index);
	if (!file) return 0;
	do
	{
		fchr = fgetc(file);
		if (fchr == EOF)
			if (wrap) { rewind(file); fchr = fgetc(file); loops++; if (loops >= 2) exok = 2; }
			else return 0;
			if (fchr == index[idxcnt])
			{
				idxcnt++; if (idxcnt == needok) exok = 1;
			}
			else
				idxcnt = 0;
	} while (!exok);
	if (exok == 1) return 1;
	return 0;
}

BYTE ReadFileLine(FILE *fhnd, char *tobuf, int maxlen) // Reads mxlen string
{			  			       // or till EOL to buf
	int cread;
	if (!fhnd) return 0;
	for (cread = 0; cread < maxlen; cread++)
	{
		*tobuf = fgetc(fhnd);
		if (!(*tobuf) || (*tobuf == EOF) || (*tobuf == 0x0A)) break;
		tobuf++;
	}
	*tobuf = 0;
	if (cread == 0) return 0;
	return 1;
}

BYTE ReadFileInfoLine(FILE *fhnd, char *info, char *tbuf, int maxlen = 256)
{
	if (!LocateInFile(fhnd, info)) return 0;
	if (!ReadFileLine(fhnd, tbuf, maxlen)) return 0;
	return 1;
}

void AdvanceFileLine(FILE *fhnd)
{
	int cread, loops = 0;
	if (!fhnd) return;
	do
	{
		cread = fgetc(fhnd);
		if (cread == EOF) { rewind(fhnd); loops++; }
	} while ((cread != 0x0A) && (loops < 2));
}

DWORD ReadFileUntil(FILE *fhnd, char *tbuf, char smark, int maxlen)
{
	DWORD rcnt = 0;
	int cread;
	while (maxlen > 0)
	{
		cread = fgetc(fhnd);
		if ((cread == smark) || (cread == EOF)) break;
		*tbuf = cread; tbuf++;
		maxlen--;
		rcnt++;
	}
	*tbuf = 0;
	return rcnt;
}
