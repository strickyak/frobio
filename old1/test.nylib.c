#include "frobio/nylib.h"

main() {
  {
    const char* s = "123.231.58.9";
    quad q = NyParseDottedDecimalQuad(&s);
    quad r = NyFormQuadFromBytes(123, 231, 58, 9);

    quad t = ((quad)123 << 24) |
             ((quad)231 << 16) |
             ((quad) 58 <<  8) |
             ((quad)  9 <<  0);

    printf("test_nylib: %08lx %08lx %08lx\n", q, r, t);
    assert(q == r);
    assert(q == t);
    assert(r == t);
  }

  {
    word port = 80;
    const char* s = "123.231.58.9";
    quad q = NyParseDottedDecimalQuadAndPort(&s, &port);
    quad r = NyFormQuadFromBytes(123, 231, 58, 9);

    quad t = ((quad)123 << 24) |
             ((quad)231 << 16) |
             ((quad) 58 <<  8) |
             ((quad)  9 <<  0);

    printf("test_nylib: %08lx %08lx %08lx\n", q, r, t);
    assert(q == r);
    assert(q == t);
    assert(r == t);
    assert(port == 80);
  }

  {
    word port = 80;
    const char* s = "123.231.58.9:8080";
    quad q = NyParseDottedDecimalQuadAndPort(&s, &port);
    quad r = NyFormQuadFromBytes(123, 231, 58, 9);

    quad t = ((quad)123 << 24) |
             ((quad)231 << 16) |
             ((quad) 58 <<  8) |
             ((quad)  9 <<  0);

    printf("test_nylib: %08lx %08lx %08lx\n", q, r, t);
    assert(q == r);
    assert(q == t);
    assert(r == t);
    assert(port == 8080);
  }

  {
    const char* s = "0.1.2.0";
    quad q = NyParseDottedDecimalQuad(&s);
    quad r = NyFormQuadFromBytes(0, 1, 2, 0);
    quad t = (quad)0x0102 << 8;
    printf("test_nylib: %08lx %08lx %08lx\n", q, r, t);
    assert(q == r);
    assert(q == t);
    assert(r == t);
  }

  {
    char* words[5] = {NULL, NULL, NULL, NULL, NULL};
    char* s = " one \n two\tthree\r\t\n";
    int n = NySplit(s, &words, 5);
    printf("NySplit -> %d\n", n);
    assert(n == 3);
    assert(NyStrEq(words[0], "one"));
    assert(NyStrEq(words[1], "two"));
    assert(NyStrEq(words[2], "three"));
    assert(words[3] == NULL);
  }

  {
    char* words[3] = {NULL, NULL, "unchanged"};
    char* s = " one \n two\tthree\rxxx";
    int n = NySplit(s, &words, 2);
    printf("NySplit -> %d\n", n);
    assert(n == 2);
    assert(NyStrEq(words[0], "one"));
    assert(NyStrEq(words[1], "two"));
    assert(NyStrEq(words[2], "unchanged"));
  }


  printf("test_nylib: OKAY\n");
  return 0;
}
