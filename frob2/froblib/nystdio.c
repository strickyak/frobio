#include "frob2/froblib.h"
#include "frob2/frobos9.h"

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
    int fd = -1;

    if (mode[0] == 'r') {
      ErrNo = Os9Open(pathname, 1/*=READ*/, &fd);
      if (ErrNo) return NULL;
    } else if (mode[0] == 'w') {
      Os9Delete(pathname);
      ErrNo = Os9Create(pathname, 2/*=WRITE*/, 3/*=READ+WRITE*/, &fd);
      if (ErrNo) return NULL;
    }
    Assert (fd>=0);

    File* f = (File*) Malloc(sizeof *f);
    f->fd = fd;
    return f;
}

// FOpen returns bytes_read, or 0 and sets ErrNo on error.
word FGets(char *buf, int size, File *f) {
    Assert(buf);
    Assert(size);
    Assert(f);
    int bytes_read = 0;
    ErrNo = Os9ReadLn(f->fd, buf, size-1, &bytes_read);
    if (ErrNo) return 0;
    buf[bytes_read] = '\0';
    return bytes_read;
}

// returns length of str, or -1 on error.
int FPuts(const char *str, File *f) {
    Assert(str);
    Assert(f);
    int n = strlen(str);
    ErrNo = WritLnAll(f->fd, str, n);
    return (ErrNo) ? -1 : n;
}

// returns 0, or -1 on error.
int FClose(File *f) {
    Assert(f);
    ErrNo = Os9Close(f->fd);
    if (ErrNo) return -1;
    f->fd = -1;
    Free(f);
    return 0;
}

void PError(const char* str) {
    int e = ErrNo;
    ErrNo = OKAY;
    Assert(str);
    FPuts(" ", StdErr);
    FPuts(str, StdErr);
    char ebuf[4];
    ebuf[0] = (char) (e / 100 + '0');
    e = e % 100;
    ebuf[1] = (char) (e / 10 + '0');
    e = e % 10;
    ebuf[2] = (char) (e + '0');
    ebuf[3] = '\0';
    FPuts(": ErrNo ", StdErr);
    FPuts(ebuf, StdErr);
    FPuts(".\n", StdErr);
}
