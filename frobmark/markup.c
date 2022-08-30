#include "frobio/nytypes.h"
#include "frobio/frobmark/markup.h"
#include "frobio/os9defs.h"
#include "frobio/nyformat.h"
#include "frobio/ncl/std.h"
#include "frobio/ncl/malloc.h"

bool StrIsWhite(const byte* s) {
    for (; *s; s++) {
        if (*s > ' ') return false;
    }
    return true;
}

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
  r->prev_line_empty = StrIsWhite(r->rbuf);

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

void EndParagraph(Rendering* r) {
        if (!r->prev_line_empty) {
          // Finish the current line:
          if (r->x) printIfNeededAndStartNewLine(r);
          // Emit an empty line.
          printIfNeededAndStartNewLine(r);
        }
}

void EmitLink(Rendering* r) {
  if (r->x) printIfNeededAndStartNewLine(r);
  ny_sprintf(r->rbuf, "LINK #%d <<%s>>", r->token);
  printIfNeededAndStartNewLine(r);
}

void FmRender(Rendering* r) {
  r->prev_line_empty = false;
  r->next_link_num = 2;
  r->x = r->y = 0;
  if (r->page) {
    // page counting starts on page 1.
    r->ybegin = (r->page - 1) * r->height;
    r->yend = r->ybegin + r->height;
  } else {
    // if page is 0, do not paginate.
    r->ybegin = 0;
    r->yend = 0xffff; // max word
  }

  error e;
  byte* bp = NULL;
  while (true) {
    if (bp) free(bp);
    bp = r->fetcher->readline(r->fetcher);

    if (!bp) break;
    byte* s = bp;

    if (StrIsWhite(s)) {
        // End-of-paragraph mark.
        EndParagraph(r);
        continue;
    }

    if (s[0]=='/' && 'A'<=Up(s[1]) && Up(s[1])<='Z') {
        // Beginning-of-line Slash Markups.
        s = NextToken(r, s+1);
        if (r->len && !strcasecmp((const char*)r->token, "L")) {
          s = NextToken(r, s+2);
          if (s) {
              EmitLink(r);  // using r->token & r->len
              // fall through for any remaining text.
          }
        }
        if (r->just_print_links) continue;
        // TODO: Other Slash commands....
    }
    if (r->just_print_links) continue;

    // Read remaining tokens and flow them into paragraph.
    while (true) {
        s = NextToken(r, s);
        if (!s) break;

        if (r->x + r->len  < r->width) {
            // It fits in the current rbuf.
            rspace(r);
            memcpy(r->rbuf+r->x, r->token, r->len);
            r->x += r->len;
        } else {
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
} // FmRender
