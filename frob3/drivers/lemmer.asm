********************************************************************
* Lemmer -- Device Driver Stub for LemMan, the Lemma Remote File Mananger.
*
* Modified from piper.asm by Henry Strickland (github.com/strickyak)

         nam   Lemmer
         ttl   Lem device driver

         ifp1
         use   defsfile
         endc

rev      set   $00
edition  set   1

tylg     set   Drivr+Objct
atrv     set   ReEnt+rev

         org 0
PreDeviceVars  rmb 6   ; 6 bytes of predefined struct DeviceVars.
base_of_ram64  rmb 2   ; base page of 64-byte allocs.
static_ram_sz  equ .   ; will be rounded up to 256 anyway.

         mod   eom,name,tylg,atrv,start,static_ram_sz

         fcb   READ.+WRITE.+DIR.

name     fcs   /Lemmer/
         fcb   edition

start    equ   *

* Dispatch Relays
Init     clrb
         rts
         daa
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
	 daa

         emod
eom      equ   *
         end
