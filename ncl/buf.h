#ifndef _FROBIO_NCL_BUF_H_
#define _FROBIO_NCL_BUF_H_

#define BUF_INITIAL_CAP 16

struct Buf {
  char *s;
  int n;
};

void BufCheck(struct Buf *p);
void BufInit(struct Buf *p);
void BufInitWith(struct Buf *p, const char *s);
void BufInitTake(struct Buf *p, char *s);
void BufDel(struct Buf *p);
char *BufFinish(struct Buf *p);
const char *BufTake(struct Buf *p);
const char *BufPeek(struct Buf *p);
void BufAppC(struct Buf *p, char c);
void BufAppS(struct Buf *p, const char *s, int n);
void BufAppElemC(struct Buf *p, char c);
void BufAppElemS(struct Buf *p, const char *s);
void BufAppDope(struct Buf *p, const char *s);
const char **BufTakeDope(struct Buf *p, int *lenP);
int ElemLen(const char *s, const char **endP);
const char *ElemDecode(const char *s);

#endif // _FROBIO_NCL_BUF_H_
