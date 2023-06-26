#ifndef _FROB3_FUSE_H_
#define _FROB3_FUSE_H_

// A fuse daemon reads messages from the FUSE manager
// with this header prepended to the payload.
struct FuseRequest {
  unsigned char operation;
  unsigned char path_num; // kernel path number.
  unsigned char a_reg;    // e.g. access mode
  unsigned char b_reg;    // e.g. file attrs, Stt code
  unsigned int size;      // buffer or bytes_used size, in or out.
}; // sizeof == 6

// A fuse daemon writes messages to the FUSE manager
// with this header prepended to the payload.
struct FuseReply {
  unsigned char status; // 0 or OS9 error number.
  unsigned int size;    // size in or size out.
}; // sizeof == 3

// Values for the `operation` field.
enum FuseClientOp {
  OP_CREATE = 1,
  OP_OPEN = 2,
  OP_CLOSE = 3,
  OP_READ = 4,
  OP_WRITE = 5,
  OP_READLN = 6,
  OP_WRITLN = 7,
  OP_MAKDIR = 8,
  OP_CHGDIR = 9,
  OP_DELETE = 10,
  OP_SEEK = 11,
  OP_GETSTAT = 12,
  OP_SETSTAT = 13,
};

#endif // _FROB3_FUSE_H_
