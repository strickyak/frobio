#include "frob3/froblib.h"
#include "frob3/frobos9.h"

//chop

// prefixed_atoi() understands initial "0" for octal and "0x" for hex, and '-' before any of that.
int prefixed_atoi(const char *s) {
  int z = 0;
  byte neg = false;
  if (*s == '-') {
    neg = 1;
    s++;
  }
  if (*s == '0') {
    s++;
    if (*s == 'x') {
      // hex if starts 0x
      while ('0' <= *s && *s <= '9' || 'A' <= CharUp(*s) && CharUp(*s) <= 'F') {
        if ('0' <= *s && *s <= '9') {
          z = z * 16 + (*s - '0');
        } else {
          z = z * 16 + (CharUp(*s) + 10 - 'A');
        }
        s++;
      }
    } else {
      // octal if starts 0
      while ('0' <= *s && *s <= '7') {
        z = z * 8 + (*s - '0');
        s++;
      }
    }
  } else {
    // else decimal
    while ('0' <= *s && *s <= '9') {
      z = z * 10 + (*s - '0');
      s++;
    }
  }
  return neg ? -z : z;
}

int strcasecmp(const char *a, const char *b) {
  while (*a && *b) {
    if ((byte) CharUp(*a) < (byte) CharUp(*b))
      return -1;
    if ((byte) CharUp(*a) > (byte) CharUp(*b))
      return +1;
    a++;
    b++;
  }
  // at least one is 0.
  if (*a)
    return -1;
  if (*b)
    return +1;
  return 0;
}

char *strdup(const char *s) {
  int n = strlen(s);
  char *p = (char *) Malloc(n + 1);
  strcpy(p, s);
  return p;
}

mstring StrDup(const byte* p) {
    return strdup((const char*)p);
}

char *strndup(const char *s, size_t n) {
  char *p = (char *) Malloc(n + 1);
  strncpy(p, s, n+1);
  return p;
}
