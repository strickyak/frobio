; github.com/strickyak/frobio/frob2/drivers/rblemma.asm : block disk device for lemma
;
; assembled by Henry Strickland (github.com/strickyak).
; Original contributions by Henry Strickland are MIT license.
;
; Based on code from nitros9/level1/modules/emudsk.asm
; by Alan DeKok and others, with the following notes and license:
;
* EmuDisk floppy disk controller driver
* Edition #1
* 04/18/96 : Written from scratch by Alan DeKok
*                                 aland@sandelman.ocunix.on.ca
*
*  This program is Copyright (C) 1996 by Alan DeKok,
*                  All Rights Reserved.
*  License is given to individuals for personal use only.
*
*  Comments: Ensure that device descriptors mark it as a hard drive


         nam   rblemma.asm
         ttl   Random Block Lemma: os9 device driver

         ifp1
         use   defsfile
         endc

N.Drives equ 4

         org   0
         rmb   DRVBEG+(DRVMEM*N.Drives) Normal RBF device mem for N Drives
				 ; no extra variables
static_storage_size     equ   .


tylg     set   Drivr+Objct
atrv     set   ReEnt+rev
rev      set   $01
edition  set   $01

         mod   eom,name,tylg,atrv,start,static_storage_size
         fcb   DIR.+SHARE.+PEXEC.+PWRIT.+PREAD.+EXEC.+UPDAT.
name     fcs   /RBLemma/
         fcb   edition

**************************************************************************
* Init
*
* Entry: Y=Ptr to device descriptor
*        U=Ptr to device mem
*        V.PAGE and V.PORT 24 bit device address
*
* Exit:  CC carry set on error
*        B  error code if any
*
* Actions:
*
*   Set V.NDRV to number of drives supported (2)
*   Set DD.TOT to something non-zero
*   Set V.TRACK to $FF
*   Initialize device control registers?
*
* Default to only one drive supported, there's really no need for more.
* Since MESS now offers second vhd drive, EmuDsk will support it. RG
**************************************************************************

INIT     ldd   #($FF00+N.Drives)  ; 'Invalid' value & # of drives
         stb   V.NDRV,u      ; Tell RBF how many drives
         leax  DRVBEG,u      ; Point to start of drive tables
init2    sta   DD.TOT+2,x    ; Set media size to bogus value $FF0000
         sta   V.TRAK,x      ; Init current track # to bogus value
         leax  DRVMEM,x      ; Advance X to next drive's mem.
         decb
         bne   init2
				 clrb
         rts


start
toInit
         bra INIT
         nop
toRead
         bra READ
         nop
toWrite
         bra WRITE
         nop
toGetStat
         clrb
         rts
         nop
toSetStat
         clrb
				 rts
				 nop
toTerm
         clrb
				 rts
				 nop

**************************************************************************
* Read
*
* Entry: B:X = LSN
*        Y   = path dsc. ptr
*        U   = Device mem ptr
*
* Exit:  CC carry set on error
*        B  error code if any
*
* Actions:
*  Load A with read command and call GetSect
*  If error return it in reg B
*  if LSN is not zero use GETSTA to return
*  If LSN is zero copy first DD.SIZ bytes of sector to drive table
*
**************************************************************************


READ     clra                 READ command value=0
         bsr   GetSect        Get the sector
         bne   reterr         error return if not zero
         tstb                 test msb of LSN
         bne   noerr          if not sector 0, return
         leax  ,x             sets CC.Z bit if lsw of LSN not $0000
         bne   noerr          if not sector zero, return
* Copy LSN0 data to the drive table each time LSN0 is read
         ldx   PD.BUF,y       get ptr to sector buffer
         leau  DRVBEG,u       point to first drive table
         lda   PD.DRV,y       get vhd drive number from descriptor RG
         beq   copy.0         go if first vhd drive
         leau  DRVMEM,u       point to second drive table
       IFNE  H6309
copy.0   ldw   #DD.SIZ        # bytes to copy over
         tfm   x+,u+
       ELSE
copy.0   ldb   #DD.SIZ        # bytes to copy over
copy.1   lda   ,x+            grab from LSN0
         sta   ,u+            save into device static storage
         decb
         bne   copy.1
       ENDC
noerr    clrb
         rts

**************************************************************************
* Write
*
* Entry: B:X = LSN
*        Y   = path dsc. ptr
*        U   = Device mem ptr
*
* Exit:  CC carry set on error
*        B  error code if any
*
* Actions:
*  Load reg A with write command and call get sect
*  Return with error if any in reg B
**************************************************************************

WRITE    lda   #$01           WRITE command = 1
         bsr   GetSect
         bne   reterr
         clrb
         rts

reterr   tfr    a,b           Move error code to reg B
         coma                 Set the carry flag
         rts

**************************************************************************
* GetSect
*
* Entry: A = read/write command code (0/1)
*        B,X = LSN to read/write
*        Y = path dsc. ptr
*        U = Device static storage ptr
*
* Exit:  B = Error code, zero if none (also sets Carry)
*        X,Y,U are preserved
*
**************************************************************************

GetSect  pshs  u,y,x,cc          ;Save regs x and a
         orcc  #$50           ; disable interrupts throughout a sector operation.
         tfr d,u              ;temporary save D

         lda   PD.DRV,y       ;Get drive number requested
         cmpa  #N.Drives
         bhs   DriveErr       ;Err if too many.

; (byte driveNum, byte lsn1, word lsn2, word path_desc)

         pshs y   ; (4) path_desc
         pshs x   ; (3) lsn2
				 clra
				 pshs d   ; (2) lsn1

		 		 tfr u,d             ; temporary restore D
				 tfr a,b
				 clra
				 pshs d  ; (1) driveNum

				 tfr u,d             ; temporary restore D
@if
				 tsta 
				 beq @then
@else
				 jsr _BlockWrite,PCR
				 bra @endif
@then
				 jsr _BlockRead,PCR
@endif

        leas 8,s       ; drop 4 args from stack
        puls u,y,x,cc

			  clra              ; clears carry
			  tstb              ; if 0, dont set carry.
				beq @dontSetCarry
				coma              ; sets carry
@dontSetCarry
				rts


* Translate emulator error code to OS-9 code and return to caller.

DriveErr
        puls u,y,x,cc
				ldb #240   ; Unit Error
        rts

				use _generated_from_rblemmac.s

         emod
eom      equ   *
