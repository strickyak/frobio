// f.say [-w wizport] [-vN] message...
//
// Sends the message... as a LOG message to the waiter server,
// prefixed with "(SAY) ".

#define LEMMA_SOCK_NUM 1
#define MAX_MSG_LEN 255

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"

// buf: 5 for quint header, 6 for "(SAY) ", then the message.
char buf[5 + 6 + MAX_MSG_LEN];

void Send(const char* p, size_t n) {
  prob err;
  do {
    err = TcpSendOrNotYet(LEMMA_SOCK_NUM, p, n);
  } while (err == NotYet);
  if (err) LogFatal("cannot TcpSend: %s", err);
}

int main(int argc, char* argv[]) {
  word localport = 0;
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "v:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
      case 'v':
         Verbosity = (byte)prefixed_atoi(FlagArg);
         break;
      case 'w':
         WizHwPort = (byte*)prefixed_atoi(FlagArg);
         break;
      default:
         LogFatal("f.say [-w wizport] [-vN] anything...");
    }
  }

  word msg_len = 0;
  for (word i = 0; i < argc; i++) {
    msg_len += strlen(argv[i]);
    if (i < argc-1) msg_len++;
  }
  if (msg_len > MAX_MSG_LEN) {
    LogFatal("say: message too long: %d bytes", msg_len); 
  }

  word pay_len = 6 + msg_len; // length after quint: 6 for "(SAY) "

  buf[0] = 200; // CMD_LOG
  *(word*)(buf+1) = pay_len;
  *(word*)(buf+3) = 0;  // paramater p not used.

  strcpy(buf+5, "(SAY) ");
  char* p = buf+5+6;
  for (int i = 0; i < argc; i++) {
    word n = strlen(argv[i]);
    memcpy(p, argv[i], n);
    p += n;
    if (i < argc-1) {
      *p++ = ' ';
    }
  }

  Send(buf, 5 + pay_len);
  return OKAY;
}
