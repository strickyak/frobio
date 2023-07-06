********************************************************************
* FuseMan - User-mode Filesystem manager for OS9
*
* 2020-03 Henry Strickland <github.com/strickyak>
* 
* MIT License
* 
* Copyright (c) 2021 Strick Yak
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* 

         nam   FuseMan
         ttl   Fuse Filesystem Manager

         ifp1  
         use   defsfile
         endc  

rev      set   $01
ty       set   FlMgr
         IFNE  H6309
lg       set   Obj6309
         ELSE
lg       set   Objct
         ENDC
tylg     set   ty+lg
atrv     set   ReEnt+rev
edition  set   1

         org   $00
size     equ   .

         mod   eom,name,tylg,atrv,start,size

name     fcs   /FuseMan/
         fcb   edition


****************************
*
* Main entry points
*
* Entry: Y = Path descriptor pointer
*        U = Register stack pointer

start    lbra  _CreateOrOpenA
         lbra  _CreateOrOpenA
         lbra  _MakDirA
         lbra  _ChgDirA
         lbra  _DeleteA
         lbra  _SeekA
         lbra  _ReadA
         lbra  _WriteA
         lbra  _ReadLnA
         lbra  _WritLnA
         lbra  _GetStatA
         lbra  _SetStatA
         lbra  _CloseA

BackToAssembly
    CLRA     ; clear the carry bit.
    TSTB     ; we want to set carry if B nonzero.
    BEQ SkipComA  ; skip the COMA
    COMA     ; sets the carry bit.
SkipComA
    PULS PC,U,Y

* Include compiled "fusec.c"

         use _generated_from_fusec_.s

         emod  
eom      equ   *
         end

