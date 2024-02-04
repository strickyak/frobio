
	include COCOIOEQU.asm
	include W5100SEQU.asm

S3_RX_BUF	equ	$5800
S3_TX_BUF	equ	$7800
SKBUFSIZE	equ	$800
SKBUFMASK	equ	$7ff



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
DwRead
	pshs u
* space for variables on stack
	leas -10,s
	leau ,s
* temp	 	rmb 2  ,u
* rptr		rmb 2 2,u
* roffset	rmb 2 4,u
* rsize 	rmb 2 6,u
* len	 	rmb 2 8,u

* loop until sufficient bytes available
	sty ,u	* exptectd bytes => temp
	sty 8,u	* exptectd bytes => len
@loop
	ldd #S3_RX_RSR0
	std CIO0ADDR
	lda CIO0DATA
	sta 6,u	* rsize msb
	ldb CIO0DATA
	stb 7,u * rsize lsb
	subd ,u * S3_RX_RSR0 - temp
	blo @loop
* all bytes we want are available for reading now

* get current pointer in receive buffer
	ldd #S3_RX_RD0
	std CIO0ADDR
	lda CIO0DATA
	sta 2,u	* rptr msb
	ldb CIO0DATA
	stb 3,u * rptr lsb

* offset into receive buffer
* roffset = rptr & SKBUFMASK
	andd #SKBUFMASK
	std 4,u * roffset

* The receieve buffer is circular. Check if we need to read from it in two
*    parts or one
*  if(roffset + len > SKBUFMASK)
	sty ,u	* exptectd bytes => temp
	addd ,u	* roffset + temp	
	cmpd #SKBUFMASK
	ble once
twice
* 1) Read to end of buffer
	pshs y * len
* rsize = SKBUFSIZE - roffset
	ldd #SKBUFSIZE
	subd 4,u * roffset
	std 6,u * rsize
* rgblkget(buf, sockp->skrbstrt + roffset, rsize)
	tfr d,y
	ldd 4,u * rofset
	addd #S3_RX_BUF
	std ,u  * temp
	jsr w51rd

* 2) Read remainder from beginning of buffer
* rgblkget(buf + rsize, sockp->skrbstrt, len - rsize)
	ldd #S3_RX_BUF
	std ,u * temp
	puls d * len
	subd 6,u * rsize
	tfr d,y	
	bra doonce
once
* rgblkget(buf, sockp->skrbstrt + roffset, len)
	ldd #S3_RX_BUF
	addd 4,u * roffset
	std ,u * temp
doonce
	jsr w51rd
advance
	ldd #S3_RX_RD0
	std CIO0ADDR
	ldd 8,u * len
	sta CIO0DATA
	stb CIO0DATA
rxdone
	leas 10,s
	puls u,pc


w51rd
* x - buf
* y - count
* temp rgaddr
	ldd ,u * temp
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


