PROD= f.arp.os9 f.config.os9 f.dhcp.os9 f.dump.os9 f.dig.os9 f.ntp.os9 f.ping.os9 f.tget.os9 f.send.os9 f.recv.os9 f.ticks.os9 f.telnetd0.os9
DEMO= x.logging.os9 x.fputs.os9

all: $(DEMO) $(PROD)
	sync; sync

HDRS=froblib.h frobnet.h frobos9.h froblib/malloc.h match/util.h ncl/ncl.h ncl/ncl_os9.h os9/os9defs.h regexp/re.h wiz/wiz5100s.h
LIBS=wiz/wiz5100s.c froblib/buf.c froblib/flag.c froblib/format.c froblib/malloc.c froblib/nylib.c froblib/nystdio.c froblib/std.c os9/frobos9.c stack300.asm

.SUFFIXES:  # Use no builtin rules.

# f-*.c are our intended frobio commands.
f.arp.os9 : f-arp.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.config.os9 : f-config.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.dhcp.os9 : f-dhcp.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.dump.os9 : f-dump.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.dig.os9 : f-dig.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.ntp.os9 : f-ntp.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.ping.os9 : f-ping.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.tget.os9 : f-tget.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.send.os9 : f-send.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.recv.os9 : f-recv.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.ticks.os9 : f-ticks.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)
f.telnetd0.os9 : f-telnetd0.c $(HDRS) $(LIBS)
	sh Compile.sh $@ $< $(LIBS)

# x-*.c are dev or demo files.
x.argv.os9 : x-argv.c
	sh Compile.sh $@ $< $(LIBS)
x.cat.os9 : x-cat.c
	sh Compile.sh $@ $< $(LIBS)
x.logging.os9 : x-logging.c
	sh Compile.sh $@ $< $(LIBS)
x.fputs.os9 : x-fputs.c
	sh Compile.sh $@ $< $(LIBS)
x.trynewcaller.os9 : try_new_caller.c
	CMOC_PRE='-DUSE_NEW_CALLER=1' sh Compile.sh $@ $< os9/new_caller.c os9/new_caller_asm.asm $(LIBS)
x.decb0.decb : x-decb0.c
	HOW=$${HOW:-decb-cmoc} sh Compile.sh $@ $< $(LIBS)

