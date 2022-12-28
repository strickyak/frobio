#include "frob2/froblib.h"
#include "frob2/frobos9.h"

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
    printf(" [STOP]");
    while (1) {
      disable_irq_count = disable_irq_count;
    }
}

IF_os9_THEN_asm void Os9Exit(byte status) {
    printf(" [EXIT]"); STOP();
}
IF_os9_THEN_asm errnum Os9Create(const char* path, int mode, int attrs, int* fd) {
    printf(" [Os9Create]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9Open(const char* path, int mode, int* fd) {
    printf(" [Os9Open]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9Delete(const char* path) {
    printf(" [Os9Delete]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9ReadLn(int path, char* buf, int buflen, int* bytes_read) {
    printf(" [Os9ReadLn]"); STOP(); return 68;
}
IF_os9_THEN_asm errnum Os9WritLn(int path, const char* buf, int max, int* bytes_written) {
    printf("{");
    for (int i = 0; i < max; i++) {
      char ch = buf[i];
      switch (ch) {
        case '\n':
        case '\r':
          printf("\r\n");
          break;
        default:
          if (' ' <= ch && ch <= '~') {
            printf("%c", ch);
          } else {
            printf("(%d)", ch);
          }
      }
    }
    printf("}");
    *bytes_written = max;
    return OKAY;
}
IF_os9_THEN_asm errnum Os9Close(int path) {
    printf(" [Os9Close]"); STOP(); return 68;
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
