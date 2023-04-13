#include "frob2/bootrom/bootrom3.h"
#include "frob2/bootrom/romapi3.h"

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

    ConfigureTextScreen,
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
