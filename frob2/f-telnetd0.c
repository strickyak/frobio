// f-ntp server-addr:123

#include "frob2/froblib.h"
#include "frob2/frobnet.h"
#include "frob2/frobos9.h"

#define SUPRESS_GO_AHEAD 3

#define WILL (char)0xfb
#define WONT (char)0xfc
#define DO (char)0xfd
#define DONT (char)0xfe
#define IAC (char)0xff

static byte SocketNum = 255;
static char Buffer[250];
static char CommandBuf[4];

static void SendCommand(byte modal, byte option) {
            CommandBuf[0] = IAC;
            CommandBuf[1] = modal;
            CommandBuf[2] = option;

            prob err = tcp_send_blocking(SocketNum, CommandBuf, 3);
            if (err) LogFatal("Cannot tcp_send: %q", err);
            LogStep("sent: modal %x option %x", modal, option);
}

// Reject all options except SUPRESS_GO_AHEAD.
// Filter out characters with high bit, for now.
static void ProcessTelnetOptions(size_t* cc_in_out) {
  // Read Buffer at i, write Buffer at j.
  // i may get ahead of j, when we trim out commands and chars with the high bit set.
  word i=0, j=0, n = *cc_in_out;
  for (; i<n; i++) {
    char a = Buffer[i];
    if (a==IAC) {
      char b=Buffer[i+1], c=Buffer[i+2];
      switch(b) {
        case WILL:
          if (c==SUPRESS_GO_AHEAD) {
            SendCommand(DO, c);
          } else {
            SendCommand(DONT, c);
          }
          break;
        case WONT:
          SendCommand(DONT, c);
          break;
        case DO:
          if (c==SUPRESS_GO_AHEAD) {
            SendCommand(WILL, c);
          } else {
            SendCommand(WONT, c);
          }
          break;
        case DONT:
          SendCommand(WONT, c);
          break;
        case IAC: // means 255 was sent as data.
          // Consume and ignore chars with high bit.
          i--; // changes "i += 3" to "i += 2" to skip the IAC.
          break;
      }
      i += 3;
    } else { // not IAC
      if (a & 0x80) {
          // Consume and ignore chars with high bit.
      } else {
        Buffer[j] = Buffer[i];
        j++;
      }
      i++;
    }
  }  // next i
  *cc_in_out = j;
}

static void FatalUsage() {
    LogFatal("Usage:  f.telnet0 -w0xFF68 -p23");
}

int main(int argc, char* argv[]) {
  word port = 23; // default telnet port.
  SkipArg(&argc, &argv); // Discard argv[0], unused on OS-9.
  while (GetFlag(&argc, &argv, "p:v:w:")) {
    // GetFlag sets FlagChar & FlagArg.
    switch (FlagChar) {
    case 'p':
      port = (word)prefixed_atoi(FlagArg);
      break;
    case 'v':
      Verbosity = (byte)prefixed_atoi(FlagArg);
      break;
    case 'w':
      wiz_hwport = (byte*)prefixed_atoi(FlagArg);
      break;
    default:
      FatalUsage();
    }
  }
  if (argc) FatalUsage();
  if (!port) FatalUsage();

  prob err = tcp_open(&SocketNum);
  if (err) LogFatal("Cannot open TCP socket: %q", err);
  LogStep("opened");

  err = tcp_listen(SocketNum, port);
  if (err) LogFatal("Cannot listen on TCP server port %d: %q", port, err);
  LogStep("listened");

  err = tcp_establish_blocking(SocketNum);
  if (err) LogFatal("Cannot accept on TCP server port %d: %q", port, err);
  LogStep("accepted");

  const char* banner = "(WELCOME)\r\n";
  err = tcp_send_blocking(SocketNum, banner, strlen(banner));
  if (err) LogFatal("Cannot send banner: %q", err);
  LogStep("welcomed");

  while (true) {
    size_t cc = 0;
    memset(Buffer, 0, sizeof Buffer);
    err = tcp_recv_blocking(SocketNum, Buffer, sizeof Buffer - 1, &cc);
    if (err) {
        LogInfo("... recv->%q", err);
        continue;
    }
    LogStep("recv cc=%x: %q", cc, Buffer);

    ProcessTelnetOptions(&cc);

    if (cc) {
      err = tcp_send_blocking(SocketNum, Buffer, cc);
      if (err) LogFatal("Cannot tcp_send: %q", err);
      LogStep("send: %q", Buffer);
    }

    if (Buffer[0]=='q' || Buffer[0]=='Q') break;
  }

  banner = "(BYE)\r\n";
  err = tcp_send_blocking(SocketNum, banner, strlen(banner));
  if (err) LogFatal("Cannot send (BYE): %q", err);
  LogStep("bye");

  err = tcp_close(SocketNum);
  if (err) LogFatal("Cannot tcp_close: %q", err);
  LogStep("closed");
  LogStatus("Excellent");

  return 0;
}

