#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

char buf[2] = { 0x30, 0x00 };

int main(int argc, char* argv[]) {
  int fd = open(argv[1], O_RDWR, 0777);
  assert (fd > 0);

  off_t off = lseek(fd, 3, SEEK_SET);
  assert (off==3);

  ssize_t cc = write(fd, buf, 2);
  assert (cc==2);

  close(fd);
  return 0;
}
