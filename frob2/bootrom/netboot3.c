#include "frob2/bootrom/bootrom3.h"

void WizOpen(PARAM_SOCK_AND const struct proto* proto, word local_port ) {
  WizPut1(B+SK_MR, proto->open_mode);
  WizPut2(B+SK_PORTR0, local_port); // Set local port.
  WizPut1(B+SK_IR, 0xFF); // Clear all interrupts.
  WizIssueCommand(SOCK_AND SK_CR_OPEN);

  WizWaitStatus(SOCK_AND proto->open_status);
}

// Only called for TCP Client.
void TcpDial(PARAM_SOCK_AND const byte* host, word port) {
  WizPut2(B+SK_TX_WR0, T); // does this help

  WizPutN(B+SK_DIPR0, host, 4);
  WizPut2(B+SK_DPORTR0, port);
  WizPut1(B+SK_IR, 0xFF); // Clear Interrupt bits.
  WizIssueCommand(SOCK_AND SK_CR_CONN);
}

// For Server or Client to accept/establish connection.
void TcpEstablish(PARAM_JUST_SOCK) {
  byte stuck = 250;
  while(1) {
    Delay(1000);
    PutChar('+');
    // Or we could wait for the connected interrupt bit,
    // and not the disconnected nor the timeout bit.
    byte status = WizGet1(B+SK_SR);
    if (!--stuck) Fatal("TEZ", status);

    if (status == SK_SR_ESTB) break;
    if (status == SK_SR_INIT) continue;
    if (status == SK_SR_SYNS) continue;

    Fatal("TE", status);

  };

  WizPut1(B+SK_IR, SK_IR_CON); // Clear the Connection bit.
}

void WizCheck(PARAM_JUST_SOCK) {
      byte ir = WizGet1(B+SK_IR); // Socket Interrupt Register.
      if (ir & SK_IR_TOUT) { // Timeout?
        Fatal("TMO", ir);
      }
      if (ir & SK_IR_DISC) { // Disconnect?
        Fatal("DIS", ir);
      }
}

bool WizRecvChunkTry(PARAM_SOCK_AND char* buf, size_t n) {
  WizCheck(JUST_SOCK);

  word bytes_waiting = WizGet2(B+SK_RX_RSR0);  // Unread Received Size.
  word rd = WizGet2(B+SK_RX_RD0);
  word wr = WizGet2(B+SK_RX_WR0);

  word begin = rd & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + n;    // end: Sum may not be beneath RING_SIZE.
  PrintH("WRCTn=%x^*%x^r%x^w%x^b/%x^e/%x", n, bytes_waiting, rd, wr, begin, end);

  if (bytes_waiting < n) return false;
  PrintH("WRCT+");

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizGetN(R+begin, buf, first_n);
    WizGetN(R, buf+first_n, second_n);
  } else {
    WizGetN(R+begin, buf, n);
  }

  WizPut2(B+SK_RX_RD0, rd + n);
  WizIssueCommand(SOCK_AND  SK_CR_RECV); // Inform socket of changed SK_RX_RD.
  return true;
}

void WizRecvChunk(PARAM_SOCK_AND char* buf, size_t n) {
  bool ok;
  do {
    // LIVENESS(0);
    ok = WizRecvChunkTry(SOCK_AND buf, n);
  } while (!ok);
}
void TcpRecv(PARAM_SOCK_AND char* p, size_t n) {
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    WizRecvChunk(SOCK1_AND (char*)p, chunk);
    n -= chunk;
    p += chunk;
  }
}

void UdpDial(PARAM_SOCK_AND  const struct proto *proto,
             const byte* dest_ip, word dest_port) {
  if (proto->is_broadcast) {
    // Broadcast to 255.255.255.255 to FF:FF:FF:FF:FF:FF.
    WizPutN(B+6/*Sn_DHAR*/, SixFFs, 6);
    WizPutN(B+SK_DIPR0, SixFFs, 4);
  } else {
    WizPutN(B+SK_DIPR0, dest_ip, 4);
  }
  WizPut2(B+SK_DPORTR0, dest_port);
}

void WizReserveToSend(PARAM_SOCK_AND  size_t n) {
  PrintH("ResTS %x", n);
  // Wait until free space is available.
  word free_size;
  do {
    // LIVENESS(1);
    free_size = WizGet2(B+SK_TX_FSR0);
    PrintH("Res free %x", free_size);
  } while (free_size < n);

  SS->tx_ptr = WizGet2(B+SK_TX_WR0) & RING_MASK;
  SS->tx_to_go = n;
  //print4("1tx.ptr=%x togo=%x", SS->tx_ptr, SS->tx_to_go);
}

void WizDataToSend(PARAM_SOCK_AND char* data, size_t n) {
  AssertLE(n, SS->tx_to_go);  // Must have already reserved.

  word begin = SS->tx_ptr;  // begin: Beneath RING_SIZE.
  word end = begin + n;       // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(T+begin, data, first_n);
    WizPutN(T, data+first_n, second_n);
  } else {
    WizPutN(T+begin, data, n);
  }
  SS->tx_to_go -= n;
  SS->tx_ptr = (n + SS->tx_ptr) & RING_MASK;
  //print4("2tx.ptr=%x togo=%x", SS->tx_ptr, SS->tx_to_go);
}

