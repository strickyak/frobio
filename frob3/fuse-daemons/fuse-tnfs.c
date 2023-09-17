// fuse-tnfs.c

#ifndef MAX_VERBOSE
#define MAX_VERBOSE LLDebug
#endif

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"
#include "frob3/os9/os9defs.h" // E_EOF
#include "frob3/fuse.h"

#define DAEMON "/Fuse/Daemon/TNFS"
#define PAYSIZ 1500
#define READ_WRITE 03
#define SHARABLE_RW 0133

#define DEFAULT_TNFS_PORT 16384
#define TMP_CLIENT 12345

struct Req {
  struct FuseRequest header;
  char payload[PAYSIZ];
} Request;

struct Rep {
  struct FuseReply header;
  char payload[PAYSIZ];
} Reply;

byte DaemonFd;

///////////////////////////////////////////////////

#define C_MOUNT   0x00 // 0x0000 0x00 0x00, minor=0x02 major=0x01 loc=/home/tnfs 0x00 user=example 0x00 pw=password 0x00 -> 0xBEEF 0x00 0x00, ok=0x00 minor=0x06 major=0x02 retry_ms=(0x88 0x13) || 0x0000 0x00 0x00, err=0x1F minor=0x05 major=0x03

#define C_UMOUNT  0x01 // 0xBEEF 0x00 0x01, -> 0xBEEF 0x00 0x01, ok=0

#define C_OPENDIR 0x10 // 0xBEEF 0x00 0x10, /home/tnfs 0x00 -> 0xBEEF 0x00 0x10, ok=0x00 handle=0x04 || 0xBEEF 0x00 0x10 err=0x1F

#define C_READDIR 0x11 // 0xBEEF 0x00 0x11, handle=0x04 -> ***  0xBEEF 0x19 0x11 ok=0x00 foo 0x00 ||  0xBEEF 0x1A 0x11 eof=0x21

#define C_CLOSEDIR 0x12 // 0xBEEF 0x00 0x12, 0x04 -> 0xBEEF 0x00 0x12, ok=0x00

#define C_OPEN 0x29 // Standard header, flags(2), unix_mode(2), then the null terminated filename. -> ok fd || err

#define C_READ 0x21 // std, fd(1), max_bytes(2) -> std, ok(1), size(2), payload || std, err=0x21 on EOF.

#define C_WRITE 0x22 // std, fd(1), size(2), payload -> std, ok(1), size_written(2) || std, err(1)

#define C_CLOSE 0x23 // std, fd(1) -> ok || err

#define C_STAT 0x24 //  0xBEEF 0x00 0x24 /foo/bar/baz.txt 0x00 -> ok stuff || err

#define C_SIZE 0x30 // 0xBEEF 0x00 0x30, -> 0xBEEF 0x00 0x30, ok=0x00 size_kB=(0xD0 0x02 0x00 0x00)

#define C_FREE 0x31 // 0xBEEF 0x00 0x31, -> 0xBEEF 0x00 0x31, ok=0x00 size_kB=(0xD0 0x02 0x00 0x00)

struct stat_info {
    word mode;     // 2 bytes: File permissions, little endian byte order
    word uid;      // 2 bytes: Numeric UID of owner
    word gid;      // 2 bytes: Numeric GID of owner
    quad size;     // 4 bytes: Unsigned 32 bit little endian size of file in bytes
    quad atime;    // 4 bytes: Access time in seconds since the epoch, little endian
    quad mtime;    // 4 bytes: Modification time (as above)
    quad ctime;    // 4 bytes: Time of last status change (as above)
    // uidstring - 0 or more bytes: Null terminated user id string
    // gidstring - 0 or more bytes: Null terminated group id string
};

// open flags

#define T_O_RDONLY  0x0001 // Open read only
#define T_O_WRONLY  0x0002 // Open write only
#define T_O_RDWR    0x0003 // Open read/write
#define T_O_APPEND  0x0008 // Append to the file, if it exists (write only)
#define T_O_CREAT   0x0100 // Create the file if it doesn't exist (write only)
#define T_O_TRUNC   0x0200 // Truncate the file on open for writing
#define T_O_EXCL    0x0400 // With O_CREAT, returns an error if the file exists

