; /* Copyright (C) 1994-2002  Matthes Bender  RedWolf Design GmbH */
; 
; /* Clonk 3.0 Radikal Source Code */
; 
; /* This is purely nostalgic. Use at your own risk. No support. */

MODEL LARGE

IGROUP group _text                ;// code segments
DGROUP group _bss, _data          ;// data segments

ASSUME CS:IGROUP, DS:DGROUP, ES:DGROUP, SS:DGROUP

_BSS segment word public 'BSS'    ;// uninitialized data (none)
_BSS ends

;//-------------------------- Constants --------------------------------------

SC_INDEX       = 3c4h             ;Indexregister des Sequencer-Ctrl.
SC_MAP_MASK    = 2                ;Nummer des Map-Mask-Registers
SC_MEM_MODE    = 4                ;Nummer des Memory-Mode-Registers

GC_INDEX       = 3ceh             ;Indexregister des Graphics-Ctrl.
GC_READ_MAP    = 4                ;Nummer des Read-Map-Registers
GC_GRAPH_MODE  = 5                ;Nummer des Graphics-Mode-Registers
GC_MISCELL     = 6                ;Nummer des Miscellaneous-Registers

CRTC_INDEX     = 3d4h             ;Indexregistes des CRT-Controllers
CC_MAX_SCAN    = 9                ;Nummer des Maximum-Scan-Line-Reg.
CC_START_HI    = 0Ch              ;Nummer des Hi-Start-Registers
CC_UNDERLINE   = 14h              ;Nummer des Underline-Registers
CC_MODE_CTRL   = 17h              ;Nummer des Mode-Control-Registers

DAC_WRITE_ADR  = 3C8h             ;DAC-Write-Adresse
DAC_READ_ADR   = 3C7h             ;DAC-Read-Adresse
DAC_DATA       = 3C9h             ;DAC-Datenregister

VERT_RETRACE   = 3DAh             ;Input Status-Register #1

MAXX           = 320              ;// horizontal screen resolution

;//----------------------------- Data ---------------------------------------

_DATA segment word public 'DATA'  ;// initialized data

vio_seg    dw 0a000h              ;// video-segment

_DATA ends

;//---------------------------- Program -------------------------------------

_TEXT  segment byte public 'CODE' ;// program code

;//----------------------- Public Declarations ------------------------------

public _InitVGA
public _SPixA
public _GPixA
public _PPage
public _LPage
public _GetFontPtr
public _BMove
public _B4Move
public _CPage

;//----------------------------- InitVGA ------------------------------------

_InitVGA   PROC FAR

	   mov   ax,0013h         ;normalen Modus 13h einstellen
	   int   10h

	   mov   dx,GC_INDEX      ;Auftrennung der Speicheradressen
	   mov   al,GC_GRAPH_MODE ;im Graphics-Mode-Register des
	   out   dx,al            ;Gaphics-Controllers über das
	   inc   dx               ;Bit #4 abschalten
	   in    al,dx
	   and   al,11101111b
	   out   dx,al
	   dec   dx

	   mov   al,GC_MISCELL    ;und das Ganze noch einmal im
	   out   dx,al            ;Miscellanous-Register über das
	   inc   dx               ;Bit #1
	   in    al,dx
	   and   al,11111101b
	   out   dx,al

	   mov   dx,SC_INDEX      ;das Memory-Mode-Register im
	   mov   al,SC_MEM_MODE   ;Sequencer-Controlller dahingehend
	   out   dx,al            ;modifizieren, daß keine Auftrennung
	   inc   dx               ;der Adressen in verschiedene Bit-
	   in    al,dx            ;Planes mehr erfolgt und die aktuelle
	   and   al,11110111b     ;Plane über das Bit-Mask-Register
	   or    al,4             ;festgelegt wird
	   out   dx,al

	   mov   ax,vio_seg       ;alle vier Bit-Planes komplett mit
	   mov   es,ax            ;dem Farbcode 00h füllen und damit
	   xor   di,di            ;den Bildschirm zunächst löschen
	   mov   ax,di
	   mov   cx,8000h
	   rep   stosw

	   mov   dx,CRTC_INDEX    ;Doppelwort-Modus über das Bit 6
	   mov   al,CC_UNDERLINE  ;im Underline-Register des CRT-
	   out   dx,al            ;Controllers abstellen
	   inc   dx
	   in    al,dx
	   and   al,10111111b
	   out   dx,al
	   dec   dx

	   mov   al,CC_MODE_CTRL  ;über das Bit 6 im Mode-Control-
	   out   dx,al            ;Register des CRT-Controllers jetzt
	   inc   dx               ;vom Word- in den Byte-Modus wechseln
	   in    al,dx
	   or    al,01000000b
	   out   dx,al

	   ret                    ;zurück zum Aufrufer

