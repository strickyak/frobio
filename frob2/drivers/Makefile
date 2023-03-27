all: rblemma.os9

rblemma.os9: rblemma.h rblemma.c rblemma.asm
	cmoc  -i --os9 -O2 --function-stack=0 --switch=ifelse -S -I../.. -o rblemma.s rblemma.c
	sed -n '/SECTION.[cr]od[ea]/, /ENDSECTION/ p' < rblemma.s | egrep -v 'SECTION|EXPORT|IMPORT' > _generated_from_rblemma.s
	lwasm -o rblemma.os9  --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export  -DH6309=0   -I/home/strick/OS9/nitros9/level1/coco1/cmds/ -I/home/strick/OS9/nitros9/defs/  --format=os9 --list=rblemma.os9.list rblemma.asm 

clean:
	rm -f *.s *.list *.map *.o rblemma.os9
