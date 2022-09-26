#ifndef FROBIO_NYFORMAT_h_
#define FROBIO_NYFORMAT_h_

#include "frobio/nytypes.h"
#include "frobio/nystdio.h"
#include "frobio/ncl/buf.h"

void BufAppStringQuoting(Buf* buf, const char* s);
void BufFormat(Buf* buf, const char* format, ...);
int ny_printf(const char* fmt, ...);
int ny_eprintf(const char* fmt, ...);
int ny_fprintf(NY_FILE* f, const char* fmt, ...);
int ny_sprintf(char* dest, const char* fmt, ...);
char* StrFormat(const char* fmt, ...);
void FixNewlines(char* s, word n);  // did not work.

// Up(c): convert to upper case for 26 ascii letters.
char Up(char c);
// Down(c): convert to lower case for 26 ascii letters.
char Down(char c);

#endif // FROBIO_NYFORMAT_h_
