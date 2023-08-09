// OS9 System Calls, lightly wrapped for calling from C.
// This module contains no global data, including no
// const char* strings.
// For CMOC only -- gcc will not work.

#include "frob3/froblib.h"
#include "frob3/frobos9.h"
#include "frob3/os9/os9defs.h"

//chop

byte disable_irq_count;
void DisableIrqsCounting() {
  // If you use -v9 for Verbosity, you're really debugging
  // and you really want to see all debug logs.
  // Since we skip debug logs when Irqs are disabled,
  // at -v9 we never disable Irqs.  So you're on your own
  // not to cause conflicts at -v9.
  if (Verbosity >= 9) return;

  // Disable interrupts and increment the counter.
  // if counter overflows ($7f to $80), then exit(13).
  asm {
    orcc #$50   ; disable interrupts
    inc disable_irq_count
    bvc DisableIrqsEND

IrqCounterOverflow ; if overflow, re-enable and exit(13).
    andcc  #^$50   ; enable interrupts
    ldd #13
    lbsr Os9Exit

DisableIrqsEND
  }
} // nochop

void EnableIrqsCounting() {
  if (Verbosity >= 9) return;

  // Decrement counter, and enable if it reaches zero.
  // if counter overflows ($80 to $7f), then exit(13).
  byte enabled = 0;
  asm {
    dec disable_irq_count
    bne EnableIrqsEND
    bvs IrqCounterOverflow

EnableIrqsENABLE
    andcc  #^$50   ; enable interrupts
    inc enabled
EnableIrqsEND
  }
  if (enabled) {
    // fmt is NULL to flush any logging while disabled.
    Logger(NULL, 0, 0, NULL);
  }
}