// open modes

#define T_S_IFMT     0170000 // bitmask for the file type bitfields
#define T_S_IFSOCK   0140000 // is a socket
#define T_S_IFLNK    0120000 // is a symlink
#define T_S_IFREG    0100000 // is a regular file
#define T_S_IFBLK    0060000 // block device
#define T_S_IFDIR    0040000 // directory
#define T_S_IFCHR    0020000 // character device
#define T_S_IFIFO    0010000 // FIFO
#define T_S_ISUID    0004000 // set UID bit
#define T_S_ISGID    0002000 // set group ID bit
#define T_S_ISVTX    0001000 // sticky bit
#define T_S_IRWXU    00700   // mask for file owner permissions
#define T_S_IRUSR    00400   // owner has read permission
#define T_S_IWUSR    00200   // owner has write permission
#define T_S_IXUSR    00100   // owner has execute permission
#define T_S_IRGRP    00040   // group has read permission
#define T_S_IWGRP    00020   // group has write permission
#define T_S_IXGRP    00010   // group has execute permission
#define T_S_IROTH    00004   // others have read permission
#define T_S_IWOTH    00002   // others have write permission
#define T_S_IXOTH    00001   // others have execute permission


/*
Byte 2 is a sequence number. This allows the receiver to determine whether
the datagram it just got is a retry or not. It should be incremented
by one for each request sent. Clients should discard datagrams from the
server if the sequence number does not match the number that was in
the request datagram.

The remaining data in the datagram are specific to each command. However,
in any command that may return more than one status (i.e. a command that
can be either succeed or fail in one or more way), byte 4 is the
status of the command, and further data follows from byte 5.

Every command should yield exactly one datagram in response. A high
level operation (such as a call to read()) asking for a buffer larger
than the size of one UDP datagram should manage this with as many requests
and responses as is necessary to fill the buffer.

The server can also ask the client to back off. If a server can operate
with interrupts enabled while the physical disc is busy, and therefore
still be able to process requests, it can tell the client that it is busy
and to try again later. In this case, the `EAGAIN` error code will be
returned for whatever command was being tried, and following the error
code, will be a 16 bit little endian value giving how long to back off in
milliseconds. Servers that have this ability should use it, as the server
can then better control contention on a slow device, like a floppy disk,
since the server can figure out how many requests clients are trying
to make and set the back-off value accordingly. Clients should retry as normal
once the back-off time expires.

*/

// errors

#define T_OKAY 0x00 // Success
#define T_EPERM 0x01 // Operation not permitted
#define T_ENOENT 0x02 // No such file or directory
#define T_EIO 0x03 // I/O error
#define T_ENXIO 0x04 // No such device or address
#define T_E2BIG 0x05 // Argument list too long
#define T_EBADF 0x06 // Bad file number
#define T_EAGAIN 0x07 // Try again
#define T_ENOMEM 0x08 // Out of memory
#define T_EACCES 0x09 // Permission denied
#define T_EBUSY 0x0A // Device or resource busy
#define T_EEXIST 0x0B // File exists
#define T_ENOTDIR 0x0C // Is not a directory
#define T_EISDIR 0x0D // Is a directory
#define T_EINVAL 0x0E // Invalid argument
#define T_ENFILE 0x0F // File table overflow
#define T_EMFILE 0x10 // Too many open files
#define T_EFBIG 0x11 // File too large
#define T_ENOSPC 0x12 // No space left on device
#define T_ESPIPE 0x13 // Attempt to seek on a FIFO or pipe
#define T_EROFS 0x14 // Read only filesystem
#define T_ENAMETOOLONG 0x15 // Filename too long
#define T_ENOSYS 0x16 // Function not implemented
#define T_ENOTEMPTY 0x17 // Directory not empty
#define T_ELOOP 0x18 // Too many symbolic links encountered
#define T_ENODATA 0x19 // No data available
#define T_ENOSTR 0x1A // Out of streams resources
#define T_EPROTO 0x1B // Protocol error
#define T_EBADFD 0x1C // File descriptor in bad state
#define T_EUSERS 0x1D // Too many users
#define T_ENOBUFS 0x1E // No buffer space available
#define T_EALREADY 0x1F // Operation already in progress
#define T_ESTALE 0x20 // Stale TNFS handle
#define T_EOF 0x21 // End of file
#define T_EHANDLE 0xFF // Invalid TNFS handle

