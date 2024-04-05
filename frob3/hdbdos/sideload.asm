;; sideload.asm
;;
;; lemma_hdb.asm adds the ability for a Lemma Server to send extra sectors
;; to the Coco to be executed, before it sends the actual sector.
;; This gives us a hook to load more code into RAM.
;; The limitation is that the extra sector loads at $0600 and executes there.
;;
;; This scrap of code at the beginning of the sector expects 8 bytes
;; of meta data from $0638 to $063F, and payload data of up to 192 bytes
;; from $0640 to $06FF.  The 56 bytes from $0600 to $0637 are defined in
;; this module, as assembly code to copy the payload to its destination.
;; This code gets duplicated at the front of every such packet.
;;
;; $0638:  word: destination to copy to
;; $063A:  word: destination to JSR to, if nonzero
;; $063C:  byte: number of bytes to copy
;; $063D:  3 bytes unused
;;;;;;;;;;;;;;;;;;;;;;;;;;;

SIDELOAD  equ $0600
SL_COPY_DEST equ $0638
SL_JSR_DEST  equ $063A
SL_COPY_LEN  equ $063C
SL_COPY_SRC_IMM  equ $0640

	ORG SIDELOAD

	pshs cc,d,x,y,u 
	orcc #$50             ; disable interrupts while we patch stuff.

	ldy SL_COPY_DEST
	ldx #SL_COPY_SRC_IMM
	ldb SL_COPY_LEN
@loop
	lda ,x+               ; copy SL_COPY_LEN bytes (at least 1).
	sta ,y+
	decb
	bne @loop

	ldx SL_JSR_DEST
	beq @skip
	jsr ,x                ; call the JSR, if nonzero.
@skip
	puls cc,d,x,y,u,pc

SIDELOAD_END

	end SIDELOAD


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; notes .... 
; coco3.rom.list
; 3474                       (        coco3.asm):02853         * THIS CODE WILL RESET THE DISPLAY PAGE REGISTER IN THE
; 3475                       (        coco3.asm):02854         * SAM CHIP TO 2 ($400) AND RESET THE SAM’S VDG CONTROL
; 3476                       (        coco3.asm):02855         * REGISTER TO 0 (ALPHA-NUMERICS). IN ADDITION, IT WILL
; 3477                       (        coco3.asm):02856         * RESET THE VDG CONTROL PINS TO ALPHA-GRAPHICS MODE.
; 3478                       (        coco3.asm):02857         * SET UP THE SAM AND VDG TO GRAPHICS MODE
; 3479 95AC 3416             (        coco3.asm):02858         L95AC   PSHS    X,B,A   SAVE REGISTERS
; 3480 95AE 8EFFC8           (        coco3.asm):02859                 LDX     #SAMREG+8       POINT X TO THE MIDDLE OF THE SAM CNTL REG
; 3481 95B1 A70A             (        coco3.asm):02860                 STA     10,X    **
; 3482 95B3 A708             (        coco3.asm):02861                 STA     $08,X   ***
; 3483 95B5 A706             (        coco3.asm):02862                 STA     $06,X   ****
; 3484 95B7 A704             (        coco3.asm):02863                 STA     $04,X   ***** RESET SAM DISPLAY PAGE TO $400
; 3485 95B9 A702             (        coco3.asm):02864                 STA     $02,X   ****
; 3486 95BB A701             (        coco3.asm):02865                 STA     $01,X   ***
; 3487 95BD A71E             (        coco3.asm):02866                 STA     $-02,X  **
; 3488 95BF A71C             (        coco3.asm):02867                 STA     $-04,X  ***
; 3489 95C1 A71A             (        coco3.asm):02868                 STA     $-06,X  **** RESET SAM’S VDG TO ALPHA-NUMERIC MODE
; 3490 95C3 A718             (        coco3.asm):02869                 STA     $-08,X  ***
; 3491 95C5 B6FF22           (        coco3.asm):02870                 LDA     PIA1+2  GET DATA FROM PIA1, PORT B
; 3492 95C8 8407             (        coco3.asm):02871                 ANDA    #$07    FORCE ALL BITS TO ZERO, KEEP ONLY CSS DATA
; 3493 95CA B7FF22           (        coco3.asm):02872                 STA     PIA1+2  PUT THE VDG INTO ALPHA-GRAPHICS MODE
; 3494 95CD 3596             (        coco3.asm):02873                 PULS    A,B,X,PC        RETURN
;
; $ - lwasm --raw sideload.asm --list=sideload.raw.list --output=sideload.raw