#if 0
//  
//  asm void Os9Exit(byte status) {
//      asm {
//  ExitLOOP
//          ldd 2,s      ; status code in b.
//          swi2
//          fcb F_EXIT
//          bra ExitLOOP ; NOTREACHED
//      }
//  }
//  
//  asm errnum Os9Create(int mode, int attrs, const char* path, int* fd) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s      ; mode (6+1)
//          ldb 9,s      ; attrs (8+1)
//          ldx 10,s      ; buf (10)
//          swi2
//          fcb 0x83
//          lbcs Os9Err
//  
//          tfr a,b
//          sex
//          std [12,s]   ; set fd
//  
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9Open(const char* path, int mode, int* fd) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s      ; mode (6+1)
//          ldx 8,s      ; buf  (8)
//          swi2
//          fcb 0x84
//          lbcs Os9Err
//  
//          tfr a,b
//          sex
//          std [10,s]   ; set fd
//  
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9Delete(const char* path) {
//      asm {
//  ZeroOrErr IMPORT
//          pshs y,u
//          ldx 6,s      ; path
//          swi2
//          fcb 0x87
//          jmp ZeroOrErr,pcr
//      }
//  }
//  
//  asm errnum Os9ChgDir(const char* path, int mode) {
//      asm {
//  ZeroOrErr IMPORT
//          pshs y,u
//          ldx 6,s      ; path
//          lda 9,s      ; mode
//          swi2
//          fcb 0x86     ; I$ChgDir
//          jmp ZeroOrErr,pcr
//      }
//  }
//  
//  asm errnum Os9MakDir(const char* path, int mode) {
//      asm {
//  ZeroOrErr IMPORT
//          pshs y,u
//          ldx 6,s      ; path
//          ldb 9,s      ; dir attrs
//          swi2
//          fcb 0x85     ; I$MakDir
//          jmp ZeroOrErr,pcr
//      }
//  }
//  
//  asm errnum Os9GetStt(int path, int func, int* dOut, int* x_inout, int* u_inout) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s      ; path
//          ldb 9,s      ; func
//          ldx [12,s]   ; x_inout
//          ldu [14,s]   ; u_inout
//          swi2
//          fcb I_GETSTT
//          lbcs Os9Err
//          std [10,s]   ; dOut
//          stx [12,s]   ; x_inout
//          stu [14,s]   ; u_inout
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9SetStt(int path, int func, int* dOut, int* x_inout, int* u_inout) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s      ; path
//          ldb 9,s      ; func
//          ldx [12,s]   ; x_inout
//          ldu [14,s]   ; u_inout
//          swi2
//          fcb I_SETSTT
//          lbcs Os9Err
//          std [10,s]   ; dOut
//          stx [12,s]   ; x_inout
//          stu [14,s]   ; u_inout
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9Read(int path, char* buf, int buflen, int* bytes_read) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s      ; path
//          ldx 8,s      ; buf
//          ldy 10,s      ; buflen
//          swi2
//          fcb 0x89
//          lbcs Os9Err
//          sty [12,s]   ; bytes_read
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9ReadLn(int path, char* buf, int buflen, int* bytes_read) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s      ; path
//          ldx 8,s      ; buf
//          ldy 10,s      ; buflen
//          swi2
//          fcb I_READLN
//          lbcs Os9Err
//          sty [12,s]   ; bytes_read
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9Write(int path, const char* buf, int max, int* bytes_written) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s      ; path
//          ldx 8,s      ; buf
//          ldy 10,s      ; max
//          os9 I_WRITE
//          lbcs Os9Err
//          sty [12,s]   ; bytes_written
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9WritLn(int path, const char* buf, int max, int* bytes_written) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s      ; path
//          ldx 8,s      ; buf
//          ldy 10,s      ; max
//          swi2
//          fcb I_WRITLN
//          lbcs Os9Err
//          sty [12,s]   ; bytes_written
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9Dup(int path, int* new_path) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          lda 7,s  ; old path.
//          swi2
//          fcb 0x82 ; I$Dup
//          lbcs Os9Err
//          tfr a,b  ; new path.
//          sex
//          std [8,s]
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9Close(int path) {
//      asm {
//  ZeroOrErr IMPORT
//          pshs y,u
//          lda 7,s  ; path.
//          swi2
//          fcb 0x8F ; I$Close
//          jmp ZeroOrErr,pcr
//      }
//  }
//  
//  asm errnum Os9Sleep(int secs) {
//      asm {
//  ZeroOrErr EXPORT
//  Os9Err    EXPORT
//          pshs y,u
//          ldx 6,s  ; ticks
//          swi2
//          fcb 0x0A ; I$Sleep
//  ZeroOrErr    lbcs Os9Err
//          ldd #0
//          puls y,u,pc
//  Os9Err
//          clra
//          puls y,u,pc
//      }
//  }
//  
//  /*
//   * OS9 F$Wait
//  MACHINE CODE: 103F 04
//  INPUT: None
//  OUTPUT: (A) = Deceased child process's process ID
//  (B) = Child process's exit status code
//  */
//  
//  asm errnum Os9Wait(int* child_id_and_exit_status) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          swi2
//          fcb 0x04 ; F$Wait
//          lbcs Os9Err
//          std [6,s]      ; return Child Id in hi byte; Exit Status in low byte.
//          ldd #0
//          puls y,u,pc
//      }
//  }
//  
//  /*
//     OS9 F$Fork
//  MACHINE CODE: 103F 03
//  INPUT: (X) = Address of module name or file name.
//  (Y) = Parameter area size.
//  (U) = Beginning address of the parameter area.
//  (A) = Language / Type code.
//  (B) = Optional data area size (pages).
//  OUTPUT: (X) = Updated past the name string.
//  (A) = New process ID number.
//  ERROR OUTPUT: (CC) = C bit set. (B) = Appropriate error code.
//  */
//  
//  asm errnum Os9Fork(const char* program, const char* params, int paramlen, int lang_type, int mem_size, int* child_id) {
//      asm {
//  Os9Err IMPORT
//          pshs y,u
//          ldx 6,s  ; program
//          ldu 8,s  ; params
//          ldy 10,s ; paramlen
//          lda 13,s  ; lang_type
//          ldb 15,s  ; mem_size
//          swi2
//          fcb 0x03  ; F$Fork
//          lbcs Os9Err
//          tfr a,b    ; move child id to D
//          clra
//          std [16,s]  ; Store D to *child_id
//          clrb        ; return D=0 no error
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9Chain(const char* program, const char* params, int paramlen, int lang_type, int mem_size) {
//      asm {
//          pshs y,u
//          ldx 6,s  ; program
//          ldu 8,s  ; params
//          ldy 10,s ; paramlen
//          lda 13,s  ; lang_type
//          ldb 15,s  ; mem_size
//          swi2
//          fcb 0x05  ; F$Chain -- if returns, then it is an error.
//          clra      ; extend unsigned error B to D
//          puls y,u,pc
//      }
//  }
//  
//  asm errnum Os9Send(int process_id, int signal_code) {
//      asm {
//  ZeroOrErr IMPORT
//          pshs y,u
//          lda 7,s      ; process_id
//          ldb 9,s      ; signal_code
//          swi2
//          fcb 0x08     ; F$Send
//          jmp ZeroOrErr,pcr
//      }
//  }
//  
//  errnum Os9STime(byte* time_packet) {
//    errnum err = OKAY;
//    asm {
//          ldx time_packet
//  
//          os9 F_STIME
//  
//          bcc STimeOK
//          stb err
//  STimeOK
//    }
//    return err;
//  }
//  
//  errnum Os9Mem(word* new_memory_size_inout, word* end_of_new_mem_out) {
//    errnum err = OKAY;
//    asm {
//      ldd [new_memory_size_inout]
//      pshs y
//      os9 F_MEM
//      bcc Os9MemOK
//      stb err
//      bra Os9MemEND
//  Os9MemOK
//      std [new_memory_size_inout]
//      sty [end_of_new_mem_out]
//  Os9MemEND
//      puls y
//    }
//    return err;
//  }
//  
//  errnum Os9AllRam(byte num_blocks, word* start_block_out) {
//    errnum err = OKAY;
//      asm {
//          ldb num_blocks
//  
//          swi2
//          fcb F_ALLRAM
//  
//          bcc AllRamOK
//          stb err
//          clra
//          clrb
//  AllRamOK
//          std [start_block_out]
//      }
//      return err;
//  }
//  
//  errnum Os9DelRam(byte num_blocks, word start_block) {
//    errnum err = OKAY;
//      asm {
//          ldb num_blocks
//          ldx start_block
//  
//          swi2
//          fcb F_DELRAM
//  
//          bcc DelRamOK
//          stb err
//  DelRamOK
//      }
//      return err;
//  }
//  
//  errnum Os9MapBlk(word starting_block, byte num_blocks, word* first_address_out) {
//    errnum err = OKAY;
//      asm {
//          ldx starting_block
//          ldb num_blocks
//  
//          pshs Y,U
//          swi2
//          fcb F_MAPBLK
//          tfr u,d  ; addr of first block was returned in U
//          puls Y,U
//  
//          bcc MapBlkOK
//          stb err
//          clra
//          clrb
//  MapBlkOK
//          std [first_address_out]
//      }
//      return err;
//  }
//  
//  errnum Os9ClrBlk(byte num_blocks, word first_address) {
//    errnum err = OKAY;
//      asm {
//          ldx first_address  ; will go to U
//          ldb num_blocks
//  
//          pshs Y,U
//          TFR X,U
//          swi2
//          fcb F_CLRBLK
//          puls Y,U
//  
//          bcc ClrBlkOK
//          stb err
//  ClrBlkOK
//      }
//      return err;
//  }
//  
//  errnum Os9ID(byte* proc_id_out, word* user_id_out) {
//    errnum err = OKAY;
//    asm {
//          pshs Y,U
//  
//          swi2
//          fcb F_ID
//  
//          puls Y,U
//          sta [proc_id_out]
//          sty [user_id_out]
//  
//          bcc Os9IDOK
//          stb err
//  Os9IDOK
//    }
//    return err;
//  }
//  
#endif

// *INDENT-ON*

void Os9_print(const char* str) {
  int n = strlen(str);
  while (n) {
    word cc = 0;
    // byte Os9WritLn(int path, const char* buf, int max, int* bytes_written)
    errnum e = Os9WritLn(2, (word)str, n, &cc);
    if (e) Os9Exit(e);
    if (!cc) Os9Exit(245); // 245 = Write Error
    Assert(cc <= n);
    n -= cc;
    str += cc;
  }
}

void GomarHyperExit(errnum status) {
  asm {
    clra
    ldb status
    swi
    fcb 107 ; Hyper Exit
  }
  // NOT REACHED if Hyper, but if no hyper, exit the OS9 process.  
  Os9Exit(status);
}

// 64-byte block routines.
errnum Os9All64(word base, word* base_out, word* block_addr, byte* block_num) {
    errnum err = OKAY;
    asm {
        pshs y,u
        ldx base

        swi2
        fcb F_ALL64
        bcs ALL64BAD

        stx [base_out]
        sty [block_addr]
        sta [block_num]
        bra ALL64OK

ALL64BAD
        stb err
ALL64OK
        puls y,u
    }
    return err;
}

errnum Os9Find64(byte block_num, word base, word* block_addr) {
    errnum err = OKAY;
    asm {
        pshs y,u
        lda block_num
        ldx base

        swi2
        fcb F_FIND64
        bcs FIND64BAD

        sty [block_addr]
        bra FIND64OK

FIND64BAD
        stb err
FIND64OK
        puls y,u
    }
    return err;
}

errnum Os9Ret64(byte block_num, word base) {
    errnum err = OKAY;
    asm {
        pshs y,u
        lda block_num
        ldx base

        swi2
        fcb F_RET64
        bcc RET64OK

        stb err
RET64OK
        puls y,u
    }
    return err;
}