_InitVGA endp                  ;Ende der Prozedur

;//----------------------------- SPixA ---------------------------------------

_SPixA     PROC FAR

	   ARG xco:WORD, yco:WORD, col:BYTE

	   push  bp               ;Adressierung der Parameter über
	   mov   bp,sp            ;das BP-Register vorbereiten

	   push  di               ;DI auf dem Stack sichern

	   mov   ax,MAXX / 4      ;// calculate offset into video-ram
	   mul   yco              ;// and load it into DI
	   mov   cx,xco
	   mov   bx,cx
	   shr   bx,1
	   shr   bx,1
	   add   ax,bx
	   mov   di,ax

	   and   cl,3             ;Bit-Maske für die zu adressierende
	   mov   ah,1             ;Map in AH berechnen
	   shl   ah,cl
	   mov   al,SC_MAP_MASK   ;nach AL die Registernummer
	   mov   dx,SC_INDEX      ;Indexadresse des Sequencers laden
	   out   dx,ax            ;Bit-Mask-Register laden

	   mov   ax,vio_seg       ;ES auf den Video-RAM setzen
	   mov   es,ax
	   mov   al,byte ptr col  ;Mframe.farbe  ;Farbe des Punktes laden
	   stosb                  ;und in die gewählte Bit-Map schreiben

	   pop   di               ;Register wieder vom Stack holen
	   pop   bp

	   ret                    ;zurück zum Aufrufer

_SPixA endp

;//----------------------------- GPixA ---------------------------------------

_GPixA     PROC FAR               ;// return value in AL

	   ARG xc2:WORD, yc2:WORD

	   push  bp               ;Adressierung der Parameter über
	   mov   bp,sp            ;das BP-Register vorbereiten

	   push  si               ;SI auf dem Stack sichern

	   mov   ax,MAXX / 4      ;Offset in den Video-RAM berechnen
	   mul   yc2              ;und nach SI laden
	   mov   si,xc2
	   mov   cx,si
	   shr   si,1
	   shr   si,1
	   add   si,ax

	   and   cl,3             ;Bit-Maske für die zu adressierende
	   mov   ah,cl            ;Map in AH berechnen
	   mov   al,GC_READ_MAP   ;nach AL die Registernummer
	   mov   dx,GC_INDEX      ;Indexadresse des Graphic-Ctrl laden
	   out   dx,ax            ;Read-Map-Register laden

	   mov   ax,vio_seg       ;ES auf den Video-RAM setzen
	   mov   es,ax
	   mov   al,es:[si]       ;Farbe des Punktes laden

	   pop   si               ;Register wieder vom Stack holen
	   pop   bp

	   ret                    ;zurück zum Aufrufer

_GPixA     endp

;//------------------------------ LPage --------------------------------------

