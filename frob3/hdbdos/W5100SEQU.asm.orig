;*********************************************************************
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
;* 00     2021         RAM     Initial equates from W5100S Data Sheet
;*********************************************************************

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

                              ; COMMON REGISTERS 0x0000-0x0088
MR:         equ   $0000       ; Mode Register
GAR0:       equ   $0001       ; Gateway Address Register 0
GAR1:       equ   $0002       ; Gateway Address Register 1
GAR2:       equ   $0003       ; Gateway Address Register 2
GAR3:       equ   $0004       ; Gateway Address Register 3
SUBR0:      equ   $0005       ; Subnet Mask Address Register 0
SUBR1:      equ   $0006       ; Subnet Mask Address Register 1
SUBR2:      equ   $0007       ; Subnet Mask Address Register 2
SUBR3:      equ   $0008       ; Subnet Mask Address Register 3
SHAR0:      equ   $0009       ; Source Hardware Address Register 0
SHAR1:      equ   $000A       ; Source Hardware Address Register 1
SHAR2:      equ   $000B       ; Source Hardware Address Register 2
SHAR3:      equ   $000C       ; Source Hardware Address Register 3
SHAR4:      equ   $000D       ; Source Hardware Address Register 4
SHAR5:      equ   $000E       ; Source Hardware Address Register 5
SIPR0:      equ   $000F       ; Source IP Address Register 0
SIPR1:      equ   $0010       ; Source IP Address Register 1
SIPR2:      equ   $0011       ; Source IP Address Register 2
SIPR3:      equ   $0012       ; Source IP Address Register 3
INTPTMR0:   equ   $0013       ; Interrupt Pending Time Register 0
INTPTMR1:   equ   $0014       ; Interrupt Pending Time Register 1
IR:         equ   $0015       ; Interrupt Register
IMR:        equ   $0016       ; Interrupt Mask Register
RTR0:       equ   $0017       ; Retransmission Time Register 0
RTR1:       equ   $0018       ; Retransmission Time Register 1
RCR:        equ   $0019       ; Retransmission Count Register
RMSR:       equ   $001A       ; RX Memory Size Register
TMSR:       equ   $001B       ; TX Memory Size Register
                              ; 0x001C-0x001F -- Reserved --
IR2:        equ   $0020       ; Interrupt2 Register
IMR2:       equ   $0021       ; Interrupt2 Mask Register
                              ; 0x0022-0x0027 -- Reserved --
PTIMER:     equ   $0028       ; PPP LCP Request Timer
PMAGIC:     equ   $0029       ; PPP LCP Magic Number
UIPR0:      equ   $002A       ; Unreachable IP Address Register 0
UIPR1:      equ   $002B       ; Unreachable IP Address Register 1
UIPR2:      equ   $002C       ; Unreachable IP Address Register 2
UIPR3:      equ   $002D       ; Unreachable IP Address Register 0
UPORTR0:    equ   $002E       ; Unreachable Port Register 0
UPORTR1:    equ   $002F       ; Unreachable Port Register 1
MR2:        equ   $0030       ; Mode2 Register
                              ; 0x0031 --  Reserved --
PHAR0:      equ   $0032       ; Destination Hardware Address on PPPoE Register 0
PHAR1:      equ   $0033       ; Destination Hardware Address on PPPoE Register 1
PHAR2:      equ   $0034       ; Destination Hardware Address on PPPoE Register 2
PHAR3:      equ   $0035       ; Destination Hardware Address on PPPoE Register 3
PHAR4:      equ   $0036       ; Destination Hardware Address on PPPoE Register 4
PHAR5:      equ   $0037       ; Destination Hardware Address on PPPoE Register 5
PSIDR0:     equ   $0038       ; Session ID on PPPoE Register 0
PSIDR1:     equ   $0039       ; Session ID on PPPoE Register 1
PMRUR0:     equ   $003A       ; Maximum Receive Unit on PPPoE Register 0
PMRUR1:     equ   $003B       ; Maximum Receive Unit on PPPoE Register 1
PHYSR0:     equ   $003C       ; PHY Status Register 0
PHYSR1:     equ   $003D       ; PHY Status Register 1
PHYAR:      equ   $003E       ; PHY Address Value
PHYRAR:     equ   $003F       ; PHY Register Address
PHYDIR0:    equ   $0040       ; PHY Data Input Register 0
PHYDIR1:    equ   $0041       ; PHY Data Input Register 1
PHYDOR0:    equ   $0042       ; PHY Data Output Register 0
PHYDOR1:    equ   $0043       ; PHY Data Output Register 1
PHYACR:     equ   $0044       ; PHY Access Register
PHYDIVR     equ   $0045       ; PHY Division Register
PHYCR0:     equ   $0046       ; PHY Control Register 0
PHYCR1:     equ   $0047       ; PHY Control Register 1
                              ; 0x0048-0x004B -- Reserved --
