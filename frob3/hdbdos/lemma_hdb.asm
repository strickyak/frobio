;; lemma_hdb.asm ;;

  use cocoio.d
  use w5100s.d

SK_CR_SEND equ  $20
SK_CR_RECV equ  $40

S1_RX_BUF  equ  $6800
S1_TX_BUF  equ  $4800

SKBUFSIZE  equ  $800
SKBUFMASK  equ  $7ff

CMD_HDBDOS_SECTOR  equ  219
CMD_HDBDOS_EXEC    equ  220

PTR_TO_TCP1READ equ $FA0C
PTR_TO_TCP1WRITE equ $FA0E

vars  struct     ; stack variable frame.
wizbuf        rmb 2
xptr        rmb 2
xoffset rmb 2
xsize   rmb 2
len        rmb 2
chunklen  rmb 2
buf     rmb 2     ; 6809 ram buffer
h_cmd    rmb 1     ; start of quint.
h_len    rmb 2     ; payload len of quint
h_param    rmb 2     ; param of quint (unused now)
h_drive_cmd     rmb 1
h_drive_num     rmb 1
h_lsn2     rmb 1
h_lsn1     rmb 1
h_lsn0     rmb 1
    endstruct

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  HDBDOS.ASM requires these to be defined.

DATAADDR  EQU $FF68
TDELAY    EQU 1
SCSIRESET EQU 1       ; safe to clear DATAADDR+SCSIRESET (the HI addr reg)
RBLK          equ $2      ; (like SDC) read-a-block opcode
WBLK      equ $3      ; (like SDC) write-a-block opcode
MAXDN     equ 255

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  SETUP
;;  Input:  A=command IDNUM=drive_num B=lsn[2] X=lsn[1,0]
;;  Output:  they've been saved in Direct Page at VCMD.

SETUP
        sta <VCMD      ; drive command       ; <$F3
        lda IDNUM
        std <VCMD+1    ; drive num & LSN[2]  ; <$F4,5
        stx <VCMD+3    ; LSN[1] & LSN[0]     ; <$F6,7
	; publish TCP Read & Write entry points.
	ldx #Tcp1Read
	stx $FA0C
	ldx #Tcp1Write
	stx $FA0E
        rts
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  EXECMD
;;  Inputs are already saved by SETUP, above.
;;  No outputs except the side-effects of a sector Read or Written.

EXECMD
  pshs d,x,y,u
* space for variables on stack
  leas -sizeof{vars},s
  leau ,s

redoEXECMD
  ldx #5    ; size for Read Sector header
  ldb <VCMD
  cmpb #WBLK
  bne @skip
  ldx #256+5   ; size for Write Sector header + payload
@skip
  stx vars.h_len,u

  ldb #CMD_HDBDOS_SECTOR  ; 219 is HDBDOS sector
  stb vars.h_cmd,u

  ldd <DCBPT          ; ptr to sector buffer, just FYI for server.
  std vars.h_param,u  ; reserved, so zero

  clr    <DCSTAT   ; assume will return no error.
  ldd <VCMD+1
  std vars.h_drive_num,u
  ldd <VCMD+3
  std vars.h_lsn1,u
  ldb <VCMD
  stb vars.h_drive_cmd,u

  leax vars.h_cmd,u   ; ptr to start of header
  ldy #10             ; header size
  lbsr Tcp1Write       ; SEND HEADER


  ldb <VCMD
  cmpb #WBLK
  beq WriteSector

ReadSector:
  leax vars.h_cmd,u  ; Re-use header for response header
  ldy #10      ; 10 byte header
  bsr Tcp1Read ; read it
  ldx <DCBPT          ; ptr to sector buffer
  ldy #256  ; sector size
  bsr Tcp1Read ; read it

  ; Special Hack to execute extra code from server!
  lda vars.h_cmd,u   ; check the response command
  cmpa #CMD_HDBDOS_SECTOR
  beq AssumeNoError
  cmpa #CMD_HDBDOS_EXEC
  beq ConsumeExecPacket
badReadCmd:
  bra badReadCmd    ; get stuck if unexpected cmd
ConsumeExecPacket:
  jsr [DCBPT]
  bra ReadSector

WriteSector:
  ldx <DCBPT          ; ptr to sector buffer
  ldy #256            ; sector size
  lbsr Tcp1Write

AssumeNoError:
  leas sizeof{vars},s
  puls d,x,y,u,pc


* Tcp1Read
*    Receive bytes from the server.
*
* Entry:
*    X  = starting address where data is to be stored
*    Y  = number of bytes expected
*
* Exit:
*    CC = carry set on framing error, Z set if all bytes received
*    X  = starting address of data received
*    U is preserved.  All accumulators are clobbered


Tcp1Read
* loop until sufficient bytes available
  stx vars.buf,u  ; 6809 ram buffer
  sty vars.len,u
  sty vars.chunklen,u

@loop
  ldx #S1_RX_RSR0
  bsr func_Get2
  subd vars.len,u ; S1_RX_RSR0 - len
  blo @loop
* all bytes we want are available for reading now

* get current pointer in receive buffer
  ldx #S1_RX_RD0
  bsr func_Get2
  std vars.xptr,u  ; rptr msb

* offset into receive buffer
* roffset = rptr & SKBUFMASK
  anda #(SKBUFMASK/256)
  std vars.xoffset,u ; roffset

* The receieve buffer is circular. Check if we need to read from it in two
*    parts or one
*  if(roffset + len > SKBUFSIZE)
  addd vars.len,u  ; roffset + temp
  cmpd #SKBUFSIZE
  ble r_once
