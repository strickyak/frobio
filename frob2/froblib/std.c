#include "frob2/froblib.h"
#include "frob2/frobos9.h"

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

#if 0
void memcpy(void *d, const void *s, size_t sz) {
  char *a = (char *) d;
  const char *b = (const char *) s;
  int i;
  for (i = 0; i < sz; i++)
    *a++ = *b++;
}
#endif

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

void strcpy(char *d, const char *s) {
  char* p = d;
  while (*s) {
    *p++ = *s++;
  }
  *p = '\0';
}

#if 0
size_t strlen(const char *p) {
  const char *q = p;
  while (*q)
    q++;
  return q - p;
}
#endif

void strcat(char *d, const char *s) {
  char* p = d + strlen(d);
  strcpy(p, s);
}

void bzero(char *p, int n) {
  for (int i = 0; i < n; i++)
    p[i] = 0;
}

#if 0
void snprintf_s(char *buf, int max, const char *fmt, const char *s) {
  int flen = strlen(fmt);
  int slen = strlen(s);
  if (flen + slen - 1 > max) {  // drop '%s' but add '\0', so net minus 1.
    PutHex('f', flen);
    PutHex('s', slen);
    PutHex('m', max);
    Panic("buf overflow snprintf_s");
  }

  char *p = buf;
  while (*fmt) {
    if (fmt[0] == '%' && 'a' <= fmt[1] && fmt[1] <= 'z') {      // who cares what letter.
      fmt += 2;
      while (*s)
        *p++ = *s++;
      break;
    } else {
      *p++ = *fmt++;
    }
  }
  while (*fmt) {
    *p++ = *fmt++;
  }
  *p = '\0';
}

void snprintf_d(char *buf, int max, const char *fmt, int x) {
  char tmp[8];
  const char *z;

  if (x == 0) {
    z = "0";
  } else {
    byte neg = false;
    char *p = tmp + 7;
    *p-- = '\0';
    word y;
    if (x < 0) {
      neg = true;
      y = -x;
    } else {
      y = x;
    }
    while (y) {
      word r = y % 10;
      y = y / 10;
      *p-- = (byte) ('0' + r);
    }
    if (neg)
      *p-- = '-';
    z = p + 1;
  }

  snprintf_s(buf, max, fmt, z);
}
#endif

#if 0
static char p_buf[BUF_SIZE];
void printf_d(const char *fmt, int x) {
  snprintf_d(p_buf, BUF_SIZE, fmt, x);
  Os9_puts(p_buf);
}

void printf_s(const char *fmt, const char *s) {
  snprintf_s(p_buf, BUF_SIZE, fmt, s);
  Os9_puts(p_buf);
}
#endif

char *strdup(const char *s) {
  int n = strlen(s);
  char *p = (char *) Malloc(n + 1);
  strcpy(p, s);
  return p;
}

mstring StrDup(const byte* p) { return strdup((const char*)p); }

char *strndup(const char *s, int n) {
  char *p = (char *) Malloc(n + 1);
  strncpy(p, s, n+1);
  return p;
}
