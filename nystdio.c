#define DONT_RENAME_NY_STDIO 1 // Use ny_ names in this source file.
#include "frobio/nystdio.h"
#include "frobio/os9call.h"
#include "frobio/ncl/malloc.h"

#ifndef unix

int ny_errno;
NY_FILE ny_stdin_file = {0};
NY_FILE ny_stdout_file = {1};
NY_FILE ny_stderr_file = {2};

NY_FILE* ny_fopen(const char* pathname, const char* mode) {
    assert(pathname);
    assert(mode);
    int fd = -1;
    assert(mode[1]=='\0' && (mode[0]=='r'||mode[0]=='w'));

    if (mode[0] == 'r') {
      ny_errno = Os9Open(pathname, 1/*=READ*/, &fd);
    } else if (mode[0] == 'w') {
      Os9Delete(pathname);
      ny_errno = Os9Create(pathname, 2/*=WRITE*/, 3/*=READ+WRITE*/, &fd);
    }
    if (fd<0) return NULL;

    NY_FILE* f = (NY_FILE*) malloc(sizeof *f);
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

int ny_fputs(const char *str, NY_FILE *f) {
    assert(str);
    assert(f);
    int bytes_written = 0;
    int n = strlen(str);
    ny_errno = Os9Write(f->fd, str, n, &bytes_written);
    if (ny_errno) return EOF;
    return bytes_written;
}

int ny_fclose(NY_FILE *f) {
    assert(f);
    ny_errno = Os9Close(f->fd);
    f->fd = -1;
    free(f);
    if (ny_errno) return EOF;
    return 0;
}

void ny_perror(const char* str) {
    assert(str);
    ny_fputs(" ", ny_stderr);
    ny_fputs(str, ny_stderr);
    ny_fputs(": ERROR TODO\n", ny_stderr);
}

#endif