SLCR:       equ   $004C       ; SOCKET-less Command Register
SLRTR0:     equ   $004D       ; SOCKET-less Retransmission Time Register 0
SLRTR1:     equ   $004E       ; SOCKET-less Retransmission Time Register 1
SLRCR:      equ   $004F       ; SOCKET-less Retransmission Count Register
SLPIPR0:    equ   $0050       ; SOCKET-less Peer IP Address Register 0
SLPIPR1:    equ   $0051       ; SOCKET-less Peer IP Address Register 1
SLPIPR2:    equ   $0052       ; SOCKET-less Peer IP Address Register 2
SLPIPR3:    equ   $0053       ; SOCKET-less Peer IP Address Register 3
SLPHAR0:    equ   $0054       ; SOCKET-less Peer Hardware Address Register 0
SLPHAR1:    equ   $0055       ; SOCKET-less Peer Hardware Address Register 1
SLPHAR2:    equ   $0056       ; SOCKET-less Peer Hardware Address Register 2
SLPHAR3:    equ   $0057       ; SOCKET-less Peer Hardware Address Register 3
SLPHAR4:    equ   $0058       ; SOCKET-less Peer Hardware Address Register 4
SLPHAR5:    equ   $0059       ; SOCKET-less Peer Hardware Address Register 5
PINGSEQR0:  equ   $005A       ; PING Sequence Number Register 0
PINGSEQR1:  equ   $005B       ; PING Sequence Number Register 1
PINGIDR0:   equ   $005C       ; PING ID Register 0
PINGIDR1:   equ   $005D       ; PING ID Register 1
SLIMR:      equ   $005E       ; SOCKET-less Interrupt Mask Register
SLIR:       equ   $005F       ; SOCKET-less Interrupt Register
                              ; 0x0060-0x006A -- Reserved --
CLKLCKR:    equ   $0070       ; Clock Lock Register
NETLCKR:    equ   $0071       ; Network Lock Register
PHYLCKR:    equ   $0072       ; PHY Lock Register
                              ; 0x0073-0x007F -- Reserved --
VERR:       equ   $0080       ; Chip Version
                              ; 0x0081 -- Reserved --
TCNTR0:     equ   $0082       ; 100us Ticker Counter Register 0
TCNTR1:     equ   $0083       ; 100us Ticker Counter Register 1
                              ; 0x0084-0x0087 -- Reserved --
TCNTCLR:    equ   $0088       ; 100us Ticker Counter Clear Register

                              ; SOCKET 0 REGISTERS 0x0400-0x04FF 45 bytes per socket