/*  https://datatracker.ietf.org/doc/html/rfc854

  SE                  240    End of subnegotiation parameters.
  NOP                 241    No operation.
  Data Mark           242    The data stream portion of a Synch.
                             This should always be accompanied
                             by a TCP Urgent notification.
  Break               243    NVT character BRK.
  Interrupt Process   244    The function IP.
  Abort output        245    The function AO.
  Are You There       246    The function AYT.
  Erase character     247    The function EC.
  Erase Line          248    The function EL.
  Go ahead            249    The GA signal.
  SB                  250    Indicates that what follows is
                      $fa    subnegotiation of the indicated
                             option.
  WILL (option code)  251    Indicates the desire to begin
                      $fb    performing, or confirmation that
                             you are now performing, the
                             indicated option.
  WON'T (option code) 252    Indicates the refusal to perform,
                      $fc    or continue performing, the
                             indicated option.
  DO (option code)    253    Indicates the request that the
                      $fd    other party perform, or
                             confirmation that you are expecting
                             the other party to perform, the
                             indicated option.
  DON'T (option code) 254    Indicates the demand that the
                      $fe    other party stop performing,
                             or confirmation that you are no
                             longer expecting the other party
                             to perform, the indicated option.
  IAC                 255    Data Byte 255.
                      $ff


  https://www.iana.org/assignments/telnet-options/telnet-options.xhtml

  0    Binary Transmission    [RFC856]
  1    Echo    [RFC857]
  2    Reconnection    [NIC 15391 of 1973]
  3    Suppress Go Ahead    [RFC858]
  4    Approx Message Size Negotiation    [NIC 15393 of 1973]
  5    Status    [RFC859]
  6    Timing Mark    [RFC860]
  7    Remote Controlled Trans and Echo    [RFC726]
  8    Output Line Width    [NIC 20196 of August 1978]
  9    Output Page Size    [NIC 20197 of August 1978]
  10    Output Carriage-Return Disposition    [RFC652]
  11    Output Horizontal Tab Stops    [RFC653]
  12    Output Horizontal Tab Disposition    [RFC654]
  13    Output Formfeed Disposition    [RFC655]
  14    Output Vertical Tabstops    [RFC656]
  15    Output Vertical Tab Disposition    [RFC657]
  16    Output Linefeed Disposition    [RFC658]
  17    Extended ASCII    [RFC698]
  18    Logout    [RFC727]
  19    Byte Macro    [RFC735]
  20    Data Entry Terminal    [RFC1043][RFC732]
  21    SUPDUP    [RFC736][RFC734]
  22    SUPDUP Output    [RFC749]
  23    Send Location    [RFC779]
  24    Terminal Type    [RFC1091]
  25    End of Record    [RFC885]
  26    TACACS User Identification    [RFC927]
  27    Output Marking    [RFC933]
  28    Terminal Location Number    [RFC946]
  29    Telnet 3270 Regime    [RFC1041]
  30    X.3 PAD    [RFC1053]
  31    Negotiate About Window Size    [RFC1073]
  32    Terminal Speed    [RFC1079]
  33    Remote Flow Control    [RFC1372]
  34    Linemode    [RFC1184]
  35    X Display Location    [RFC1096]
  36    Environment Option    [RFC1408]
  37    Authentication Option    [RFC2941]
  38    Encryption Option    [RFC2946]
  39    New Environment Option    [RFC1572]
  40    TN3270E    [RFC2355]
  41    XAUTH    [Rob_Earhart]
  42    CHARSET    [RFC2066]
  43    Telnet Remote Serial Port (RSP)    [Robert_Barnes]
  44    Com Port Control Option    [RFC2217]
  45    Telnet Suppress Local Echo    [Wirt_Atmar]
  46    Telnet Start TLS    [Michael_Boe]
  47    KERMIT    [RFC2840]
  48    SEND-URL    [David_Croft]
  49    FORWARD_X    [Jeffrey_Altman]
  50-137    Unassigned    [IANA]
  138    TELOPT PRAGMA LOGON    [Steve_McGregory]
  139    TELOPT SSPI LOGON    [Steve_McGregory]
  140    TELOPT PRAGMA HEARTBEAT    [Steve_McGregory]
  141-254    Unassigned
  255    Extended-Options-List    [RFC861]

My first observations in practice:

  My telnet client on Linux sent these:
    Do    3 Supress Go Ahead
    Will 24 Terminal Type
    Will 31 Negotiate about Window Size
    Will 32 Terminal Speed
    Will 34 Linemode
    Will 39 New Environment Option
    Do    5 Status
    Will 35 X Display Location

  My stupid telnet daemon dittoed that back.

  Then my telnet client sent these:
    Will  3 Supress Go Ahead
    Dont 24 Terminal Type
    Dont 31 Negotiate about Window Size
    Dont 32 Terminal Speed
    Dont 34 Linemode
    Dont 39 New Environment Option
    Wont  5 Status
    Dont 35 X Display Location

*/
