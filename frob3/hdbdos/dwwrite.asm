    .area   .text
    .globl  LemmaWrite
_Emit IMPORT
_Hex IMPORT
_Hex2 IMPORT
_Stop IMPORT

	include COCOIOEQU.asm
	include W5100SEQU.asm

S1_TX_BUF	equ	$4800
S1_RX_BUF	equ	$6800
SK_CR_SEND	equ	$20
SKBUFSIZE	equ	$800
SKBUFMASK	equ	$7ff

CMD_DW          equ     218


*******************************************************
*  Major things that were wrong in these files I copied:
*  * TX_BUF and RX_BUF were swapped.
*  * ANDD is not a 6809 instruction (rather a 6309 one).
*  * Don't up date Rd/Wr pointer by writing size to it.
*  * Not waiting for zero, after writing to CR.
*******************************************************
*
* DWWrite
*    Send a packet to the DriveWire server.
*    Serial data format:  1-8-N-1
*    4/12/2009 by Darren Atkinson
*
* Entry:
*    X  = starting address of data to send
*    Y  = number of bytes to send
*
* Exit:
*    X  = address of last byte sent + 1
*    Y  = 0
*    All others preserved
*

DWWrite
	pshs x,y,u
	leas -5,s
	leax ,s        ; quint buffer
	ldy #5
	bsr LemmaWrite
	leas 5,s
	puls x,y,u
	bra LemmaWrite

; LOCAL VARS offset from U
v_temp	 	equ 0
v_ptr		equ 2
v_offset	equ 4
v_size 	equ 6
v_len	 	equ 8
V_SIZE	 	equ 10


LemmaWrite
	pshs u
* space for variables on stack
	leas -V_SIZE,s
	leau ,s
* loop until sufficient bytes available
	sty v_temp,u	* exptectd bytes => temp
	sty v_len,u	* exptectd bytes => len
@loop
	ldd #S1_TX_FSR0
	std CIO0ADDR
	lda CIO0DATA
	sta v_size,u	* size msb
	ldb CIO0DATA
	stb v_size+1,u * size lsb
  ;
  ;v; pshs d,x,y
  ;v; tfr d,x
  ;v; ldb #'f'
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;
	subd v_temp,u * S1_TX_FSR0 - temp
	blo @loop
* all bytes we want are available for reading now

* get current pointer in receive buffer
	ldd #S1_TX_WR0
	std CIO0ADDR
	lda CIO0DATA
	sta v_ptr,u	* wptr msb
	ldb CIO0DATA
	stb v_ptr+1,u * wptr lsb
  ;
  ;v; pshs d,x,y
  ;v; tfr d,x
  ;v; ldb #'w'
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;

* offset into receive buffer
* offset = ptr & SKBUFMASK
	;H; andd #SKBUFMASK
        anda #(SKBUFMASK/256)
        andb #(SKBUFMASK&255)
	std v_offset,u * offset
  ;
  ;v; pshs d,x,y
  ;v; tfr d,x
  ;v; ldb #'o'
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;

* The receieve buffer is circular. Check if we need to read from it in two
*    parts or one
*  if(offset + len > SKBUFMASK)
	sty v_temp,u	* exptectd bytes => temp
  ;
  ;v; pshs d,x,y
  ;v; tfr y,x
  ;v; ldb #'y'
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;
	addd v_temp,u	* offset + temp	
  ;
  ;v; pshs d,x,y
  ;v; tfr d,x
  ;v; ldb #'+'
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;
	cmpd #SKBUFMASK
	ble once
twice
  ;
  ;v; pshs d,x,y
  ;v; ldb #'2'
  ;v; ldx #$2222
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;
* 1) Read to end of buffer
	pshs y * len
* size = SKBUFSIZE - offset
	ldd #SKBUFSIZE
	subd v_offset,u * offset
	std v_size,u * size
* rgblkget(buf, sockp->skrbstrt + offset, size)
	tfr d,y
	ldd v_offset,u * rofset
	addd #S1_TX_BUF
	std v_temp,u  * temp
	jsr lem51wr

* 2) Read remainder from beginning of buffer
* rgblkget(buf + size, sockp->skrbstrt, len - size)
	ldd #S1_TX_BUF
	std v_temp,u * temp
	puls d * len
	subd v_size,u * size
	tfr d,y	
	bra doonce
once
  ;
  ;v; pshs d,x,y
  ;v; ldb #'1'
  ;v; ldx #$1111
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;
* rgblkget(buf, sockp->skrbstrt + woffset, len)
	ldd #S1_TX_BUF
	addd v_offset,u * woffset
	std v_temp,u * temp
  ;
  ;v; pshs d,x,y
  ;v; tfr d,x
  ;v; ldb #'t'
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;
doonce
	jsr lem51wr
advance
	ldd #S1_TX_WR0
	std CIO0ADDR
	ldd v_len,u * len
	addd v_ptr,u
	sta CIO0DATA
	stb CIO0DATA
  ;
  ;v; pshs d,x,y
  ;v; tfr d,x
  ;v; ldb #'v'
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;
send
	ldd #S1_CR
	std CIO0ADDR
	lda #SK_CR_SEND
	sta CIO0DATA
@wait
	ldd #S1_CR
	std CIO0ADDR
	lda CIO0DATA  ; read back that control register
	bne @wait     ; wait for it to be zero
	
rxdone
	leas V_SIZE,s
	clrb          ; Z=1 C=0 (no error, did all)
	puls u,pc


lem51wr
* x - buf
* y - count
* temp rgaddr
	ldd v_temp,u * temp
	std CIO0ADDR
  ;
  ;v; pshs d,x,y
  ;v; tfr d,x
  ;v; ldb #'z'
  ;v; lbsr _Hex2
  ;v; puls d,x,y
  ;
	bra @check
@loop
	lda	,x+
	sta 	CIO0DATA
	leay -1,y
@check
	cmpy #0
	bne @loop
	rts