_LPage     PROC FAR

	   ARG pge:BYTE

	   push  bp		  ;// prepare parameter access through BP
	   mov   bp,sp

	   mov   al,4             ;// hi-byte of segment address:
	   mul   pge		  ;// page*4+A0h
	   or    al,0A0h

	   mov   byte ptr vio_seg + 1,al ;// load new segment address

	   pop bp

	   ret

_LPage     endp

;//------------------------------ PPage --------------------------------------

_PPage     PROC FAR

	   ARG pge:BYTE

	   push  bp		  ;// prepare parameter access through BP
	   mov   bp,sp

	   mov   al,64            ;Hi-Byte des Offsets = Seite * 64
	   mul   pge
	   mov   ah,al            ;Hi-Byte des Offsets nach AH

	   ;-- jetzt die neue Startadresse laden -----------------------

	   mov   dx,CRTC_INDEX    ;CRT-Controller adressieren
	   mov   al,CC_START_HI   ;Registernummer nach AL
	   out   dx,ax            ;und ab gehts

	   ;-- wieder auf den Start eines Bildschirmaufbaus warten -----

	   mov   dx,VERT_RETRACE  ;zunächst auf Ende des vertikalen
sp3:       in    al,dx            ;Rücklaufs warten
	   test  al,8
	   jne   sp3

sp4:       in    al,dx            ;dann auf den Start des Rücklaufs
	   test  al,8
	   je    sp4

	   pop bp

	   ret                    ;zurück zum Aufrufer

_PPage     endp

;//-------------------------- GetFontPtr ------------------------------------

_GetFontPtr PROC FAR		  ;// return value in DX:AX

	  push  bp                ;BP sichern

	  mov   ax,1130h          ;Register für Funktionsaufruf laden
	  mov   bh,3
	  int   10h               ;BIOS-Video-Interrupt aufrufen

	  mov   dx,es             ;Zeiger ES:BP in DX:AX zurückliefern
	  mov   ax,bp

	  pop   bp                ;BP vom Stack zurückholen
	  ret                     ;zurück zum Aufrufer

_GetFontPtr endp

;//---------------------------- BMove ----------------------------------------

_BMove     PROC FAR

sframe4    struc                  ;Struktur zum Zugriff auf den Stack
bp4        dw ?                   ;nimmt BP auf
additiv    dw ?                   ;lokale variable
restz      dw ?
movec      dw ?
datasgm    dw ?
ret_adr4   dd ?                   ;Rücksprungadresse zum Aufrufer
fpge       dw ?                   ;von Seite
fx         dw ?                   ;von X-Ordinate
fy         dw ?                   ;von Y-Ordinate
tpge       dw ?                   ;nach Seite
tx	   dw ?                   ;nach X-Ordinate
ty         dw ?                   ;nach Y-Ordinate
breite     dw ?                   ;Breite
hoehe      dw ?                   ;Höhe
bmskps     dw ?                   ;Zeiger auf Puffer mit der Bit-Mask
bmskpo     dw ?
sframe4    ends                   ;Ende der Struktur

