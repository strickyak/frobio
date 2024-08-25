*** WARNING *** UNTESTED ***

********************************************************************
* LemMan - Magic Filesystem manager for things Lemma.
*
* 2023-09 Henry Strickland <github.com/strickyak>
* 
* MIT License
* 
* Copyright (c) 2023 Henry Strickland (a.k.a. Strick Yak)
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

         nam   LemMan
         ttl   Lem Filesystem Manager

         ifp1  
         use   defsfile
         endc  

rev      set   $01
edition  set   1

         IFNE  H6309
lang       set   Obj6309
         ELSE
lang       set   Objct
         ENDC

tylg     set   FlMgr+lang
atrv     set   ReEnt+rev

         org   $00
size     equ   .

         mod   eom,name,tylg,atrv,start,size

name     fcs   /LemMan/
         fcb   edition


****************************
*
* Main entry points
*
* Entry: Y = Path descriptor pointer
*        U = Register stack pointer

start    lbra  Lem_CreateA
         lbra  Lem_OpenA
         lbra  Lem_MakDirA
         lbra  Lem_ChgDirA
         lbra  Lem_DeleteA
         lbra  Lem_SeekA
         lbra  Lem_ReadA
         lbra  Lem_WriteA
         lbra  Lem_ReadLnA
         lbra  Lem_WritLnA
         lbra  Lem_GetStatA
         lbra  Lem_SetStatA
         lbra  Lem_CloseA

Lem_CreateA
        ldb #1
        bra Lem_bridge
Lem_OpenA
        ldb #2
        bra Lem_bridge
Lem_MakDirA
        ldb #3
        bra Lem_bridge
Lem_ChgDirA
        ldb #4
        bra Lem_bridge
Lem_DeleteA
        ldb #5
        bra Lem_bridge
Lem_SeekA
        ldb #6
        bra Lem_bridge
Lem_ReadA
        ldb #7
        bra Lem_bridge
Lem_WriteA
        ldb #8
        bra Lem_bridge
Lem_ReadLnA
        ldb #9
        bra Lem_bridge
Lem_WritLnA
        ldb #10
        bra Lem_bridge
Lem_GetStatA
        ldb #11
        bra Lem_bridge
Lem_SetStatA
        ldb #12
        bra Lem_bridge
Lem_CloseA
        ldb #13
        ; fallthrough

Lem_bridge
    clra          ; D holds the op number
    pshs u,y,x,d  ; match args to Bridge
    ldu #0        ; start new Frame Pointer

    orcc #IntMasks    ; very conservative for now
    lbsr _Bridge      ; call C function Bridge() in lemmanc.c
    andcc #^IntMasks  ; very conservative for now

    CLRA          ; clear the carry bit
    TSTB          ; test for error
    BEQ SkipComA  ; skip the COMA if no error
    COMA          ; sets the carry bit
SkipComA
    LEAS 2,S      ; drop old D from stack
    PULS x,y,u,PC

* Include compiled "lemmanc.c"
         use _generated_from_lemmanc.s

         emod  
eom      equ   *
         end
