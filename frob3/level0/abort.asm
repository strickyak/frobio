_abort    EXPORT
  .area .text

_abort:
	daa
	daa
*	ldx #AbortMessage
*	swi      ; HyperPrintf
*	fcb 111
	daa
	daa
abortLoop:
  bra abortLoop

* AbortMessage:
*   fcs / abort() /
* 	fcb 0

