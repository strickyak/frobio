#ifndef _FROBIO_FROBIO_H_
#define _FROBIO_FROBIO_H_

#include "frobio/nytypes.h"


#define IP4ADDR(A,B,C,D) (((quad)((A)&255) << 24) | ((quad)((B)&255) << 16) | ((quad)((C)&255) << 8) | (quad)((D)&255) )

struct FrobioConfig {
    quad ip_addr;
    quad ip_mask;
    quad ip_gateway;
    byte ether_mac[6];
};

// Set this true for spammy verbosity.
extern bool wiz_verbose;

// First call wiz_reset, then call wiz_configure.
void wiz_reset(word wiz_ioport);
void wiz_configure(struct FrobioConfig* cf);

// Non-Socket commands return OKAY or error.
error wiz_arp(quad dest_ip);
error wiz_ping(quad dest_ip);

// Socket commands return OKAY or error.
error udp_open(byte socknum, word src_port);
error udp_send(byte socknum, byte* payload, word size, quad dest_ip, word dest_port);
error udp_recv(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out);
error udp_close(byte socknum);

// Extra utilities.
void wiz_delay(int n);
void sock_show(byte socknum);

#endif // _FROBIO_FROBIO_H_
