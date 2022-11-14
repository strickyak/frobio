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

File* FOpen(const char* pathname, const char* mode) {
    Assert(pathname);
    Assert(mode);
    Assert(mode[1]=='\0' && (mode[0]=='r'||mode[0]=='w'));
    int fd = -1;

    if (mode[0] == 'r') {
      errnum e = Os9Open(pathname, 1/*=READ*/, &fd);
      if (e) {FailE(e, "Cannot open: %s", pathname); return NULL;}
    } else if (mode[0] == 'w') {
      Os9Delete(pathname);
      errnum e = Os9Create(pathname, 2/*=WRITE*/, 3/*=READ+WRITE*/, &fd);
      if (e) {FailE(e, "Cannot create: %s", pathname); return NULL;}
    }
    Assert (fd>=0);

    File* f = (File*) Malloc(sizeof *f);
    f->fd = fd;
    return f;
}

word FGets(char *buf, int size, File *f) {
    Assert(buf);
    Assert(size);
    Assert(f);
    int bytes_read = 0;
    errnum e = Os9ReadLn(f->fd, buf, size-1, &bytes_read);
    buf[bytes_read] = '\0';
    if (e) {FailE(e, "ReadLn fails on path %d", f->fd);}
    return bytes_read;
}

void FPuts(const char *str, File *f) {
    Assert(str);
    Assert(f);
    int n = strlen(str);
    WritLnAll(f->fd, str, n);
}

void FClose(File *f) {
    Assert(f);
    errnum e = Os9Close(f->fd);
    f->fd = -1;
    Free(f);
    if (e) {FailE(e, "Close fails on path %d", f->fd);}
}

void PError(const char* str) {
    Assert(str);
    FPuts(" ", StdErr);
    FPuts(str, StdErr);
    FPuts(": ERROR TODO\n", StdErr);
}
