// github.com/strickyak/frobio/frob2/wiz/w5100s_defs.h
// copied from github.com/n6il/cocoio-dw/tests/w5100s.h
// Just the `#define`s.

#ifndef W5100S_DEFS_H_
#define W5100S_DEFS_H_

// #include <stdint.h>

/*********************************************************************
;* Title: W5100SEQU.asm
;*********************************************************************
;* Author: R. Allen Murphey
;*
;* License: Contributed 2021 by R. Allen Murphey to CoCoIO Development
;*
;* Description: WIZnet W5100S driver equates
;*
;* Documentation: https://www.wiznet.io/product-item/w5100s/
;*
;* Include Files: none
;*
;* Assembler: lwasm 1.4.2
;*
;* Revision History:
;* Rev #     Date      Who     Comments
;* -----  -----------  ------  ---------------------------------------
;* 01     2022/05/23   MRF     Convert from ASM to C
;* 00     2021         RAM     Initial equates from W5100S Data Sheet
;*********************************************************************/

/*
                              ; W5100S REGISTER / MEMORY MAP
                              ; 0x0000 - 0x002F Common Registers W5100 compatible
                              ; 0x0030 - 0x0088 Common Registers New W5100S
                              ; 0x0089 - 0x03FF -- Reserved --
                              ; 0x0400 - 0x04FF Socket 0 Registers
                              ; 0x0500 - 0x05FF Socket 1 Registers
                              ; 0x0600 - 0x06FF Socket 2 Registers
                              ; 0x0700 - 0x07FF Socket 3 Registers
                              ; 0x0800 - 0x3FFF -- Reserved --
                              ; 0x4000 - 0x5FFF TX Memory - default 2KB per socket
                              ; 0x6000 - 0x7FFF RX Memory - default 2KB per socket
*/

                                /* COMMON REGISTERS 0x0000-0x0088 */
#define MR         0x0000       /* Mode Register */
#define MR_IND     0x01         /* MR: Indirect */
#define MR_AINC    0x02         /* MR: Auto-Incrememt */
#define MR_RST     0x80         /* MR: Reset */ 
#define GAR0       0x0001       /* Gateway Address Register 0 */
#define GAR1       0x0002       /* Gateway Address Register 1 */
#define GAR2       0x0003       /* Gateway Address Register 2 */
#define GAR3       0x0004       /* Gateway Address Register 3 */
#define SUBR0      0x0005       /* Subnet Mask Address Register 0 */
#define SUBR1      0x0006       /* Subnet Mask Address Register 1 */
#define SUBR2      0x0007       /* Subnet Mask Address Register 2 */
#define SUBR3      0x0008       /* Subnet Mask Address Register 3 */
#define SHAR0      0x0009       /* Source Hardware Address Register 0 */
#define SHAR1      0x000A       /* Source Hardware Address Register 1 */
#define SHAR2      0x000B       /* Source Hardware Address Register 2 */
#define SHAR3      0x000C       /* Source Hardware Address Register 3 */
#define SHAR4      0x000D       /* Source Hardware Address Register 4 */
#define SHAR5      0x000E       /* Source Hardware Address Register 5 */
#define SIPR0      0x000F       /* Source IP Address Register 0 */
#define SIPR1      0x0010       /* Source IP Address Register 1 */
#define SIPR2      0x0011       /* Source IP Address Register 2 */
#define SIPR3      0x0012       /* Source IP Address Register 3 */
#define INTPTMR0   0x0013       /* Interrupt Pending Time Register 0 */
#define INTPTMR1   0x0014       /* Interrupt Pending Time Register 1 */
#define IR         0x0015       /* Interrupt Register */
#define IMR        0x0016       /* Interrupt Mask Register */
#define RTR0       0x0017       /* Retransmission Time Register 0 */
#define RTR1       0x0018       /* Retransmission Time Register 1 */
#define RCR        0x0019       /* Retransmission Count Register */
#define RMSR       0x001A       /* RX Memory Size Register */
#define TMSR       0x001B       /* TX Memory Size Register */
                                /* 0x001C-0x001F -- Reserved -- */
#define IR2        0x0020       /* Interrupt2 Register */
#define IMR2       0x0021       /* Interrupt2 Mask Register */
                                /* 0x0022-0x0027 -- Reserved -- */