frame      equ [ bp - bp4 ]       ;adressiert die Elemente der Struktur

	   sub   sp,8             ;8 Bytes für lokale Variablen

	   push  bp               ;Adressierung der Parameter über
	   mov   bp,sp            ;das BP-Register vorbereiten

	   push  ds
	   push  si
	   push  di

	   mov   frame.datasgm,ds

	   mov   dx,GC_INDEX      ;aktuellen Write-Mode holen und
	   mov   al,GC_GRAPH_MODE ;Write-Mode 1 einstellen
	   out   dx,al
	   inc   dx
	   in    al,dx
	   push  ax               ;aktuellen Modus auf Stack merken
	   and   al,not 3
	   or    al,1
	   out   dx,al

	   mov   al,4             ;DS auf Anfang der VON-Seite
	   mov   cl,byte ptr frame.fpge
	   mul   cl
	   or    al,0A0h
	   xchg  ah,al
	   mov   ds,ax

	   mov   al,4             ;ES auf Anfang der NACH-Seite
	   mov   cl,byte ptr frame.tpge
	   mul   cl
	   or    al,0A0h
	   xchg  ah,al
	   mov   es,ax

	   mov   ax,MAXX / 4      ;SI auf Startposition VON
	   mul   frame.fy
	   mov   si,frame.fx
	   shr   si,1
	   shr   si,1
	   add   si,ax

	   mov   ax,MAXX / 4      ;DI auf Zielposition NACH
	   mul   frame.ty
	   mov   di,frame.tx
	   shr   di,1
	   shr   di,1
	   add   di,ax

	   mov   dh,byte ptr frame.hoehe   ;DH = Zeilen
	   mov   dl,byte ptr frame.breite  ;DL = Bytes
	   shr   dl,1
	   shr   dl,1

	   mov   bx,MAXX / 4      ;BX als Offset zur nächsten Zeile
	   sub   bl,dl
	   xor   ch,ch            ;Hi-Byte des Counters ist immer 0
	   cmp   frame.bmskps,0   ;kein Hintergrund beachten?
	   jne   mt2              ;Doch, andere Kopierroutine

	   push  dx               ;DX auf dem Stack sichern
	   mov   dx,SC_INDEX      ;Zugriff auf alle Bit-Planes sichern
	   mov   ah,0Fh
	   mov   al,SC_MAP_MASK
	   out   dx,ax
	   pop   dx               ;DX zurückholen

	   ;-- Kopierroutine für alle vier Bit-Planes, also ohne Berück-
	   ;-- sichtigung des Hintergrundes

mt1:       mov   cl,dl            ;Anzahl Bytes nach CL

	   rep movsb              ;Zeile kopieren
	   add   di,bx            ;DI in nächste Zeile
	   add   si,bx            ;und auch SI
	   dec   dh               ;noch eine Zeile?
	   jne   mt1              ;Ja ---> weiter
	   jmp short mtend        ;Nein, Ausstieg aus der Routine vorb.

	   ;-- Kopierroutine für individuelle Bit-Planes unter Zuhilfe-
	   ;-- nahme des übergebenen Bit-Mask-Arrays

mt2:       mov   byte ptr frame.restz,dh ;zunächst Variablen merken, die
	   mov   byte ptr frame.movec,dl ;hier auf dem Stack in lokalen
	   mov   frame.additiv,bx        ;Variablen festgeh. werden

	   mov   al,SC_MAP_MASK   ;jetzt permanent MAP-MASK-Register
	   mov   dx,SC_INDEX      ;adressieren
	   out   dx,al
	   inc   dx               ;DX jetzt auf Datenregister

	   push  ds
	   mov   ds,frame.bmskps

	   mov   bx,frame.bmskpo  ;BX als Zeiger auf Bit-Mask-Array
	   mov   al,[bx]          ;erstes Byte laden

	   xor   ah,ah            ;Anfang mit einem geraden Byte
	   pop   ds

mt3:       mov   cl,byte ptr frame.movec ;Anzahl Bytes nach CL

mt4:       out   dx,al            ;Bit-Mask setzen
	   movsb                  ;4-Bytes kopieren

	   inc   ah               ;Odd/Even-Zähler inkrementieren
	   test  ah,1             ;jetzt wieder ungerade?
	   jne   mt5              ;Ja, Nibble verschieben

	   ;-- wieder auf geradem Byte, nächstes Byte aus Puffer -------

	   inc   bx               ;BX auf das nächste Bit-Masken-Byte
	   push  ds
	   mov   ds,frame.bmskps

	   mov   al,[bx]          ;nächste Byte laden

	   pop   ds
	   loop  mt4              ;nächsten vier Latches
	   jmp   short mt6

mt5:       shr   al,1             ;Bit-Maske des ungeraden Bytes in das
	   shr   al,1             ;Lo-Nibble holen
	   shr   al,1
	   shr   al,1
	   loop  mt4              ;nächsten vier Latches

