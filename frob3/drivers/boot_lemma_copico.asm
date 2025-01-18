********************************************************************
* Boot - NitrOS-9 Lemma_Copico Boot Module
*
* Edt/Rev  YYYY/MM/DD  Modified by
* Comment
* ------------------------------------------------------------------
*   1      2023/03/15  Henry Strickland W6REK (github.com/strickyak)
*
* Created, based on boot_rom by Boisy G. Pitre for boilerplate,
*          and Alan DeKok's disassembly of boot_ide with mods by others,
*          and Michael Furman N6IL's cocoio-dw repository (github.com/n6il).

******
*
* Lemma Definitions
Q.BfBegin equ 211  ; quint { cmd total_size.hi total_size.lo 0 0 }
Q.BfChunk equ 212  ; quint { cmd chunk_size.hi chunk_size.lo 0 0 }
Q.BfEnd   equ 213  ; quint { cmd total_size.hi total_size.lo 0 0 }

;; Page 0 between $11 and $20 is unused in OS9 Level 1.
;; TODO use stack instead.  Drivewire scribbles stats here.
BeginPtr  equ $12
CurrPtr   equ $14
Quint     equ $16
QuintN    equ $17
QuintSize equ 5     ; quint is 5 bytes

Value equ $0004   ; for ShowHex
Ptr equ $0006     ; for ShowHex

SK_CR_RECV equ $40
*
******

         nam   Boot
         ttl   NitrOS-9 Level 1 Lemma Boot Module

         use   defsfile

tylg     set   Systm+Objct
atrv     set   ReEnt+rev
rev      set   $01
edition  set   1

         mod   eom,name,tylg,atrv,start,size

size     equ   .

name     fcs   /Boot/
         fcb   edition


;;  Load Quint (for Q.BfBegin)
;;  Assert BfSize, and get sz.
;;  
;;  F$SRqMem D=sz => U=begin
;;  current := begin
;;  
;;  LOOP:
;;    Load Quint.
;;    If BfEnd{ s } , return: X=begin D=sz
;;  
;;    Assert BfChunk{ chunk_sz }
;;  
;;    Read(current, chunk_sz }
;;    current += chunk_sz
;;  
;;  GOTO LOOP;

start    pshs  u,y,cc,dp
    daa            ; daa: searchable in emulator log
    orcc #$50      ; paranoid: no interrupts please.
    clra
    tfr a,dp       ; we use DP=0.


    bsr ReadQuint
	daa
    ldd <QuintN    ; D<-sz

    os9 F$SRqMem
    stu <BeginPtr 
    stu <CurrPtr 


LoopLemma
    bsr ReadQuint  ; read Q.BfChunk or Q.BfEnd quint.
	daa
    lda <Quint
    cmpa #Q.BfEnd
    beq Return
  
    ldx <CurrPtr
    ldy <QuintN
	daa
    ;; inc   $8006
    bsr Tcp1Read     ; read data packet
	daa
    ;; inc   $8007

    ldd <CurrPtr
    addd <QuintN
    std <CurrPtr

    bra LoopLemma

* We return to the kernel with
*    X = address of bootfile
*    D = size of bootfile
Return
    daa            ; daa: searchable in emulator log
    clra           ; resets carry.
    ldx <BeginPtr
    ldd <QuintN
    puls  u,y,dp,cc,pc

ReadQuint
    ;; inc   $8005
    LDX #Quint
    LDY #$0005
	daa
    bsr Tcp1Read

    clra
	ldb Quint
    rts


**************************************************
*
*  {{{ The following is copied from github.com/n6il/cocoio-dw/dwread.asm
*  with the subroutine `DwRead` renamed to `Tcp1Read`
*  and the socket number changed from Socket 3 to Socket 1.
*
*  I suspect the code is by Michael Furman N6IL
*  rather than the name in the comment, which seems to refer to
*  serial drivewire instead of CocoIO ethernet.

S1_RX_BUF  equ  $6800
S1_TX_BUF  equ  $4800
SKBUFSIZE  equ  $800
SKBUFMASK  equ  $7ff

* Tcp1Read
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

Tcp1Read
    pshs x,y,u
    pshs y
    ; now with buffer in X and count on the Stack
    lbsr _N4Recv

    leas 2,s
    puls x,y,u,pc

    use copicoio_raw.s    ; has _N4Recv

             IFGT      Level-1
* L2 kernel file is composed of rel, boot, krn. The size of each of these
* is controlled with filler, so that (after relocation):
* rel  starts at $ED00 and is $130 bytes in size
* boot starts at $EE30 and is $1D0 bytes in size
* krn  starts at $F000 and ends at $FEFF (there is no 'emod' at the end
*      of krn and so there are no module-end boilerplate bytes)
*
* Filler to get to a total size of $1D0. -3 represents the emod CRC size.
* the filler: the end boilerplate for the module, fdb and fcb respectively.
Filler         FILL      $39,$1D0-3-*
             ENDC

         emod
eom      equ   *