#define PTIMER     0x0028       /* PPP LCP Request Timer */
#define PMAGIC     0x0029       /* PPP LCP Magic Number */
#define UIPR0      0x002A       /* Unreachable IP Address Register 0 */
#define UIPR1      0x002B       /* Unreachable IP Address Register 1 */
#define UIPR2      0x002C       /* Unreachable IP Address Register 2 */
#define UIPR3      0x002D       /* Unreachable IP Address Register 0 */
#define UPORTR0    0x002E       /* Unreachable Port Register 0 */
#define UPORTR1    0x002F       /* Unreachable Port Register 1 */
#define MR2        0x0030       /* Mode2 Register */
                                /* 0x0031 --  Reserved -- */
#define PHAR0      0x0032       /* Destination Hardware Address on PPPoE Register 0 */
#define PHAR1      0x0033       /* Destination Hardware Address on PPPoE Register 1 */
#define PHAR2      0x0034       /* Destination Hardware Address on PPPoE Register 2 */
#define PHAR3      0x0035       /* Destination Hardware Address on PPPoE Register 3 */
#define PHAR4      0x0036       /* Destination Hardware Address on PPPoE Register 4 */
#define PHAR5      0x0037       /* Destination Hardware Address on PPPoE Register 5 */
#define PSIDR0     0x0038       /* Session ID on PPPoE Register 0 */
#define PSIDR1     0x0039       /* Session ID on PPPoE Register 1 */
#define PMRUR0     0x003A       /* Maximum Receive Unit on PPPoE Register 0 */
#define PMRUR1     0x003B       /* Maximum Receive Unit on PPPoE Register 1 */
#define PHYSR0     0x003C       /* PHY Status Register 0 */
#define PHYSR1     0x003D       /* PHY Status Register 1 */
#define PHYAR      0x003E       /* PHY Address Value */
#define PHYRAR     0x003F       /* PHY Register Address */
#define PHYDIR0    0x0040       /* PHY Data Input Register 0 */
#define PHYDIR1    0x0041       /* PHY Data Input Register 1 */
#define PHYDOR0    0x0042       /* PHY Data Output Register 0 */
#define PHYDOR1    0x0043       /* PHY Data Output Register 1 */
#define PHYACR     0x0044       /* PHY Access Register */
#define PHYDIVR    0x0045       /* PHY Division Register */
#define PHYCR0     0x0046       /* PHY Control Register 0 */
#define PHYCR1     0x0047       /* PHY Control Register 1 */
                                /* 0x0048-0x004B -- Reserved -- */
#define SLCR       0x004C       /* SOCKET-less Command Register */
#define SLCR_P     0x01         /* Ping */
#define SLCR_A     0x02         /* ARP */
#define SLRTR0     0x004D       /* SOCKET-less Retransmission Time Register 0 */
#define SLRTR1     0x004E       /* SOCKET-less Retransmission Time Register 1 */
#define SLRCR      0x004F       /* SOCKET-less Retransmission Count Register */
#define SLPIPR0    0x0050       /* SOCKET-less Peer IP Address Register 0 */
#define SLPIPR1    0x0051       /* SOCKET-less Peer IP Address Register 1 */
#define SLPIPR2    0x0052       /* SOCKET-less Peer IP Address Register 2 */
#define SLPIPR3    0x0053       /* SOCKET-less Peer IP Address Register 3 */
#define SLPHAR0    0x0054       /* SOCKET-less Peer Hardware Address Register 0 */
#define SLPHAR1    0x0055       /* SOCKET-less Peer Hardware Address Register 1 */
#define SLPHAR2    0x0056       /* SOCKET-less Peer Hardware Address Register 2 */
#define SLPHAR3    0x0057       /* SOCKET-less Peer Hardware Address Register 3 */
#define SLPHAR4    0x0058       /* SOCKET-less Peer Hardware Address Register 4 */
#define SLPHAR5    0x0059       /* SOCKET-less Peer Hardware Address Register 5 */
#define PINGSEQR0  0x005A       /* PING Sequence Number Register 0 */
#define PINGSEQR1  0x005B       /* PING Sequence Number Register 1 */
#define PINGIDR0   0x005C       /* PING ID Register 0 */
#define PINGIDR1   0x005D       /* PING ID Register 1 */
#define SLIMR      0x005E       /* SOCKET-less Interrupt Mask Register */
#define SLIMR_P    0x01         /* Ping */
#define SLIMR_A    0x02         /* ARP */
#define SLIMR_T    0x04         /* Timeout */
#define SLIR       0x005F       /* SOCKET-less Interrupt Register */
#define SLIR_P     0x01         /* Ping */
#define SLIR_A     0x02         /* ARP */
#define SLIR_T     0x04         /* Timeout */
                                /* 0x0060-0x006A -- Reserved -- */
