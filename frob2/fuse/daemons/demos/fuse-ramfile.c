// demo fuse daemon: fuse.ramfile (/FUSE/RamFile)

#ifndef MAX_VERBOSE
#define MAX_VERBOSE LLStep /* Log one banner, then only errors. */
#endif

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"
#include "frob2/os9/os9defs.h" // E_EOF
#include "frob2/fuse/fuse.h"

#define DAEMON "/Fuse/Daemon/RamFile"
#define PAYSIZ 2048
#define READ_WRITE 03
#define SHARABLE_RW 0133

struct Req {
  struct FuseRequest header;
  char payload[PAYSIZ];
} Request;

struct Rep {
  struct FuseReply header;
  char payload[PAYSIZ];
} Reply;

int DaemonFd;

//////////////////////////////////////////

#define MAX_PATHS 32 // Limit on internal OS9 path number.

// Requests identify which internal OS9 path they are coming from.
// Since OS9 cannot support more than a couple dozen paths,
// we map them one-to-one into this array, even if most slots
// are never used.
struct PathInfo {
  bool writing; // 1=Writing 0=Reading
  word offset;
  struct FileInfo* file;
} Paths[MAX_PATHS];

#define MAX_FILES 50 // How may files we can contain.

// This daemon holds in RAM the contents of a few files.
// They are stored here.
// It's only designed for small text files.
struct FileInfo {
  const char* name;
  char* contents;
  word size;
} Files[MAX_FILES];

//////////////////////////////////////////

