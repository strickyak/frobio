* _main IMPORT
* _SayReturnToLemma IMPORT
_SayAbort EXPORT
*  .area .constants
  SECTION code

; (Tentative Initial) Memory Conventions
; System Stack from Lemma: $7FFF and downward.
  org 0
Sys_Data equ $7E00  ; and upward
Sys_Load equ $7000  ; and upward

Usr_Load equ $3000  ; and upward
Usr_Stack equ $2FF0 ; and downward
Usr_Data  equ $0800 ; and upward

VDG_Text equ $400  ; and upward thru $5FF

; Above $8000 (ROM space) was problematic?
; $0600-$07FF is unused, I think.
; $0400-$05FF is VDG Ram.
; $0000-$03FF unused except CASBUF at $01DA

; Global Vars
G_SysSP equ $7E00  ; word
G_UsrSP equ $7E02  ; word
G_Which  equ $7E04  ; which interrupt vector

G_Quint  equ $7E10  ; Quint Buffer, 5 bytes.
G_QuintN equ $7E11  ; N inside Quint
G_QuintP equ $7E13  ; P inside Quint

; Interrupt vector locations.
IV_SWI3  equ  $FFF2
IV_SWI2  equ  $FFF4
IV_FIRQ  equ  $FFF6
IV_IRQ  equ  $FFF8
IV_SWI  equ  $FFFA
IV_NMI  equ  $FFFC
IV_RESTART  equ  $FFFE

  .area .text
Boot0:
  orcc #$50        ; disable interrupts
  sts  G_SysSP     ; remember our return stack

	ldx #Do_SWI3
	stx IV_SWI3
	ldx #Do_SWI2
	stx IV_SWI2
	ldx #Do_FIRQ
	stx IV_FIRQ
	ldx #Do_IRQ
	stx IV_IRQ
	ldx #Do_SWI
	stx IV_SWI
	ldx #Do_NMI
	stx IV_NMI
	ldx #Do_RESTART
	stx IV_RESTART

	jsr _main

	ldd #$FFFF
	swi2
	fcb $FF ; Ready for first process.

Do_SWI3:
	ldb #7&(IV_SWI3/2)
	stb G_Which
  bra Do_Interrupt
Do_FIRQ:
	ldb #7&(IV_FIRQ/2)
	stb G_Which
  bra Do_Interrupt
Do_IRQ:
	ldb #7&(IV_IRQ/2)
	stb G_Which
  bra Do_Interrupt
Do_SWI:
	ldb #7&(IV_SWI/2)
	stb G_Which
  bra Do_Interrupt
Do_NMI:
	ldb #7&(IV_NMI/2)
	stb G_Which
  bra Do_Interrupt
Do_RESTART:
	ldb #7&(IV_RESTART/2)
	stb G_Which
  bra Do_Interrupt
Do_SWI2:
	ldb #7&(IV_SWI2/2)
	stb G_Which
  bra Do_Interrupt ; or fall through

Do_Interrupt:
  jsr _SayReturnToLemma
	sts G_UsrSP
  lds G_SysSP     ; remember our return stack
	rts             ; return to Lemma Loop.
  ENDSECTION

	use level0.s