////////////////////////////////////////////

struct THeader {
    word session;
    byte sequence;
    byte command;
};

//////////////////////////////

#define MAX_PATHS 32 // Limit on internal OS9 path number.

// Requests identify which internal OS9 path they are coming from.
// Since OS9 cannot support more than a couple dozen paths,
// we map them one-to-one into this array, even if most slots
// are never used.
struct PathInfo {
  byte handle;
  struct mount* mount;
  const char* filename;
} Paths[MAX_PATHS];

/////////////////////////////////

struct {
  struct THeader header;
  char payload[PAYSIZ];
} TRequest, TReply;

byte socknum;  // One client can work for all mounts.

#define Hi(X) ((byte)((word)(X)>>8))
#define Lo(X) ((byte)(X))
#define HiLo(H,L) ( ((word)(255&(H))<<8) | (word)((L)&255) )

char* PutB(char*z, byte b) {
    *z = b;
    return z+1;
}
char* PutW(char*z, word w) {
    *z++ = Lo(w);  // Little Endian for TNFS!
    *z++ = Hi(w);
    return z;
}
char* PutS(char*z, const char* s) {
    size_t n = strlen(s);
    memcpy(z, s, n+1);
    return z+n+1;
}

void HexDump(char* payload, word size) {  // Just verbosity.
    if (size > 1030) {
      LogStep("HexDump: too big: %x", size);
      return;
    }
    for (word i = 0; i < size; i += 16) {
      if (i>=32) break; ///////////////////////// STOP SHORT
      Buf buf;
      BufInit(&buf);
      BufFormat(&buf, "%04x: ", i);
      for (word j = 0; j < 16; j++) {
        if (i+j < size) {
          BufFormat(&buf, "%02x ", 255&payload[i+j]);
        } else {
          BufFormat(&buf, "   ");
        }
        if ((j&3)==3) BufAppC(&buf, ' ');
      }
      BufAppC(&buf, ' ');
      for (word j = 0; j < 16 && (i+j) < size; j++) {
        char ch = payload[i+j];
        if (32 <= ch && ch <= 126) {
          BufAppC(&buf, ch);
        } else {
          BufAppC(&buf, '.');
        }
      }
      BufAppC(&buf, '\n');
      FPuts(BufFinish(&buf), StdOut);
      BufDel(&buf);
    }
    Printf("\n");
}

struct mount {
    struct mount* next;
    const char* name;
    const char* location;
    const char* path;
    const char* user;
    const char* password;
    quad ip_addr;
    word port;
    word session;
    byte sequence;
} *Mounts;

word SwapWord(word x) {
    return HiLo(Lo(x), Hi(x));
};

void DoMount(struct mount* p) {
  TRequest.header.session = 0;
  TRequest.header.sequence = 0;
  TRequest.header.command = C_MOUNT;
  char* z = TRequest.payload;
  z = PutB(z, 0); // minor version
  z = PutB(z, 0); // major version
  z = PutS(z, p->path); 
  z = PutS(z, p->user);
  z = PutS(z, p->password);
  char* treq = (char*)(&TRequest);
  size_t len = z - treq;

  {
    LogInfo("Sending MOUNT to %s...", p->name);
    prob ps = UdpSend(socknum, (byte*)treq, len, p->ip_addr, p->port);
    if (ps) LogFatal("cannot UdpSend MOUNT %s: %s", p->name, ps);
  }

  quad from_addr = 0;
  word from_port = 0;
  word size = sizeof TReply;
  char* reply = (char*)(&TReply);
  {
    LogInfo("Receiving MOUNT from %s...", p->name);
    prob ps = UdpRecv(socknum, (byte*)reply, &size, &from_addr, &from_port);
    if (ps) LogFatal("cannot UdpRecv MOUNT %s: %s", p->name, ps);
  }
  if (TReply.header.command != C_MOUNT) {
    LogFatal("Expected C_MOUNT got %d", TReply.header.command);
  }
  if (TReply.payload[0]) {
    LogFatal("C_MOUNT bad %d", TReply.payload[0]);
  }
  p->session = TReply.header.session;
  p->sequence = 1;
  LogInfo("MOUNT %q OK.", p->name);
}

