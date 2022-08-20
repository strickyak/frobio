#include "frobio/nytypes.h"
#include "frobio/frobmark/markup.h"
#include "frobio/os9defs.h"

static byte* NextToken(Rendering* r, byte* s) {
    // Skip white.
    while (*s && *s < ' ') s++;  // Skip white.
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
      printf("\n*** r->print_line: ERROR %d\n", e);
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
  printf("# page=%d ybegin=%d yend=%d\n", r->page, r->ybegin, r->yend);

  error e;
  while (true) {
    // TODO -- get_src_line should RETURN a str.
    e = r->get_src_line(r->inbuf, sizeof r->inbuf - 1);
    if (e == E_EOF) break;
    if (e) {
      printf("\n*** get_src_line: ERROR %d\n", e);
      exit(e);
    }

    byte* s = r->inbuf;
    while (true) {
        s = NextToken(r, s);
        if (!s) break;

        if (r->x + r->len  < r->width) {
            printf("(fits x=%d y=%d) ", r->x, r->y);
            // It fits in the current rbuf.
            rspace(r);
            memcpy(r->rbuf+r->x, r->token, r->len);
            r->x += r->len;
        } else {
            printf("(didnt x=%d y=%d) ", r->x, r->y);
            printIfNeededAndStartNewLine(r);
            // Now we will force the token to print,
            // even if we have to chop it up.
            for (word i = 0; i < r->len; i++) {
                rWrapPut(r, r->token[i]);
            }
        }  // endif it fits.
    }  // next token
  }  // next source line
  printIfNeededAndStartNewLine(r);
}
