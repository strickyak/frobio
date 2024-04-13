#include "frob2/froblib.h"
#include "frob2/tweb/markup.h"

bool StrIsWhite(const byte* s) {
    for (; *s; s++) {
        if (*s > ' ') return false;
    }
    return true;
}

static byte* GetNextToken(Rendering* r, byte* s) {
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

    errnum e = r->print_line(r->rbuf);
    if (e) LogFatal("\n*** r->print_line: ERROR %d\n", e);
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

static void EndParagraph(Rendering* r) {
        if (!r->prev_line_empty) {
          // Finish the current line:
          if (r->x) printIfNeededAndStartNewLine(r);
          // Emit an empty line.
          printIfNeededAndStartNewLine(r);
        }
}

static void EmitWord(Rendering* r, const byte* tok, word len) {
        if (r->x + len  < r->width) {
            // It fits in the current rbuf.
            rspace(r);
            memcpy(r->rbuf+r->x, tok, len);
            r->x += len;
        } else {
            printIfNeededAndStartNewLine(r);
            // Now we will force the tok to print,
            // even if we have to chop it up.
            for (word i = 0; i < len; i++) {
                rWrapPut(r, tok[i]);
            }
        }  // endif it fits.
}

static void EmitLink(Rendering* r) {
  ++ r->link_num;

  char*s = StrFormat("[%d]", r->link_num);
  EmitWord(r, s, strlen(s));
  Free(s);

  EmitWord(r, (const byte*)r->token, r->len);
}

mstring FmGetNthLink(Rendering* r, word n) {
  int link_counter=0;
  char* ptr = NULL;
  while (true) {
    if (ptr) Free(ptr);
    ptr = r->fetcher->readline(r->fetcher);

    if (!ptr) break;
    // s will advance, ptr will not.  ptr is needed for Free().
    byte* s = (byte*) ptr;
    byte c0 = s[0];
    byte cu1 = CharUp(s[1]);

    // Is it a "/L ..." link?
    if (c0=='/' && cu1=='L') {
          s = GetNextToken(r, s+2);  // next token is URL
          if (s) {
            ++link_counter;
            return strndup((const char*)r->token, r->len);
          }
    }
  }
  return NULL;
}

void EmitRemainingTokens(Rendering* r, const byte* s) {
    while (true) {
        s = GetNextToken(r, (byte*)s);
        if (!s) break;
        EmitWord(r, r->token, r->len);
    }  // next token
}

void FmRender(Rendering* r) {
  r->prev_line_empty = false;
  r->link_num = 0;
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

  char* ptr = NULL;
  while (true) {
    if (ptr) Free(ptr);
    ptr = r->fetcher->readline(r->fetcher);

    if (!ptr) break;
    byte* s = (byte*) ptr;
    byte c0 = s[0];
    byte cu1 = CharUp(s[1]);

    // Is it a "/L ..." link?
    if (c0=='/' && cu1=='L') {
          s = GetNextToken(r, s+2);  // next token is URL
          if (s) {
              EmitLink(r);  // using r->token & r->len
              // fall through for any remaining text.
          } else {
            // Un-see the "/L".
            s = (byte*)ptr;
          }
    } else {
        if (r->just_print_links) continue;
    }

    if (StrIsWhite(s)) {
        // End-of-paragraph mark.
        EndParagraph(r);
        continue;
    }

    // Read remaining tokens and flow them into paragraph.
    EmitRemainingTokens(r, s);
  }  // next source line
  printIfNeededAndStartNewLine(r);
} // FmRender
