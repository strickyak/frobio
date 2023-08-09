#include "frob3/froblib.h"
#include "frob3/frobos9.h"
#include "frob3/os9/os9defs.h"

// forward
extern File StdIn_File;
extern File StdOut_File;
extern File StdErr_File;

//chop
File StdIn_File = {0};
//chop
File StdOut_File = {1};
//chop
File StdErr_File = {2};
//chop

// FOpen returns NULL and sets ErrNo on error.
File* FOpen(const char* pathname, const char* mode) {
    Assert(pathname);
    Assert(mode);
    Assert(mode[1]=='\0' && (mode[0]=='r'||mode[0]=='w'));
    byte fd = 255;

    if (mode[0] == 'r') {
      ErrNo = Os9Open(1/*=READ*/, pathname, &fd);
      if (ErrNo) LogDebug("Os9Open returned %x", ErrNo);
      if (ErrNo) return NULL;
    } else if (mode[0] == 'w') {
      Os9Delete(pathname);
      ErrNo = Os9Create(2/*=WRITE*/, 3/*=READ+WRITE*/, pathname, &fd);
      if (ErrNo) LogDebug("Os9Create returned %x", ErrNo);
      if (ErrNo) return NULL;
    }

    File* f = (File*) Malloc(sizeof *f);
    f->fd = fd;
    LogDebug("FOpen returning (File*) %x with fd=%x", f, fd);
    return f;
}

// FOpen returns bytes_read, or 0 on EOF, and sets ErrNo on error.
word FGets(char *buf, int size, File *f) {
    Assert(buf);
    Assert(size);
    Assert(f);
    word bytes_read = 0;
    ErrNo = Os9ReadLn((byte)f->fd, (word)buf, size-1, &bytes_read);
    if (ErrNo == E_EOF) {
      ErrNo = OKAY;
      bytes_read = 0;
    }
    if (ErrNo) return 0;
    buf[bytes_read] = '\0';
    return bytes_read;
}

// returns length of str, or -1 on error.
int FPuts(const char *str, File *f) {
    Assert(str);
    Assert(f);
    int n = strlen(str);
    ErrNo = WritLnAll((byte)f->fd, (word)str, n);
    return (ErrNo) ? -1 : n;
}

// returns 0, or -1 on error.
int FClose(File *f) {
    Assert(f);
    ErrNo = Os9Close((byte)f->fd);
    if (ErrNo) return -1;
    f->fd = -1;
    Free(f);
    return 0;
}

void PErrNum(errnum e) {
    errnum saved = ErrNo;
    char ebuf[4];

    ebuf[0] = (char) (e / 100 + '0');
    e = e % 100;
    ebuf[1] = (char) (e / 10 + '0');
    e = e % 10;
    ebuf[2] = (char) (e + '0');
    ebuf[3] = '\0';

    FPuts(" Err", StdErr);
    FPuts(ebuf, StdErr);
    FPuts(".", StdErr);
    ErrNo = saved;
}

void PErrorFatal(const char* str) {
    errnum saved = ErrNo;
    FPuts("*** FATAL:", StdErr);
    ErrNo = saved;
    PError(str);
    exit(127);
}

void PError(const char* str) {
    errnum e = ErrNo;
    ErrNo = OKAY;
    Assert(str);
    FPuts(" ", StdErr);
    FPuts(str, StdErr);
    FPuts(" :", StdErr);
    PErrNum(e);
    FPuts("\n", StdErr);
    
    // Printing resets ErrNo.
    // TODO: when does <stdlib> reset errno?
    ErrNo = OKAY;
}
