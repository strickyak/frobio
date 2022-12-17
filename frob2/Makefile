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
x.logging.os9 : x-logging.c
	sh Compile.sh $@ $< $(LIBS)
x.fputs.os9 : x-fputs.c
	sh Compile.sh $@ $< $(LIBS)

f.b-unix:
	gcc -I.. -g -fsanitize=address -static-libasan   f-b.c $$(ls tweb/*.c | grep -v _test) froblib/*.c unix/*.c
	./a.out -a tweb/testdata/index.fm

netboot.coco: bootrom/netboot.c bootrom/preboot.asm
	cmoc -i -c bootrom/netboot.c
	sed 's/program_start/program_unused/g' < netboot.s > netboot.tmp
	mv netboot.tmp netboot.s
	cmoc -i --org=2600 --coco -o netboot.coco bootrom/preboot.asm netboot.s
	ls -l netboot.coco
netboot.wav: netboot.coco
	makewav -r -s'48000' -d'0x2600' -e'0x2600' -n'NETBOOT ' -o'netboot.wav' -v netboot.coco
	ls -l netboot.wav
nullboot.coco: bootrom/nullboot.c bootrom/preboot.asm
	cmoc -i -c bootrom/nullboot.c
	sed 's/program_start/program_unused/g' < nullboot.s > nullboot.tmp
	mv nullboot.tmp nullboot.s
	cmoc -i --org=2600 --coco -o nullboot.coco bootrom/preboot.asm nullboot.s
	dd bs=1 skip=5 if=nullboot.coco of=/tmp/cocoio.boot
	ls -l nullboot.coco
nullboot.wav: nullboot.coco
	makewav -r -s'48000' -d'0x2600' -e'0x2600' -n'NULLBOOT' -o'nullboot.wav' -v nullboot.coco
	ls -l nullboot.wav
floppy.dsk: netboot.coco nullboot.coco
	rm -f floppy.dsk
	decb dskini floppy.dsk -4
	decb copy -2 -b nullboot.coco floppy.dsk,NULLBOOT
	decb copy -2 -b netboot.coco floppy.dsk,NETBOOT
	decb dir floppy.dsk
	ls -l floppy.dsk
z: floppy.dsk
	dd if=floppy.dsk of=/media/strick/APRIL3/z
	sync ; sync
z2:
	DISK=/tmp/disk2 sh Install.sh *.os9
	os9 dir /tmp/disk2,CMDS


ci:
	set -x; ci-l */*.asm ` find . -name '*.[ch]' `
	sync; sync

clean:
	rm -f ./*.list ./*.map ./*.asmap ./*.sym ./*.o ./*.s ./*.os9 ./*.lst ./*.s-orig ./*.link *.bin *.raw *.wav *.coco
	rm -f */*.list */*.map */*.asmap */*.sym */*.o */*.s */*.os9 */*.lst */*.s-orig */*.link
	rm -f __*_.*
	sync; sync

# To install on a disk image, add DISK=... to the location of the disk image.
# Example:  make install DISK=/tmp/my-disk.dsk
install: all
	sh Install.sh *.os9 *.shell

chop:
	python chopper.py froblib/*.c
	for x in __*_.c; do cmoc -c --os9 -I.. $$x || kill $$$$; done
	lwar -c lib__chopped.a __*_.o
	# cmoc -i --os9 -I.. -DMAX_VERBOSE=9 -o f.ntp f-ntp.c -L. -l__chopped wiz/wiz5100s.c os9/frobos9.c  stack300.asm
	sh Compile.sh f.ntp f-ntp.c  wiz/wiz5100s.c os9/frobos9.c  stack300.asm  -L. -l__chopped
