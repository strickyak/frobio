#define DONT_RENAME_FOPEN 1 // Use ny_ names in this source file.
#include "frobio/nystdio.h"

#ifndef unix

typedef struct file { int fd; } NY_FILE;

int ny_errno;
NY_FILE ny_stdin = {0};
NY_FILE ny_stdout = {1};
NY_FILE ny_stderr = {2};

byte Guard0[8];  // unchecked, delete me later.
byte StaticBuffer[256];
byte Guard1[8];  // unchecked, delete me later.
byte* BufferP;
byte* BufferEnd;

NY_FILE* ny_fopen(const char* pathname, const char* mode) {
    assert(pathname);
    assert(mode);
    int fd = -1;
    assert(mode[1]=='\0' && (mode[0]=='r'||mode[0]=='w'));

    if (mode[0] == 'r') {
      ny_errno = Os9Open(pathname, 1, &fd);
    } else if (mode[0] == 'w') {
      ny_errno = Os9Create(pathname, 2, &fd);
    }

    if (ny_errno) return NULL;
    NY_FILE* f = malloc(sizeof NY_FILE);
    f->fd = fd;
    return f;
}

char *ny_fgets(char *buf, int size, NY_FILE *f) {
    assert(buf);
    assert(size);
    assert(f);
    int bytes_read = 0;
    ny_errno = Os9ReadLn(f->fd, buf, size-1, &bytes_read);
    if (ny_errno) return NULL;
    buf[bytes_read] = '\0';
    return buf;
}

int ny_fputs(const char *s, NY_FILE *f) {
    assert(s);
    assert(f);
    int bytes_written = 0;
    int n = strlen(s);
    ny_errno = Os9Write(f->fd, buf, n, &bytes_written);
    if (ny_errno) return EOF;
    return bytes_written;
}

int ny_fclose(NY_FILE *f) {
    assert(f);
    ny_errno = Os9Close(f->fd);
    f->fd = -1;
    if (ny_errno) return EOF;
    return 0;
}

int ny_perror(const char* s) {
    assert(s);
    ny_fputs(" ", stderr);
    ny_fputs(s, stderr);
    ny_fputs(": ERROR TODO\n", stderr);
}

#endif