void HexDump(char* payload, word size) {  // Just verbosity.
#if MAX_VERBOSE >= LLDebug
    if (size > 1030) {
      LogFatal("HexDump: too big: %x", size);
    }
    for (word i = 0; i < size; i += 16) {
      Buf buf;
      BufInit(&buf);
      BufFormat(&buf, "%04x: ", i);
      for (word j = 0; j < 16; j++) {
        if (i+j < size) {
          BufFormat(&buf, "%02x ", payload[i+j]);
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
#endif
}

void ShowFileInfo(struct FileInfo* f) {  // Just verbosity.
  if (f) {
    LogDebug("FileInfo[%x] { name=%q contents=%x size=%x }",
        f-Files, f->name, f->contents, f->size);
    if (f->size) {
      HexDump(f->contents, f->size);
    }
  } else {
    LogDebug("f=NULL");
  }
}

void ShowPathInfo(struct PathInfo* p) {  // Just verbosity.
  if (p) {
      LogDebug("PathInfo[%x] { writing:%x offset=%x file=%x }", 
          p-Paths, p->writing, p->offset, p->file);
      if (p->file) {
        ShowFileInfo(p->file);
      }
  } else {
    LogDebug("p=NULL");
  }
}

//////////////////////////////////////////

struct FileInfo* FindFile(const char* name) {
  for (struct FileInfo* p = Files; p < Files+MAX_FILES; p++) {
    if (p->name && !strcasecmp(p->name, name)) {
      return p;
    }
  }
  return NULL;
}

struct FileInfo* FindUnusedFile() {
  for (struct FileInfo* p = Files; p < Files+MAX_FILES; p++) {
    if (!p->name) return p;
  }
  return NULL;
}

const char* RequestedFileName() {
  // Where we get the filename.
  char* pay = Request.payload;
  word n = Request.header.size;

  // Malloc a string for the filename. 
  char* s = (char*) Malloc(n + 1);
  memcpy(s, pay, n);
  s[n] = '\0';

  // BUG WORKAROUND: I saw trailing space in filename?
  for (word i = 0; i < n; i++) {
    if (s[i] <= 32) s[i] = '\0';
  }

  return (const char*)s;
}

errnum DoOpenWrite() {
  const char* name = RequestedFileName();
  struct FileInfo* f = FindFile(name);
  if (!f) {
    f = FindUnusedFile();
    if (!f) {
      Free((void*)name);
      return E_FULL; // "Media Full"
    }
    f->name = StrDup((byte*)name);
    f->contents = (char*) Malloc(16);  // Starter memory.
  }
  f->size = 0; // Truncate if existing.

  struct PathInfo *p = &Paths[Request.header.path_num];
  p->file = f;
  p->offset = 0;
  p->writing = true;

  LogInfo("DoOpenWrite: path=%x file=%q", Request.header.path_num, name);
  Free((void*)name);
  return 0;
}

errnum DoOpenRead() {
  const char* name = RequestedFileName();
  struct FileInfo* f = FindFile(name);
  if (!f) {
    Free((void*)name);
    return E_PNNF; // "Path Name Not Found"
  }

  struct PathInfo *p = &Paths[Request.header.path_num];
  p->file = f;
  p->offset = 0;
  p->writing = false;

  LogInfo("DoOpenRead: path=%x file=%q", Request.header.path_num, name);
  Free((void*)name);
  return 0;
}

void DoOpen(bool unused_create) {
  errnum e;
  switch (Request.header.a_reg) {
    case 1: // Read
        e = DoOpenRead();
        break;
    case 2: // Write
        e = DoOpenWrite();
        break;
    default:
        e = E_BMODE; // "Bad Mode"
  }

  Reply.header.status = e;
  Reply.header.size = 0;
}
void DoClose() {
  struct PathInfo *p = &Paths[Request.header.path_num];
  LogInfo("DoClose: path=%x file=%q", Request.header.path_num, p->file->name);
  p->file = NULL;
  p->offset = 0;
  p->writing = false;

  Reply.header.status = 0;
  Reply.header.size = 0;
}

void DoRead(bool linely) {
  errnum e = 0;
  struct PathInfo *p = &Paths[Request.header.path_num];
  struct FileInfo *f = p->file;
  Assert(f);
  LogInfo("DoRead%s path=%x", (linely? "ln" : ""), Request.header.path_num);
  ShowPathInfo(p);

  if (p->writing) { e = E_BMODE; goto ERROR; }

  if (p->offset >= f->size) {
    e = E_EOF;
    goto ERROR;
  }

  word i;
  if (linely) {
    // Find the end of the next line.
    for (i = p->offset; i < f->size; i++) {
      char ch = f->contents[i];
      if (ch=='\r') {
        i++; // Keep the CR.
        break;
      }
    }
  } else {
    i = MIN(p->offset+Request.header.size, f->size);
  }

  word n = i - p->offset; // num bytes to return.
  memcpy(Reply.payload, f->contents+p->offset, n);
  p->offset = i;  // Advance.
  Reply.header.status = 0;
  Reply.header.size = n;
  return;

ERROR:
  Reply.header.status = e;
  Reply.header.size = 0;
}


void DoWrite(bool unused_linely) {
  errnum e = 0;
  struct PathInfo *p = &Paths[Request.header.path_num];
  struct FileInfo* f = p->file;
  Assert(f);
  LogInfo("DoWritLn path=%x", Request.header.path_num);
  ShowPathInfo(p);

  if (!p->writing) { e = E_BMODE; goto ERROR; }

  word n = Request.header.size;

  // Append n bytes to the contents.
  f->contents = ReAlloc((void*)f->contents, f->size + n);
  memcpy(f->contents + f->size, Request.payload, n);
  f->size += n;

  Reply.header.status = 0;
  Reply.header.size = n;
  return;

ERROR:
  Reply.header.status = e;
  Reply.header.size = 0;
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
  // We can ignore argc, argv.
  LogStep("Starting.");

  // Open the /FUSE device in *Daemon Mode*.
  // That is, open the path name "/Fuse/Daemon/RamFile"
  // where the second component "Daemon" means we are the daemon,
  // and the third component "RamFile" is which fuse we will serve.
  // Clients opening "/Fuse/RamFile/..." will send their
  // filesystem operations to this daemon.
  CheckE(Os9Create, (DAEMON, /*mode=*/READ_WRITE, /*attrs=*/SHARABLE_RW, &DaemonFd));

  // Now go into an infinite loop calling Os9Read to get the request
  // and then Os9Write to send the reply.
  while (true) {
    int cc;
    LogInfo("Reading...");
    CheckE(Os9Read, (DaemonFd, (char*)&Request, sizeof Request, &cc));
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

    CheckE(Os9Write, (DaemonFd, (char*)&Reply, n, &cc));
    LogInfo("Wrote Reply.");
  }
  // NOTREACHED
  return 0;
}
