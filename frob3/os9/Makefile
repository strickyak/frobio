_generated_os9api_for_gcc.s: os9gengcc.go os9api.go os9gen.go
	go run *.go --gcc
	gcc6809 -S -std=gnu99  _generated_os9api_for_gcc.c -fomit-frame-pointer
	sed -e /[.]module/d -e /[.]area/d -e /[.]globl/d _generated_os9api_for_gcc.s > _generated_os9api_for_gcc.asm
	lwasm _generated_os9api_for_gcc.asm --format=os9 --pragma=pcaspcr,nosymbolcase,condundefzero,undefextern,dollarnotlocal,noforwardrefmax
