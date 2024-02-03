;*********************************************************************
;* Title: COCOIOEQU.asm
;*********************************************************************
;* Author: R. Allen Murphey
;*
;* License: Contributed 2021 by R. Allen Murphey to CoCoIO Development
;*
;* Description: CoCoIO for Color Computer Equates
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
;* 00     2021         RAM     Initial equates from Rick Ulland's Notes
;*********************************************************************

                              ; CoCoIO with Jumper set to 0xFF6x Range
CIO0CMND:   equ   $FF68       ; CoCoIO Command Register 
CIO0ADDR:   equ   $FF69       ; CoCoIO Address Register MSB
                 ;$FF6A       ; CoCoIO Address Register LSB
CIO0DATA:   equ   $FF6B       ; CoCoIO Read/Write Data Register

                              ; CoCoIO with Jumper set to 0xFF7x Range
CIO1CMND:   equ   $FF78       ; CoCoIO Command Register 
CIO1ADDR:   equ   $FF79       ; CoCoIO Address Register MSB
                 ;$FF7A       ; CoCoIO Address Register LSB
CIO1DATA:   equ   $FF7B       ; CoCoIO Read/Write Data Register

                              ; End of CoCoIO equates
