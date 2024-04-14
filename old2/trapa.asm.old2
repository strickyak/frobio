  IMPORT  _trapped_sp
  IMPORT  _DecbTrap
  EXPORT  TRAP

  SECTION code
TRAP
  pshs cc,dp,a,b,x,y,u
  tfr  s,x
  stx  _trapped_sp
  jsr  _DecbTrap
  ldx  _trapped_sp
  tfr  x,s
  puls cc,dp,a,b,x,y,u,pc
  ENDSECTION
