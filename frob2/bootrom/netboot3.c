#include "frob2/bootrom/bootrom3.h"

void WizOpen(const struct sock* sockp, const struct proto* proto, word local_port ) {
  WizPut1(B+SK_MR, proto->open_mode);
  WizPut2(B+SK_PORTR0, local_port); // Set local port.
  WizPut1(B+SK_IR, 0xFF); // Clear all interrupts.
  WizIssueCommand(SOCK_AND SK_CR_OPEN);

  WizWaitStatus(SOCK_AND proto->open_status);
}

// Only called for TCP Client.
void TcpDial(const struct sock* sockp, const byte* host, word port) {
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
    Delay(2000);
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

errnum WizCheck(PARAM_JUST_SOCK) {
      byte ir = WizGet1(B+SK_IR); // Socket Interrupt Register.
      if (ir & SK_IR_TOUT) { // Timeout?
        return SK_IR_TOUT;
      }
      if (ir & SK_IR_DISC) { // Disconnect?
        return SK_IR_DISC;
      }
      return OKAY;
}

errnum WizRecvGetBytesWaiting(const struct sock* sockp, word* bytes_waiting_out) {
  errnum e = WizCheck(JUST_SOCK);
  if (e) return e;

  *bytes_waiting_out = WizGet2(B+SK_RX_RSR0);  // Unread Received Size.
  return OKAY;
}

errnum WizRecvChunkTry(const struct sock* sockp, char* buf, size_t n) {
  word bytes_waiting = 0;
  errnum e = WizRecvGetBytesWaiting(SOCK_AND &bytes_waiting);
  if (e) return e;
  if( bytes_waiting < n) return NOTYET;

  word rd = WizGet2(B+SK_RX_RD0);
  word begin = rd & RING_MASK; // begin: Beneath RING_SIZE.
  word end = begin + n;    // end: Sum may not be beneath RING_SIZE.

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
  return OKAY;
}

errnum WizRecvChunk(const struct sock* sockp, char* buf, size_t n) {
  errnum e;
  do {
    e = WizRecvChunkTry(SOCK_AND buf, n);
  } while (e == NOTYET);
  return e;
}
errnum WizRecvChunkBytes(const struct sock* sockp, byte* buf, size_t n) {
  return WizRecvChunk(sockp, (char*)buf, n);
}
errnum TcpRecv(const struct sock* sockp, char* p, size_t n) {
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    errnum e = WizRecvChunk(SOCK_AND  p, chunk);
    if (e) return e;
    n -= chunk;
    p += chunk;
  }
  return OKAY;
}

