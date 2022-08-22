#ifndef FROBIO_NYFORMAT_h_
#define FROBIO_NYFORMAT_h_

#include "frobio/nytypes.h"
#include "frobio/nystdio.h"
#include "frobio/ncl/buf.h"

void BufAppStringQuoting(Buf* buf, const char* s);
void BufFormat(Buf* buf, const char* format, ...);
int ny_printf(const char* fmt, ...);
int ny_eprintf(const char* fmt, ...);
int ny_fprintf(FILE* f, const char* fmt, ...);
int ny_sprintf(char* dest, const char* fmt, ...);

#endif // FROBIO_NYFORMAT_h_
