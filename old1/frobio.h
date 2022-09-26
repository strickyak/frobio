#ifndef _FROBIO_FROBIO_H_
#define _FROBIO_FROBIO_H_

#include "frobio/nytypes.h"

extern bool wiz_verbose;
extern byte* wiz_hwport;

// First call wiz_reset, then call wiz_configure.
void wiz_reset();
void wiz_configure(quad ip_addr, quad ip_mask, quad ip_gateway);

// For DHCP, use these instead of wiz_configure().
// They require a 4-letter name, unique to this DHCP client.
void wiz_configure_for_DHCP(const char* name4, byte* hw6_out);
void wiz_reconfigure_for_DHCP(quad ip_addr, quad ip_mask, quad ip_gateway);

// Non-Socket commands return OKAY or error.
error wiz_arp(quad dest_ip, byte* mac_out);
error wiz_ping(quad dest_ip);
word wiz_ticks();
word suggest_client_port();
byte find_available_socket(word* base_out);

// Socket commands return OKAY or error.
error udp_open(word src_port, byte* socknum_out);
error udp_send(byte socknum, byte* payload, word size, quad dest_ip, word dest_port);
error udp_recv(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out);
error udp_close(byte socknum);

error macraw_open(byte* socknum_out);
error macraw_recv(byte socknum, byte* buffer, word* size_in_out);
error macraw_close(byte socknum);

// Extra utilities.
void wiz_delay(word n);
word wiz_ticks();  // 0.1ms but may have readbyte,readbyte error?
void sock_show(byte socknum);

/* On packet sizes:
Proposing 600 bytes for buffers.  Rationale:

MAX IPv4 Datagram Size = 576 bytes.
For TCP, - 20(tcp) - 20(ip) = 536 MSS (maximum TCP segment size).
For UDP, -  8(udp) - 20(ip) = 556 max UDP payload.

576(ip) + 6(src) + 6(dest) + 2(len) = 590 max ethernet (without crc).
590(ethernet) + 2(len) = 592 (macraw buffer).

So that leaves 8 bytes spare, in a macraw buffer.

BTW, TFTP uses UDP payload of 2(opcode) + 2(block) + 512(data) = 516 bytes.

How many 600 byte buffers in an 8K page?  13.65 buffers.
*/

#endif // _FROBIO_FROBIO_H_
