;;  axiom41.asm
;;
;;     Take control after 'D' 'K'.
;;     Run with $FF as the Direct Page.
;;     Repair busted ROM to RAM copy.
;;     Glue the pieces together.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

THIS_ROM       EQU  $C000
COPY_IN_RAM    EQU  $2000
ROM_RAM_DELTA  EQU  THIS_ROM-COPY_IN_RAM
POST_RESCUE    EQU  $C300   ; Non-Rescue entry point
TRAILING_DATA  EQU  $DF80   ; Trailing Data starts here.
PAD            EQU  $FF

  IFDEF JUST_C300
        ORG POST_RESCUE
  ELSE
	ORG   THIS_ROM
	FCB   'D'		; magic bytes for ROM at $C000
	FCB   'K'
	NEGA            ; opcode $40 twice, for Version $41
	NEGA            ; opcode $40 twice, for Version $41

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
	jsr  POST_RESCUE     ; this should never return.
@stuck
	bra @stuck          ; but if it does, get stuck.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

RepairRomToRam:
	dec $C000        ; was 'D'
	ldb $C000
	cmpb #'C'        ; did it become 'C'?
	bne @return      ; no, we're not in RAM.
	inc $C000        ; restore 'D'
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
	ldb   #$ff      ; in the case of Axiom, set DP right back to $FF.
	tfr   b,dp
	SETDP $FF	; optimized for the $FF device page
	jmp  _AxiomMain    ; Main entry at POST_RESCUE
	
	use _axcore.s
  
	FILL PAD,TRAILING_DATA-.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; struct axiom4_rom_tail {     // $DFC0..$DFFF
;   byte rom_reserved_14[14];  // $DFC0
;   word rom_waiter_port;      // $DFCE
;   byte rom_waiter[4];        // $DFD0
;   byte rom_dns[4];
;   byte rom_hailing[8];
;   byte rom_hostname[8];  // $DFE0
;   byte rom_reserved_3[3];
;   byte rom_mac_tail[5];  // After initial $02 byte, 5 random bytes!
;   byte rom_secrets[16];  // For Challenge/Response Authentication Protocols.
; };

;TRAILING_DATA:
	; 64 bytes $DF80-$DFBF
        FILL 0,64               ; reserved 64 bytes, for anyone.

	; 32 bytes $DFC0-$DFDF
        FILL 0,14               ; reserved 14 bytes, for frobio. [not yet used by axiom41]
	FDB 2321                ; Default waiter port (2321 for V41; 2319 for older)
	FCB 159,89,247,236      ; $DFD0: default waiter: reserved IP for pizga.net
	FCB 8,8,8,8             ; $DFD4: default DNS server: dns.google. [not yet used by axiom41]
	FCC /SWI4SWI5/          ; $DFD8: 8-byte hailing frequency 

	; 32 bytes $DFE0-$DFFF
	FCC /UNKNOWN /          ; $DFE0: default 8-byte hostname (space padded)
	FILL 0,3                ; $DFE8: default 3-byte flags
	FCC /AXIOM/             ; $DFEB: default 5-byte MAC tail
	FCC /ABCDEFGHIJKLMNOP/  ; $DFF0: default 16-byte secret

MUST_BE_E000:
	FILL 255,MUST_BE_E000-$E000  ; Assertion: difference should be 0; breaks if negative
	FILL 255,$E000-MUST_BE_E000  ; Assertion: difference should be 0; breaks if negative

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Detail of what needs repair.

***** ROMZAP ***** "=="+430c N 430c:a780     {sta   ,x+              }  a=7e b=03 x=c0da:ffff y=4314:e29d u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502958 {{        STA     ,X+     STORE THE PATCH BYTE}}  c0d9:7e
***** ROMZAP ***** "=="+430c o 430c:a780     {sta   ,x+              }  a=e2 b=02 x=c0db:ffff y=4315:9d03 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhINzvC dp=00 #502992 {{        STA     ,X+     STORE THE PATCH BYTE}}  c0da:e2
***** ROMZAP ***** "=="+430c o 430c:a780     {sta   ,x+              }  a=9d b=01 x=c0dc:ffff y=4316:037e u=3ea2:0000 s=5efc:40f8,0000 cc=eFhINzvC dp=00 #503026 {{        STA     ,X+     STORE THE PATCH BYTE}}  c0db:9d


***** ROMZAP ***** "=="+42fc N 42fc:a780     {sta   ,x+              }  a=12 b=0b x=c8b5:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502668 {{SC33B   STA     ,X+     STORE A NOP}}  c8b4:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=0a x=c8b6:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502690 {{SC33B   STA     ,X+     STORE A NOP}}  c8b5:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=09 x=c8b7:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502712 {{SC33B   STA     ,X+     STORE A NOP}}  c8b6:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=08 x=c8b8:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502734 {{SC33B   STA     ,X+     STORE A NOP}}  c8b7:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=07 x=c8b9:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502756 {{SC33B   STA     ,X+     STORE A NOP}}  c8b8:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=06 x=c8ba:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502778 {{SC33B   STA     ,X+     STORE A NOP}}  c8b9:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=05 x=c8bb:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502800 {{SC33B   STA     ,X+     STORE A NOP}}  c8ba:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=04 x=c8bc:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502822 {{SC33B   STA     ,X+     STORE A NOP}}  c8bb:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=03 x=c8bd:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502844 {{SC33B   STA     ,X+     STORE A NOP}}  c8bc:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=02 x=c8be:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502866 {{SC33B   STA     ,X+     STORE A NOP}}  c8bd:12
***** ROMZAP ***** "=="+42fc o 42fc:a780     {sta   ,x+              }  a=12 b=01 x=c8bf:ffff y=e000:0000 u=3ea2:0000 s=5efc:40f8,0000 cc=eFhInzvC dp=00 #502888 {{SC33B   STA     ,X+     STORE A NOP}}  c8be:12

; END
