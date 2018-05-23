/* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */

/* Clonk 3.0 Radikal Source Code */

/* This is purely nostalgic. Use at your own risk. No support. */

// RedWolf Design  SVI  STANDARD VISUAL INTERFACE Header (C) by M.Bender

// Version and date see library file

//------------------- Window and Object Type Definitions ---------------------

// Window types

#define NOBACK   0
#define STANDARD 1
#define CARDBOX  2
#define PLAIN    3
#define DELETED  4

// Object types

#define BUTTON     0
#define TEXT       1
#define PICTURE    2
#define HSCROLL    3
#define VSCROLL    4
#define VALBOX     5
#define FRAME      6
#define FLAGBOX    7
#define VALBOXB    8 // Extended only
#define ICON       9
#define LISTBOX   10 // Extended
#define SELECTBOX 11
#define TEXTBOX   12
#define SELECTOR  13
#define HYPERTEXT 14 // Extended

// ULink types

#define OBJFUNC  0
#define VALBINC  1
#define VALBDEC  2
#define EXCREDR  3
#define TOGHID   4
#define LBCACT   5
#define CLOSEXC  6
#define LBSCRUP  7
#define LBSCRDN  8
#define OBJFUNCA 9
#define CLWINEXC 10

// ExitReturnValues (or SVI-actions)

// 0 Nothing

// 1-199 Exit SVI, return value

#define XRVCLOSE 200
#define XRVHELP  201
#define XRVTAB_O 202

/*

Object Data Usage
-----------------

Object	  bd0   bd1   bd2   id0   id1   id2   id3   id4   bptr  iptr

BUTTON    XRVal NoULR ----- ----- ----- ----- ----- ----- ----- -----
TEXT      Form  CType TType FCol  BCol  ----- ----- ----- (Txt) (Idx)
PICTURE   SrcPg TType STWdt SrcX  SrcY  ----- ----- ----- ----- (Idx)
HSCROLL   TType ----- ----- RngLo RngHi Step  KStep ----- ----- Value
VSCROLL   ----- ----- ----- RngLo RngHi Step  KStep ----- ----- Value
VALBOX    TType UnitC LongU RngLo RngHi Step  Desgn ----- ----- Value
FRAME     TType ----- ----- FCol  BCol  TBCol ----- ----- ----- -----
FLAGBOX   TType ----- ----- ----- ----- ----- ----- ----- ----- Value
VALBOXB   TType UnitC ----- RngLo RngHi Step  Desgn ----- ----- Value
ICON      SrcPg STWdt TType SrcX  SrcY  SrcWd SrcHg ----- ----- Index
LISTBOX   LstID ClNmX ClNmY ClWdt ClHgt 1stCl ClNum Selct ----- (Idx)
SELECTBOX SrcPg ClSrW ClSrH RngLo RngHi ClSrX ClSrY TType ----- Value
TEXTBOX   TType XRVal ----- MaxLn Cursr ----- ----- ----- Text  -----
SELECTOR  TType ----- ----- Elmt# ----- ----- ----- ----- ElmtN Value
HYPERTEXT ----- ----- ----- LIndx MaxLI ----- ----- ----- HTBuf -----


UpdateLink Types
----------------

Type      Type of update                             Uses

OBJFUNC   Update tobj iptr with value from exfunc    fwin/fobj,tobj,exfunc
VALBINC   Increase ValBox value (iptr)               fobj,tobj
VALBDEC   Decrease ValBox value (iptr)		     fobj,tobj
EXCREDR   Call exfnc if set, then redraw tobj if set fwin/fobj,(tobj),(exfunc)
TOGHID    Toggle hiding and redraw of all objs in
	  win except HIDENEVER                       fobj
LBCACT    Calls ExtLBCAction (aid=par), tobj is list fobj,tobj
LBSCRUP   Scrolls up ListBox (LB1stCell-=LBCellNumX) fobj,tobj
LBSCRUP   Scrolls down L.Box (LB1stCell+=LBCellNumX) fobj,tobj
CLOSEXC   Executed by CloseWin, call exfunc(expar)   fwin,exfunc
	  Aborts CloseWin if exfunc returns 1
OBJFUNCA  Same as OBJFUNC, executed by ExecAULinks
CLWINEXC  Closes fwin, calls exfunc    		     fobj,fwin,exfunc
	  fwin-action is no valid link exec trigger

*/

#define HIDEOFF   0
#define HIDE      1
#define HIDENEVER 2

#define ACC   1
#define NOACC 0

//----------------------------- Color Constants ------------------------------

const BYTE CBlack=0,CGray1=1,CGray2=2,CGray3=3,CGray4=4,CGray5=5,CWhite=6;
const BYTE CDRed=7,CDGreen=8,CDBlue=9,CRed=10,CGreen=11,CBlue=12,CYellow=13;

const BYTE CIMsg=CGray3;

//---------------------------- Interface Types ------------------------------

typedef struct COMMAND { int x,y,b; BYTE cn,xn; };

//------------------------------ Object Types -------------------------------

const int WinTitleLen=30;
const int ObjTitleLen=30;

typedef struct WINDOW; // hui

typedef struct OBJECT { BYTE type,acc,hid;
			int x,y,wdt,hgt;
			char title[ObjTitleLen+1];
			BYTE bdata[3];
			int idata[5];
			BYTE *bptr;
			int *iptr;
			WINDOW *inwin;
			OBJECT *next;
		      };


typedef struct WINDOW { BYTE type,priority;
			BYTE escxrv;
			int x,y,wdt,hgt;
			int cbnum,cboutof;
			char title[WinTitleLen+1];
			BYTE *hindex;
			BYTE drawn;
			OBJECT *fobj;
			WINDOW *next;
		      };

typedef struct ULINK { BYTE type;
		       WINDOW *fwin;   // Exec after any action in that win
		       OBJECT *fobj;   // Exec after action on that obj
		       OBJECT *tobj;   // Update this obj according to type
		       int (*exfunc)(int); // MultiPurpose extern function
		       int expar;      // Parameter for excall
		       ULINK *next;
		     };


//--------------------------- Keyboard Codes --------------------------------

// Standard

#define KCENTER  13
#define KCTAB     9
#define KCESC    27
#define KCBACK    8

// Extended

#define KCSHTAB  15
#define KCALTAB 165
#define KCCRSLF  75
#define KCCRSRT  77
#define KCCRSUP  72
#define KCCRSDN  80
#define KCHOME   71
#define KCEND    79
#define KCDEL    83
#define KCF1     59
#define KCPAGEUP 73
#define KCPAGEDN 81