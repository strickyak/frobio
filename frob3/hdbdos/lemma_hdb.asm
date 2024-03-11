p12   struct
cmd_    rmb 1
len_    rmb 2
ptr_    rmb 2
xcmd_     rmb 1
xdrive_     rmb 1
xlsn2_     rmb 1
xlsn1_     rmb 1
xlsn0_     rmb 1
xspare_    rmb 2
     endstruct

*** lemma_cmd  equ $00F3  ; = FCBTMP + 2
*** lemma_drv  equ $00F4
*** lemma_lsn  equ $00F5  ; three bytes
*** lemma_next equ $00F8  ; next unused byte

* * Dynamic Storage
*                org       $F3
* 
* VCMD           rmb       1                   SCSI/IDE unit command
* VAD0           rmb       1                   L.U.N. / sector (hi-byte)
* VAD1           rmb       2                   Sector (lo-word)
* VBLKS          rmb       2                   Block count / options
* VEXT           rmb       4                   Reserved 10 byte SCSI commands

DATAADDR  EQU $FF68
TDELAY    EQU 1
SCSIRESET EQU 1       ; safe to clear DATAADDR+SCSIRESET
RBLK	  equ $2      ; (like SDC) read-a-block opcode
WBLK      equ $3      ; (like SDC) write-a-block opcode
MAXDN     equ 255


SETUP
	sta <VCMD      ; drive command       ; <$F3
	lda IDNUM
	std <VCMD+1    ; drive num & LSN[2]  ; <$F4,5
	stx <VCMD+3    ; LSN[1] & LSN[0]     ; <$F6,7
	daa
	rts

*EXECMD
*	pshs cc,d,x,y,u
*        clr       <DCSTAT             assume no errors
*
*        lda       <VCMD               Get command byte
*        cmpa      #WBLK               WRITE command?
*        bne       Tcp1ReadSector
*	lbra	  Tcp1WriteSector


	use wiztcp1.asm


;
;
;	leas -sizeof{p12},s
;	leau ,s
;
;	sta p12.xcmd_,u  ; store disk command
;	lda IDNUM         ; get drive number
;	std p12.xdrive_,u     ; drive number & LSN[2]
;	stx p12.xlsn1_,u     ; LSN[1] & LSN[0]
;
;	clra
;	clrb
;	std p12.xspare_,u  ; zero spare
;	std p12.ptr_,u  ; zero ptr
;	ldb #12-5       ; header size minus quint
;	std p12.len_,u
;	ldb #100        ; TODO
;	std p12.cmd_,u  ; quint cmd
;	