mt6:       add   di,frame.additiv ;DI in nächste Zeile
	   add   si,frame.additiv ;und auch SI
	   dec   byte ptr frame.restz ;noch eine Zeile?
	   jne   mt3                  ;Ja ---> weiter

mtend:     mov   dx,GC_INDEX      ;wieder den alten Write-Mode ein-
	   pop   ax               ;stellen
	   mov   ah,al
	   mov   al,GC_GRAPH_MODE
	   out   dx,ax

	   pop   di
	   pop   si
	   pop   ds
	   pop   bp

	   add   sp,8             ;lokale Variablen wieder löschen
	   ret

_BMove endp

;//----------------------------- B4Move --------------------------------------

_B4Move    PROC FAR

sframe4    struc                  ;Struktur zum Zugriff auf den Stack
bp4        dw ?                   ;nimmt BP auf
additiv    dw ?                   ;lokale variable
restz      dw ?
movec      dw ?
datasgm    dw ?
ret_adr4   dd ?                   ;Rücksprungadresse zum Aufrufer
fpge       dw ?                   ;von Seite
fx         dw ?                   ;von X-Ordinate
fy         dw ?                   ;von Y-Ordinate
tpge       dw ?                   ;nach Seite
tx	   dw ?                   ;nach X-Ordinate
ty         dw ?                   ;nach Y-Ordinate
breite     dw ?                   ;Breite
hoehe      dw ?                   ;Höhe
sframe4    ends                   ;Ende der Struktur

frame      equ [ bp - bp4 ]       ;adressiert die Elemente der Struktur

	   sub   sp,8             ;8 Bytes für lokale Variablen

	   push  bp               ;Adressierung der Parameter über
	   mov   bp,sp            ;das BP-Register vorbereiten

	   push  ds
	   push  si
	   push  di

	   mov   frame.datasgm,ds

	   mov   dx,GC_INDEX      ;aktuellen Write-Mode holen und
	   mov   al,GC_GRAPH_MODE ;Write-Mode 1 einstellen
	   out   dx,al
	   inc   dx
	   in    al,dx
	   push  ax               ;aktuellen Modus auf Stack merken
	   and   al,not 3
	   or    al,1
	   out   dx,al

	   mov   al,4             ;DS auf Anfang der VON-Seite
	   mov   cl,byte ptr frame.fpge
	   mul   cl
	   or    al,0A0h
	   xchg  ah,al
	   mov   ds,ax

	   mov   al,4             ;ES auf Anfang der NACH-Seite
	   mov   cl,byte ptr frame.tpge
	   mul   cl
	   or    al,0A0h
	   xchg  ah,al
	   mov   es,ax

	   mov   ax,MAXX / 4      ;SI auf Startposition VON
	   mul   frame.fy
	   mov   si,frame.fx
	   add   si,ax

	   mov   ax,MAXX / 4      ;DI auf Zielposition NACH
	   mul   frame.ty
	   mov   di,frame.tx
	   add   di,ax

	   mov   dh,byte ptr frame.hoehe   ;DH = Zeilen
	   mov   dl,byte ptr frame.breite  ;DL = Spalten (4 Pix wdt)

	   mov   bx,MAXX / 4      ;BX als Offset zur nächsten Zeile
	   sub   bl,dl
	   xor   ch,ch            ;Hi-Byte des Counters ist immer 0

	   push  dx               ;DX auf dem Stack sichern
	   mov   dx,SC_INDEX      ;Zugriff auf alle Bit-Planes sichern
	   mov   ah,0Fh
	   mov   al,SC_MAP_MASK
	   out   dx,ax
	   pop   dx               ;DX zurückholen

	   ;-- Kopierroutine für alle vier Bit-Planes, also ohne Berück-
	   ;-- sichtigung des Hintergrundes

