// buf.c
//
// Buffers for accumulating strings and list items.

#include "frob3/froblib.h"

//chop

void BufCheck(Buf *p) {
  Assert (p->n >= 0);
  Assert (p->s != NULL);
#ifdef unix
  {}
#else
  Assert (p->s <= (char*)0xC000);
#endif
};

void BufInit(Buf *p) {
  p->s = (char*) Malloc(BUF_INITIAL_CAP);
  p->n = 0;
}

void BufInitWith(Buf *p, const char *s) {
  p->s = StrDup((const byte*)s);
  p->n = strlen(s);
}

void BufInitTake(Buf *p, char *s) {
  p->s = s;
  p->n = strlen(s);
}

void BufDel(Buf *p) {
  // OK to delete more than once, or after BufTake().
  Free(p->s);
  p->s = NULL;
  p->n = -1;
}

char *BufFinish(Buf *p) {
  BufCheck(p);
  p->s = (char*) ReAlloc(p->s, p->n + 1);
  p->s[p->n] = '\0';
  return p->s;
}

char *BufTake(Buf *p) {
  BufCheck(p);
  char *z = p->s;
  p->s = NULL;
  p->n = -1;
  return z;
}

const char *BufPeek(Buf *p) {
  return p->s;
}

void BufAppC(Buf *p, char c) {
  BufCheck(p);
  ++p->n;
  p->s = (char*) ReAlloc(p->s, p->n);
  p->s[p->n - 1] = c;
}

void BufAppS(Buf *p, const char *s, int n) {
  BufCheck(p);
  if (n < 0)
    n = strlen(s);
  p->s = (char*) ReAlloc(p->s, p->n + n);
  char *t = p->s + p->n;
  for (int i = 0; i < n; i++) {
    *t++ = *s++;
  }
  p->n += n;
}
void BufAppStr(Buf *p, const char *s) {
    BufAppS(p, s, strlen(s));
}

// Appending Elements.

void BufAppElemC(Buf *p, char c) {
  BufCheck(p);
  if (c <= ' ' || c > 'z' || c == '\\') {
    p->n += 2;
    p->s = (char*) ReAlloc(p->s, p->n);
    p->s[p->n - 2] = '\\';
    p->s[p->n - 1] = c;
  } else {
    p->n += 1;
    p->s = (char*) ReAlloc(p->s, p->n);
    p->s[p->n - 1] = c;
  }
}

void BufAppElemS(Buf *p, const char *s) {
  BufCheck(p);

  // Add space before next element, unless buf is empty.
  if (p->n)
    BufAppC(p, ' ');

  byte clean = true;
  for (const char *t = s; *t; t++) {
    if (*t <= ' ' || *t > 'z' || *t == '\\') {
      clean = false;
      break;
    }
  }
  if (clean && *s) {            // empty strings should not be clean.
    for (const char *t = s; *t; t++) {
      BufAppC(p, *t);
    }
  } else {
    BufAppC(p, '{');
    while (*s) {
      BufAppElemC(p, *s);
      s++;
    }
    BufAppC(p, '}');
  }
}


void BufAppDope(Buf *p, const char *s) {
  // (word) cast: Div optimizes to shift, if unsigned.
  int n = (word)p->n / sizeof(const char *);
  BufAppS(p, "        ", sizeof(const char *));
  ((const char **) p->s)[n] = s;
}

const char **BufTakeDope(Buf *p, int *lenP) {
  // (word) cast: Div optimizes to shift, if unsigned.
  *lenP = (word)p->n / sizeof(const char *);
  return (const char **) BufTake(p);
}

// Decoding list elements for Tcl / Picol / NCL.

// Return length of decoded element.
// Also the end of parsing.
int ElemLen(const char *s, const char **endP) {
  int n = 0;
  if (*s == '{') {
    // brace-wrapped element.
    s++;
    while (*s) {
      if (*s == '}') {
        s++;
        break;
      }
      if (*s == '\\') {
        ++s;                    // extra to jump over the backslash.
        ++n;
      }
      ++s;
      ++n;
    }
    // Warning: truncated element.
    *endP = s;
  } else {
    // bare element.
    const char *t = s;
    while (*t > ' ')
      ++t;
    *endP = t;
    n = t - s;
  }
  return n;
}

// Return decoded element.
const char *ElemDecode(const char *s) {
  Buf buf;
  BufInit(&buf);
  if (*s == '{') {
    // brace-wrapped element.
    s++;
    while (*s) {
      if (*s == '}') {
        s++;
        break;
      }
      if (*s == '\\') {
        ++s;                    // extra to jump over the backslash.
      }
      BufAppC(&buf, *s);
      ++s;
    }
    // Warning: truncated element.
  } else {
    // bare element.
    const char *t = s;
    while (*t > ' ')
      ++t;
    BufAppS(&buf, s, t - s);
  }
  BufFinish(&buf);
  return BufTake(&buf);
}
