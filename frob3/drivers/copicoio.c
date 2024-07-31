// copicoio.c

#define true (bool)1
#define false (bool)0
#define OKAY (errnum)0
#define NOTYET (errnum)1
#define NULL ((void*)0)

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned char errnum;
typedef unsigned int word;
typedef unsigned int size_t;
typedef void (*func_t)();

#define PEEK1(A) (*(volatile byte*)(A))
#define PEEK2(A) (*(volatile word*)(A))
#define POKE1(A,X) (*(volatile byte*)(A) = (X))
#define POKE2(A,X) (*(volatile word*)(A) = (X))

#define N4_CHUNK_SIZE 100

#define N4_CONTROL 0xFF78
#define N4_STATUS 0xFF79
#define N4_TX 0xFF7A
#define N4_RX 0xFF7B
#define N4_MID_TX_BIAS N4_CHUNK_SIZE
#define N4_MID_RX_BIAS 0
#define N4_STATUS_GOOD 2

#define WAIT_ON_STATUS_REG()  { byte status; do { status = PEEK1(N4_STATUS); } while (status == 0); }

// N4 Send and Recv for small n ( 1 <= n <= 100 ).

#ifndef RECV_ONLY
void N4SendSmall(char* buf, word n) {
    // Unfortunately, in gcc6809, "byte n" produces much more code.
    POKE1(N4_CONTROL, n+N4_MID_RX_BIAS);
    WAIT_ON_STATUS_REG();  // After control.

    do {
        POKE1(N4_RX, *buf++);
        WAIT_ON_STATUS_REG();  // After Rx.
        n--;
    } while (n);
}
#endif

void N4RecvSmall(char* buf, word n) {
    // Unfortunately, in gcc6809, "byte n" produces much more code.
    POKE1(N4_CONTROL, n+N4_MID_TX_BIAS);
    WAIT_ON_STATUS_REG();  // After control.

    do {
        WAIT_ON_STATUS_REG(); // Before Tx.
        *buf++ = PEEK1(N4_TX);
        n--;
    } while (n);
}

// N4 Send and Recv based on N4 mid-level.

#ifndef RECV_ONLY
void N4Send(char* p, size_t n) {
  do {
    word chunk = (n < N4_CHUNK_SIZE) ? n : N4_CHUNK_SIZE;
    N4SendSmall(p, chunk);
    p += chunk;
    n -= chunk;
  } while (n);
}
#endif

void N4Recv(char* p, size_t n) {
  do {
    word chunk = (n < N4_CHUNK_SIZE) ? n : N4_CHUNK_SIZE;
    N4RecvSmall(p, chunk);
    p += chunk;
    n -= chunk;
  } while (n);
}
