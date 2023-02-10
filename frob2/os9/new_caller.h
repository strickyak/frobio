#ifndef _FROB2_OS9_NEW_CALLER_H_
#define _FROB2_OS9_NEW_CALLER_H_

#include "frob2/frobtype.h"

struct new_caller {
  byte code[6];
  union {      // offset 6
    word d;
    struct {
      byte a;
      byte b;
    } ab;
  } dab;
  word x;      // offset 8
  word y;      // offset 10
  word u;      // offset 12
  byte os9num; // offset 14
  errnum err;  // offset 15
};

void NewCall(struct new_caller *nc);

#endif // _FROB2_OS9_NEW_CALLER_H_
