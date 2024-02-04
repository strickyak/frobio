    .area   .text
    .globl  LemmaRead
_Emit IMPORT
_Hex IMPORT
_Hex2 IMPORT
_Stop IMPORT

	include COCOIOEQU.asm
	include W5100SEQU.asm

S1_TX_BUF	equ	$4800
S1_RX_BUF	equ	$6800
SK_CR_RECV	equ	$40
SKBUFSIZE	equ	$800
SKBUFMASK	equ	$7ff

CMD_DW          equ     218


*******************************************************
*  Major things that were wrong in these files I copied:
*  * TX_BUF and RX_BUF were swapped.
*  * ANDD is not a 6809 instruction (rather a 6309 one).
*  * Don't up date Rd/Wr pointer by writing size to it.
*  * Not sending SK_CR_RECV.
*  * Not waiting for zero, after writing to CR.
*******************************************************
*
* DWRead
*    Receive a response from the DriveWire server.
*    Times out if serial port goes idle for more than 1.4 (0.7) seconds.
*    Serial data format:  1-8-N-1
*    4/12/2009 by Darren Atkinson
*
* Entry:
*    X  = starting address where data is to be stored
*    Y  = number of bytes expected
*
* Exit:
*    CC = carry set on framing error, Z set if all bytes received
*    X  = starting address of data received
*    Y  = checksum
*    U is preserved.  All accumulators are clobbered
*

DWRead
	pshs x,y,u
	leas -5,s
	leax ,s        ; quint buffer
	ldy #5
	bsr LemmaRead
	ldy #$ffff        ; not used
	ldb ,s    ; C from Waiter
	cmpb #CMD_DW
	beq @ok_b
	lbsr _Stop
@ok_b
	ldy 1,s   ; N from Waiter (not used)
	ldd 9,s   ; old Y
	cmpd 1,s  ; N from Waiter
	beq @ok_n
	lbsr _Stop
@ok_n
	leas 5,s
	puls x,y,u
	bra LemmaRead

; LOCAL VARS offset from U
v_temp	 	equ 0
v_ptr		equ 2
v_offset	equ 4
v_size 	equ 6
v_len	 	equ 8
V_SIZE	 	equ 10

LemmaRead
	pshs u
* space for variables on stack
	leas -V_SIZE,s
	leau ,s
* loop until sufficient bytes available
	sty v_temp,u	* exptectd bytes => temp
	sty v_len,u	* exptectd bytes => len
@loop
	ldd #S1_RX_RSR0
	std CIO0ADDR
	lda CIO0DATA
	sta v_size,u	* size msb
	ldb CIO0DATA
	stb 7,u * size lsb
	subd v_temp,u * S1_RX_RSR0 - temp
	blo @loop
* all bytes we want are available for reading now

* get current pointer in receive buffer
	ldd #S1_RX_RD0
	std CIO0ADDR
	lda CIO0DATA
	sta v_ptr,u	* rptr msb
	ldb CIO0DATA
	stb v_ptr+1,u * rptr lsb

* offset into receive buffer
* offset = ptr & SKBUFMASK
	;H; andd #SKBUFMASK
        anda #(SKBUFMASK/256)
        andb #(SKBUFMASK&255)
	std v_offset,u * offset

* The receieve buffer is circular. Check if we need to read from it in two
*    parts or one
*  if(offset + len > SKBUFMASK)
	sty v_temp,u	* exptectd bytes => temp
	addd v_temp,u	* offset + temp	
	cmpd #SKBUFMASK
	ble once
twice
* 1) Read to end of buffer
	pshs y * len
* size = SKBUFSIZE - offset
	ldd #SKBUFSIZE
	subd v_offset,u * offset
	std v_size,u * size
* rgblkget(buf, sockp->skrbstrt + offset, size)
	tfr d,y
	ldd v_offset,u * rofset
	addd #S1_RX_BUF
	std v_temp,u  * temp
	jsr lem51rd

* 2) Read remainder from beginning of buffer
* rgblkget(buf + size, sockp->skrbstrt, len - size)
	ldd #S1_RX_BUF
	std v_temp,u * temp
	puls d * len
	subd v_size,u * size
	tfr d,y	
	bra doonce
once
* rgblkget(buf, sockp->skrbstrt + roffset, len)
	ldd #S1_RX_BUF
	addd v_offset,u * roffset
	std v_temp,u * temp
doonce
	jsr lem51rd
advance
	ldd #S1_RX_RD0
	std CIO0ADDR
	ldd v_len,u * len
	addd v_ptr,u
	sta CIO0DATA
	stb CIO0DATA
recv
	ldd #S1_CR
	std CIO0ADDR
	lda #SK_CR_RECV
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


lem51rd
* x - buf
* y - count
* temp rgaddr
	ldd v_temp,u * temp
	std CIO0ADDR
	bra @check
@loop
	lda 	CIO0DATA
	sta	,x+
	leay -1,y
@check
	cmpy #0
	bne @loop
	rts