S0_MR:      equ   $0400       ; Socket 0 Mode Register
S0_CR:      equ   $0401       ; Socket 0 Command Register
S0_IR:      equ   $0402       ; Socket 0 Interrupt Register
S0_SR:      equ   $0403       ; Socket 0 Status Register
S0_PORTR0:  equ   $0404       ; Socket 0 Source Port Register 0
S0_PORTR1:  equ   $0405       ; Socket 0 Source Port Register 1
S0_DHAR0:   equ   $0406       ; Socket 0 Destination Hardware Address Register 0
S0_DHAR1:   equ   $0407       ; Socket 0 Destination Hardware Address Register 1
S0_DHAR2:   equ   $0408       ; Socket 0 Destination Hardware Address Register 2
S0_DHAR3:   equ   $0409       ; Socket 0 Destination Hardware Address Register 3
S0_DHAR4:   equ   $040A       ; Socket 0 Destination Hardware Address Register 4
S0_DHAR5:   equ   $040B       ; Socket 0 Destination Hardware Address Register 5
S0_DIPR0:   equ   $040C       ; Socket 0 Destination IP Address Register 0
S0_DIPR1:   equ   $040D       ; Socket 0 Destination IP Address Register 1
S0_DIPR2:   equ   $040E       ; Socket 0 Destination IP Address Register 2
S0_DIPR3:   equ   $040F       ; Socket 0 Destination IP Address Register 3
S0_DPORTR0: equ   $0410       ; Socket 0 Destination Port Register 0
S0_DPORTR1: equ   $0411       ; Socket 0 Destination Port Register 1
S0_MSS0:    equ   $0412       ; Socket 0 Maximum Segment Size Register 0
S0_MSS1:    equ   $0413       ; Socket 0 Maximum Segment Size Register 1
S0_PROTOR:  equ   $0414       ; Socket 0 IP Protocol Register
S0_TOS:     equ   $0415       ; Socket 0 IP Type Of Service
S0_TTL:     equ   $0416       ; Socket 0 IP Time To Live
                              ; 0x0417-0x041D -- Reserved --
S0_RXBUF_SIZE:  equ  $041E    ; Socket 0 RX Buffer Size
S0_TXBUF_SIZE:  equ  $041F    ; Socket 0 TX Buffer Size
S0_TX_FSR0: equ   $0420       ; Socket 0 TX Free Size Register 0
S0_TX_FSR1: equ   $0421       ; Socket 0 TX Free Size Register 1
S0_TX_RD0:  equ   $0422       ; Socket 0 TX Read Pointer 0
S0_TX_RD1:  equ   $0423       ; Socket 0 TX Read Pointer 1
S0_TX_WR0:  equ   $0424       ; Socket 0 TX Write Pointer 0
S0_TX_WR1:  equ   $0425       ; Socket 0 TX Write Pointer 1
S0_RX_RSR0: equ   $0426       ; Socket 0 RX Received Size Register 0
S0_RX_RSR1: equ   $0427       ; Socket 0 RX Received Size Register 1
S0_RX_RD0:  equ   $0428       ; Socket 0 RX Read Pointer 0
S0_RX_RD1:  equ   $0429       ; Socket 0 RX Read Pointer 1
S0_RX_WR0:  equ   $042A       ; Socket 0 RX Write Pointer 0
S0_RX_WR1:  equ   $042B       ; Socket 0 RX Write Pointer 1
S0_IMR:     equ   $042C       ; Socket 0 Interrupt Mask Register
S0_FRAGR0:  equ   $042D       ; Socket 0 Fragment Offset in IP Header Register 0
S0_FRAGR1:  equ   $042E       ; Socket 0 Fragment Offset in IP Header Register 1
S0_MR2:     equ   $042F       ; Socket 0 Mode 2 Register
S0_KPALVTR: equ   $0430       ; Socket 0 Keep-alive Timer Register
                              ; 0x0431 -- Reserved --
S0_RTR0:    equ   $0432       ; Socket 0 Retransmission Time Register 0
S0_RTR1:    equ   $0433       ; Socket 0 Retransmission Time Register 1
S0_RCR:     equ   $0434       ; Socket 0 Retransmission Count Register

                              ; SOCKET 1 REGISTERS 0x0500-0x05FF 45 bytes per socket
