#ifndef _FROBIO_FROBMARK_MARKUP_H_
#define _FROBIO_FROBMARK_MARKUP_H_

#include "frob2/froblib.h"

#include "frob2/tweb/fetch.h"
#include "frob2/tweb/parseurl.h"

typedef errnum (*putline_fn)(byte* buf);
typedef errnum (*prompt_and_input_fn)(byte* buf, word max);

#define FM_RBUF_SIZE 101 // Screens up to 100 wide.
typedef struct rendering {
    // Caller fills in these before each Render:
    word page;
    bool just_print_links;
    Fetcher* fetcher;
    // Caller fills in these once, for configuration:
    word width;
    word height;
    putline_fn print_line;
    prompt_and_input_fn prompt_and_input;

    // Private to the implementation:
    word x, y, ybegin, yend;
    byte* token;
    word len;
    bool prev_line_empty;
    word link_num;
    byte rbuf[FM_RBUF_SIZE];
} Rendering;

void FmRender(Rendering* r);

#endif // _FROBIO_FROBMARK_MARKUP_H_
