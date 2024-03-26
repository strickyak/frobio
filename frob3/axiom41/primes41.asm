;;  primes41.asm
;;
;;     Sieve of Eratosthenes
;;
;;     Loads at $C300.
;;     If you define JUST_C300 on the command line,
;;        you get only $C300 and beyond.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

THIS_ROM       EQU  $C000
COPY_IN_RAM    EQU  $2000
ROM_RAM_DELTA  EQU  THIS_ROM-COPY_IN_RAM
POST_RESCUE    EQU  $C300   ; Non-Rescue entry point
PAD            EQU  $FF

  IFDEF JUST_C300
        ORG POST_RESCUE
  ELSE
	ORG   THIS_ROM
	FCB   'D'		; magic bytes for ROM at $C000
	FCB   'K'

	orcc  #$50	; disable interrupts

	ldb   #$ff
	tfr   b,dp
	SETDP $FF	; optimized for the $FF device page

	bsr RepairRomToRam

	; duplicate [CD]xxx ROM space to [23]xxx in RAM.
	ldy #(POST_RESCUE-THIS_ROM)/2
	ldx #THIS_ROM
	ldu #COPY_IN_RAM
@loop
	ldd ,x++
	std ,u++
	leay -1,y
	bne @loop

	; If this actually rescues the EERPOM, it does not return.
	; Use the copy that was relocated to RAM.
	jsr  R_AttemptRescue123-ROM_RAM_DELTA

	; POST_RESCUE is the normal main() program.
	clrb                 ; Normalize DP to $00 before calling POST_RESCUE.
	tfr   b,dp
	jsr  POST_RESCUE           ; this should never return.
@stuck
	bra @stuck          ; but if it does, get stuck.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

RepairRomToRam:
	dec $C000        ; was 'D'
	ldb $C000
	cmpb #'C'        ; did it become 'C'?
	bne @return      ; no, we're not in RAM.
	ldx #$C0D0       ; just before first spot that needs repair.
	bsr Repair16BytesAfterX
	ldx #$C8B0       ; just before second spot that needs repair.
	bsr Repair16BytesAfterX
@return
	rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Repair16BytesAfterX:
	lda #$CC        ; Was INIT0 mode $CE (rom internal), make it $CC (16k int, 16k ext)
	sta $FF90      ; poke to INIT0
	ldb #16        ; FOR b = 16 TO 1 STEP -1
@loop
	clr $FFDE      ; enable ROM
	lda b,x
	clr $FFDF      ; enable RAM
	sta b,x
	decb
	bne @loop      ; NEXT b
	lda #$CE        ; Restore INIT0 mode $CE
	sta $FF90      ; poke to INIT0
	rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	use _rescue.s

	FILL PAD,POST_RESCUE-.
  ENDC
	SETDP $00	; Normalize DP to $00 for POST_RESCUE

;POST_RESCUE:
	jmp  _PrimesMain    ; Main entry at POST_RESCUE
	
	use _primes.s
  
;; END
