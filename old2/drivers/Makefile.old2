all: rblemma.os9

rblemma.os9: rblemmac.h rblemmac.c rblemma.asm
	cmoc  -i --os9 -O2 --function-stack=0 --switch=ifelse -S -I../.. -o rblemmac.s rblemmac.c
	sed -n '/SECTION.[cr]od[ea]/, /ENDSECTION/ p' < rblemmac.s | egrep -v 'SECTION|EXPORT|IMPORT' > _generated_from_rblemmac.s
	lwasm -o rblemma.os9  --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export  -DH6309=0   -I$$SHELF/nitros9/level1/coco1/cmds/ -I$$SHELF/nitros9/defs/  --format=os9 --list=rblemma.os9.list rblemma.asm
	lwasm -o ddb0.os9  --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export  -DH6309=0   -I$$SHELF/nitros9/level1/coco1/cmds/ -I$$SHELF/nitros9/defs/  --format=os9 --list=ddb0.os9.list ddb0.asm
	lwasm -o b0.os9  --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export  -DH6309=0   -I$$SHELF/nitros9/level1/coco1/cmds/ -I$$SHELF/nitros9/defs/  --format=os9 --list=b0.os9.list b0.asm
	lwasm -o b1.os9  --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export  -DH6309=0   -I$$SHELF/nitros9/level1/coco1/cmds/ -I$$SHELF/nitros9/defs/  --format=os9 --list=b1.os9.list b1.asm
	lwasm -o b2.os9  --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export  -DH6309=0   -I$$SHELF/nitros9/level1/coco1/cmds/ -I$$SHELF/nitros9/defs/  --format=os9 --list=b2.os9.list b2.asm
	lwasm -o b3.os9  --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export  -DH6309=0   -I$$SHELF/nitros9/level1/coco1/cmds/ -I$$SHELF/nitros9/defs/  --format=os9 --list=b3.os9.list b3.asm
	lwasm -o n_fuse.os9  --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export  -DH6309=0   -I$$SHELF/nitros9/level1/coco1/cmds/ -I$$SHELF/nitros9/defs/  --format=os9 --list=n_fuse.os9.list n_fuse.asm

clean:
	rm -f *.s *.list *.map *.o *.os9 _generated_*
