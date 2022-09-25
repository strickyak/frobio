HDRS=froblib.h frobnet.h frobos9.h froblib/malloc.h match/util.h ncl/ncl.h ncl/ncl_os9.h os9/os9defs.h regexp/re.h wiz/wiz5100s.h
LIBS=wiz/wiz5100s.c froblib/buf.c froblib/flag.c froblib/format.c froblib/malloc.c froblib/nylib.c froblib/nystdio.c froblib/std.c os9/frobos9.c

COMPILE=

all: x-logging.os9
	sync; sync

x-logging.os9: x-logging.c $(LIBS) $(HDRS)
	sh Compile x-logging.c $(LIBS)
	os9 ident x-logging.os9

ZZx-logging: x-logging.c $(LIBS) $(HDRS)
	/home/strick/go/bin/cmocly --cmoc_pre='-DMAX_VERBOSE=9' -I=.. --o=x-logging x-logging.c $(LIBS)
	os9 ident x-logging.os9

Zx-logging: x-logging.c $(LIBS) $(HDRS)
	cmoc -i --os9 -I.. x-logging.c $(LIBS)
	os9 ident x-logging.os9

f-ping.os9: f-ping.c $(LIBS) $(HDRS)
	sh Compile f-ping.c $(LIBS)
	os9 ident f-ping.os9

ci:
	set -x; ci-l ` find . -name '*.[ch]' `
	sync; sync

clean:
	rm -f *.list *.map *.asmap *.sym *.o *.s
	rm -f *.os9 *.lst *.s-orig *.link
	sync; sync
