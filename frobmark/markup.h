#ifndef _FROBIO_FROBMARK_MARKUP_H_
#define _FROBIO_FROBMARK_MARKUP_H_

#include "frobio/nytypes.h"
#include "frobio/frobmark/parseurl.h"
#include "frobio/frobmark/fetch.h"

//< typedef error (*getline_fn)(byte* buf, word max);
typedef error (*putline_fn)(byte* buf);
typedef error (*prompt_and_input_fn)(byte* buf, word max);

#define FM_INBUF_SIZE 201 // Lines up to 200 long.
#define FM_RBUF_SIZE 201 // Screens up to 200 wide.
typedef struct rendering {
    // Caller fills in these before each Render:
    word page;
    // Caller fills in these once, for configuration:
    word width;
    word height;
    Fetcher* fetcher;
    putline_fn print_line;
    prompt_and_input_fn prompt_and_input;

    // Private to the implementation:
    word x, y, ybegin, yend;
    byte* token;
    word len;
    //< byte inbuf[FM_INBUF_SIZE];
    byte rbuf[FM_RBUF_SIZE];
} Rendering;

void FmRender(Rendering* r);

#endif // _FROBIO_FROBMARK_MARKUP_H_
