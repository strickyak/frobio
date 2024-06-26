POLCAT equ $A000
CASBUF equ $01DA
MYBUF equ CASBUF+60
PUTCHR equ $A282

PTR_TO_TCP1READ equ $FA0C
PTR_TO_TCP1WRITE equ $FA0E
OLD_POLCAT  equ $FA10
INKEY_TRAP_INIT  equ $FA12
HIJACK_KEY_ASCII equ 12  ; Form Feed ^L, made by coco CLEAR key.

  ORG INKEY_TRAP_INIT

  pshs cc,d,x
  orcc #$50      ; no interrupts, please
  ldb #$7E          ; "JMP [extended]"
  stb $a1cb         ; beginning of KEYIN
  ldx #INKEY_TRAP_EACH  ; new KEYIN replacement routine.
  stx $a1cc         ; target of JMP
  puls cc,d,x,pc    ; restore interrupts & return

INKEY_TRAP_EACH:
  pshs b,x,y,u
  bsr CALL_OLD_KEYIN      ; actually do a KEYIN call
  cmpa #HIJACK_KEY_ASCII  ; is this a hijack?
  bne @skip               ; skip if not.
  bsr INKEY_HIJACK        ; do the hijack, if so.
  clra           ; now act as if nothing had happened.
@skip
  tsta           ; caller expects A==0 and Z set, if no keystroke.
  puls b,x,y,u,pc

CALL_OLD_KEYIN:
  pshs u,x,b     ; first instruction that was at KEYIN
  ldu #$ff00     ; second instruction that was at KEYIN
  jmp $a1d0      ; join old KEYIN at third instruction

INKEY_HIJACK:
  pshs cc
  orcc #$50      ; no interrupts, please

  leas -24,s     ; local vars (alternative?: ldu #CASBUF)
  leau ,s        ; U points to them.

  ldx #HIJACK_KEY_PACKET   ; tell lemma that HIJACK occurred.
  ldy #5
  jsr [PTR_TO_TCP1WRITE]

  ; Send page 0, MMAP, and Palette registers to server.
  ldx #$0000     ; page 0
  ldy #256
  jsr [PTR_TO_TCP1WRITE]
  ldx #$FFA0     ; MMAP and Palette regs.
  ldy #32
  jsr [PTR_TO_TCP1WRITE]

SERVE_HIJACK:
  ldx #MYBUF
  ldy #5
  jsr [PTR_TO_TCP1READ]  ; get command header

  ldb MYBUF
  cmpb #CMD_HDBDOS_HIJACK
  lbeq DONE_HIJACK

  cmpb #CMD_PEEK2
  beq DO_PEEK2

  cmpb #CMD_POKE
  beq DO_POKE

  cmpb #CMD_GETCHAR
  lbeq DO_GETCHAR

  cmpb #CMD_CALL
  beq DO_CALL

  ; otherwise
  ldx #$0400    ; splash junk on the screen, to show it.
  bra POKE_AT

DO_POKE:
  ldx MYBUF+3   ; P
POKE_AT:
  ldy MYBUF+1   ; N
  jsr [PTR_TO_TCP1READ]  ; poke payload to P
  bra SERVE_HIJACK

DO_PEEK2:
  ldx #CASBUF
  ldy #4
  jsr [PTR_TO_TCP1READ]  ; read arguments to CASBUF

  ldy CASBUF    ; payload N
  sty PEEK2_PACKET+1  ; N
  ldx CASBUF+2        ; payload P
  stx PEEK2_PACKET+3  ; P

  ldx #PEEK2_PACKET   ; send the packet header.
  ldy #5
  jsr [PTR_TO_TCP1WRITE]

  ldx CASBUF+2        ; address (from payload P)
  ldy CASBUF          ; count (from payload N)
  jsr [PTR_TO_TCP1WRITE]  ; send the peeked data

  lbra SERVE_HIJACK

DO_CALL:
  ldx MYBUF+3    ; P
  jsr ,x
  lbra SERVE_HIJACK

DO_GETCHAR:
  lbsr CALL_OLD_KEYIN
  beq DO_GETCHAR   ; keep looking while no key
  sta GETCHAR_PACKET+4
  ldx #GETCHAR_PACKET
  ldy #5
  jsr [PTR_TO_TCP1WRITE]
  lbra SERVE_HIJACK

DONE_HIJACK:
  leas 24,s
  puls cc,pc    ; restore interrupts & return

  IFDEF HEX_DEBUG_PRINTS

HEXCHARS:
  fcc /0123456789ABCDEF/

PUTHEX: ; emit four hex chars of X, with prefix in A
  pshs d,x,y,u
  pshs x

  jsr PUTCHR
  lda #'$'
  jsr PUTCHR

  puls d
  bsr PUTHEX4

  lda #' '
  jsr PUTCHR
  puls d,x,y,u,pc

PUTHEX4: ; emit two hex chars of D
  pshs d
  bsr PUTHEX2
  puls d
  tfr b,a
  bsr PUTHEX2
  rts

PUTHEX2: ; emit two hex chars of A
  tfr a,b
  lsra
  lsra
  lsra
  lsra
  bsr PUTHEX1
  tfr b,a
  bsr PUTHEX1
  rts

PUTHEX1:  ; emit Hex Char of low nybble of A
  pshs d,x,y,u
  anda #15
  ldx #HEXCHARS
  ldb #'!'
  lda a,x
  jsr PUTCHR
  puls d,x,y,u,pc

  ENDC

HIJACK_KEY_PACKET:
  fcb CMD_HDBDOS_HIJACK
  fdb 256+32  ; len(p0) + len(MMU + Palette)
  fdb 0
GETCHAR_PACKET:
  fcb CMD_GETCHAR
  fdb 0     ; empty payload
  fcb 0
  fcb 0     ; poke keystroke here
PEEK2_PACKET:
  fcb CMD_PEEK2
  fdb 0     ; N
  fdb 0     ; P


CMD_POKE    equ 0
CMD_CALL    equ 255
CMD_GETCHAR equ 192
CMD_KEYBOARD equ 193
CMD_SUM      equ 194
CMD_PEEK2    equ 195  // request: n=4, p=FFFF (wanted n, wanted p)  reply: n, p, data.
CMD_BEGIN_MUX equ 196
CMD_MID_MUX   equ 197
CMD_END_MUX   equ 198
CMD_LEVEL0  equ 199 // when Level0 needs attention, it sends 199.
CMD_LOG     equ 200
CMD_INKEY   equ 201
CMD_PUTCHAR equ 202
CMD_PEEK    equ 203
CMD_DATA    equ 204
CMD_SP_PC   equ 205
CMD_REV     equ 206
CMD_BLOCK_READ     equ 207 // block device
CMD_BLOCK_WRITE    equ 208 // block device
CMD_BLOCK_ERROR    equ 209 // nack
CMD_BLOCK_OKAY     equ 210 // ack
CMD_BOOT_BEGIN     equ 211 // boot_lemma
CMD_BOOT_CHUNK     equ 212 // boot_lemma
CMD_BOOT_END       equ 213 // boot_lemma
CMD_LEMMAN_REQUEST equ 214 // LemMan
CMD_LEMMAN_REPLY   equ 215 // LemMan
CMD_RTI equ 216
CMD_ECHO equ 217  // reply with CMD_DATA, with high bits toggled.
CMD_DW equ 218
CMD_HDBDOS_SECTOR equ 219
CMD_HDBDOS_EXEC equ 220
CMD_HDBDOS_HIJACK equ 221

FIN
  END INKEY_TRAP_INIT
