// f.say [-w wizport] [-vN] message...
//
// Sends the message... as a LOG message to the waiter server,
// prefixed with "(SAY) ".

#define LEMMA_SOCK_NUM 1
#define MAX_MSG_LEN 255

#include "frob3/froblib.h"
#include "frob3/frobnet.h"
#include "frob3/frobos9.h"
#include "frob3/os9/os9defs.h"

#define CMD_BEGIN_MUX 196
#define CMD_MID_MUX 197
#define CMD_END_MUX 198


// buf: 5 for quint header, 4 for "(R) ", then the message.
char buf[5 + 4 + MAX_MSG_LEN];
char inbuf[5 + 4 + MAX_MSG_LEN];

File* FilePtr; // For whatever file is open.

void PayToStdErr(char* p, size_t recv_paylen) {
  p[5+recv_paylen] = '\0';
  char last = p[5+recv_paylen-1];
  if (last != '\n' && last != '\r') {
    p[5+recv_paylen] = '\r';
    p[5+recv_paylen+1] = '\0';
  }
  FPuts(p+5, StdErr);
}

void ShowPay(char* p, size_t paylen) {
  p[5+paylen] = '\0';
  LogDetail("%02x:%02x%02x ==> [%d.] %q", (byte)p[0], (byte)p[3], (byte)p[4], paylen, p+5);
}

void Send(const char* p, size_t n) {
  prob err;
  do {
    err = TcpSendOrNotYet(LEMMA_SOCK_NUM, p, n);
  } while (err == NotYet);
  if (err) LogFatal("cannot TcpSend: %s", err);
}

size_t Recv(char* p, byte cmd, word channel) {
  byte* up = (byte*) p;
  /*XXX*/ memset(inbuf, 0, sizeof inbuf);

  size_t num_bytes_out;
  prob err = TcpRecvBlocking(1, p, 5, &num_bytes_out);
  if (err) LogFatal("Cannot recv quint: %s", err);
/**/
  LogDetail("Hey: Got %d. Expected %d. (%02x %02x %02x %02x)", *up, cmd, up[1], up[2], up[3], up[4]);
/**/
  size_t pay_len = ((size_t)(up[1]) << 8) + up[2];

  if (!pay_len) LogFatal("why is pay_len zero");
  if (pay_len>1200) LogFatal("why is pay_len so big: %x", pay_len);

  err = TcpRecvBlocking(1, p+5, pay_len, &num_bytes_out);
  if (err) LogFatal("Cannot recv payload: %s", err);

  if (*up != cmd) {
    if (*up == CMD_END_MUX) {
      LogFatal("FATAL: %q", p+5);
    }
    LogFatal("Cmd: Got %d. Expected %d.", *up, cmd);
  }
  word chan = *(word*)(p+3);
  if (chan != channel) {
    LogFatal("Channel: Got $%04x, expected $%04x", chan, channel);
  }

  p[5+pay_len] = '\0';
  return pay_len;
}

int main(int argc, char* argv[]) {
  Verbosity = LLStatus;  // Default: Show status, error, and fatal.

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

  word channel = WizTicks();

  word send_paylen = 4 + msg_len; // length after quint: 4 for "(R) "

  buf[0] = CMD_BEGIN_MUX;
  *(word*)(buf+1) = send_paylen;
  *(word*)(buf+3) = channel;

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

  //IRQ// DisableIrqsCounting();
  Send(buf, 5 + send_paylen);

  size_t recv_paylen = Recv(inbuf, CMD_BEGIN_MUX, channel);
  //IRQ// EnableIrqsCounting();
  ShowPay(inbuf, recv_paylen);

  while (true) {
    buf[0] = CMD_MID_MUX;
    *(word*)(buf+1) = 0 /*send_paylen*/;
    //IRQ// DisableIrqsCounting();
    Send(buf, 5);

JUST_RECV:
    recv_paylen = Recv(inbuf, CMD_MID_MUX, channel);
    //IRQ// EnableIrqsCounting();
    ShowPay(inbuf, recv_paylen);

    Assert(recv_paylen);

    switch (inbuf[5]) {
    case '.':
      goto END_WHILE;
    case '1':
      FPuts(inbuf+6, FilePtr ? FilePtr : StdOut);
      break;
    case '2':
      FPuts(inbuf+6, StdErr);
      break;
    case '3':
      LogInfo("info: %s", inbuf+6);
      break;
    case '4':
      errnum e = FWrite(inbuf+6, recv_paylen-1, FilePtr ? FilePtr : StdOut);
      if (e) {
        LogError("cannot FWrite: e=%d", e);
        return e;
      }
      break;
    case '6':
      {
        word nb = FGets(buf+6, sizeof buf - 7, FilePtr ? FilePtr : StdIn);
        if (!nb) {
          if (ErrNo == 0 || ErrNo == E_EOF) {
            buf[0] = CMD_MID_MUX;
            *(word*)(buf+1) = 0; // No payload on EOF
            Send(buf, 5);
          } else {
            buf[0] = CMD_END_MUX;
            SPrintf(buf+5, "- Cannot Read: OS9 ErrNo %d.", ErrNo);
            *(word*)(buf+1) = strlen(buf+5); // No payload on EOF
            Send(buf, 5+strlen(buf+5));
          }
        } else {
          buf[0] = CMD_MID_MUX;
          word n = strlen(buf+6);
          *(word*)(buf+1) = 1+n /*send_paylen*/;
          //IRQ// DisableIrqsCounting();
          buf[5] = '6';
          Send(buf, 6+n);

        }
        goto JUST_RECV;  // We do the last Send in this sub-protocol.
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
  } // end while
END_WHILE:

  buf[0] = CMD_END_MUX;
  //IRQ// DisableIrqsCounting();
  Send(buf, 5);

  recv_paylen = Recv(inbuf, CMD_END_MUX, channel);
  //IRQ// EnableIrqsCounting();
  ShowPay(inbuf, recv_paylen);

  PayToStdErr(inbuf, recv_paylen);
  return (inbuf[5]=='+') ? OKAY : 255;
}
