	include COCOIOEQU.asm
	include W5100SEQU.asm


S3_RX_BUF	equ	$5800
S3_TX_BUF	equ	$7800
SK_CR_SEND	equ	$20
SKBUFSIZE	equ	$800
SKBUFMASK	equ	$7ff



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
	pshs u
* space for variables on stack
	leas -10,s
	leau ,s
* temp	 	rmb 2  ,u
* wptr		rmb 2 2,u
* woffset	rmb 2 4,u
* wsize 	rmb 2 6,u
* len	 	rmb 2 8,u

* Loop until TX Buffer has space
	sty ,u	* exptectd bytes => temp
	sty 8,u	* exptectd bytes => len
@loop
	ldd #S3_TX_FSR0
	std CIO0ADDR
	lda CIO0DATA
	sta 6,u	* wsize msb
	ldb CIO0DATA
	stb 7,u * wsize lsb
	subd ,u * S3_TX_FSR0 - temp
	blo @loop
* all bytes we want are available for reading now

* get current pointer in receive buffer
	ldd #S3_TX_WR0
	std CIO0ADDR
	lda CIO0DATA
	sta 2,u	* wptr msb
	ldb CIO0DATA
	stb 3,u * wptr lsb

* offset into receive buffer
* woffset = wptr & SKBUFMASK
	andd #SKBUFMASK
	std 4,u * woffset

* The receieve buffer is circular. Check if we need to read from it in two
*    parts or one
*  if(woffset + len > SKBUFMASK)
	sty ,u	* exptectd bytes => temp
	addd ,u	* woffset + temp	
	cmpd #SKBUFMASK
	ble once
twice
* 1) Read to end of buffer
	pshs y * len
* wsize = SKBUFSIZE - woffset
	ldd #SKBUFSIZE
	subd 4,u * woffset
	std 6,u * wsize
* rgblkget(buf, sockp->skrbstrt + woffset, wsize)
	tfr d,y
	ldd 4,u * rofset
	addd #S3_TX_BUF
	std ,u  * temp
	jsr w51wr

* 2) Read remainder from beginning of buffer
* rgblkget(buf + wsize, sockp->skrbstrt, len - wsize)
	ldd #S3_TX_BUF
	std ,u * temp
	puls d * len
	subd 6,u * wsize
	tfr d,y	
	bra doonce
once
* rgblkget(buf, sockp->skrbstrt + woffset, len)
	ldd #S3_TX_BUF
	addd 4,u * woffset
	std ,u * temp
doonce
	jsr w51wr
advance
	ldd #S3_TX_WR0
	std CIO0ADDR
	ldd 8,u * len
	sta CIO0DATA
	stb CIO0DATA
send
	ldd #S3_CR
	std CIO0ADDR
	lda #SK_CR_SEND
	sta CIO0DATA
rxdone
	leas 10,s
	puls u,pc


w51wr
* x - buf
* y - count
* temp rgaddr
	ldd ,u * temp
	std CIO0ADDR
	bra @check
@loop
	lda	,x+
	sta 	CIO0DATA
	leay -1,y
@check
	cmpy #0
	bne @loop
	rts
