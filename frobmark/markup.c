#include "frobio/frobmark/markup.h"
#include "frobio/os9defs.h"

#ifndef true
#define true 1
#define false 0
#endif

static byte buf1[FM_BUFSIZE];

#define RBUF_SIZE 201 // Screens up to 200 wide.
typedef struct rendering {
    word x, y, ybegin, yend;
    word width;
    word height;
    word page;
    getline_fn get_src_line;
    putline_fn print_line;
    prompt_and_input_fn prompt_and_input;

    byte* token;
    word len;
    byte rbuf[RBUF_SIZE];
} Rendering;

// rend could be allocated in FMRender to make it re-entrant.
Rendering rend;

static byte* NextToken(byte* s, byte** token_out, word *len_out) {
    // Skip white.
    while (*s) {
        if (*s>' ') break;
        s++;
    }
    if (!*s) return NULL;  // Line ended.

    byte* begin = s;  // Start of string.
    while (*s) {
        if (*s<=' ') {
            break;  // found white.
        }
        s++;
    }
    *len_out = s - begin;
    *token_out = begin;
    return s; // Where to start next time.
}

static void DumpToken(byte* s, word len) {
    printf(" Tok<");
    for (word i = 0; i < len; i++) {
        byte ch = s[i];
        if (ch < ' ' || ch > '~') ch = '~';
        putchar(ch);
    }
    printf(">%d ", len);
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

void Rend(Rendering* r) {
  printf("page=%d ybegin=%d yend=%d\n", r->page, r->ybegin, r->yend);

  error e;
  while (true) {
    e = r->get_src_line(buf1, FM_BUFSIZE-1);
    if (e == E_EOF) break;
    if (e) {
      printf("\n*** get_src_line: ERROR %d\n", e);
      exit(e);
    }

    byte* s = buf1;
    while (true) {
        s = NextToken(s, &r->token, &r->len);
        if (!s) break;
        //# DumpToken(r->token, r->len);

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

void FmRender(
    word width, word height, word page,
    getline_fn get_src_line,
    putline_fn print_line,
    prompt_and_input_fn prompt_and_input) {
  // r could be malloc'd in FMRender to make it re-entrant.
  Rendering* r = &rend;

  r->x = r->y = 0;
  r->ybegin = page * height;
  r->yend = r->ybegin + height;
  r->width = width;
  r->height = height;
  r->page = page;
  r->get_src_line = get_src_line;
  r->print_line = print_line;
  r->prompt_and_input = prompt_and_input;

  Rend(r);
}
