*** frob2/netboot/preboot.asm
***
*** The purpose of this code is to disable interrupts,
*** choose a Stack region, and call the C RomMain() function.
*** It replaces the ordinary "cstart" code,
*** for use in a Boot ROM.
***
*** Assumes VDG is in default initialization with screen
*** buffer at $0400.  This code uses no ROM calls.
*** The code is relocatable.  Regardless where you load it,
*** jump to (or Basic `EXEC`) its first byte to execute it.
***
*** First prints a banner at the top of the screen.
*** Then calls _RomMain.
*** It is not intended that the _RomMain function ever returns.
*** But if _RomMain returns, print the banner backwards across the
*** middle of the screen, and get stuck.
***
*** You can CLOADM and EXEC a binary to test it.
*** But it is intended that this will eventually call
*** the RomMain() routine for the netboot ROM.

    IMPORT _RomMain

    SECTION preboot
		fcb 'D'
		fcb 'K'

    EXPORT program_start
    EXPORT _abort
** for-experimental-jump {
    EXPORT _sys_rti
** for-experimental-jump }

program_start
    orcc #$50   ; disable interrupts
    ;;;;; lds #$0400  ; 1KB stack in lowest memory

    clrb
    tfr b,dp    ; dp=0

*** Display an ID at top of screen, to show preboot is running.
    leax id_string,pcr
    ldy #$0400
id_loop
    lda ,x
    anda #63
    sta ,y+
    lda ,x+
    bpl id_loop  ; while no N bit

    lbsr _main  ; and call main with no args, which probably never returns.

*** Show id backwards if RomMain returns or aborts, and get stuck.
_abort
    orcc #$50   ; disable interrupts
    leax id_string,pcr
    ldy #$0500  ; middle of screen
di_loop
    lda ,x
    anda #63
    sta ,-y
    lda ,x+
    bpl di_loop
di_stuck
    bra di_stuck

** for-experimental-jump {
; Saves state.
; But first, turns off interrupts.
_sys_rti_setjmp
    stu $7ff8 ; for U
		ldu #$7ff8
		orcc #$D0 ; set bits I, F, E.
		pshu y,x,dp,b,a,cc

		ldu _sys_rti_setjmp_comefrom,pcr
		stu $7ffa ; for PC

		sts $7ffc ; for S

    ldu $7ff8 ; restore U

_sys_rti_setjmp_comefrom
    rts

*** sys_rti() uses 0x7FF0..0x7FFF for desired struct regs6809.
* struct regs6809 {
*   byte CC; // 7FF0
*   word D; // 7FF1
*   byte DP; // 7FF3
*   word X; // 7FF4
*   word Y; // 7FF6
*   word U; // 7FF8
*   word PC; // S points here for RTI. // 7FFA
*   word S;  // Not consumed by RTI. // 7FFC
*   word Z;  // temp PC and padding for 16 bytes. // 7FFE
* };
* typedef byte jmp_buf[16];  // matching struct regs6809.
_sys_rti
    ldx $7FFA  ; desired PC, first in struct regs6809.
		stx $7FFE  ; save in Z slot.
		ldx #rti_finisher
    stx $7FFA  ; substitute our finisher for PC slot.
		lds #$7FF0 ; prepare S for RTI.
		rti
rti_finisher
    lds $7FFC   ; desired S slot.
    jmp [$7FFE] ; desired PC is in Z slot..
; and now we're running the user program.
** for-experimental-jump }

id_string  fcs / -- STRICKYAK FROBIO PREBOOT -- /

    fill $FF,256-.  ; avoid unusable second half of $C000.

*** Notice https://github.com/beretta42/sdboot/blob/master/rom.asm
*** that second halves of $C000 and $C800 are unusable.

    ENDSECTION
