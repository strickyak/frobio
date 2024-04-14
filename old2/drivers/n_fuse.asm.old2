********************************************************************
; A device named /N/ that uses FuseMan/Fuse instead of Drivewire.
; (we plan to fix FuseMan to recoginze non-/FUSE/* paths.
********************************************************************
         nam   N_for_FUSE
         ttl   N_for_FUSE disk device descriptor

         ifp1  
         use   defsfile
         endc  

tylg     set   Devic+Objct
atrv     set   ReEnt+rev
rev      set   $00

         mod   eom,name,tylg,atrv,mgrnam,drvnam

         fcb   $87          ; mode byte
         fcb   $00          ; M$PORT[3]: $0E: extended controller address
         fdb   $0000        ;                 physical controller address
         fcb   iEnd-iBegin  ; M$Opt:  $11: initialization table size

				 ; Initialization Table
iBegin   equ   *
         fcb   23           ; M$DTyp: $12: device type
         fcb   0            ; IT.DRV: $13: drive number
         fcb   1            ; IT.STP: $14: step rate
         fcb   TYP.HARD     ; IT.TYP: $15: hard drive
iEnd     equ   *

name     fcs   /N/
mgrnam   fcs   /FuseMan/
drvnam   fcs   /Fuser/

         emod  
eom      equ   *
         end   
