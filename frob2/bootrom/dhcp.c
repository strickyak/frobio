
struct dhcp {
    byte opcode; // 1=request 2=response
    byte htype;  // 1=ethernet
    byte hlen;   // == 6 bytes
    byte hops;   // 1 or 0, on a LAN
//4
    byte xid[4];  // transaction id
//8
    word secs;
    word flags;  // $80 broadcast, $00 unicast.
//12
    byte ciaddr[4];      // Client IP addr
    byte yiaddr[4];      // Your IP addr
//20
    byte siaddr[4];      // Server IP addr
    byte giaddr[4];      // Gateway IP addr
//28
    byte chaddr[16];  // client hardware addr
//44
    byte sname[64];   // server name
//108
    byte bname[128];  // boot file name
//236
    byte options[300];  // not really 300?
};
const byte LocalEtherAddr[6] = { 2, 2, 'C', 'O', 'C', 'O'};
const byte DiscoverOffset0Len12[] = {
  1, // opcode: request
  1, // htype: ethernet
  6, // hlen: 6-byte hw addrs
  0, // hops: none.
  'C', 'O', 'C', 'O', // xid: reuse the Hostname.`
  0, 0, // secs
  0, 0x80, // flags:  0x80=Broadcast or 0x00=Unicast.
};
const byte OptionsBytes[] = {
  99, 130, 83, 99, // magic cookie for DHCP.
  53, 1, 1, // 53=DHCP Option, 1=length, 1=Discover.
  12, 4, 'C', 'O', 'C', 'O', // 12=Hostname, 4=length.
  255, 0   // 255=End, 0=length.
};

const byte EightZeros[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
void SendNZerosUDPSock1(byte n) {
  byte quotient = n>>3;  // div 8
  byte remainder = n&7;  // mod 8
  for (byte i = quotient; i; i--) {
    SendDataUDP(SOCK1_AND
        EightZeros, sizeof(EightZeros));
  }
  if (remainder) {
    SendDataUDP(SOCK1_AND
        EightZeros, remainder);
  }
}

bool SendFirstDhcpS1() {
  const word request_len = 236 + sizeof(OptionsBytes);
  PrepareToSendUDP(SOCK1_AND request_len);

  SendDataUDP(SOCK1_AND DiscoverOffset0Len12, 12);

  SendNZerosUDPSock1(16); // {c,y,s,g}iaddr: 4x4

  SendDataUDP(SOCK1_AND LocalEtherAddr, 6);
  // 16 - 6 = 10 chaddr: pad bytes should be zero.
  // 10 (rest of chaddr) + 64 (sname) + 128 (bname)
  const byte pad = 10 + 64 + 128;
  SendNZerosUDPSock1(pad); // rest of chaddr, sname, bname
                           //
  SendDataUDP(SOCK1_AND OptionsBytes, 6);

  FinallySendUDP(SOCK1_AND request_len);
}

bool RunDhcpS1() {
  SendFirstDhcpS1();
  RecvFirstDhcpS1();
  SendSecondDhcpS1();
  RecvSecondDhcpS1();
}