S1_MR:      equ   $0500       ; Socket 1 Mode Register
S1_CR:      equ   $0501       ; Socket 1 Command Register
S1_IR:      equ   $0502       ; Socket 1 Interrupt Register
S1_SR:      equ   $0503       ; Socket 1 Status Register
S1_PORTR0:  equ   $0504       ; Socket 1 Source Port Register 0
S1_PORTR1:  equ   $0505       ; Socket 1 Source Port Register 1
S1_DHAR0:   equ   $0506       ; Socket 1 Destination Hardware Address Register 0
S1_DHAR1:   equ   $0507       ; Socket 1 Destination Hardware Address Register 1
S1_DHAR2:   equ   $0508       ; Socket 1 Destination Hardware Address Register 2
S1_DHAR3:   equ   $0509       ; Socket 1 Destination Hardware Address Register 3
S1_DHAR4:   equ   $050A       ; Socket 1 Destination Hardware Address Register 4
S1_DHAR5:   equ   $050B       ; Socket 1 Destination Hardware Address Register 5
S1_DIPR0:   equ   $050C       ; Socket 1 Destination IP Address Register 0
S1_DIPR1:   equ   $050D       ; Socket 1 Destination IP Address Register 1
S1_DIPR2:   equ   $050E       ; Socket 1 Destination IP Address Register 2
S1_DIPR3:   equ   $050F       ; Socket 1 Destination IP Address Register 3
S1_DPORTR0: equ   $0510       ; Socket 1 Destination Port Register 0
S1_DPORTR1: equ   $0511       ; Socket 1 Destination Port Register 1
S1_MSS0:    equ   $0512       ; Socket 1 Maximum Segment Size Register 0
S1_MSS1:    equ   $0513       ; Socket 1 Maximum Segment Size Register 1
S1_PROTOR:  equ   $0514       ; Socket 1 IP Protocol Register
S1_TOS:     equ   $0515       ; Socket 1 IP Type Of Service
S1_TTL:     equ   $0516       ; Socket 1 IP Time To Live
                              ; 0x0517-0x051D -- Reserved --
S1_RXBUF_SIZE:  equ  $051E    ; Socket 1 RX Buffer Size
S1_TXBUF_SIZE:  equ  $051F    ; Socket 1 TX Buffer Size
S1_TX_FSR0: equ   $0520       ; Socket 1 TX Free Size Register 0
S1_TX_FSR1: equ   $0521       ; Socket 1 TX Free Size Register 1
S1_TX_RD0:  equ   $0522       ; Socket 1 TX Read Pointer 0
S1_TX_RD1:  equ   $0523       ; Socket 1 TX Read Pointer 1
S1_TX_WR0:  equ   $0524       ; Socket 1 TX Write Pointer 0
S1_TX_WR1:  equ   $0525       ; Socket 1 TX Write Pointer 1
S1_RX_RSR0: equ   $0526       ; Socket 1 RX Received Size Register 0
S1_RX_RSR1: equ   $0527       ; Socket 1 RX Received Size Register 1
S1_RX_RD0:  equ   $0528       ; Socket 1 RX Read Pointer 0
S1_RX_RD1:  equ   $0529       ; Socket 1 RX Read Pointer 1
S1_RX_WR0:  equ   $052A       ; Socket 1 RX Write Pointer 0
S1_RX_WR1:  equ   $052B       ; Socket 1 RX Write Pointer 1
S1_IMR:     equ   $052C       ; Socket 1 Interrupt Mask Register
S1_FRAGR0:  equ   $052D       ; Socket 1 Fragment Offset in IP Header Register 0
S1_FRAGR1:  equ   $052E       ; Socket 1 Fragment Offset in IP Header Register 1
S1_MR2:     equ   $052F       ; Socket 1 Mode 2 Register
S1_KPALVTR: equ   $0530       ; Socket 1 Keep-alive Timer Register
                              ; 0x0531 -- Reserved --
S1_RTR0:    equ   $0532       ; Socket 1 Retransmission Time Register 0
S1_RTR1:    equ   $0533       ; Socket 1 Retransmission Time Register 1
S1_RCR:     equ   $0534       ; Socket 1 Retransmission Count Register

                              ; SOCKET 2 REGISTERS 0x0600-0x06FF 45 bytes per socket
