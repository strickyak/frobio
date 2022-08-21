#ifndef FROBIO_NYFORMAT_h_
#define FROBIO_NYFORMAT_h_

#include "frobio/nytypes.h"
#include "frobio/ncl/buf.h"

// These all append to the Buf:

#if 0
// Put one char.
void BPutChar(Buf* buf, byte x);
// Put one str.
void BPutStr(Buf* buf, const char* s);
// Put one str with n.
void BPutStrN(Buf* buf, const char* s, byte n);
// Put a decimal digit.
void BPutDec(Buf* buf, byte x);
// Put an unsigned decimal word.
void BPutU(Buf* buf, word x);
// Put a signed decimal word.
void BPutI(Buf* buf, int x);
// Put a hex digit.
void BPutHex(Buf* buf, byte x);
// Put a hex word.
void BPutX(Buf* buf, word x);
// Put a char, curly encoded.
void BPutCurly(Buf* buf, byte c);
// Put a str with n, curly encoded.
void BEncodeCurly(Buf* buf, byte* str, int n);
#endif

void BufFormat(Buf* buf, const char* format, ... /*va_list arg*/);


#endif // FROBIO_NYFORMAT_h_
