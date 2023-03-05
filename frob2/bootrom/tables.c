#include "frob2/bootrom/bootrom.h"

const byte HexAlphabet[] = "0123456789abcdef";

#if MULTI_SOCK
// If multiple sockets are supported,
// this table has the relevant facts about each.

// Sorry about the awkward construction of the SockState location,
// but I had to make it a constant integer.
const struct sock Socks[4] = {
    { 0x400, 0x4000, 0x6000, VARS_RAM+sizeof(struct vars)+sizeof(struct sock)*0, 0 },
    { 0x500, 0x4800, 0x6800, VARS_RAM+sizeof(struct vars)+sizeof(struct sock)*1, 1 },
    { 0x600, 0x5000, 0x7000, VARS_RAM+sizeof(struct vars)+sizeof(struct sock)*2, 2 },
    { 0x700, 0x5800, 0x7800, VARS_RAM+sizeof(struct vars)+sizeof(struct sock)*3, 3 },
};
#endif

#if X220NET
// For my coco3(10.1.2.3) with ethernet(255.0.0.0) wired to a laptop(10.2.2.2).
const byte BR_ADDR    [4] = { 10, 1, 2, 3 };
const byte BR_MASK    [4] = { 255, 0, 0, 0 };
const byte BR_GATEWAY [4] = { 10, 2, 2, 2 };
const byte BR_WAITER  [4] = { 10, 2, 2, 2 };
const byte BR_RESOLV  [4] = { 8, 8, 8, 8 };
#endif

#if LOCALNET
// For my emulator, which opens a port for TFTP services on localhost.
const byte BR_ADDR    [4] = { 127, 0, 0, 1 };
const byte BR_MASK    [4] = { 255, 0, 0, 0 };
const byte BR_GATEWAY [4] = { 127, 0, 0, 1 };
const byte BR_WAITER  [4] = { 127, 0, 0, 1 };
const byte BR_RESOLV  [4] = { 127, 0, 0, 1 };
#endif

// END
