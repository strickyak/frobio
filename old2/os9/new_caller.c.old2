// newcaller.c : New OS9 system call caller
//
// I want it to work with both CMOC and gcc6809.
// Wrapper macros will be machine-generated.

#if USE_NEW_CALLER

#include "frob2/os9/new_caller.h"

// From new_caller_asm.asm:
asm void MakeNewCall(struct new_caller *nc);
asm void FinishNewCall();

void NewCall(struct new_caller *nc) {
  // Build machine code for SWI2 trap in the code[6] buffer.
  *(word*)(nc->code) = 0x103f;  // SWI2
  nc->code[2] = nc->os9num;
  nc->code[3] = 0x7e;  // JMP extended
  *(word*)(nc->code+4) = (word)(FinishNewCall);

  // Clear the err and call the code.
  nc->err = 0;
  MakeNewCall(nc);
}

#endif
