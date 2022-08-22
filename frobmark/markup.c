#include "frobio/nytypes.h"
#include "frobio/frobmark/markup.h"
#include "frobio/os9defs.h"
#include "frobio/nyformat.h"

static byte* NextToken(Rendering* r, byte* s) {
    // Skip white.
    while (*s && *s <= ' ') s++;  // Skip white.
    if (!*s) return NULL;  // Line ended.

    r->token = s;  // Start of token.
    while (*s > ' ') s++;  // Skip nonwhite.
    r->len = s - r->token;
    return s; // Where to start next time.
}

static void printIfNeededAndStartNewLine(Rendering* r) {
  // Printing is only needed when y is in ybegin to yend.
  if (r->ybegin <= r->y && r->y < r->yend) {
    // Y is in the range we want to print.
    r->rbuf[r->x] = '\0';  // Terminate rbuf.

    error e = r->print_line(r->rbuf);
    if (e) {
      ny_eprintf("\n*** r->print_line: ERROR %d\n", e);
      exit(e);
    }
  }
  // Start new line.
  ++r->y;
  r->x = 0;
}
static void rPut(Rendering* r, byte ch) {
  r->rbuf[r->x] = ch;
  r->x++;
  if (r->x == r->width) printIfNeededAndStartNewLine(r);
}
static void rWrapPut(Rendering* r, byte ch) {
  if (r->x+1 >= r->width) {
    // Will not fit, so append \ and go to next line.
    r->rbuf[r->x] = '\\';
    printIfNeededAndStartNewLine(r);
    ++r->y;
    r->x = 0;
  }
  r->rbuf[r->x] = ch;
  r->x++;
}
static void rspace(Rendering* r) {
  // Add space unless we are at start of line.
  if (r->x) rPut(r, ' ');
}

void FmRender(Rendering* r) {
  r->x = r->y = 0;
  r->ybegin = r->page * r->height;
  r->yend = r->ybegin + r->height;
  ny_eprintf("# page=%d ybegin=%d yend=%d\n", r->page, r->ybegin, r->yend);

  error e;
  while (true) {
ny_eprintf(" .%d. ", __LINE__);
    byte* bp = r->fetcher->readline(r->fetcher);
ny_eprintf(" .%d. ", __LINE__);
    ny_eprintf(" --r->fetcher->readline: %q\n", bp);
    //< e = r->get_src_line(r->inbuf, sizeof r->inbuf - 1);
    if (!bp) break;
    byte* s = bp;
    while (true) {
        s = NextToken(r, s);
        ny_eprintf("NextToken->%x %d %q\n", (long)s, r->len, r->token);
ny_eprintf(" .%d. ", __LINE__);
        if (!s) break;
ny_eprintf(" .%d. ", __LINE__);

        if (r->x + r->len  < r->width) {
ny_eprintf(" .%d. ", __LINE__);
            ny_eprintf("(fits x=%d y=%d) ", r->x, r->y);
            // It fits in the current rbuf.
            rspace(r);
            memcpy(r->rbuf+r->x, r->token, r->len);
            r->x += r->len;
        } else {
ny_eprintf(" .%d. ", __LINE__);
            ny_eprintf("(didnt x=%d y=%d) ", r->x, r->y);
            printIfNeededAndStartNewLine(r);
            // Now we will force the token to print,
            // even if we have to chop it up.
            for (word i = 0; i < r->len; i++) {
                rWrapPut(r, r->token[i]);
            }
        }  // endif it fits.
    }  // next token
  }  // next source line
ny_eprintf(" .%d. ", __LINE__);
  printIfNeededAndStartNewLine(r);
ny_eprintf(" .%d. ", __LINE__);
}
