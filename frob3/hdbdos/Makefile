all: test1.lemma _COPY_

S=../../..

_COPY_: test1.lemma
	cp -fv $< $S/build-frobio/results/LEMMINGS/

test1.lemma: dwread.o dwwrite.o test1.o
	$S/bin/lwlink  -o $@ --entry=_main  $^

dwread.o: dwread.asm
	$S/bin/lwasm  --obj -o $@  $<
dwwrite.o: dwwrite.asm
	$S/bin/lwasm  --obj -o $@  $<

test1.o: test1.c
	PATH="$$(cd $S/bin; pwd):$$PATH" $S/bin/gcc6809 -S -I'../..' -std=gnu99  $<
	PATH="$$(cd $S/bin; pwd):$$PATH" $S/bin/gcc6809 -c -I'../..' -std=gnu99  $<
