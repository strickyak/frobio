all: test.cciodw

#F=$$HOME/coco-shelf/frobio/frob3
#
#OBJS = buf.o flag.o format.o malloc.o nylib.o nystdio.o std.o
#LIBS = $F/froblib/buf.c $F/froblib/flag.c $F/froblib/format.c $F/froblib/malloc.c $F/froblib/nylib.c $F/froblib/nystdio.c $F/froblib/std.c

test.cciodw: cciodw.o test-cciodw.o
	#for x in $(LIBS); do (PATH=/home/strick/coco-shelf/bin/:$$PATH $$HOME/coco-shelf/bin/gcc6809 -c -I'../..' -std=gnu99 -DBASENAME="$(basename $$x .c)"  $$x); done
	#$$HOME/coco-shelf/bin/lwlink  -o $@  $^  -L'/home/strick/coco-shelf/lib/gcc/m6809-unknown/4.6.4/' $(OBJS) -lgcc
	$$HOME/coco-shelf/bin/lwlink  -o $@  $^

cciodw.o: cciodw.asm
	$$HOME/coco-shelf/bin/lwasm  --obj -o $@  $<

test-cciodw.o: test-cciodw.c
	PATH=/home/strick/coco-shelf/bin/:$$PATH $$HOME/coco-shelf/bin/gcc6809 -S -I'../..' -std=gnu99  $<
	PATH=/home/strick/coco-shelf/bin/:$$PATH $$HOME/coco-shelf/bin/gcc6809 -c -I'../..' -std=gnu99  $<
