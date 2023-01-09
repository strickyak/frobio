all: fuseman.os9 fuser.os9 fuse.os9

CMOC=cmoc
LWASM=lwasm

_generated_from_fusec_.a : fusec.c ../fuse.h
	$(CMOC) -i --os9 -O2 --function-stack=0 --switch=ifelse -I'../../..' -S fusec.c
	sed -n '/SECTION.[cr]od[ea]/, /ENDSECTION/ p' < fusec.s | egrep -v 'SECTION|EXPORT' > _generated_from_fusec_.a

fuseman.os9: _generated_from_fusec_.a fuseman.asm defsfile
	$(LWASM) --6809 --format=os9 -l'$@.list' -I'../../..' -D'H6309=0' -o '$@' fuseman.asm

fuser.os9: fuser.asm defsfile
	$(LWASM) --6809 --format=os9 -l'$@.list' -I'../../..' -D'H6309=0' -o '$@' fuser.asm

fuse.os9: fuse.asm defsfile
	$(LWASM) --6809 --format=os9 -l'$@.list' -I'../../..' -D'H6309=0' -o '$@' fuse.asm

clean:
	rm -f *.o *.s *.list *.lst *.os9 *.map *.link _generated_from_fusec_.*