#define CLKLCKR    0x0070       /* Clock Lock Register */
#define NETLCKR    0x0071       /* Network Lock Register */
#define PHYLCKR    0x0072       /* PHY Lock Register */
                                /* 0x0073-0x007F -- Reserved -- */
#define VERR       0x0080       /* Chip Version */
                                /* 0x0081 -- Reserved -- */
#define TCNTR0     0x0082       /* 100us Ticker Counter Register 0 */
#define TCNTR1     0x0083       /* 100us Ticker Counter Register 1 */
                                /* 0x0084-0x0087 -- Reserved -- */
#define TCNTCLR    0x0088       /* 100us Ticker Counter Clear Register */

                    /* SOCKET 0 REGISTERS 0x0400-0x04FF 45 bytes per socket */
                    /* SOCKET 1 REGISTERS 0x0500-0x05FF 45 bytes per socket */
                    /* SOCKET 2 REGISTERS 0x0600-0x06FF 45 bytes per socket */
                    /* SOCKET 3 REGISTERS 0x0700-0x07FF 45 bytes per socket */

/* Socket Register Macros */

#define SKREGBASE   0x0400      /* Base Address for Sock Registers (Sock 0) */
#define SKREGBLK   0x0100      /* Size of Sock Register Blocks */
#define SKREGBLKOFS(k) (SKREGBLK*k) /* Socket k Register Block Offset */
#define SKREGADR(k) (SKREGBASE+SKREGBLKOFS(k)) /* Sock k Reg Blk Address */
#define SKREGABS(k, r) (SKREGADR(k)+r) /* Sock k Register r Address Absolute */
#define SKREGREL(o, r) (SKREGBASE+o+r) /* Sock k Register r Address Relative */
#define SKPTRREG(p, r) (p->skrgaddr + r)

/* Socket Buffer Macros */
#define SKTXBUFBASE  0x4000
#define SKRXBUFBASE  0x6000
#define SKBUFSIZE    0x0800
#define SKBUFMASK    0x07FF
#define SKRXBSTRT(k)    (SKRXBUFBASE+(k*SKBUFSIZE))
#define SKTXBSTRT(k)    (SKTXBUFBASE+(k*SKBUFSIZE))
#define SKRXBEND(k)    (SKRXBUFBASE+((k+1)*SKBUFSIZE)-1)
#define SKTXBEND(k)    (SKTXBUFBASE+((k+1)*SKBUFSIZE)-1)

/* Socket Register Defs */

