PROD= f.arp.os9 f.config.os9 f.dhcp.os9 f.dump.os9 f.dig.os9 f.ntp.os9 f.ping.os9 f.tget.os9 f.send.os9 f.recv.os9 f.ticks.os9
DEMO= x.logging.os9 x.fputs.os9

all: $(DEMO) $(PROD)
	sync; sync

HDRS=froblib.h frobnet.h frobos9.h froblib/malloc.h match/util.h ncl/ncl.h ncl/ncl_os9.h os9/os9defs.h regexp/re.h wiz/wiz5100s.h
LIBS=wiz/wiz5100s.c froblib/buf.c froblib/flag.c froblib/format.c froblib/malloc.c froblib/nylib.c froblib/nystdio.c froblib/std.c os9/frobos9.c stack300.asm

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

# x-*.c are dev or demo files.
x.logging.os9 : x-logging.c
	sh Compile.sh $@ $< $(LIBS)
x.fputs.os9 : x-fputs.c
	sh Compile.sh $@ $< $(LIBS)

f.b-unix:
	gcc -I.. -g -fsanitize=address -static-libasan   f-b.c $$(ls tweb/*.c | grep -v _test) froblib/*.c unix/*.c 
	./a.out -a tweb/testdata/index.fm

ci:
	set -x; ci-l ` find . -name '*.[ch]' `
	sync; sync

clean:
	rm -f ./*.list ./*.map ./*.asmap ./*.sym ./*.o ./*.s ./*.os9 ./*.lst ./*.s-orig ./*.link
	rm -f */*.list */*.map */*.asmap */*.sym */*.o */*.s */*.os9 */*.lst */*.s-orig */*.link
	sync; sync

# To install on a disk image, add DISK=... to the location of the disk image.
# Example:  make install DISK=/tmp/my-disk.dsk
install: all
	sh Install.sh *.os9 *.shell