void UdpDial(const struct sock* sockp,  const struct proto *proto,
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

void WizReserveToSend(const struct sock* sockp,  size_t n) {
  PrintH("ResTS %x;", n);
  // Wait until free space is available.
  word free_size;
  do {
    free_size = WizGet2(B+SK_TX_FSR0);
    PrintH("Res free %x;", free_size);
  } while (free_size < n);

  struct sock_vars *sv = SV;
  sv->tx_ptr = WizGet2(B+SK_TX_WR0) & RING_MASK;
  sv->tx_to_go = n;
}

void WizBytesToSend(const struct sock* sockp, const byte* data, size_t n) {
  WizDataToSend(SOCK_AND (char*)data, n);
}
void WizDataToSend(const struct sock* sockp, const char* data, size_t n) {
  struct sock_vars *sv = SV;
  AssertLE(n, sv->tx_to_go);  // Must have already reserved.

  word begin = sv->tx_ptr;  // begin: Beneath RING_SIZE.
  word end = begin + n;       // end:  Sum may not be beneath RING_SIZE.

  if (end >= RING_SIZE) {
    word first_n = RING_SIZE - begin;
    word second_n = n - first_n;
    WizPutN(T+begin, data, first_n);
    WizPutN(T, data+first_n, second_n);
  } else {
    WizPutN(T+begin, data, n);
  }
  sv->tx_to_go -= n;
  sv->tx_ptr = (n + sv->tx_ptr) & RING_MASK;
}

void WizFinalizeSend(const struct sock* sockp, const struct proto *proto, size_t n) {
  struct sock_vars *sv = SV;
  word tx_wr = WizGet2(B+SK_TX_WR0);
  tx_wr += n;
  WizPut2(B+SK_TX_WR0, tx_wr);
  AssertEQ(sv->tx_to_go, 0);
  WizIssueCommand(SOCK_AND  proto->send_command);
}

errnum WizSendChunk(const struct sock* sockp,  const struct proto* proto, char* data, size_t n) {
  errnum e = WizCheck(JUST_SOCK);
  if (e) return e;
  WizReserveToSend(SOCK_AND  n);
  WizDataToSend(SOCK_AND data, n);
  WizFinalizeSend(SOCK_AND proto, n);
  return OKAY;
}
errnum TcpSend(const struct sock* sockp,  char* p, size_t n) {
  while (n) {
    word chunk = (n < TCP_CHUNK_SIZE) ? n : TCP_CHUNK_SIZE;
    errnum e = WizSendChunk(SOCK_AND &TcpProto, p, chunk);
    if (e) return e;
    n -= chunk;
    p += chunk;
  }
  return OKAY;
}

void WizClose(PARAM_JUST_SOCK) {
  WizIssueCommand(SOCK_AND 0x10/*CLOSE*/);
  WizPut1(B+SK_MR, 0x00/*Protocol: Socket Closed*/);
  WizPut1(B+0x0002/*_IR*/, 0x1F); // Clear all interrupts.
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

errnum LemmaClientS1() {
  char quint[5];
  char inkey;
  bool e;

    inkey = PolCat();
    if (inkey) {
      memset(quint, 0, sizeof quint);
      quint[0] = CMD_INKEY;
      quint[4] = inkey;
      e = WizSendChunk(SOCK1_AND &TcpProto,  quint, sizeof quint);
      if (e) return e;
    }

    e = WizRecvChunkTry(SOCK1_AND quint, sizeof quint);
    if (e == OKAY) {
      word n = *(word*)(quint+1);
      word p = *(word*)(quint+3);
      switch ((byte)quint[0]) {
        case CMD_POKE:
          {
            PrintH("POKE(%x@%x)", n, p);
            TcpRecv(SOCK1_AND (char*)p, n);
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
            PrintH("PEEK(%x@%x)", n, p);

            quint[0] = CMD_DATA;
            WizSendChunk(SOCK1_AND &TcpProto, quint, 5);
            TcpSend(SOCK1_AND (char*)p, n);
          }
          break;
        case CMD_JSR:
          {
            func fn = (func)p;
            fn();
          }
          break;
        default:
          Fatal("WUT?", quint[0]);
          break;
      } // switch
    }
    return (e==NOTYET) ? OKAY : e;
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

extern void DoKeyboardCommands();
extern errnum RunDhcp(const struct sock* sockp, const char* name4, word ticks);
const char name4[] = "NONE"; // unused

int main() {
    // Clear our global variables to zero.
    memset(Vars, 0, sizeof (struct vars));
    // From the address of main, we can determine if we
    // are netboot3 at $Cxxx or exoboot3 at $6xxx.
    Vars->rom_api_3 = (struct RomApi3*)(
        ((word)(&main) & 0xE000) | 0x0100);

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
    PrintF("main=%x stack=%x\n", &main, s_reg);
    Vars->s_reg = s_reg;

    DoKeyboardCommands();

    // Use ticks to create a local port with high bit set.
    // Get ticks before the reset!
    word ticks = WizTicks();
    word local_port = 0x8000 | ticks;

    PrintF("WizReset; ");
    WizReset();

    if (Vars->need_dhcp) {
      PrintF("DHCP:");
      errnum e = RunDhcp(SOCK0_AND name4, ticks);
      if (e) Fatal("RunDhcp", e);
    } else {
      // Set Hostname
      for (byte i = 0; i < 4; i++) {
        Vars->hostname[i+i+0] = HexAlphabet[15&(Vars->ip_addr[i]>>4)];
        Vars->hostname[i+i+1] = HexAlphabet[15&(Vars->ip_addr[i]>>0)];
      }
      // Set MAC
      Vars->mac_addr[0] = 2;  // 2 == self-assigned.
      Vars->mac_addr[1] = 255;
      memcpy(Vars->mac_addr+2, Vars->ip_addr, 4);
    }

    WizConfigure();

    WizOpen(SOCK1_AND &TcpProto, local_port);
    PrintF("tcp dial %a:%u;", Vars->ip_waiter, Vars->waiter_port);
    TcpDial(SOCK1_AND Vars->ip_waiter, Vars->waiter_port);

    TcpEstablish(JUST_SOCK1);
    PrintF(" CONN; ");
    Delay(50000);

    while (1) {
        errnum e = LemmaClientS1();
        if (e) Fatal("BOTTOM", e);
    }

    return 0;
}