void CreateMounts(int argc, char* argv[]) {
    if (argc == 0 || (argc % 5) != 0) {
        LogFatal("Bad number of args, should be positive multiple of 5: %d", argc);
    }
    for (int i = 0; i+4 < argc; i+=5) {
        struct mount* p = (struct mount*) Malloc(sizeof *p);
        memset(p, 0, sizeof *p);
        p->name = (const char*) argv[i];
        p->location = (const char*) argv[i+1];
        p->path = (const char*) argv[i+2];
        p->user = (const char*) argv[i+3];
        if (!strcmp(p->user, ".")) p->user = "";
        p->password = (const char*) argv[i+4];
        if (!strcmp(p->password, ".")) p->password = "";
        p->next = Mounts;
        Mounts = p;
        LogStep("Mounting /FUSE/TNFS/%s from %s at %s (user %s pw %s)", p->name, p->path, p->location, p->user, p->password);

        // TODO: reach for my resolver.
        const char * tmp = (const char*)p->location;
        p->port = DEFAULT_TNFS_PORT;
        p->ip_addr = NyParseDottedDecimalQuadAndPort(&tmp, &p->port);

        DoMount(p);
    }
}

const char* StrDupRequestedFileName() {
  // Where we get the filename.
  char* pay = Request.payload;
  word n = Request.header.size;
  LogStep("Dup: n=%x pay=%s", n, pay);

  // Malloc a string for the filename. 
  char* s = (char*) Malloc(n + 1);
  Assert(s);
  memcpy(s, pay, n);
  s[n] = '\0';

  // PrsNam includes trailing spaces at the end
  // of the path.  This gets rid of trailing
  // whitespace.
  for (word i = 0; i < n; i++) {
    if (s[i] <= 32) s[i] = '\0';
  }

  return (const char*)s;
}

bool SplitThirdAndRest(const char* s, const char** third_out, const char** rest_out) {
  // Skip first name, e.g. "FUSE"
  while (*s == '/') s++; // Skip Slashes
  if (*s <= 32) return false;

  while (*s > 32 && *s != '/') s++;  // Skip component
  if (*s <= 32) return false;

  // Skip second name, e.g. "TFTP"
  while (*s == '/') s++; // Skip Slashes
  if (*s <= 32) return false;

  while (*s > 32 && *s != '/') s++;  // Skip component
  if (*s <= 32) return false;

  // Remember third name, e.g. "10.5.5.5"
  while (*s == '/') s++; // Skip Slashes
  if (*s <= 32) return false;

  const char* third_begin = s;
  while (*s > 32 && *s != '/') s++;  // Skip component
  if (*s <= 32) return false;

  *(char*)s = '\0'; // Temporarily alter input string.
  *third_out = StrDup((byte*)third_begin);
  *(char*)s = '/'; // Restore input string.
  s++;

  while (*s == '/') s++; // Skip Slashes
  if (*s <= 32) return false;

  *rest_out = StrDup((byte*)s);
  return true;
}

