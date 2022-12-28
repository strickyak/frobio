#ifndef _FROB2_FROBNET_H_
#define _FROB2_FROBNET_H_

#include "frob2/froblib.h"

extern byte* WizHwPort;  // Hardware Port.

// First call WizReset, then call WizConfigure.
void WizReset();
void WizConfigure(quad ip_addr, quad ip_mask, quad ip_gateway);

// For DHCP, use these instead of WizConfigure().
// They require a 4-letter name, unique to this DHCP client.
void wiz_configure_for_DHCP(const char* name4, byte* hw6_out);
void WizReconfigureForDhcp(quad ip_addr, quad ip_mask, quad ip_gateway);

// Non-Socket commands return OKAY or errnum.
errnum WizArp(quad dest_ip, byte* mac_out);
errnum WizPing(quad dest_ip);
word WizTicks();
word suggest_client_port();
byte find_available_socket(word* base_out);

// Socket commands return OKAY or errnum.
errnum UdpOpen(word src_port, byte* socknum_out);
errnum UdpSend(byte socknum, byte* payload, word size, quad dest_ip, word dest_port);
errnum UdpRecv(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out);
errnum UdpClose(byte socknum);

errnum MacRawOpen(byte* socknum_out);
errnum MacRawRecv(byte socknum, byte* buffer, word* size_in_out);
errnum MacRawClose(byte socknum);

// Extra utilities.
void WizDelay(word n);
word WizTicks();  // 0.1ms but may have readbyte,readbyte error?
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


// USED IN TCP -- TODO move to froblib.
// Error Compromise:
//   Use literal const char* for errors.
//   Use NotYet to mean try again later, because an asynchronous event hasn't happened yet.
//   Use GOOD (or NULL) for good status.
// To communicate better error messages, application can LogError.
typedef const char* prob;
#define GOOD ((const char*)NULL)
extern const char NotYet[]; // defined as "NotYet"

prob tcp_open(byte* socknum_out);
prob tcp_connect(byte socknum, quad host, word port);
prob tcp_listen(byte socknum, word listen_port);
prob tcp_establish_or_not_yet(byte socknum); // may return NotYet
prob tcp_recv_or_not_yet(byte socknum, char* buf, size_t buflen, size_t *num_bytes_out);
prob tcp_send_or_not_yet(byte socknum, const char* buf, size_t num_bytes_to_send);
prob tcp_establish_blocking(byte socknum); // may return NotYet
prob tcp_recv_blocking(byte socknum, char* buf, size_t buflen, size_t *num_bytes_out);
prob tcp_send_blocking(byte socknum, const char* buf, size_t num_bytes_to_send);
prob tcp_close(byte socknum);

#endif // _FROB2_FROBNET_H_
