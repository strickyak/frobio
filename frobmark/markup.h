#ifndef _FROBIO_FROBMARK_MARKUP_H_
#define _FROBIO_FROBMARK_MARKUP_H_

#include "frobio/nytypes.h"

#define FM_BUFSIZE 300

typedef error (*getline_fn)(byte* buf, word max);
typedef error (*putline_fn)(byte* buf);
typedef error (*prompt_and_input_fn)(byte* buf, word max);

void FmRender(
    word width, word height, word page,
    getline_fn get_src_line,
    putline_fn print_line,
    prompt_and_input_fn prompt_and_input);
    



#endif // _FROBIO_FROBMARK_MARKUP_H_