S2_MR:      equ   $0600       ; Socket 2 Mode Register
S2_CR:      equ   $0601       ; Socket 2 Command Register
S2_IR:      equ   $0602       ; Socket 2 Interrupt Register
S2_SR:      equ   $0603       ; Socket 2 Status Register
S2_PORTR0:  equ   $0604       ; Socket 2 Source Port Register 0
S2_PORTR1:  equ   $0605       ; Socket 2 Source Port Register 1
S2_DHAR0:   equ   $0606       ; Socket 2 Destination Hardware Address Register 0
S2_DHAR1:   equ   $0607       ; Socket 2 Destination Hardware Address Register 1
S2_DHAR2:   equ   $0608       ; Socket 2 Destination Hardware Address Register 2
S2_DHAR3:   equ   $0609       ; Socket 2 Destination Hardware Address Register 3
S2_DHAR4:   equ   $060A       ; Socket 2 Destination Hardware Address Register 4
S2_DHAR5:   equ   $060B       ; Socket 2 Destination Hardware Address Register 5
S2_DIPR0:   equ   $060C       ; Socket 2 Destination IP Address Register 0
S2_DIPR1:   equ   $060D       ; Socket 2 Destination IP Address Register 1
S2_DIPR2:   equ   $060E       ; Socket 2 Destination IP Address Register 2
S2_DIPR3:   equ   $060F       ; Socket 2 Destination IP Address Register 3
S2_DPORTR0: equ   $0610       ; Socket 2 Destination Port Register 0
S2_DPORTR1: equ   $0611       ; Socket 2 Destination Port Register 1
S2_MSS0:    equ   $0612       ; Socket 2 Maximum Segment Size Register 0
S2_MSS1:    equ   $0613       ; Socket 2 Maximum Segment Size Register 1
S2_PROTOR:  equ   $0614       ; Socket 2 IP Protocol Register
S2_TOS:     equ   $0615       ; Socket 2 IP Type Of Service
S2_TTL:     equ   $0616       ; Socket 2 IP Time To Live
                              ; 0x0617-0x061D -- Reserved --
S2_RXBUF_SIZE:  equ  $061E    ; Socket 2 RX Buffer Size
S2_TXBUF_SIZE:  equ  $061F    ; Socket 2 TX Buffer Size
S2_TX_FSR0: equ   $0620       ; Socket 2 TX Free Size Register 0
S2_TX_FSR1: equ   $0621       ; Socket 2 TX Free Size Register 1
S2_TX_RD0:  equ   $0622       ; Socket 2 TX Read Pointer 0
S2_TX_RD1:  equ   $0623       ; Socket 2 TX Read Pointer 1
S2_TX_WR0:  equ   $0624       ; Socket 2 TX Write Pointer 0
S2_TX_WR1:  equ   $0625       ; Socket 2 TX Write Pointer 1
S2_RX_RSR0: equ   $0626       ; Socket 2 RX Received Size Register 0
S2_RX_RSR1: equ   $0627       ; Socket 2 RX Received Size Register 1
S2_RX_RD0:  equ   $0628       ; Socket 2 RX Read Pointer 0
S2_RX_RD1:  equ   $0629       ; Socket 2 RX Read Pointer 1
S2_RX_WR0:  equ   $062A       ; Socket 2 RX Write Pointer 0
S2_RX_WR1:  equ   $062B       ; Socket 2 RX Write Pointer 1
S2_IMR:     equ   $062C       ; Socket 2 Interrupt Mask Register
S2_FRAGR0:  equ   $062D       ; Socket 2 Fragment Offset in IP Header Register 0
S2_FRAGR1:  equ   $062E       ; Socket 2 Fragment Offset in IP Header Register 1
S2_MR2:     equ   $062F       ; Socket 2 Mode 2 Register
S2_KPALVTR: equ   $0630       ; Socket 2 Keep-alive Timer Register
                              ; 0x0631 -- Reserved --
S2_RTR0:    equ   $0632       ; Socket 2 Retransmission Time Register 0
S2_RTR1:    equ   $0633       ; Socket 2 Retransmission Time Register 1
S2_RCR:     equ   $0634       ; Socket 2 Retransmission Count Register

                              ; SOCKET 3 REGISTERS 0x0700-0x07FF 45 bytes per socket
