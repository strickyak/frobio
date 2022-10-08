#include "frob2/froblib.h"
#include "frob2/frobos9.h"

#ifdef unix

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

byte disable_irq_count;

void Os9Exit(byte status) {
    exit(status);
}

errnum Os9Delete(const char* path) {
    int e = unlink(path);
    if (e < 0) return errno;
    return 0;
}

errnum Os9Create(const char* path, int mode, int attrs, int* fd) {
    int p = creat(path, 0777);
    if (p < 0) return errno;
    *fd = p;
    return 0;
}

errnum Os9Open(const char* path, int mode, int* fd) {
    int p = open(path, O_RDONLY, 0777);
    if (p < 0) return errno;
    *fd = p;
    return 0;
}

errnum Os9Read(int path, char* buf, int buflen, int* bytes_read) {
    int cc = read(path, buf, buflen);
    if (cc < 0) return errno;
    *bytes_read = cc;
    return 0;
}

errnum Os9ReadLn(int path, char* buf, int buflen, int* bytes_read) {
    int cc = read(path, buf, buflen);
    if (cc < 0) return errno;
    *bytes_read = cc;
    return 0;
}

errnum Os9Write(int path, const char* buf, int max, int* bytes_written) {
    int cc = write(path, buf, max);
    if (cc < 0) return errno;
    *bytes_written = cc;
    return 0;
}

errnum Os9WritLn(int path, const char* buf, int max, int* bytes_written) {
    int cc = write(path, buf, max);
    if (cc < 0) return errno;
    *bytes_written = cc;
    return 0;
}

errnum Os9Dup(int path, int* new_path) {
    int fd = dup(path);
    if (fd < 0) return errno;
    *new_path = fd;
    return 0;
}

errnum Os9Close(int path) {
    int e = close(path);
    if (e < 0) return errno;
    return 0;
}

void GomarHyperExit(errnum status) {  // exits unix process
    exit(status);
}
#endif