m4t1:      mov   cl,dl            ;Anzahl Bytes nach CL

	   rep movsb              ;Zeile kopieren
	   add   di,bx            ;DI in nächste Zeile
	   add   si,bx            ;und auch SI
	   dec   dh               ;noch eine Zeile?
	   jne   m4t1             ;Ja ---> weiter

	   mov   dx,GC_INDEX      ;wieder den alten Write-Mode ein-
	   pop   ax               ;stellen
	   mov   ah,al
	   mov   al,GC_GRAPH_MODE
	   out   dx,ax

	   pop   di
	   pop   si
	   pop   ds
	   pop   bp

	   add   sp,8             ;lokale Variablen wieder löschen
	   ret

_B4Move endp

;//------------------------------ CPage --------------------------------------

_CPage     PROC FAR

sframe4    struc                  ;// structure to access stack
bp4        dw ?                   ;// stores BP
additiv    dw ?                   ;// local variable
restz      dw ?
movec      dw ?
datasgm    dw ?
ret_adr4   dd ?                   ;// return address
fpge2      dw ?                   ;// from page
tpge2      dw ?                   ;// to page
sframe4    ends                   ;// end of structure

frame      equ [ bp - bp4 ]       ;// addresses structure elements

	   sub   sp,8             ;// 8 bytes for local variables

	   push  bp               ;Adressierung der Parameter über
	   mov   bp,sp            ;das BP-Register vorbereiten

	   push  ds
	   push  si
	   push  di

	   mov   frame.datasgm,ds

	   mov   dx,GC_INDEX      ;aktuellen Write-Mode holen und
	   mov   al,GC_GRAPH_MODE ;Write-Mode 1 einstellen
	   out   dx,al
	   inc   dx
	   in    al,dx
	   push  ax               ;aktuellen Modus auf Stack merken
	   and   al,not 3
	   or    al,1
	   out   dx,al

	   mov   al,4             ;DS auf Anfang der VON-Seite
	   mov   cl,byte ptr frame.fpge2
	   mul   cl
	   or    al,0A0h
	   xchg  ah,al
	   mov   ds,ax

	   mov   al,4             ;ES auf Anfang der NACH-Seite
	   mov   cl,byte ptr frame.tpge2
	   mul   cl
	   or    al,0A0h
	   xchg  ah,al
	   mov   es,ax

	   mov   si,0      	  ;SI auf Startposition VON

	   mov   di,0             ;DI auf Zielposition NACH

	   mov   dh,200           ;// DH number of lines
	   mov   dl,80            ;// DL bytes per line (/4)

	   mov   bx,MAXX / 4      ;BX als Offset zur nächsten Zeile
	   sub   bl,dl

	   xor   ch,ch            ;Hi-Byte des Counters ist immer 0

	   push  dx               ;DX auf dem Stack sichern
	   mov   dx,SC_INDEX      ;Zugriff auf alle Bit-Planes sichern
	   mov   ah,0Fh
	   mov   al,SC_MAP_MASK
	   out   dx,ax
	   pop   dx               ;DX zurückholen

clp1:      mov   cl,dl            ;Anzahl Bytes nach CL

	   rep movsb              ;Zeile kopieren
	   add   di,bx            ;DI in nächste Zeile
	   add   si,bx            ;und auch SI
	   dec   dh               ;noch eine Zeile?
	   jne   clp1             ;Ja ---> weiter

	   mov   dx,GC_INDEX      ;wieder den alten Write-Mode ein-
	   pop   ax               ;stellen
	   mov   ah,al
	   mov   al,GC_GRAPH_MODE
	   out   dx,ax

	   pop   di
	   pop   si
	   pop   ds
	   pop   bp

	   add   sp,8             ;lokale Variablen wieder löschen
	   ret

_CPage endp

;//--------------------------- Program End -----------------------------------

_text    ends                     ;// end of program segment

	 end                      ;// end of asm code