#define SK_MR      0x0000       /* Socket K Mode Register */
#define SK_MR_PCLR 0xF0         /* Protocol: Clear */
#define SK_MR_PMSK 0x0F         /* Protocol: Mask */
#define SK_MR_CLSD 0x00         /* Protocool: Socket Closed */
#define SK_MR_TCP  0x01         /* Protocol: TCP */
#define SK_MR_UDP  0x02         /* Protocol: UDP */
#define SK_MR_IPRW 0x03         /* Protocol: IP Raw */
#define SK_MR_MACR 0x04         /* Protocol: MAC Raw */
#define SK_MR_ND   0x20         /* Protocol: TCP No Delay*/
#define SK_MR_CLND 0xDF         /* Protocol: Clear TCP No Delay */
#define SK_CR      0x0001       /* Socket K Command Register */
#define SK_CR_OPEN 0x01
#define SK_CR_LSTN 0x02
#define SK_CR_CONN 0x04
#define SK_CR_DISC 0x08
#define SK_CR_CLOS 0x10
#define SK_CR_SEND 0x20
#define SK_CR_KEEP 0x22
#define SK_CR_RECV 0x40
#define SK_IR      0x0002       /* Socket K Interrupt Register */
#define SK_IR_CON  0x01
#define SK_IR_DISC 0x02
#define SK_IR_RECV 0x04
#define SK_IR_TOUT 0x08
#define SK_IR_TXOK 0x10
#define SK_SR      0x0003       /* Socket K Status Register */
#define SK_SR_CLOS 0x00
#define SK_SR_INIT 0x13
#define SK_SR_LSTN 0x14
#define SK_SR_SYNS 0x15
#define SK_SR_SYNR 0x16
#define SK_SR_ESTB 0x17
#define SK_SR_FINW 0x18
#define SK_SR_TIMW 0x1B
#define SK_SR_CLWT 0x1C
#define SK_SR_LACK 0x1D
#define SK_SR_UDP  0x22
#define SK_SR_IPRW 0x32
#define SK_SR_MACR 0x42
#define SK_PORTR0  0x0004       /* Socket K Source Port Register 0 */
#define SK_PORTR1  0x0005       /* Socket K Source Port Register 1 */
#define SK_DHAR0   0x0006       /* Socket K Destination Hardware Address Register 0 */
#define SK_DHAR1   0x0007       /* Socket K Destination Hardware Address Register 1 */
#define SK_DHAR2   0x0008       /* Socket K Destination Hardware Address Register 2 */
#define SK_DHAR3   0x0009       /* Socket K Destination Hardware Address Register 3 */
#define SK_DHAR4   0x000A       /* Socket K Destination Hardware Address Register 4 */
#define SK_DHAR5   0x000B       /* Socket K Destination Hardware Address Register 5 */
#define SK_DIPR0   0x000C       /* Socket K Destination IP Address Register 0 */
#define SK_DIPR1   0x000D       /* Socket K Destination IP Address Register 1 */
#define SK_DIPR2   0x000E       /* Socket K Destination IP Address Register 2 */
#define SK_DIPR3   0x000F       /* Socket K Destination IP Address Register 3 */
#define SK_DPORTR0 0x0010       /* Socket K Destination Port Register 0 */
#define SK_DPORTR1 0x0011       /* Socket K Destination Port Register 1 */
#define SK_MSS0    0x0012       /* Socket K Maximum Segment Size Register 0 */
#define SK_MSS1    0x0013       /* Socket K Maximum Segment Size Register 1 */
#define SK_PROTOR  0x0014       /* Socket K IP Protocol Register */
#define SK_TOS     0x0015       /* Socket K IP Type Of Service */
#define SK_TTL     0x0016       /* Socket K IP Time To Live */
                                /* 0x0X17-0x0X1D -- Reserved -- */
#define SK_RXBUF_SIZE  $041E    /* Socket K RX Buffer Size */
#define SK_TXBUF_SIZE  $041F    /* Socket K TX Buffer Size */
#define SK_TX_FSR0 0x0020       /* Socket K TX Free Size Register 0 */
#define SK_TX_FSR1 0x0021       /* Socket K TX Free Size Register 1 */
#define SK_TX_RD0  0x0022       /* Socket K TX Read Pointer 0 */
#define SK_TX_RD1  0x0023       /* Socket K TX Read Pointer 1 */
#define SK_TX_WR0  0x0024       /* Socket K TX Write Pointer 0 */
#define SK_TX_WR1  0x0025       /* Socket K TX Write Pointer 1 */
#define SK_RX_RSR0 0x0026       /* Socket K RX Received Size Register 0 */
#define SK_RX_RSR1 0x0027       /* Socket K RX Received Size Register 1 */
#define SK_RX_RD0  0x0028       /* Socket K RX Read Pointer 0 */
#define SK_RX_RD1  0x0029       /* Socket K RX Read Pointer 1 */
#define SK_RX_WR0  0x002A       /* Socket K RX Write Pointer 0 */
#define SK_RX_WR1  0x002B       /* Socket K RX Write Pointer 1 */
#define SK_IMR     0x002C       /* Socket K Interrupt Mask Register */
#define SK_FRAGR0  0x002D       /* Socket K Fragment Offset in IP Header Register 0 */
#define SK_FRAGR1  0x002E       /* Socket K Fragment Offset in IP Header Register 1 */
#define SK_MR2     0x002F       /* Socket K Mode 2 Register */
#define SK_KPALVTR 0x0030       /* Socket K Keep-alive Timer Register */
                                /* 0x0X31 -- Reserved -- */
#define SK_RTR0    0x0032       /* Socket K Retransmission Time Register 0 */
#define SK_RTR1    0x0033       /* Socket K Retransmission Time Register 1 */
#define SK_RCR     0x0034       /* Socket K Retransmission Count Register */

                                /* End of Equates */

#endif // W5100S_DEFS_H_