twice
; 1) Read to end of buffer
; rsize = SKBUFSIZE - roffset
  ldd #SKBUFSIZE
  subd vars.xoffset,u ; roffset
  std vars.chunklen,u ; rsize
; rgblkget(buf, sockp->skrbstrt + roffset, rsize)
  ldd vars.xoffset,u ; rofset
  addd #S1_RX_BUF
  std vars.wizbuf,u  ; temp
  bsr readChunk    ; ----------------->
; 2) Read remainder from beginning of buffer
; rgblkget(buf + rsize, sockp->skrbstrt, len - rsize)

  ldd #S1_RX_BUF
  std vars.wizbuf,u ; wizbuf

  ldd vars.len,u
  subd vars.chunklen,u ; subtract chunk already written
  std vars.chunklen,u  ; the remaining chunk len

  bra r_doonce

r_once
; rgblkget(buf, sockp->skrbstrt + roffset, len)
  ldd #S1_RX_BUF
  addd vars.xoffset,u ; roffset
  std vars.wizbuf,u ; wizbuf

r_doonce
  bsr readChunk

r_advance
  ldx #S1_RX_RD0   ; Increment the read pointer by vars.len
  lbsr func_incr_wizreg_by_len
  lda #SK_CR_RECV  ; Command wiz that we received.
  lbsr func_command
r_done
  rts


readChunk
* buf - ram buf dest, will be advanced.
* chunklen - count
* wizbuf rgaddr
  ldx vars.buf,u
  ldy vars.chunklen,u
  ldd vars.wizbuf,u ; wiz buffer source
  std >CIO0ADDR
@loop
  lda   >CIO0DATA
  sta  ,x+
  leay -1,y
  bne @loop
  stx vars.buf,u  ; save advanced X.
  rts

func_Get2 ; ( X: wizreg ) -> D:word_got
        stx >CIO0ADDR
        lda >CIO0DATA
        ldb >CIO0DATA
        rts

*******************************************************
*
* Tcp1Write
*
* Entry:
*    X  = starting address of data to send
*    Y  = number of bytes to send
*
* Exit:
*    n/a

Tcp1Write
        stx vars.buf,u  ; 6809 ram buffer
        sty vars.len,u        ; num bytes to send
        sty vars.chunklen,u ; in case of 1 chunk
* Loop until TX Buffer has space
w_loop
        ldx #S1_TX_FSR0
        bsr func_Get2
        ;notused; std vars.xsize,u
        subd vars.len,u   ; S1_TX_FSR0 - len
        blo w_loop
* enough bytes we want are available for writing now

* get current pointer in transmit buffer
        ldx #S1_TX_WR0    ; tx buf write pointer
        bsr func_Get2
        std vars.wizbuf,u   ; where to start writing

* next write offset into transmit buffer
* xoffset = xptr & SKBUFMASK
        anda #(SKBUFMASK/256)   ; B ands with $FF so B stays the same.
        std vars.xoffset,u ; xoffset

* The transmit buffer is circular. Check if we need to write to it in two
*    parts or one
*  if(xoffset + len > SKBUFMASK)
        ; D still has xoffset
        addd vars.len,u          ; xoffset + len
        cmpd #SKBUFSIZE   ; does it exceed the buf size?
        ble w_once        ; no, do it in One shot.

w_twice
* 1) Read to end of buffer
        ;;; pshs y ; len
* xsize = SKBUFSIZE - xoffset
        ldd #SKBUFSIZE
        subd vars.xoffset,u ; xoffset
        std vars.chunklen,u ; xsize
* rgblkget(buf, sockp->skrbstrt + xoffset, xsize)
        ldd #S1_TX_BUF
        addd vars.xoffset,u ; xoffset
        std vars.wizbuf,u
        bsr writeChunk

* 2) Read remainder from beginning of buffer
* rgblkget(buf + xsize, sockp->skrbstrt, len - xsize)
        ldd #S1_TX_BUF
        std vars.wizbuf,u
        ldd vars.len,u   ;   was the total len to write
        subd vars.chunklen,u ; amount already written
        std vars.chunklen,u ; remaining len to write
        bra w_doonce
w_once
* rgblkget(buf, sockp->skrbstrt + xoffset, len)
        ldd #S1_TX_BUF
        addd vars.xoffset,u ; xoffset
        std vars.wizbuf,u
w_doonce
        bsr writeChunk


        ldx #S1_TX_WR0  ; Increment the write pointer by vars.len
        bsr func_incr_wizreg_by_len

        lda #SK_CR_SEND
        bsr func_command

txdone
        rts


writeChunk
* < buf --- ram buffer to write
* < chunklen -- num bytes to copy
* < wizbuf  -- wiz buffer to read
        ldx vars.buf,u
        ldy vars.chunklen,u
        ldd vars.wizbuf,u
        std >CIO0ADDR
@loop
        lda        ,x+
        sta         >CIO0DATA
        leay -1,y
        bne @loop
        stx vars.buf,u  ; save advanced X.
        rts


func_incr_wizreg_by_len ; ( X=wizreg len=augend )
        lbsr func_Get2
        leay vars.len,u ; point to augend
        addd ,y          ; add the augend
        stx >CIO0ADDR
        sta >CIO0DATA
        stb >CIO0DATA
        rts

func_command ; ( A : command byte to wiznet )
        ldx #S1_CR
        stx >CIO0ADDR
        sta >CIO0DATA
@wait_for_zero
        stx >CIO0ADDR
        ldb >CIO0DATA
        bne @wait_for_zero
        rts

;; END lemma_hdb.asm ;;
