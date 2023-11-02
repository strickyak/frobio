// f.say [-w wizport] [-vN] message...
//
// Sends the message... as a LOG message to the waiter server,
// prefixed with "(SAY) ".

#define LEMMA_SOCK_NUM 1
#define MAX_MSG_LEN 255

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"

// buf: 5 for quint header, 4 for "(R) ", then the message.
char buf[5 + 4 + MAX_MSG_LEN];
char inbuf[5 + 4 + MAX_MSG_LEN];

File* FilePtr; // For whatever file is open.

void ShowPay(char* p, size_t n) {
  p[5+n] = '\0';
  LogInfo("%x.%x.%x ==> [%d.] %q", (byte)p[0], (byte)p[3], (byte)p[4], n, p+5);
}

void Send(const char* p, size_t n) {
  prob err;
  do {
    err = TcpSendOrNotYet(LEMMA_SOCK_NUM, p, n);
  } while (err == NotYet);
  if (err) LogFatal("cannot TcpSend: %s", err);
}

size_t Recv(char* p) {
  size_t num_bytes_out;
  prob err = TcpRecvBlocking(1, p, 5, &num_bytes_out);
  if (err) LogFatal("Cannot recv quint: %s", err);
  size_t pay_len = ((size_t)(p[1]) << 8) + p[2];
  if (!pay_len) LogFatal("why is pay_len zero");
  if (pay_len>1200) LogFatal("why is pay_len so big: %x", pay_len);

  err = TcpRecvBlocking(1, p+5, pay_len, &num_bytes_out);
  if (err) LogFatal("Cannot recv payload: %s", err);
  return pay_len;
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

  word pay_len = 4 + msg_len; // length after quint: 4 for "(R) "

  buf[0] = 196; // CMD_BEGIN_MUX
  *(word*)(buf+1) = pay_len;
  *(word*)(buf+3) = 0;  // parameter p not used.

  strcpy(buf+5, "(R) ");
  char* p = buf+5+4;
  for (int i = 0; i < argc; i++) {
    word n = strlen(argv[i]);
    memcpy(p, argv[i], n);
    p += n;
    if (i < argc-1) {
      *p++ = ' ';
    }
  }

  DisableIrqsCounting();
  Send(buf, 5 + pay_len);

  size_t paylen = Recv(inbuf);
  EnableIrqsCounting();
  ShowPay(inbuf, paylen);

  while (true) {
    buf[0] = 197; // CMD_MID_MUX
    DisableIrqsCounting();
    Send(buf, 5 + pay_len);

    paylen = Recv(inbuf);
    EnableIrqsCounting();
    ShowPay(inbuf, paylen);

    Assert(paylen);

    switch (inbuf[5]) {
    case '1':
      FPuts(inbuf+6, FilePtr ? FilePtr : StdOut);
      break;
    case '2':
      FPuts(inbuf+6, StdErr);
      break;
    case '3':
      LogError("r got err: %s", inbuf+6);
      break;
    case '6':
      errnum e = FWrite(inbuf+6, paylen-6, FilePtr ? FilePtr : StdOut);
      if (e) {
        LogError("cannot FWrite stdout: e=%d", e);
        return e;
      }
      break;
    case 'a':
      {
        File* f = FOpen(inbuf+6, "w");
        if (!f) {
          LogError("cannot Create file %q: e=%d", inbuf+6, ErrNo);
          return ErrNo;
        }
        FilePtr = f;
      }
      break;
    case 'b':
      {
        File* f = FOpen(inbuf+6, "r");
        if (!f) {
          LogError("cannot Open file %q: e=%d", inbuf+6, ErrNo);
          return ErrNo;
        }
        FilePtr = f;
      }
      break;
    case 'c':
      {
        FClose(FilePtr);
        FilePtr = (File*)NULL;
      }
      break;
    } // end switch

    if (inbuf[5] == '.') break;
  } // end while

  buf[0] = 198; // CMD_END_MUX
  DisableIrqsCounting();
  Send(buf, 5 + pay_len);

  paylen = Recv(inbuf);
  EnableIrqsCounting();
  ShowPay(inbuf, paylen);

  return OKAY;
}
