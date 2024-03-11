;	include COCOIOEQU.asm
;	include W5100SEQU.asm

  use cocoio.d
  use w5100s.d

SK_CR_SEND	equ	$20
SK_CR_RECV equ $40

S1_RX_BUF  equ  $6800
S1_TX_BUF  equ  $4800

SKBUFSIZE  equ  $800
SKBUFMASK  equ  $7ff

vars  struct     ; stack variable frame.
temp	rmb 2
xptr	rmb 2
xoffset rmb 2
xsize   rmb 2
len	rmb 2
buf     rmb 2     ; 6809 ram buffer
cmd_    rmb 1     ; start of p10 header; also start of quint.
len_    rmb 2
ptr_    rmb 2     ; unused yet.
xcmd_     rmb 1
xdrive_     rmb 1
xlsn2_     rmb 1
xlsn1_     rmb 1
xlsn0_     rmb 1
    endstruct

EXECMD
  pshs dp,d,x,y,u
* space for variables on stack
  leas -sizeof{vars},s
  leau ,s

  ldd #219  ; 219 is HDBDOS sector
  tfr a,dp  ; make sure DP is $00
  stb vars.cmd_,u
  ldb #5    ; num bytes after quint
  std vars.len_,u
  clrb
  std vars.ptr_,u  ; reserved, so zero

  clr    <DCSTAT   ; assume will return no error.
  ldd <VCMD+1
  std vars.xdrive_,u
  ldd <VCMD+3
  std vars.xlsn1_,u

  ldb <VCMD
  stb vars.xcmd_,u
  ldx >DCBPT          ; ptr to sector buffer
  pshs b,x            ; (((((

; Now Use Direct Page FF
  lda #$FF          
  tfr a,dp

  leax vars.cmd_,u   ; ptr to start of header
  ldy #10             ; header size
  lbsr Tcp1Write       ; SEND HEADER

  puls b,x            ; )))))
  ldy #256            ; sector size
  cmpb #WBLK
  beq WriteSector

ReadSector
  bsr Tcp1Read ; DP must be FF when calling Tcp1Read
  bra AssumeNoError

WriteSector
  lbsr Tcp1Write ; DP must be FF when calling Tcp1Read

AssumeNoError
  leas sizeof{vars},s
  puls dp,d,x,y,u

  

* Tcp1Read
*    Receive bytes from the server.
*
* Entry:
*    X  = starting address where data is to be stored
*    Y  = number of bytes expected
*    DP = $FF
*
* Exit:
*    CC = carry set on framing error, Z set if all bytes received
*    X  = starting address of data received
*    U is preserved.  All accumulators are clobbered
*    DP = $FF


Tcp1Read

* temp     rmb 2  ,u
* rptr    rmb 2 2,u
* roffset  rmb 2 4,u
* rsize   rmb 2 6,u
* len     rmb 2 8,u

* loop until sufficient bytes available
  stx vars.buf,u  ; 6809 ram buffer
  sty vars.len,u  ; exptectd bytes => len
LoopW
  ldd #S1_RX_RSR0
  std <CIO0ADDR
  lda <CIO0DATA
  sta vars.xsize,u  ; xsize msb
  ldb <CIO0DATA
  stb 1+vars.xsize,u ; xsize lsb
  subd vars.len,u ; S1_RX_RSR0 - len
  blo LoopW
* all bytes we want are available for reading now

* get current pointer in receive buffer
  ldx #S1_RX_RD0
  bsr func_Get2
  std vars.xptr,u  ; rptr msb

* offset into receive buffer
* roffset = rptr & SKBUFMASK
  anda #(SKBUFMASK/256)
  ; no need to andb #(SKBUFMASK&255) because it is 255.
  std vars.xoffset,u ; roffset

* The receieve buffer is circular. Check if we need to read from it in two
*    parts or one
*  if(roffset + len > SKBUFMASK)
  sty vars.temp,u  ; exptectd bytes => temp
  addd vars.temp,u  ; roffset + temp  
  cmpd #SKBUFMASK
  ble @once
twice
* 1) Read to end of buffer
  pshs y ; len
* rsize = SKBUFSIZE - roffset
  ldd #SKBUFSIZE
  subd vars.xoffset,u ; roffset
  std vars.xsize,u ; rsize
* rgblkget(buf, sockp->skrbstrt + roffset, rsize)
  tfr d,y
  ldd vars.xoffset,u ; rofset
  addd #S1_RX_BUF
  std vars.temp,u  ; temp
  bsr readChunk
