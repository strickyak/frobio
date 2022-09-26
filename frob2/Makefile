PROD= f-arp.os9 f-config.os9 f-dhcp.os9 f-dump.os9 f-dig.os9 f-ntp.os9 f-ping.os9 f-tget.os9
DEMO= x-logging.os9 x-fputs.os9

all: $(DEMO) $(PROD)
	sync; sync

HDRS=froblib.h frobnet.h frobos9.h froblib/malloc.h match/util.h ncl/ncl.h ncl/ncl_os9.h os9/os9defs.h regexp/re.h wiz/wiz5100s.h
LIBS=wiz/wiz5100s.c froblib/buf.c froblib/flag.c froblib/format.c froblib/malloc.c froblib/nylib.c froblib/nystdio.c froblib/std.c os9/frobos9.c

# f-*.c are our intended frobio commands.
f-arp.os9 : f-arp.c $(HDRS) $(LIBS)
	sh Compile $< $(LIBS)
f-config.os9 : f-config.c $(HDRS) $(LIBS)
	sh Compile $< $(LIBS)
f-dhcp.os9 : f-dhcp.c $(HDRS) $(LIBS)
	sh Compile $< $(LIBS)
f-dump.os9 : f-dump.c $(HDRS) $(LIBS)
	sh Compile $< $(LIBS)
f-dig.os9 : f-dig.c $(HDRS) $(LIBS)
	sh Compile $< $(LIBS)
f-ntp.os9 : f-ntp.c $(HDRS) $(LIBS)
	sh Compile $< $(LIBS)
f-ping.os9 : f-ping.c $(HDRS) $(LIBS)
	sh Compile $< $(LIBS)
f-tget.os9 : f-tget.c $(HDRS) $(LIBS)
	sh Compile $< $(LIBS)

# x-*.c are dev or demo files.
x-logging.os9 : x-logging.c
	sh Compile $< $(LIBS)
x-fputs.os9 : x-fputs.c
	sh Compile $< $(LIBS)

ci:
	set -x; ci-l ` find . -name '*.[ch]' `
	sync; sync

clean:
	rm -f *.list *.map *.asmap *.sym *.o *.s
	rm -f *.os9 *.lst *.s-orig *.link
	sync; sync

#x-logging: x-logging.c $(LIBS) $(HDRS)
#	/home/strick/go/bin/cmocly --cmoc_pre='-DMAX_VERBOSE=9' -I=.. --o=x-logging x-logging.c $(LIBS)
#	os9 ident x-logging.os9
#
#x-logging: x-logging.c $(LIBS) $(HDRS)
#	cmoc -i --os9 -I.. x-logging.c $(LIBS)
#	os9 ident x-logging.os9
