#include "frob2/bootrom/bootrom3.h"
#include "frob2/bootrom/romapi3.h"

void WizOpen(PARAM_SOCK_AND const struct proto* proto, word local_port );
void TcpDial(PARAM_SOCK_AND const byte* host, word port);
void TcpEstablish(PARAM_JUST_SOCK);
void WizCheck(PARAM_JUST_SOCK);
bool WizRecvChunkTry(PARAM_SOCK_AND char* buf, size_t n);
void WizRecvChunk(PARAM_SOCK_AND char* buf, size_t n);
void TcpRecv(PARAM_SOCK_AND char* p, size_t n);
void UdpDial(PARAM_SOCK_AND  const struct proto *proto,
             const byte* dest_ip, word dest_port);
void WizReserveToSend(PARAM_SOCK_AND  size_t n);
void WizDataToSend(PARAM_SOCK_AND char* data, size_t n);
void WizFinalizeSend(PARAM_SOCK_AND const struct proto *proto, size_t n);
void WizSendChunk(PARAM_SOCK_AND const struct proto* proto, char* data, size_t n);
void TcpSend(PARAM_SOCK_AND  char* p, size_t n);
void WizClose(PARAM_JUST_SOCK);

struct RomApi3 RomApi = {
    ROM_API3_MAGIC,

    &Socks[0],
    Vars,
    RevDate,
    RevTime,

    &TcpProto,
    &UdpProto,
    &BroadcastUdpProto,

    memcpy,
    memset,
    strcpy,
    strncpy,
    strlen,

    StackPointer,
    PolCat,
    Delay,
    PutChar,
    PutStr,
    PutHex,
    PutDec,
    Fatal,
    AssertEQ,
    AssertLE,
    PrintF,

    WizGet1,
    WizGet2,
    WizGetN,
    WizPut1,
    WizPut2,
    WizPutN,

    WizTicks,
    WizReset,
    WizConfigure,
    WizIssueCommand,
    WizWaitStatus,

    //

    WizOpen,
    TcpDial,
    TcpEstablish,
    WizCheck,
    WizRecvChunkTry,
    WizRecvChunk,
    TcpRecv,
    UdpDial,
    WizReserveToSend,
    WizDataToSend,
    WizFinalizeSend,
    WizSendChunk,
    TcpSend,
    WizClose,

};
