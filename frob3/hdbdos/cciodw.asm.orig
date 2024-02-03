
	include COCOIOEQU.asm
	include W5100SEQU.asm

S3_RX_BUF	equ	$5800
S3_TX_BUF	equ	$7800
SKBUFSIZE	equ	$800
SKBUFMASK	equ	$7ff

ConstTbl
* 
C_sizeR		equ 	0	* W5100 avaiable/read/write register
C_ptrR		equ	2	* W5100 read/write pointer register
C_bufP		equ	4	* W5100 data pointer
C_cmpF		equ	6	* compare function
C_opF		equ	8	* operation function
* 
RdConst		equ 	.-ConstTbl
CRsizeR		fdb 	S3_RX_RSR0
CRptrR		fdb	S3_RX_RD0
CRbufP		fdb	S3_RX_BUF
CRcmpF		fdb	RdCmpF
CRopF		fdb	w51rd
* 
WrConst		equ 	.-ConstTbl
CWsizeR		fdb 	S3_TX_FSR0
CWptrR		fdb	S3_TX_WR0
CWbuf		fdb	S3_TX_BUF
CWcmpF		fdb	WrCmpF
CWopF		fdb	w51wr

DWRead
	ldb	RdConst
	bsr	CCIODW
	rts

DWWrite
	ldb	WrConst
	bsr	CCIODW
	rts

RdCmpF
	blo	@set
@clear
	andcc	#$fe	* clear carry
	bra	@rts
@set
	orcc	#$01	* set carry
@rts
	rts
WrCmpF
	bhi	@set
	bra	@clear

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
*    B  = Constants Table Offset
*
* Exit:
*    CC = carry set on framing error, Z set if all bytes received
*    X  = starting address of data received
*    Y  = checksum
*    U is preserved.  All accumulators are clobbered
*
CCIODW
	pshs u
* space for variables on stack
	leas -12,s
	leau ,s
* temp	 	rmb 2  ,u
* rptr		rmb 2 2,u
* roffset	rmb 2 4,u
* rsize 	rmb 2 6,u
* len	 	rmb 2 8,u
* tbl		rmb 2 10,u

* loop until sufficient bytes available
	sty ,u	* exptectd bytes => temp
	sty 8,u	* exptectd bytes => len
	ldy ConstTbl,pcr
	leay b,y
	sty 10,u * tbl
@loop
	ldd C_sizeR,y
	std CIO0ADDR
	lda CIO0DATA
	sta 6,u	* rsize msb
	ldb CIO0DATA
	stb 7,u * rsize lsb
	subd ,u * S3_RX_RSR0 - temp
	jsr	[C_cmpF,y]
	bcs @loop
* all bytes we want are available for reading now

* get current pointer in receive buffer
	ldd C_ptrR,y
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
	* sty ,u	* exptectd bytes => temp
	addd 8,u	* roffset + len	
	cmpd #SKBUFMASK
	ble once
twice
* 1) Read to end of buffer
	* pshs y * len
* rsize = SKBUFSIZE - roffset
	ldd #SKBUFSIZE
	subd 4,u * roffset
	std 6,u * rsize
* rgblkget(buf, sockp->skrbstrt + roffset, rsize)
	pshs d * tfr d,y
	ldd 4,u * rofset
	addd 4,y * C_buf
	std ,u  * temp
	puls y
	jsr w51rd

* 2) Read remainder from beginning of buffer
* rgblkget(buf + rsize, sockp->skrbstrt, len - rsize)
	ldy 10,u *tbl
	ldd 4,y *c_buf
	std ,u * temp
	ldd 8,u * puls d * len
	subd 6,u * rsize
	tfr d,y	
	bra doonce
once
* rgblkget(buf, sockp->skrbstrt + roffset, len)
	ldy 10,u *tbl
	ldd 4,y *c_buf
	addd 4,u * roffset
	std ,u * temp
	ldy 8,u *len
doonce
	jsr w51rd
advance
	ldd #S3_RX_RD0
	std CIO0ADDR
	ldd 8,u * len
	sta CIO0DATA
	stb CIO0DATA
rxdone
	leas 12,s
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

