; lem.blk : block disk device for lemma

         nam   lem.blk
         ttl   os9 device driver

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

         org   0
         mod   eom,name,tylg,atrv,start,static_storage_size

         fcb   DIR.+SHARE.+PEXEC.+PWRIT.+PREAD.+EXEC.+UPDAT.  ; driver permissions
name     fcs   /lem.blk/
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

INIT     ldd   #($FF+N.Drives)  ; 'Invalid' value & # of drives
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
*        B,X,Y,U are preserved
*
**************************************************************************

GetSect  pshs  u,y,x,a        ;Save regs x and a
         tfr d,u              ;temporary save D

         lda   PD.DRV,y       ;Get drive number requested
         cmpa  #N.Drives
         bhs   DriveErr       ;Err if too many.

; (byte driveNum, byte lsn1, word lsn2, word path_desc)

         pshs y   ; path_desc
         pshs x   ; lsn2
				 clra
				 pshs d   ; lsn1

		 		 tfr u,d             ; temporary restore D
				 tfr a,b
				 clra
				 pshs d  ; driveNum

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

        puls u,y,x,a

			  clra              ; clears carry
			  tstb              ; if 0, dont set carry.
				beq @dontSetCarry
				coma              ; sets carry
@dontSetCarry
				rts


* Translate emulator error code to OS-9 code and return to caller.

DriveErr
        puls u,y,x,a
				ldb #240   ; Unit Error
        rts

				use _generated_from_rblemma.s

         emod
eom      equ   *
