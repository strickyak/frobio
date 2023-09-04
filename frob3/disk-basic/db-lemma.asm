*** TODO *** work in progress ***

; ROM Addresses
DSKCON equ $D75F
EXTRA equ  $D8D0  ; Start unused part of Disk Basic ROM Space

DOP.RESTORE equ 0
DOP.NOP equ 1
DOP.RD equ 2
DOP.WR equ 3

; Direct Page parameters for DSKCON
DCOPC equ $EA ; opcode byte 0..3 0=restore 1=nop 2=rd 3=wr
DCDRV equ $EB ; drive byte 0..3
DCTRK equ $EC ; track byte 0..34
DSEC  equ $ED ; sector byte 1..18
DCBPT equ $EE ; (word) data pointer
DCSTA equ $F0 ; status byte
FCBTMP equ $F1 ; (word) temporary fcb pointer
SPARE equ $F3  ; 13 unused bytes

    org DSKCON
    pshs U,Y,X,B,A
    lbrs X_DSKCON
    puls A,B,X,Y,U,PC

    org EXTRA

X_DSKCON
    clr <DCSTA   ; clear status
    lda <DCOPC
    cmpa #DOP.RD
    beq X_RD
    cmpa #DOP.WR
    beq X_WR
    rts ; if neither RD nor WR

X_RD
    lda #LEMMA_CMD_FLOPPY_RD
    jmp X_JOIN  ; or use CMPD to jump 2.

X_WR
    lda #LEMMA_CMD_FLOPPY_WR

X_JOIN
    sta <SPARE
    clr <SPARE+1 ; unused byte
    lda <DCDRV
    sta <SPARE+2
    ldd <DCTRK   ; track & sector
    std <SPARE+3

    ldx #SPARE
    ldy #5       ; size of quint command
    bsr TcpWrite

    ldx DCBPT    ; get buffer location
    ldy #256

    lda DCOPC
    cmpa #DOP.WR
    beq XX_WR

XX_RD
    bsr TcpRead
    rts

XX_WR
    bsr TcpWrite
    rts




**************************************************
*
*  The following is based on github.com/n6il/cocoio-dw/dwread.asm

S1_RX_BUF  equ  $6800
S1_TX_BUF  equ  $4800
SKBUFSIZE  equ  $800
SKBUFMASK  equ  $7ff

* TcpRead
*   Entry:
*     X  = starting address where data is to be stored
*     Y  = number of bytes expected
*
*   Exit:
*     CC = carry set on framing error, Z set if all bytes received
*     X  = starting address of data received
*     Y  = checksum
*     U is preserved.
*     A & B are clobbered

TcpRead
  pshs u
* space for variables on stack
  leas -10,s
  leau ,s

* temp     rmb 2  ,u
* rptr    rmb 2 2,u
* roffset  rmb 2 4,u
* rsize   rmb 2 6,u
* len     rmb 2 8,u

* loop until sufficient bytes available
  sty ,u  ; exptectd bytes => temp
  sty 8,u  ; exptectd bytes => len
LoopW

  ldd #S1_RX_RSR0
  std CIO0ADDR
  lda CIO0DATA
  sta 6,u  ; rsize msb
  ldb CIO0DATA
  stb 7,u ; rsize lsb
  subd ,u ; S1_RX_RSR0 - temp
  blo LoopW
* all bytes we want are available for reading now

* get current pointer in receive buffer
  ldd #S1_RX_RD0
  std CIO0ADDR
  lda CIO0DATA
  sta 2,u  ; rptr msb
  ldb CIO0DATA
  stb 3,u ; rptr lsb

* offset into receive buffer
* roffset = rptr & SKBUFMASK
  anda #(SKBUFMASK/256)
  ; no need to andb #(SKBUFMASK&255) because it is 255.
  std 4,u ; roffset

* The receieve buffer is circular. Check if we need to read from it in two
*    parts or one
*  if(roffset + len > SKBUFMASK)
  sty ,u  ; exptectd bytes => temp
  addd ,u  ; roffset + temp  
  cmpd #SKBUFMASK
  ble once
twice
* 1) Read to end of buffer
  pshs y ; len
* rsize = SKBUFSIZE - roffset
  ldd #SKBUFSIZE
  subd 4,u ; roffset
  std 6,u ; rsize
* rgblkget(buf, sockp->skrbstrt + roffset, rsize)
  tfr d,y
  ldd 4,u ; roffset
  addd #S1_RX_BUF
  std ,u  ; temp
  bsr w51rd

* 2) Read remainder from beginning of buffer
* rgblkget(buf + rsize, sockp->skrbstrt, len - rsize)
  ldd #S1_RX_BUF
  std ,u ; temp
  puls d ; len
  subd 6,u ; rsize
  tfr d,y  
  bra doonce
once
* rgblkget(buf, sockp->skrbstrt + roffset, len)
  ldd #S1_RX_BUF
  addd 4,u ; roffset
  std ,u ; temp
doonce
  bsr w51rd

advance
  ldy #S1_RX_RD0   ; Get read pointer
  sty CIO0ADDR
  lda CIO0DATA
  ldb CIO0DATA

  addd 8,u ; len   ; add len to read pointer

  ldy #S1_RX_RD0   ; put back the read pointer
  sty CIO0ADDR
  sta CIO0DATA
  stb CIO0DATA

  ldd #S1_CR        ; Poke RECV into Command Register,
  std CIO0ADDR      ;     to notify we updated read pointer.
	ldb #SK_CR_RECV
  stb CIO0DATA
@wait0
  ldd #S1_CR        ; wait for 0 for command done.
  std CIO0ADDR
  ldb CIO0DATA
	bne @wait0
  
rxdone
  leas 10,s
  puls u,pc


w51rd
* x - buf
* y - count
* temp rgaddr
  ldd ,u ; temp
  std CIO0ADDR
  bra @check
loopRD
  lda   CIO0DATA
  sta  ,x+
  leay -1,y
@check
  cmpy #0
  bne loopRD
  rts



*  End copy from github.com/n6il/cocoio-dw/dwread.asm }}}
*
**************************************************
