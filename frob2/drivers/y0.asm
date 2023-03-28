********************************************************************
* Y0 : LEMMA Block Disk Device 0
*
* Modified from pipe.asm by Henry Strickland (github.com/strickyak)

         nam   Y0
         ttl   Y0 lemma block disk device descriptor

         ifp1  
         use   defsfile
         endc  

tylg     set   Devic+Objct
atrv     set   ReEnt+rev
rev      set   $00

         mod   eom,name,tylg,atrv,mgrnam,drvnam

         ;;;; fcb   READ.+WRITE. ; mode byte
         fcb   $87          ; mode byte
         fcb   $00          ; M$PORT[3]: $0E: extended controller address
         fdb   $0000        ;                 physical controller address
         fcb   iEnd-iBegin  ; M$Opt:  $11: initialization table size

				 ; Initialization Table
iBegin   equ   *
         fcb   DT.RBF       ; M$DTyp: $12: device type
         fcb   0            ; IT.DRV: $13: drive number
         fcb   1            ; IT.STP: $14: step rate
         fcb   TYP.HARD     ; IT.TYP: $15: step rate
iEnd     equ   *

name     fcs   /DD/         ; /Y0/ is the default disk, so /DD/.
mgrnam   fcs   /RBF/
drvnam   fcs   /RBLemma/

         emod  
eom      equ   *
         end   
