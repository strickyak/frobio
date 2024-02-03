    .area   .text
    .globl  DWRead
    .globl  DWWrite

        include COCOIOEQU.asm
        include W5100SEQU.asm

S1_RX_BUF        equ        $4800
S1_TX_BUF        equ        $6800
SKBUFSIZE        equ        $800
SKBUFMASK        equ        $7ff


C_sizeR   equ        0        ; W5100 avaiable/read/write register
C_ptrR    equ        2        ; W5100 read/write pointer register
C_bufP    equ        4        ; W5100 data pointer
C_cmpF    equ        6        ; compare function
C_opF     equ        8        ; operation function
 
RdConst:
CRsizeR   fdb        S1_RX_RSR0
CRptrR    fdb        S1_RX_RD0
CRbufP    fdb        S1_RX_BUF
CRcmpF    fdb        RdCmpF
CRopF     fdb        w51rd
 
WrConst:
CWsizeR   fdb        S1_TX_FSR0
CWptrR    fdb        S1_TX_WR0
CWbuf     fdb        S1_TX_BUF
CWcmpF    fdb        WrCmpF
CWopF     fdb        w51wr

RdCmpF
        blo        @set
@clear
        andcc        #$fe        ; clear carry
        bra        @rts
@set
        orcc        #$01        ; set carry
@rts
        rts
WrCmpF
        bhi        @set
        bra        @clear

;******************************************************
;
; DWRead
;    Receive a response from the DriveWire server.
;    Times out if serial port goes idle for more than 1.4 (0.7) seconds.
;    Serial data format:  1-8-N-1
;    4/12/2009 by Darren Atkinson
;
; Entry:
;    X  = starting address where data is to be stored
;    Y  = number of bytes expected
;    B  = Constants Table Offset
;
; Exit:
;    CC = carry set on framing error, Z set if all bytes received
;    X  = starting address of data received
;    Y  = checksum
;    U is preserved.  All accumulators are clobbered

v_temp      equ 0
v_rptr      equ 2
v_roffset   equ 4
v_rsize     equ 6
v_len       equ 8
v_tbl       equ 10

DWRead:
        ldd        RdConst,pcr
        bra        CCIODW

DWWrite:
        ldd        WrConst,pcr
	; fall through

CCIODW:
        pshs u
; space for variables on stack
        leas -12,s
        leau ,s

; loop until sufficient bytes available
        sty v_temp,u        ; exptectd bytes => temp
        sty v_len,u        ; exptectd bytes => len
        std v_tbl,u ; tbl
@loop
        ldd C_sizeR,y
        std CIO0ADDR
        lda CIO0DATA
        sta v_rsize,u        ; rsize msb
        ldb CIO0DATA
        stb v_rsize+1,u      ; rsize lsb
        subd v_temp,u ; S1_RX_RSR0 - temp
        jsr        [C_cmpF,y]
        bcs @loop
; all bytes we want are available for reading now

; get current pointer in receive buffer
        ldd C_ptrR,y
        std CIO0ADDR
        lda CIO0DATA
        sta v_rptr,u        ; rptr msb
        ldb CIO0DATA
        stb v_rptr+1,u      ; rptr lsb

; offset into receive buffer
; roffset = rptr & SKBUFMASK
        ;;;; this is a 6309 instruction: andd #SKBUFMASK
        anda #(SKBUFMASK/256)
        andb #(SKBUFMASK&255)
        std v_roffset,u ; roffset

; The receieve buffer is circular. Check if we need to read from it in two
;    parts or one
;  if(roffset + len > SKBUFMASK)
        ; sty v_temp,u        * exptectd bytes => temp
        addd v_len,u        ; roffset + len        
        cmpd #SKBUFMASK
        ble @once
;@twice
; 1) Read to end of buffer
        ; pshs y ; len
; rsize = SKBUFSIZE - roffset
        ldd #SKBUFSIZE
        subd v_roffset,u ; roffset
        std v_rsize,u ; rsize
; rgblkget(buf, sockp->skrbstrt + roffset, rsize)
        pshs d ; tfr d,y
        ldd v_roffset,u ; rofset
        addd 4,y ; C_buf
        std v_temp,u  ; temp
        puls y
        jsr w51rd
; 2) Read remainder from beginning of buffer
; rgblkget(buf + rsize, sockp->skrbstrt, len - rsize)
        ldy v_tbl,u *tbl
        ldd 4,y *c_buf
        std v_temp,u ; temp
        ldd v_len,u ; puls d ; len
        subd v_rsize,u ; rsize
        tfr d,y        
        bra @doonce
@once
; rgblkget(buf, sockp->skrbstrt + roffset, len)
        ldy v_tbl,u *tbl
        ldd 4,y *c_buf
        addd v_roffset,u ; roffset
        std v_temp,u ; temp
        ldy v_len,u *len
@doonce
        jsr w51rd
;@advance
        ldd #S1_RX_RD0
        std CIO0ADDR
        ldd v_len,u ; len
        sta CIO0DATA
        stb CIO0DATA
;@rxdone
        leas 12,s
        puls u,pc


w51rd:
; x - buf
; y - count
; temp rgaddr
        ldd v_temp,u ; temp
        std CIO0ADDR
        bra @check
@loop
        lda         CIO0DATA
        sta        ,x+
        leay -1,y
@check
        cmpy #0
        bne @loop
        rts

w51wr:
; x - buf
; y - count
; temp rgaddr
        ldd v_temp,u ; temp
        std CIO0ADDR
        bra @check
@loop
        lda        ,x+
        sta         CIO0DATA
        leay -1,y
@check
        cmpy #0
        bne @loop
        rts

