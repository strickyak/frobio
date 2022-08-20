#include "frobio/nytypes.h"

#include "frobio/ncl/buf.h"

static char buf[200];

char** FetchSourceAsLines(const char* url) {
    // For now, URL must be a local filepath.

#if unix
    FILE* r = fopen(url, "r");
    if (!r) {
        perror("cannot open src file");
        exit(2);
    }

    Buf b;
    BufInit(&b);
    while (true) {
        char* s = fgets(buf, sizeof buf, r);
        if (!s) break;
        BufAppDope(buf);
    }
    fclose(r);
    return BufTakedope(&b);
#else
    int r = -1;
    error e = Os9Open(url, 1/*=read*/, &r);
    if (e) panic("cannot open src file");

    Buf b;
    BufInit(&b);
    while (true) {
        Os9ReadLn(buf);
        BufAppDope(line);
    }
    return BufTakedope(&b);
#endif

}

#ifndef unix

typedef struct file { int fd; } FILE;

FILE* fopen(const char* pathname, const char* mode) {
    assert(pathname);
    assert(mode);
    int m = 1;
    if (mode[0] == 'w') m = 2;
    int fd = -1;
    error e = Os9Open(pathname, m, &fd);
    if (e) return NULL;
    FILE* f = malloc(sizeof FILE);
    f->fd = fd;
    return f;
}

char *fgets(char *buf, int size, FILE *f) {
    assert(buf);
    assert(size);
    assert(f);
    int bytes_read;
    error e = Os9ReadLn(f->fd, buf, size-1, &bytes_read);
    if (e) return NULL;
    buf[bytes_read] = '\0';
    return buf;
}

int fclose(FILE *f) {
    assert(f);
    error e = Os9Close(f->fd);
    if (e) return EOF;
    return 0;
}


#endif
