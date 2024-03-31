POLCAT equ $A000

PTR_TO_TCP1READ equ $FA0C
PTR_TO_TCP1WRITE equ $FA0E
OLD_POLCAT  equ $FA10
INKEY_TRAP_INIT  equ $FA12

  ORG INKEY_TRAP_INIT

  pshs cc,d,x
  orcc #$50      ; no interrupts, please
  ldb #$7E          ; "JMP [extended]"
  stb $a1cb         ; beginning of KEYIN
  ldx #INKEY_TRAP_EACH
  stx $a1cc         ; target of JMP
  puls cc,d,x,pc

INKEY_TRAP_EACH:
  pshs b,x,y,u
  bsr CALL_OLD_KEYIN
  cmpa #12            ; CLEAR key produces From Feed ^L
  bne @skip
  bsr INKEY_CLEAR
@skip
  tsta                ; caller expects Z if no key (A is 0).
  puls b,x,y,u,pc

CALL_OLD_KEYIN:
  pshs u,x,b     ; first instruction that was at KEYIN
  ldu #$ff00     ; second instruction that was at KEYIN
  jmp $a1d0      ; join old KEYIN at third instruction

INKEY_CLEAR:
  pshs cc,d
  orcc #$50      ; no interrupts, please

  lda #$96      ; yellow checkerboard char
  ldx #$0400    ; top of screen
  ldb #64        ; zap top two lines
@loop
  sta ,x+
  decb
  bne @loop

  ldx #50000     ; delay count
@loop
  mul            ; delay
  mul
  mul
  mul
  mul
  leax -1,x
  bne @loop

  puls cc,d,pc

  END INKEY_TRAP_INIT

;;;;;;;;;;;;;;;;;;;; old
;POLCAT equ $A000
;
;PTR_TO_TCP1READ equ $FA0C
;PTR_TO_TCP1WRITE equ $FA0E
;OLD_POLCAT  equ $FA10
;INKEY_TRAP_INIT  equ $FA12
;
;  ORG INKEY_TRAP_INIT
;
;  ldx POLCAT        ; load what is at $A000, the POLCAT inkey relay
;  stx OLD_POLCAT    ; and store a copy
;  ldx #INKEY_TRAP_EACH
;  stx POLCAT        ; alter the relay to come to us
;  rts
;
;INKEY_TRAP_EACH:
;  jsr [OLD_POLCAT]    ; do the actual inkey
;  cmpa #12            ; CLEAR key produces From Feed ^L
;  bne @skip
;  bsr INKEY_CLEAR
;@skip
;  tsta                ; caller expects Z if no key (A is 0).
;  rts
;
;INKEY_CLEAR:
;  pshs cc,d,x,y,u
;  orcc #50      ; no interrupts, please
;
;  lda #$96      ; yellow checkerboard char
;  ldx #$0400    ; top of screen
;  ldb #64        ; zap top two lines
;@loop
;  sta ,x+
;  decb
;  bne @loop
;
;  ldx #50000     ; delay count
;@loop
;  mul            ; delay
;  mul
;  mul
;  mul
;  mul
;  leax -1,x
;  bne @loop
;
;  puls cc,d,x,y,u,pc
;
;  END INKEY_TRAP_INIT
