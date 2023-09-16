********************************************************************
* Lemmer - FUSE device driver
*
* Modified from piper.asm by Henry Strickland (github.com/strickyak)

         nam   Lemmer
         ttl   Lem device driver

         ifp1
         use   defsfile
         endc

tylg     set   Drivr+Objct
atrv     set   ReEnt+rev
rev      set   $00
edition  set   1

         mod   eom,name,tylg,atrv,start,static_ram_sz

PreDeviceVars  rmb 6   ; 6 bytes of predefined struct DeviceVars.
base_of_ram64  rmb 2   ; base page of 64-byte allocs.
static_ram_sz  equ .   ; will be rounded up to 256 anyway.

         fcb   READ.+WRITE.

name     fcs   /Lemmer/
         fcb   edition

start    equ   *

* Dispatch Relays
Init     clrb
         rts
         nop
Read     clrb  ; never called.
         comb
         rts
Write    clrb  ; never called.
         comb
         rts
GetStat  clrb  ; never called.
         comb
         rts
SetStat  clrb  ; never called.
         comb
         rts
Term     clrb
         rts
				 nop

         emod
eom      equ   *
         end