void DoOpen(bool unused_create) {
  const char* name = StrDupRequestedFileName();
  const char* third = NULL;
  const char* rest = NULL;
  CheckB(SplitThirdAndRest, (name, &third, &rest));

  struct PathInfo *p = &Paths[Request.header.path_num];
  p->filename = (const char*)strdup(rest); 

  struct mount* m = Mounts;
  for (; m; m=m->next) {
    if (!strcasecmp(m->name, third)) break;
  }
  if (!m) {
    LogFatal("Cannot found tnfs mount named %q", third);
  }
  p->mount = m; 

  //////////

  TRequest.header.session = m->session;
  TRequest.header.sequence = m->sequence++;
  TRequest.header.command = C_OPEN;
  char* z = TRequest.payload;
  z = PutW(z, 0); // flags
  z = PutW(z, 1); // mode -- read
  z = PutS(z, rest);
  char* treq = (char*)(&TRequest);
  size_t len = z - treq;

  {
    LogInfo("Sending OPEN to %s...", m->name);
    LogDebug("treq=%x len=%x", treq, len);
    HexDump(treq, len);
    prob ps = UdpSend(socknum, (byte*)treq, len, m->ip_addr, m->port);
    if (ps) LogFatal("cannot UdpSend OPEN %s: %s", m->name, ps);
  }

  quad from_addr = 0;
  word from_port = 0;
  word size = sizeof TReply;
  char* tReply = (char*)(&TReply);
  {
    LogInfo("Receiving OPEN from %s...", m->name);
    prob ps = UdpRecv(socknum, (byte*)tReply, &size, &from_addr, &from_port);
    if (ps) LogFatal("cannot UdpRecv OPEN %s: %s", m->name, ps);
  }
  if (TReply.header.command != C_OPEN) {
    LogFatal("Expected C_OPEN got %d", TReply.header.command);
  }
  if (TReply.payload[0]) {
    LogFatal("C_OPEN bad %d", TReply.payload[0]);
  }
  LogFatal("TODO after OPEN");
}
void DoClose() {
}
void DoRead(bool linely) {
}
void DoWrite(bool unused_linely) {
}
//////////////////////////////////////////

// Fixing the size for where the first Carriage Return appears
// is the job if the daemon, becuase the FuseMan copies bytes
// here without looking at them.
void FixSizeForWritLn() {
  word n = MIN(Request.header.size, sizeof Request.payload);
  char* p = Request.payload;
  word i;
  for (i = 0; i < n; i++) {
    if (*p == '\r') {
      Request.header.size = i+1;
      return;
    }
    p++;
  }
  Request.header.size = n;
}

int main(int argc, char* argv[]) {
  Verbosity = 9;
  LogStep("Starting tnfs daemon.");

  // Open the /FUSE device in *Daemon Mode*.
  // That is, open the path name "/Fuse/Daemon/TNFS"
  // where the second component "Daemon" means we are the daemon,
  // and the third component "TNFS" is which fuse we will serve.
  // Clients opening "/Fuse/TNFS/..." will send their
  // filesystem operations to this daemon.
  CheckE(Os9Create, (/*mode=*/READ_WRITE, /*attrs=*/SHARABLE_RW, DAEMON, &DaemonFd));

  CheckP(UdpOpen, (TMP_CLIENT, &socknum));

  CreateMounts(argc-1, argv+1);

  // Now go into an infinite loop calling Os9Read to get the request
  // and then Os9Write to send the reply.
  while (true) {
    size_t cc;
    LogInfo("Reading...");
    CheckE(Os9Read, (DaemonFd, (word)&Request, sizeof Request, &cc));
    Assert(cc >= sizeof Request.header);

    LogInfo("Read: op=%x path=%x a=%x b=%x size=%x cc=%x",
        Request.header.operation,
        Request.header.path_num,
        Request.header.a_reg,
        Request.header.b_reg,
        Request.header.size,
        cc); // Just verbosity.
    if (cc > sizeof Request.header) { // Just verbosity.
      HexDump(Request.payload, cc - sizeof Request.header);
    }

    // What we do depends on the operation, so we have a big switch.
    // Different operations need different handling.
    int n = sizeof Reply.header;
    switch (Request.header.operation) {
      case OP_CREATE: DoOpen(true); break;
      case OP_OPEN: DoOpen(false); break;
      case OP_CLOSE: DoClose(); break;
      case OP_READ: DoRead(false); n += Reply.header.size; break;
      case OP_READLN: DoRead(true); n += Reply.header.size; break;
      case OP_WRITE: DoWrite(false); break;
      case OP_WRITLN: FixSizeForWritLn(); DoWrite(true); break;
      default:
        Reply.header.status = E_UNKSVC;
        Reply.header.size = 0;
        LogError("Bad operation: %d", Request.header.operation);
    }

    LogInfo("Reply: status=%x size=%x n=%x",
        Reply.header.status,
        Reply.header.size,
        n); // Just verbosity.
    if (n > sizeof Reply.header) { // Just verbosity.
      HexDump(Reply.payload, n - sizeof Reply.header);
    }

    CheckE(Os9Write, (DaemonFd, (word)&Reply, n, &cc));
    LogInfo("Wrote Reply.");
  }

  // NOTREACHED
  return 0;
}
