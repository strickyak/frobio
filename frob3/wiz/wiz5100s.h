#ifndef _FROBIO_WIZ_WIZ5100S_H
#define _FROBIO_WIZ_WIZ5100S_H

#include "frob3/froblib.h"

extern bool wiz_verbose;
extern byte* WizHwPort;

// Short names for hardware ports depend on `WizHwPort`.
#define P0 WizHwPort[0] // control reg
#define P1 WizHwPort[1] // addr hi
#define P2 WizHwPort[2] // addr lo
#define P3 WizHwPort[3] // data

// Keep this at the default of 2K for each.
#define TX_SIZE 2048u
#define RX_SIZE 2048u
#define TX_SHIFT 11
#define RX_SHIFT 11
#define TX_MASK (TX_SIZE - 1)
#define RX_MASK (RX_SIZE - 1)
#define TX_BUF(N) (0x4000u + ((word)(N)<<TX_SHIFT))
#define RX_BUF(N) (0x6000u + ((word)(N)<<RX_SHIFT))

// Socket register offsets:
#define SockMode 0x00
#define SockCommand 0x01
#define SockInterrupt 0x02
#define SockStatus 0x03
#define SockSourcePort 0x04
#define SockDestIp 0x0C
#define SockDestPort 0x10
#define TxFreeSize 0x20
#define TxReadPtr 0x22
#define TxWritePtr 0x24
#define RxSize 0x26
#define RxReadPtr 0x28
#define RxWritePtr 0x2A

struct UdpRecvHeader {
    quad addr;
    word port;
    word len;
};
#endif // _FROBIO_WIZ_WIZ5100S_H