* 2) Read remainder from beginning of buffer
* rgblkget(buf + rsize, sockp->skrbstrt, len - rsize)
  ldd #S1_RX_BUF
  std vars.temp,u ; temp
  puls d ; len
  subd vars.xsize,u ; rsize
  tfr d,y  ; TODO: consider ldd vars.temp,u, then pass in D.
  bra @doonce
@once
* rgblkget(buf, sockp->skrbstrt + roffset, len)
  ldd #S1_RX_BUF
  addd vars.xoffset,u ; roffset
  std vars.temp,u ; temp
@doonce
  bsr readChunk

@advance
  ldx #S1_RX_RD0   ; Get read pointer
  ldy vars.len,u   ; length to add
  lbsr func_advance

  ldd #S1_CR        ; Poke RECV into Command Register,
  std <CIO0ADDR      ;     to notify we updated read pointer.
	ldb #SK_CR_RECV
  stb <CIO0DATA
@wait0
  ldd #S1_CR        ; wait for 0 for command done.
  std <CIO0ADDR
  ldb <CIO0DATA
	bne @wait0

rxdone
  rts


readChunk
* x - buf
* y - count
* temp rgaddr
  ldd vars.temp,u ; temp
  std <CIO0ADDR
@loop
  lda   <CIO0DATA
  sta  ,x+
  leay -1,y
@check
  bne @loop
  rts

func_Get2 ; ( X: wizreg ) -> D:word_got
	stx <CIO0ADDR
	lda <CIO0DATA
	ldb <CIO0DATA
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
	sty vars.len,u	; needed bytes => len
* Loop until TX Buffer has space
@loop
	ldx #S1_TX_FSR0
	bsr func_Get2
	;notused; std vars.xsize,u
	subd vars.len,u   ; S1_TX_FSR0 - len
	blo @loop
* enough bytes we want are available for writing now

* get current pointer in transmit buffer
	ldx #S1_TX_WR0    ; tx buf write pointer
	bsr func_Get2
	std vars.xptr,u   ; where to start writing

* next write offset into transmit buffer
* xoffset = xptr & SKBUFMASK
	anda #(SKBUFMASK/256)   ; B ands with $FF so B stays the same.
	std vars.xoffset,u ; xoffset

* The transmit buffer is circular. Check if we need to write to it in two
*    parts or one
*  if(xoffset + len > SKBUFMASK)
	sty vars.temp,u	* exptectd bytes => temp
	addd vars.temp,u	* xoffset + temp	
	cmpd #SKBUFMASK
	ble w_once
w_twice
* 1) Read to end of buffer
	pshs y ; len
* xsize = SKBUFSIZE - xoffset
	ldd #SKBUFSIZE
	subd vars.xoffset,u ; xoffset
	std vars.xsize,u ; xsize
* rgblkget(buf, sockp->skrbstrt + xoffset, xsize)
	tfr d,y
	ldd vars.xoffset,u ; rofset
	addd #S1_TX_BUF
	std vars.temp,u  ; temp
	bsr writeChunk

* 2) Read remainder from beginning of buffer
* rgblkget(buf + xsize, sockp->skrbstrt, len - xsize)
	ldd #S1_TX_BUF
	std vars.temp,u ; temp
	puls d ; len
	subd vars.xsize,u ; xsize
	tfr d,y	
	bra @doonce
w_once
* rgblkget(buf, sockp->skrbstrt + xoffset, len)
	ldd #S1_TX_BUF
	addd vars.xoffset,u ; xoffset
	std vars.temp,u ; temp
@doonce
        ldx vars.buf,u  ; 6809 ram buffer
	bsr writeChunk


	ldx #S1_TX_WR0         ; wiz reg to use
	leay vars.len,u          ; point to augend
	bsr func_advance

	lda #SK_CR_SEND
	bsr func_command

txdone
	rts


writeChunk
* x - buf
* y - count
* temp rgaddr
	ldd vars.temp,u ; temp
	std <CIO0ADDR
@loop
	lda	,x+
	sta 	<CIO0DATA
	leay -1,y
	bne @loop
	rts


func_advance ; ( X=wizreg Y->augend )
	lbsr func_Get2
	addd ,y          ; add the augend
	stx <CIO0ADDR
	sta <CIO0DATA
	stb <CIO0DATA
	rts

func_command ; ( A : command byte to wiznet )
	ldx #S1_CR
	stx <CIO0ADDR
	sta <CIO0DATA
	rts



