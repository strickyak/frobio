#ifndef _FROBIO_FROB2_ROMAPI3_H_
#define _FROBIO_FROB2_ROMAPI3_H_

#include "frob3/rom/bootrom3.h"

struct RomApi3 {
  #define ROM_API3_MAGIC 0x5233
  word api_RomApi3Magic;
  const struct sock* api_SockStructs;
  struct vars* api_VarsStruct;

  const struct proto* api_TcpProto;
  const struct proto* api_UdpProto;
  const struct proto* api_BroadcastUdpProto;

  #define restrict
  void* (*api_memcpy)(void *restrict dest, const void *restrict src, size_t n);
  void* (*api_memset)(void *s, int c, size_t n);
  char* (*api_strcpy)(char *restrict dest, const char *restrict src);
  char* (*api_strncpy)(char *restrict dest, const char *restrict src, size_t n);
  size_t (*api_strlen)(const char *s);
  #undef restrict

  word (*api_StackPointer)();
  char (*api_PolCat)();
  void (*api_Delay)(word n);

  void (*api_PutChar)(char ch);
  void (*api_PutStr)(const char* s);
  void (*api_PutHex)(word x);
  void (*api_PutDec)(word x);
  void (*api_Fatal)(const char* wut, word arg);
  void (*api_AssertEQ)(word a, word b);
  void (*api_AssertLE)(word a, word b);
  void (*api_PrintF)(const char* format, ...);

  byte (*api_WizGet1)(word reg);
  word (*api_WizGet2)(word reg);
  void (*api_WizGetN)(word reg, void* buffer, word size);
  void (*api_WizPut1)(word reg, byte value);
  void (*api_WizPut2)(word reg, word value);
  void (*api_WizPutN)(word reg, const void* data, word size);

  word (*api_WizTicks)();
  void (*api_WizReset)();
  void (*api_WizConfigure)();
  void (*api_WizIssueCommand)(PARAM_SOCK_AND byte cmd);
  void (*api_WizWaitStatus)(PARAM_SOCK_AND byte want);

  void (*api_WizOpen)(PARAM_SOCK_AND const struct proto* proto, word local_port );
  void (*api_TcpDial)(PARAM_SOCK_AND const byte* host, word port);
  void (*api_TcpEstablish)(PARAM_JUST_SOCK);
  errnum (*api_WizCheck)(PARAM_JUST_SOCK);
  errnum (*api_WizRecvChunkTry)(PARAM_SOCK_AND char* buf, size_t n);
  errnum (*api_WizRecvChunk)(PARAM_SOCK_AND char* buf, size_t n);
  errnum (*api_TcpRecv)(PARAM_SOCK_AND char* p, size_t n);
  void (*api_UdpDial)(PARAM_SOCK_AND  const struct proto *proto, const byte* dest_ip, word dest_port);
  void (*api_WizReserveToSend)(PARAM_SOCK_AND  size_t n);
  void (*api_WizDataToSend)(PARAM_SOCK_AND const char* data, size_t n);
  void (*api_WizFinalizeSend)(PARAM_SOCK_AND const struct proto *proto, size_t n);
  errnum (*api_WizSendChunk)(PARAM_SOCK_AND const struct proto* proto, char* data, size_t n);
  errnum (*api_TcpSend)(PARAM_SOCK_AND  char* p, size_t n);
  void (*api_WizClose)(PARAM_JUST_SOCK);
};

#endif // _FROBIO_FROB2_ROMAPI3_H_
