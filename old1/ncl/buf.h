#ifndef _FROBIO_NCL_BUF_H_
#define _FROBIO_NCL_BUF_H_

#define BUF_INITIAL_CAP 16

typedef struct Buf {
  char *s;
  int n;
} Buf;

void BufCheck(Buf *buf);
void BufInit(Buf *buf);
void BufInitWith(Buf *buf, const char *s);
void BufInitTake(Buf *buf, char *s);
void BufDel(Buf *buf);
char *BufFinish(Buf *buf);
char *BufTake(Buf *buf);
const char *BufPeek(Buf *buf);
void BufAppC(Buf *buf, char c);
void BufAppS(Buf *buf, const char *s, int n);
void BufAppElemC(Buf *buf, char c);
void BufAppElemS(Buf *buf, const char *s);
void BufAppDope(Buf *buf, const char *s);
const char **BufTakeDope(Buf *buf, int *lenP);

// For NCL list elements.
int ElemLen(const char *s, const char **endP);
const char *ElemDecode(const char *s);

#endif // _FROBIO_NCL_BUF_H_
