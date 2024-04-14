#include "frob2/froblib.h"
#include "frob2/frobos9.h"
#include "frob2/decb/std4gcc.h"

/* we need
EnableIrqsCounting
DisableIrqsCounting
Os9Open
Os9Close
Os9ReadLn
Os9WritLn
Os9Mem
Os9Delete
*/

byte disable_irq_count;

void DisableIrqsCounting() {}
void EnableIrqsCounting() {}

void STOP() {
    decb_putstr(" [STOP]");
    while (1) {
      disable_irq_count = disable_irq_count;
    }
}

IF_os9_THEN_asm void Os9Exit(byte status) {
    decb_putstr(" [EXIT]"); STOP();
}
IF_os9_THEN_asm errnum Os9Create(const char* path, int mode, int attrs, int* fd) {
    decb_putstr(" [Os9Create]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9Open(const char* path, int mode, int* fd) {
    decb_putstr(" [Os9Open]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9Delete(const char* path) {
    decb_putstr(" [Os9Delete]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9ReadLn(int path, char* buf, int buflen, int* bytes_read) {
    decb_putstr(" [Os9ReadLn]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9WritLn(int path, const char* buf, int max, int* bytes_written) {
    decb_putstr("{");
    int i;
    for (i = 0; i < max; i++) {
      if (buf[i]==0) break;
      decb_putchar(buf[i]);
      if (buf[i]==10 || buf[i]==13) { i++; break; }
    }
    decb_putstr("}");
    *bytes_written = i;
    return OKAY;
}
IF_os9_THEN_asm errnum Os9Close(int path) {
    decb_putstr(" [Os9Close]"); STOP(); return 68;
}

#define MAX_DECB_MEMORY_SIZE 0x8000
word decb_memory_size = 0x6000;
errnum Os9Mem(word* new_memory_size_inout, word* end_of_new_mem_out) {
    if (*new_memory_size_inout > MAX_DECB_MEMORY_SIZE) {
        *new_memory_size_inout = decb_memory_size;
        *end_of_new_mem_out = decb_memory_size;
        return 32 /* Memory Full */;
    }
    if (*new_memory_size_inout <= decb_memory_size) {
        *new_memory_size_inout = decb_memory_size;
        *end_of_new_mem_out = decb_memory_size;
        return OKAY;
    }

    *end_of_new_mem_out = MAX_DECB_MEMORY_SIZE;
    return OKAY;
}

void decb_putstr(const char* s) {
    while (*s) {
        decb_putchar(*s);
        ++s;
    }
}

#ifdef FOR_DECB
extern char* readline();
char* decb_readline() {
  return readline();
}
extern void putchar(char c);
void decb_putchar(int c) {
  putchar((char)c);
}
#endif