f.b-unix:
	gcc -I.. -g -fsanitize=address -static-libasan   f-b.c $$(ls tweb/*.c | grep -v _test) froblib/*.c unix/*.c
	./a.out -a tweb/testdata/index.fm


netboots: netboot.cmoc.exp netboot.gcc.exp netboot.smallest.exp

netboot.cmoc.exp:
	sh RomCompile.sh bootrom/netboot.c bootrom/common.c  bootrom/tables.c  bootrom/checksum.c
	mv netboot.exp netboot.cmoc.exp
netboot.gcc.exp:
	sh RomCompile.sh bootrom/netboot.c bootrom/common.c  bootrom/tables.c  bootrom/checksum.c
	mv netboot.exp netboot.gcc.exp
netboot.smallest.exp:
	FRAME=0 OPT=-Os COMPILER=whole-gcc sh RomCompile.sh bootrom/netboot.c bootrom/common.c  bootrom/tables.c  bootrom/checksum.c
	mv netboot.exp netboot.smallest.exp


netboot2s: netboot2.cmoc.exp netboot2.gcc.exp netboot2.smallest.exp

netboot2.cmoc.exp:
	sh RomCompile.sh bootrom/netboot2.c bootrom/common.c  bootrom/tables.c  bootrom/checksum.c
	mv netboot2.exp netboot2.cmoc.exp
netboot2.gcc.exp:
	sh RomCompile.sh bootrom/netboot2.c bootrom/common.c  bootrom/tables.c  bootrom/checksum.c
	mv netboot2.exp netboot2.gcc.exp
netboot2.smallest.exp:
	FRAME=0 OPT=-Os COMPILER=whole-gcc sh RomCompile.sh bootrom/netboot2.c bootrom/common.c  bootrom/tables.c  bootrom/checksum.c
	mv netboot2.exp netboot2.smallest.exp

#WHOLE_PROGRAM=0
#
#netboot.coco: bootrom/netboot.c bootrom/preboot.asm
#	cmoc -i -S bootrom/netboot.c
#	lwasm -D"WHOLE_PROGRAM=$(WHOLE_PROGRAM)" --obj -lnetboot.list -onetboot.o netboot.s
#	lwasm -D"WHOLE_PROGRAM=$(WHOLE_PROGRAM)" --obj -lpreboot.list -opreboot.o bootrom/preboot.asm
#	lwlink --entry=program_start --format=decb --output=netboot.coco --script=bootrom/bootrom.script --map=netboot.coco.map -L/opt/yak/cmoc-0.1.80/share/cmoc/lib -lcmoc-crt-ecb -lcmoc-std-ecb preboot.o netboot.o
#	ls -l netboot.coco
#	dd bs=1 skip=5 if=netboot.coco of=netboot.cart
#netboot.wav: netboot.coco
#	makewav -r -s'48000' -d'0x2600' -e'0x2600' -n'NETBOOT ' -o'netboot.wav' -v netboot.coco
#	ls -l netboot.wav
#
#netboot.gcc.coco: bootrom/netboot.c bootrom/preboot.asm
#	lwasm -D"WHOLE_PROGRAM=$(WHOLE_PROGRAM)" --format=obj -lpreboot.list -o preboot.o bootrom/preboot.asm
#	gcc6809 $(OPT) -S --std=gnu99 bootrom/stdlib.c
#	gcc6809 $(OPT) -S --std=gnu99 $(EXTRA) bootrom/netboot.c
#	lwasm --format=obj \
#    --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
#    -l'stdlib.list' -o'stdlib.o' \
#    stdlib.s
#	lwasm --format=obj \
#    --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
#    -l"netboot.list" -o"netboot.o" \
#    netboot.s
#	lwlink --decb --entry=program_start --output=netboot.decb --map=netboot.map \
#    --script=bootrom/bootrom-c000.script \
#    preboot.o netboot.o stdlib.o \
#    -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
#    -lgcc
#	hd netboot.decb | head -1
#	wc -c netboot.decb
#	dd bs=1 skip=5 if=netboot.decb of=netboot.cart
#
#nullboot.gcc.coco: bootrom/nullboot.c bootrom/preboot.asm
#	lwasm -D"WHOLE_PROGRAM=$(WHOLE_PROGRAM)" --format=obj -lpreboot.list -o preboot.o bootrom/preboot.asm
#	gcc6809 $(OPT) -S --std=gnu99 bootrom/stdlib.c
#	gcc6809 $(OPT) -S --std=gnu99 $(EXTRA) bootrom/nullboot.c
#	lwasm --format=obj \
#    --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
#    -l'stdlib.list' -o'stdlib.o' \
#    stdlib.s
#	lwasm --format=obj \
#    --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
#    -l"nullboot.list" -o"nullboot.o" \
#    nullboot.s
#	lwlink --decb --entry=program_start --output=nullboot.decb --map=nullboot.map \
#    --script=bootrom/bootrom-c000.script \
#    preboot.o nullboot.o stdlib.o \
#    -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
#    -lgcc
#	hd nullboot.decb | head -1
#	wc -c nullboot.decb
#	dd bs=1 skip=5 if=nullboot.decb of=nullboot.cart
#
#nullboot.coco: bootrom/nullboot.c bootrom/preboot.asm
#	cmoc -i -S bootrom/nullboot.c
#	lwasm -D"WHOLE_PROGRAM=$(WHOLE_PROGRAM)" --obj -lnullboot.list -onullboot.o nullboot.s
#	lwasm -D"WHOLE_PROGRAM=$(WHOLE_PROGRAM)" --obj -lpreboot.list -opreboot.o bootrom/preboot.asm
#	lwlink --entry=program_start --format=decb --output=nullboot.coco --script=bootrom/bootrom-c000.script --map=nullboot.coco.map -L/opt/yak/cmoc-0.1.80/share/cmoc/lib -lcmoc-crt-ecb -lcmoc-std-ecb preboot.o nullboot.o
#	hd nullboot.coco | head -1
#	ls -l nullboot.coco
#	dd bs=1 skip=5 if=nullboot.coco of=nullboot.cart
#
##	cmoc -i -S bootrom/nullboot.c
##	cmoc -i --org=2600 --decb -o nullboot.coco bootrom/preboot.asm nullboot.s || echo IGNORE THOSE ERRORS && lwlink --entry=program_start --format=decb --output=nullboot.coco --script=nullboot.link --map=nullboot.coco.map -L/opt/yak/cmoc-0.1.80/share/cmoc/lib -lcmoc-crt-ecb -lcmoc-std-ecb preboot.o nullboot.o
##	#cmoc -i --org=2600 --coco -o nullboot.coco bootrom/preboot.asm nullboot.s
##	ls -l nullboot.coco
#
#nullboot.wav: nullboot.coco
#	makewav -r -s'48000' -d'0x2600' -e'0x2600' -n'NULLBOOT' -o'nullboot.wav' -v nullboot.coco
#	ls -l nullboot.wav
#floppy.dsk: netboot.coco nullboot.coco
#	rm -f floppy.dsk
#	decb dskini floppy.dsk -4
#	decb copy -2 -b nullboot.coco floppy.dsk,NULLBOOT.DEC
#	decb copy -2 -b netboot.coco floppy.dsk,NETBOOT.DEC
#	decb dir floppy.dsk
#	ls -l floppy.dsk
#z: floppy.dsk
#	dd if=floppy.dsk of=/media/strick/APRIL3/z
#	sync ; sync
#z2:
#	DISK=/tmp/disk2 sh Install.sh *.os9
#	os9 dir /tmp/disk2,CMDS


ci:
	set -x; ci-l */*.asm ` find . -name '*.[ch]' `
	sync; sync

clean:
	rm -f ./*.list ./*.map ./*.asmap ./*.sym ./*.o ./*.s ./*.lst ./*.s-orig ./*.link *.bin *.raw *.wav *.coco
	rm -f __*_.* *.tmp *.decb *.rom *.os9 *.exp
	rm -f lib*.a _*.a # object archives
	rm -f *.c.[0-9][0-9][0-9]r.* # for gcc6809 debug
	sync; sync

# To install on a disk image, add DISK=... to the location of the disk image.
# Example:  make install DISK=/tmp/my-disk.dsk
install: all
	sh Install.sh *.os9 *.shell

chop:
	python chopper.py froblib/*.c wiz/*.c
	MAX_VERBOSE=$${MAX_VERBOSE:-9} sh -c 'for x in __*_.c; do cmoc -c --os9 -I.. -D"MAX_VERBOSE"="$$MAX_VERBOSE" $$x || kill $$$$; done'
	lwar -c lib__chopped.a __*_.o
	sh Compile.sh f.ntp f-ntp.c  os9/frobos9.c  stack300.asm  -L. -l__chopped
	tr '()' '  ' < f.map | grep '^Section: code ' | awk '{print $$3}' | sort -u | grep ^__  | wc -l
	ls __*.c | wc -l

f.ncl.os9:
	sh Compile.sh f.ncl.os9 ncl/ncl.c ncl/ncl_os9.c $(LIBS) regexp/re.c match/util.c

g.ncl.os9:
	HOW=gcc sh Compile.sh f.ncl.os9 ncl/ncl.c ncl/ncl_os9.c $(LIBS) regexp/re.c match/util.c os9/frobos9_gcc.c platform/std_gcc.c 
	lwlink --format=os9 buf.o flag.o format.o frobos9_gcc.o frobos9.o malloc.o ncl.o ncl_os9.o nylib.o nystdio.o re.o stack300.o std_gcc.o std.o util.o wiz5100s.o -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4 -lgcc

trapa.o:
	lwasm --obj -o trapa.o trapa.asm 
trapc.o:
	/opt/yak/fuzix/bin/m6809-unknown-gcc-4.6.4 --std=gnu99 -funsigned-char -funsigned-bitfields -S -I.. -Os  trapc.c
	lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource --list=trapc.list   -I.. -o trapc.o trapc.s

z2a.bin:
	cat z2.c froblib/*.c  decb/*.c  > z2a.c
	/opt/yak/fuzix/bin/m6809-unknown-gcc-4.6.4 --std=gnu99 -funsigned-char -funsigned-bitfields -S -I.. -Os -fwhole-program  z2a.c 
	lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource --list=z2a.list --map=z2a.map  -I.. -o z2a.o z2a.s 
	lwlink --format=decb --entry=_main -o z2a.bin --map=z2a.bin.map -s decb.script z2a.o -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ -lgcc
	# # # scp z2a.bin root@yak.net:/tmp/
	decb dir /media/strick/APRIL3/YDECB.DSK
	-decb kill /media/strick/APRIL3/YDECB.DSK,Z.BIN
	decb copy -2 -b -r z2a.bin /media/strick/APRIL3/YDECB.DSK,Z.BIN
	decb dir /media/strick/APRIL3/YDECB.DSK

FORCE:

yadd.os9: yadd.c os9/frobos9_gcc.c FORCE
	rm -f yadd.os9 yadd.o yadd.s
	gcc6809 -S  -I.. -std=gnu99 -nostdlib -fpic yadd.c os9/frobos9_gcc.c
	lwasm-orig -o yadd.o -l'yadd.o.list' --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource yadd.s
	lwasm-orig -o frobos9_gcc.o -l'frobos9_gcc.o.list' --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource frobos9_gcc.s
	ls -l yadd.o frobos9_gcc.o
	lwlink --format=os9 --entry=_main -o yadd --map=yadd.map yadd.o frobos9_gcc.o
	mv yadd yadd.os9
	os9 ident yadd.os9

yadd2.os9: yadd2.c os9/frobos9_gcc.c FORCE
	rm -f yadd2.os9 yadd2.o yadd2.s
	gcc6809 -O2 -S  -I.. -std=gnu99 -nostdlib -fpic -dP -fdump-unnumbered-links -fdump-unnumbered -fdump-noaddr yadd2.c os9/frobos9_gcc.c
	sed 's/area\t[.]text[.]startup/area\t.text/' < yadd2.s > yadd2.s+ ; mv yadd2.s+ yadd2.s
	lwasm-orig -o yadd2.o -l'yadd2.o.list' --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource yadd2.s
	lwasm-orig -o frobos9_gcc.o -l'frobos9_gcc.o.list' --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource frobos9_gcc.s
	ls -l yadd2.o frobos9_gcc.o
	lwlink --format=os9 --entry=_main -o yadd2 --map=yadd2.map yadd2.o frobos9_gcc.o
	mv yadd2 yadd2.os9
	os9 ident yadd2.os9


OPT_LEVEL_1 = -fdefer-pop -fmerge-constants -fthread-jumps -floop-optimize -fcrossjumping -fif-conversion -fif-conversion2 -fdelayed-branch -fguess-branch-probability -fcprop-registers
OPT_LEVEL_1X = -fdefer-pop -fmerge-constants -fthread-jumps -floop-optimize -fcrossjumping -fif-conversion -fif-conversion2 -fdelayed-branch -fguess-branch-probability

yadd2all.os9: yadd2.c os9/frobos9_gcc.c FORCE
	rm -f yadd2all.os9 yadd2all.o yadd2all.s
	cat yadd2.c os9/frobos9_gcc.c > yadd2all.c 
	# gcc6809 -Os -S  -I.. -std=gnu99 -nostdlib -fpic -da -dP -fdump-unnumbered-links -fdump-unnumbered -fdump-noaddr -fwhole-program yadd2all.c
	gcc6809 -O2 -S  -I.. -std=gnu99 -nostdlib -fpic -da -dP -fwhole-program yadd2all.c
	sed 's/area\t[.]text[.]startup/area\t.text/' < yadd2all.s > yadd2all.s+ ; mv yadd2all.s+ yadd2all.s
	lwasm-orig -o yadd2all.o -l'yadd2all.o.list' --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource yadd2all.s
	ls -l yadd2all.o
	lwlink --format=os9 --entry=_main -o yadd2all --map=yadd2all.map yadd2all.o
	mv yadd2all yadd2all.os9
	os9 ident yadd2all.os9