void WizFinalizeSend(PARAM_SOCK_AND const struct proto *proto, size_t n) {
  //print4("FSend %x", n);
  word tx_wr = WizGet2(B+SK_TX_WR0);
  //print4("3tx..%x", tx_wr);
  tx_wr += n;
  WizPut2(B+SK_TX_WR0, tx_wr);
  //print4("..%x", tx_wr);
  AssertEQ(SS->tx_to_go, 0);
  WizIssueCommand(SOCK_AND  proto->send_command);
}

void WizSendChunk(PARAM_SOCK_AND  const struct proto* proto, char* data, size_t n) {
  //print4("SEND");
  WizCheck(JUST_SOCK);
  WizReserveToSend(SOCK_AND  n);
  WizDataToSend(SOCK_AND data, n);
  WizFinalizeSend(SOCK_AND proto, n);
}
void TcpSend(PARAM_SOCK_AND  char* p, size_t n) {
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    WizSendChunk(SOCK1_AND &TcpProto, p, chunk);
    n -= chunk;
    p += chunk;
  }
}

void WizClose(PARAM_JUST_SOCK) {
  //print4("Z");
  WizIssueCommand(SOCK1_AND 0x10/*CLOSE*/);
  WizPut1(B+SK_MR, 0x00/*Protocol: Socket Closed*/);
  WizPut1(B+0x0002/*_IR*/, 0x1F); // Clear all interrupts.
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

void LemmaClientS1() {
  char quint[5];
  char inkey;
  bool ok;

    inkey = PolCat();
    if (inkey) {
      memset(quint, 0, sizeof quint);
      quint[0] = CMD_INKEY;
      quint[4] = inkey;
      WizSendChunk(SOCK1_AND &TcpProto,  quint, sizeof quint);
    }

    ok = WizRecvChunkTry(SOCK1_AND quint, sizeof quint);
    if (ok) {
      word n = *(word*)(quint+1);
      word p = *(word*)(quint+3);
      switch ((byte)quint[0]) {
        case CMD_POKE:
          {
            // print3("POKE(%x@%x)", n, p);
            PrintH("POKE(%x@%x)", n, p);
            TcpRecv(SOCK1_AND (char*)p, n);

            // print3(".");
          }
          break;
        case CMD_PUTCHARS:
          {
            for (word i = 0; i < n; i++) {
              PutChar(*(char*)(p+i));
            }
          }
          break;
        case CMD_PEEK:
          {
            // print3("PEEK(%x@%x)", n, p);
            PrintH("PEEK(%x@%x)", n, p);

            quint[0] = CMD_DATA;
            WizSendChunk(SOCK1_AND &TcpProto, quint, 5);
            TcpSend(SOCK1_AND (char*)p, n);

            // print3(",");
          }
          break;
        case CMD_JSR:
          {
            func fn = (func)p;
            // PrintH("CALLING %x", fn); Delay(9000);
            fn();
            // PrintH("RETURNING FROM %x", fn); Delay(9000);
          }
          break;
        default:
          Fatal("WUT?", quint[0]);
          break;
      } // switch
    } // ok
}

#if 0
void Send5(byte cmd, word n, word p) {
    char quint[5] = {
      cmd,
      (byte)(n>>8), (byte)(n),
      (byte)(p>>8), (byte)(p) };
    WizSendChunk(SOCK1_AND &TcpProto, quint, 5);
    PutChar('#');
}
#endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

int main() {
    // Clear our global variables to zero.
    memset(Vars, 0, sizeof (struct vars));
    Vars->vars_magic = VARS_MAGIC;
    Vars->wiz_port = (struct wiz_port*) WIZ_PORT;

    // Set VDG 32x16 text screen to dots.
    // Preserve the top line.
    Vars->vdg_addr = VDG_RAM;
    Vars->vdg_begin = VDG_RAM+32;
    Vars->vdg_ptr = VDG_RAM+32;
    Vars->vdg_end = VDG_END;
    word vdg_n = Vars->vdg_end - Vars->vdg_begin;
    memset((char*)Vars->vdg_begin, '.', vdg_n);

    // Record Vars->s_reg, to give some idea of memory size.
    word s_reg = StackPointer();
    PrintF("stack=%x; ", s_reg);
    Vars->s_reg = s_reg;

    // Use Ticks to create a local port with high bit set.
    word ticks = WizTicks();
    word local_port = 0x8000 | ticks;

    PrintF("WizReset; ");
    WizReset();
#if BR_STATIC
    // PrintF("CONF");
    WizConfigure(BR_ADDR, BR_MASK, BR_GATEWAY);
#endif
    WizOpen(SOCK1_AND &TcpProto, local_port);
    PrintF("tcp dial %a %x;", BR_WAITER, 14511);
    TcpDial(SOCK1_AND BR_WAITER, 14511);
    TcpEstablish(JUST_SOCK1);
    PrintF(" CONN; ");

    while (1) {
        LemmaClientS1();
    }

    return 0;
}
