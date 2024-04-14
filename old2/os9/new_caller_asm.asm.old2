  SECTION code

  EXPORT _MakeNewCall
  EXPORT _FinishNewCall

_MakeNewCall
  pshs y,u     ; STASH Y,U

  ldu 6,s      ; struct new_caller*

  leau 6,u     ; & .dab
  pulu d,x,y

  ldu 6,s      ; struct new_caller*
  ldu 12,u     ; & .u

  jmp [6,s]      ; .code: execute the os9 system call.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_FinishNewCall
  pshs u
  ldu 8,s      ; struct new_caller*
  leau 12,u    ; points to .u which is after .y
  pshu d,x,y   ; saves to .y, .x, .d
  bcc OkNewCall

  ldu 8,s      ; struct new_caller*
  stb 15,u     ; & .err

OkNewCall
  ldu 8,s      ; struct new_caller*
  puls x       ; was U
  stx 12,u     ; saves to .u

  puls y,u,pc  ; RESTORE STASHED Y,U and RETURN.

  ENDSECTION code
