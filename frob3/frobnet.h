#ifndef _FROB3_FROBNET_H_
#define _FROB3_FROBNET_H_

#include "frob3/froblib.h"

extern byte* WizHwPort;  // Hardware Port.

// First call WizReset, then call WizConfigure.
void WizReset();
void WizConfigure(quad ip_addr, quad ip_mask, quad ip_gateway);

// For DHCP, use these instead of WizConfigure().
// They require a 4-letter name, unique to this DHCP client.
void wiz_configure_for_DHCP(const char* name4, byte* hw6_out);
void WizReconfigureForDhcp(quad ip_addr, quad ip_mask, quad ip_gateway);

// Non-Socket commands return GOOD or prob.
prob WizArp(quad dest_ip, byte* mac_out);
prob WizPing(quad dest_ip);
word WizTicks();
word suggest_client_port();
byte find_available_socket(word* base_out);

// Socket commands return GOOD or prob.
prob UdpOpen(word src_port, byte* socknum_out);

prob UdpSend(byte socknum, byte* payload, word size, quad dest_ip, word dest_port);
prob UdpRecv(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out);
prob UdpRecvOrNotYet(byte socknum, byte* payload, word* size_in_out, quad* from_addr_out, word* from_port_out);

// TODO:
prob UdpSendAndRecvOrRepeat(byte socknum, word millis_until_retry, word num_tries,
    byte* send_payload, word send_size, quad send_addr, word send_port,
    byte* recv_payload, word* recv_size_in_out, quad* recv_addr_out, word* recv_port_out);

prob UdpClose(byte socknum);

prob MacRawOpen(byte* socknum_out);
prob MacRawRecv(byte socknum, byte* buffer, word* size_in_out);
prob MacRawClose(byte socknum);

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


prob TcpOpen(byte* socknum_out);
prob TcpDial(byte socknum, quad host, word port);
prob TcpListen(byte socknum, word listen_port);
prob TcpEstablishOrNotYet(byte socknum); // may return NotYet
prob TcpRecvOrNotYet(byte socknum, char* buf, size_t buflen, size_t *num_bytes_out);
prob TcpSendOrNotYet(byte socknum, const char* buf, size_t num_bytes_to_send);
prob TcpEstablishBlocking(byte socknum); // may return NotYet
prob TcpRecvBlocking(byte socknum, char* buf, size_t buflen, size_t *num_bytes_out);
prob TcpSendBlocking(byte socknum, const char* buf, size_t num_bytes_to_send);
prob TcpClose(byte socknum);

#endif // _FROB3_FROBNET_H_
