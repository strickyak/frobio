*** frob2/netboot/preboot.asm
***
*** The purpose of this code is to disable interrupts,
*** choose a Stack region, and call the C main() function.
*** It replaces the ordinary "cstart" code,
*** for use in a Boot ROM.
***
*** Assumes VDG is in default initialization with screen
*** buffer at $0400.  This code uses no ROM calls.
*** The code is relocatable.  Regardless where you load it,
*** jump to (or Basic `EXEC`) its first byte to execute it.
***
*** First prints a banner at the top of the screen.
*** Then calls _main.
*** It is not intended that the main function ever returns.
*** But if _main returns, print the banner backwards across the
*** middle of the screen, and get stuck.
***
*** You can CLOADM and EXEC a binary to test it.
*** But it is intended that this will eventually call
*** the main() routine for the netboot ROM.

    IMPORT program_start
    IMPORT _main

    SECTION start

preboot
    orcc #$50   ; disable interrupts
    lds #$0400  ; 1KB stack in lowest memory

    clrb
    tfr b,dp    ; dp=0

    ldy #$0000  ; no global vars
    ldu #$0000  ; no previous frame

*** Display an ID at top of screen, to show preboot is running.
    leax id_string,pcr
    ldy #$0400
id_loop
    lda ,x
    anda #63
    sta ,y+
    lda ,x+
    bpl id_loop  ; while no N bit

    pshs y,u    ; 8 bytes of zeros onto stack
    pshs y,u
    lbsr _main  ; and call main with no args, which never returns.

*** Show id backwards if main returns, and get stuck.
not_reached
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

id_string  fcs / -- STRICKYAK FROBIO PREBOOT -- /

    ENDSECTION