S3_MR:      equ   $0700       ; Socket 3 Mode Register
S3_CR:      equ   $0701       ; Socket 3 Command Register
S3_IR:      equ   $0702       ; Socket 3 Interrupt Register
S3_SR:      equ   $0703       ; Socket 3 Status Register
S3_PORTR0:  equ   $0704       ; Socket 3 Source Port Register 0
S3_PORTR1:  equ   $0705       ; Socket 3 Source Port Register 1
S3_DHAR0:   equ   $0706       ; Socket 3 Destination Hardware Address Register 0
S3_DHAR1:   equ   $0707       ; Socket 3 Destination Hardware Address Register 1
S3_DHAR2:   equ   $0708       ; Socket 3 Destination Hardware Address Register 2
S3_DHAR3:   equ   $0709       ; Socket 3 Destination Hardware Address Register 3
S3_DHAR4:   equ   $070A       ; Socket 3 Destination Hardware Address Register 4
S3_DHAR5:   equ   $070B       ; Socket 3 Destination Hardware Address Register 5
S3_DIPR0:   equ   $070C       ; Socket 3 Destination IP Address Register 0
S3_DIPR1:   equ   $070D       ; Socket 3 Destination IP Address Register 1
S3_DIPR2:   equ   $070E       ; Socket 3 Destination IP Address Register 2
S3_DIPR3:   equ   $070F       ; Socket 3 Destination IP Address Register 3
S3_DPORTR0: equ   $0710       ; Socket 3 Destination Port Register 0
S3_DPORTR1: equ   $0711       ; Socket 3 Destination Port Register 1
S3_MSS0:    equ   $0712       ; Socket 3 Maximum Segment Size Register 0
S3_MSS1:    equ   $0713       ; Socket 3 Maximum Segment Size Register 1
S3_PROTOR:  equ   $0714       ; Socket 3 IP Protocol Register
S3_TOS:     equ   $0715       ; Socket 3 IP Type Of Service
S3_TTL:     equ   $0716       ; Socket 3 IP Time To Live
                              ; 0x0717-0x071D -- Reserved --
S3_RXBUF_SIZE:  equ  $071E    ; Socket 3 RX Buffer Size
S3_TXBUF_SIZE:  equ  $071F    ; Socket 3 TX Buffer Size
S3_TX_FSR0: equ   $0720       ; Socket 3 TX Free Size Register 0
S3_TX_FSR1: equ   $0721       ; Socket 3 TX Free Size Register 1
S3_TX_RD0:  equ   $0722       ; Socket 3 TX Read Pointer 0
S3_TX_RD1:  equ   $0723       ; Socket 3 TX Read Pointer 1
S3_TX_WR0:  equ   $0724       ; Socket 3 TX Write Pointer 0
S3_TX_WR1:  equ   $0725       ; Socket 3 TX Write Pointer 1
S3_RX_RSR0: equ   $0726       ; Socket 3 RX Received Size Register 0
S3_RX_RSR1: equ   $0727       ; Socket 3 RX Received Size Register 1
S3_RX_RD0:  equ   $0728       ; Socket 3 RX Read Pointer 0
S3_RX_RD1:  equ   $0729       ; Socket 3 RX Read Pointer 1
S3_RX_WR0:  equ   $072A       ; Socket 3 RX Write Pointer 0
S3_RX_WR1:  equ   $072B       ; Socket 3 RX Write Pointer 1
S3_IMR:     equ   $072C       ; Socket 3 Interrupt Mask Register
S3_FRAGR0:  equ   $072D       ; Socket 3 Fragment Offset in IP Header Register 0
S3_FRAGR1:  equ   $072E       ; Socket 3 Fragment Offset in IP Header Register 1
S3_MR2:     equ   $072F       ; Socket 3 Mode 2 Register
S3_KPALVTR: equ   $0730       ; Socket 3 Keep-alive Timer Register
                              ; 0x0731 -- Reserved --
S3_RTR0:    equ   $0732       ; Socket 3 Retransmission Time Register 0
S3_RTR1:    equ   $0733       ; Socket 3 Retransmission Time Register 1
S3_RCR:     equ   $0734       ; Socket 3 Retransmission Count Register

                              ; End of Equates
