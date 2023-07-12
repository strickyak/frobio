#include "frob3/level0/lib0.h"

void Using(void* p) {
  asm volatile("nop ; %0" : : "r" (p));
}

const char NotYet[] = "NotYet";

void SayAbort() {
  PrintF(" ABORT. ");
  PrintH("ABORT");
}
void SayReturnToLemma() {
  PrintF(" ReturnToLemma. ");
  PrintH("ReturnToLemma");
}

int main() {
  PrintH("level0c main");
  Using(&SayReturnToLemma);
  Using(&SayAbort);

  for (word p=VDG_RAM+32; p<VDG_END; p++) {
    *(char*)p = '-';
  }
  Vars->vdg_begin = VDG_RAM + 32;
  Vars->vdg_end = VDG_END - 32;
  Vars->vdg_ptr = VDG_RAM + 128;

  PutChar('@');
  Fatal("END", 0xFFFF);

  return 0;
}
