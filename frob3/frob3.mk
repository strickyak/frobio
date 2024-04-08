# This file `frob3.mk` lives in frobio/frob3 but is included
# from the directory where you actually build (whose Makefile
# is created and configured by ./configure).

LAN=10.23.23.23
DHCP=0

all: all-net-cmds all-fuse-modules all-fuse-daemons all-drivers all-axiom all-hdbdos results1 results2 all-lemmings results3
	find results -type f -print | LC_ALL=C sort
	sync

all-without-gccretro: all-net-cmds all-fuse-modules all-fuse-daemons all-drivers results1 all-lemmings results3-without-gccretro
	find results -type f -print | LC_ALL=C sort
	sync

# Quick assertion that we have the right number of things.
# Change these when you add more things.
NUM_CMDS = 18
NUM_MODULES = 14

###############################################

VPATH = $F $F/axiom $F/froblib $F/drivers $F/fuse-modules $F/fuse-daemons $F/net-cmds $F/hdbdos

FROBLIB_C = $F/froblib/buf.c $F/froblib/flag.c $F/froblib/format.c $F/froblib/malloc.c $F/froblib/nylib.c $F/froblib/nystdio.c $F/froblib/std.c
WIZ_C = $F/wiz/wiz5100s.c
OS9_C = $F/os9/frobos9.c
NCL_C = $F/ncl/match.c $F/ncl/ncl.c $F/ncl/ncl_os9.c $F/ncl/regexp.c
NCL_H = $F/ncl/match.h $F/ncl/ncl.h $F/ncl/ncl_os9.h $F/ncl/regexp.h
LIB1 = $(FROBLIB_C) $(WIZ_C) $(OS9_C)
HDRS = $F/froblib.h $F/frobnet.h $F/frobos9.h $F/frobtype.h

CMOCI=$F/../../share/cmoc/include
CMOCL=$F/../../share/cmoc/lib

#LWASM_C = $(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource
LWASM_C = $(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport

###############################################

results1:
	rm -rf results
	mkdir -p results/CMDS results/MODULES
	ln -s . results/lem
	date | tr -c ' -~' '\r' > results/one
	uname -a | tr -c ' -~' '\r' > results/two
	cat /proc/zoneinfo | tr -c ' -~' '\r' > results/three
	set -x; for x in *.os9cmd; do cp -v $$x results/CMDS/$$(basename $$x .os9cmd) ; done
	set -x; for x in *.os9mod; do cp -v $$x results/MODULES/$$(basename $$x .os9mod) ; done
	n=$$(ls results/CMDS/* | wc -l) ; set -x; test $(NUM_CMDS) -eq $$n
	n=$$(ls results/MODULES/* | wc -l) ; set -x; test $(NUM_MODULES) -eq $$n

results2: results1 os9disks
	mkdir -p results/OS9DISKS results/LEMMINGS


NOS9_6809_L1_coco1_80d.bigdup : ../nitros9/level1/coco1/NOS9_6809_L1_coco1_80d.dsk
	bash ../frobio/frob3/helper/make-hard-drive.sh $< $@
NOS9_6809_L1_coco1_80d.dsk : NOS9_6809_L1_coco1_80d.bigdup results1
	rm -f $@
	cp -vf $< $@
	sh ../frobio/frob3/helper/os9-install.sh -r $@,CMDS -pe -pr -e -r results/CMDS/*
	sh ../frobio/frob3/helper/os9-install.sh -r $@,MODULES -pe -pr -e -r results/MODULES/*

NOS9_6809_L2_coco3_80d.bigdup : ../nitros9/level2/coco3/NOS9_6809_L2_80d.dsk
	bash ../frobio/frob3/helper/make-hard-drive.sh $< $@
NOS9_6809_L2_coco3_80d.dsk : NOS9_6809_L2_coco3_80d.bigdup results1
	rm -f $@
	cp -vf $< $@
	sh ../frobio/frob3/helper/os9-install.sh -r $@,CMDS -pe -pr -e -r results/CMDS/*
	sh ../frobio/frob3/helper/os9-install.sh -r $@,MODULES -pe -pr -e -r results/MODULES/*

NOS9_6309_L2_coco3_80d.bigdup : ../nitros9/level2/coco3_6309/NOS9_6309_L2_80d.dsk 
	bash ../frobio/frob3/helper/make-hard-drive.sh $< $@
NOS9_6309_L2_coco3_80d.dsk : NOS9_6309_L2_coco3_80d.bigdup results1
	rm -f $@
	cp -vf $< $@
	bash ../frobio/frob3/helper/os9-install.sh -r $@,CMDS -pe -pr -e -r results/CMDS/*
	bash ../frobio/frob3/helper/os9-install.sh -r $@,MODULES -pe -pr -e -r results/MODULES/*

os9disks: NOS9_6809_L1_coco1_80d.dsk NOS9_6809_L2_coco3_80d.dsk NOS9_6309_L2_coco3_80d.dsk
	mkdir -p results/OS9DISKS results/LEMMINGS
	set -x; for x in $^; do rm -f results/LEMMINGS/$$x; ln $$x results/LEMMINGS/; done
	set -x; for x in $^; do rm -f results/OS9DISKS/$$x; ln $$x results/OS9DISKS/; done


results2b: results2 all-axiom all-axiom4
	mkdir -p results/BOOTING
	cp -v $F/booting/README.md results/BOOTING/README.md
	:
	decb dskini results/BOOTING/netboot3.dsk
	decb copy -0 -b $F/booting/INSTALL4.BAS results/BOOTING/netboot3.dsk,INSTALL4.BAS
	decb copy -0 -b $F/booting/INSTALL5.BAS results/BOOTING/netboot3.dsk,INSTALL5.BAS
	decb copy -3 -a $F/booting/README.md results/BOOTING/netboot3.dsk,README.md
	decb copy -2 -b axiom-whole.l3k results/BOOTING/netboot3.dsk,NETBOOT.DEC
	decb dir results/BOOTING/netboot3.dsk
	decb dskini results/BOOTING/exoboot3.dsk
	decb copy -2 -b axiom-whole6k.decb results/BOOTING/exoboot3.dsk,EXOBOOT3.DEC
	decb dir results/BOOTING/exoboot3.dsk
	:
	decb dskini results/BOOTING/future4.dsk
	decb copy -0 -b $F/booting/INSTALL4.BAS results/BOOTING/future4.dsk,INSTALL4.BAS
	decb copy -0 -b $F/booting/INSTALL5.BAS results/BOOTING/future4.dsk,INSTALL5.BAS
	decb copy -3 -a $F/booting/README.md results/BOOTING/future4.dsk,README.md
	decb copy -2 -b axiom4-whole.l3k results/BOOTING/future4.dsk,NETBOOT.DEC
	decb dir results/BOOTING/future4.dsk
	decb dskini results/BOOTING/exofoot4.dsk
	decb copy -2 -b axiom4-whole6k.decb results/BOOTING/exofoot4.dsk,EXOFOOT4.DEC
	decb dir results/BOOTING/exofoot4.dsk
	:
	ls -l results/BOOTING/

results3: results2b
	cp -v $F/../built/wip-2023-03-29-netboot2/demo-dgnpeppr.coco1.loadm results/LEMMINGS
	cp -v $F/../built/wip-2023-03-29-netboot2/demo-nyancat.coco3.loadm results/LEMMINGS

results3-without-gccretro: results2
	cp -v $F/../built/wip-2023-03-29-netboot2/demo-dgnpeppr.coco1.loadm results/LEMMINGS
	cp -v $F/../built/wip-2023-03-29-netboot2/demo-nyancat.coco3.loadm results/LEMMINGS

###############################################

clean: _FORCE_
	rm -f *.o *.map *.lst *.link *.os9 *.s *.os9cmd *.os9mod _*
	rm -f *.list *.loadm *.script *.decb *.rom *.l3k
	rm -f *.dsk *.lem *.a *.sym *.asmap *.bin *.bigdup *.raw *.lwraw
	rm -f utility-* burn
	rm -rf results

_FORCE_:

###############################################

all-hdbdos: hdbdos.rom hdbdos.lem sideload.lwraw inkey_trap.lwraw

%.lwraw : %.asm
	$(LWASM) --raw $< -o'$@' -I$HOME/coco-shelf/toolshed/hdbdos/ -I../wiz/ --pragma=condundefzero,nodollarlocal,noindex0tonone --list='$@.list' --map='$@.map'

hdbdos.lem: hdbdos.rom
	cat $F/../../toolshed/hdbdos/preload $< $F/../../toolshed/hdbdos/postload > $@
hdbdos.rom: ecb_equates.asm hdbdos.asm 
	$(LWASM) -D'LEMMA=1' -r $^ --output=$@ -I$F/../../toolshed/hdbdos/ -I$F/wiz/ --pragma=condundefzero,nodollarlocal,noindex0tonone --list=hdbdos.rom.list --map=hdbdos.rom.map


all-axiom: axiom-whole.rom axiom-whole.l3k axiom-whole6k.decb
all-axiom4: axiom4-whole.rom axiom4-whole.l3k axiom4-whole6k.decb axiom41.rom axiom41.decb axiom41-c300.decb primes41.decb primes41-c300.decb burn-rom-fast.lem burn-primes-fast.lem 

burn: burn.o
	$(LWLINK) --format=decb --entry=_main --section-base=.text=0C00 -o burn  $<
burn.o: burn.s
	$(LWASM) --format=obj --pragma=newsource --pragma=cescapes --output=$@ --list=burn.list --map=burn.map  $<
burn.s: $F/burning/burn.c
	gcc6809  -S  -I$F/..  -O1 --std=gnu99 -Wall -Werror -o $@  $<
burn-decb.bin: burn-decb.o
	$(LWLINK) --format=decb --entry=_main --script=$F/helper/a00.script -o $@  $<
burn-decb.o: burn-decb.s
	$(LWASM) --format=obj --pragma=newsource --pragma=cescapes --output=$@ --list=$@.list --map=$@.map  $<
burn-decb.s: $F/burning/burn-decb.c
	gcc6809  -S  -I$F/..  -Os --std=gnu99 -Wall -Werror -o $@ $<
########################################################################
burn-rom-fast.lem: axiom41.decb burn-decb.bin
	cat burn-decb.bin $< > $@
burn-primes-fast.lem: primes41.decb burn-decb.bin
	cat burn-decb.bin $< > $@
########################################################################

### axiom41 is the newest axiom.
axiom41.rom: _FORCE_  primes41.rom
	gcc6809 -S --std=gnu99 -Os -fomit-frame-pointer -fwhole-program -I$F/.. $F/axiom41/axcore.c 
	sed -e 's/[.]area/;area/' -e 's/[.]globl/;globl/' -e '/^_main:/,/^$$/d' < axcore.s > _axcore.s
	gcc6809 -S --std=gnu99 -Os -fomit-frame-pointer -fwhole-program -I$F/.. $F/axiom41/rescue.c 
	sed -e 's/jmp/lbra/' -e 's/jsr/lbsr/' -e 's/\<_[A-Za-z0-9_]*\>/R&/' -e 's/\<L[0-9]*\>/R&/' -e 's/[.]area/;area/' -e 's/[.]globl/;globl/' -e '/^R_main:/,$$d' < rescue.s > _rescue.s
	lwasm --raw $F/axiom41/axiom41.asm -I`pwd` --pragma=newsource --pragma=cescapes --list=axiom41.list --map=axiom41.map -o$@
axiom41.decb: axiom41.rom
	lwasm --decb $F/axiom41/axiom41.asm -I`pwd` --pragma=newsource --pragma=cescapes --list=$@.list --map=$@.map -o$@
axiom41-c300.decb: axiom41.rom
	lwasm --decb $F/axiom41/axiom41.asm -D'JUST_C300' -I`pwd` --pragma=newsource --pragma=cescapes --list=axiom41-c300.list --map=axiom41-c300.map -o$@

primes41.rom: _FORCE_
	gcc6809 -S --std=gnu99 -Os -fomit-frame-pointer -fwhole-program -I$F/.. $F/axiom41/primes.c 
	sed -e 's/[.]area/;area/' -e 's/[.]globl/;globl/' -e '/^_main:/,/^$$/d' < primes.s > _primes.s
	gcc6809 -S --std=gnu99 -Os -fomit-frame-pointer -fwhole-program -I$F/.. $F/axiom41/rescue.c 
	sed -e 's/jmp/lbra/' -e 's/jsr/lbsr/' -e 's/\<_[A-Za-z0-9_]*\>/R&/' -e 's/\<L[0-9]*\>/R&/' -e 's/[.]area/;area/' -e 's/[.]globl/;globl/' -e '/^R_main:/,$$d' < rescue.s > _rescue.s
	lwasm --raw $F/axiom41/primes41.asm -I`pwd` --pragma=newsource --pragma=cescapes --list=primes41.list --map=primes41.map -o$@
primes41.decb: primes41.rom
	lwasm --decb $F/axiom41/primes41.asm -I`pwd` --pragma=newsource --pragma=cescapes --list=$@.list --map=$@.map -o$@
primes41-c300.decb: primes41.rom
	lwasm --decb $F/axiom41/primes41.asm -D'JUST_C300' -I`pwd` --pragma=newsource --pragma=cescapes --list=primes41-c300.list --map=primes41-c300.map -o$@

### axiom4 is the new axiom.
axiom4-whole.rom: axiom4.c preboot3.asm
	gcc6809 -S -I$F/.. -Os -fwhole-program -fomit-frame-pointer --std=gnu99 -Wall -Werror -D'NEED_STDLIB_IN_NETLIB3' $<
	cat $F/axiom/preboot3.asm axiom4.s > _work4.s
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  _work4.s --output=_work4.o --list=_work4.list
	$(LWLINK) --output=_work4.rom --map=_work4.map --raw --script=$F/helper/axiom.script _work4.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc
	$(LWOBJDUMP) _work4.o > _work.4objdump
	$(GO) run $A/helper/insert-gap-in-asm/main.go  --asm _work4.s --map _work4.map -o axiom4-whole.s
	rm -f axiom4-whole.list
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  axiom4-whole.s --output=axiom4-whole.o --list=axiom4-whole.list
	$(LWLINK) --output=axiom4-whole.rom --map=axiom4-whole.map --raw --script=$F/helper/axiom.script axiom4-whole.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc
	$(GO) run $A/helper/customize-axiom/customize-axiom.go --hostname="$$COCOHOST" --secret="$$COCOSECRET" axiom4-whole.rom

axiom4-whole6k.decb: axiom4-whole.rom
	$(LWLINK) --output=axiom4-whole6k.decb --map=axiom4-whole6k.map --decb --script=$F/helper/axiom6k.script axiom4-whole.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc

axiom4-whole.l3k: axiom4-whole.rom
	$(GO) run $A/helper/shift-rom-to-3000/main.go < axiom4-whole.rom > axiom4-whole.l3k

### axiom (without the 4) is the old axiom.

axiom-whole.rom: axiom.c commands.c dhcp3.c netlib3.c romapi3.c
	cat  $^ > _axiom_whole.c
	gcc6809 -S -I$F/.. -Os -fwhole-program -fomit-frame-pointer --std=gnu99 -Wall -Werror -D'NEED_STDLIB_IN_NETLIB3' _axiom_whole.c
	cat $F/axiom/preboot3.asm _axiom_whole.s > _work.s
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  _work.s --output=_work.o --list=_work.list
	$(LWLINK) --output=_work.rom --map=_work.map --raw --script=$F/helper/axiom.script _work.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc
	$(LWOBJDUMP) _work.o > _work.objdump
	$(GO) run $A/helper/insert-gap-in-asm/main.go  --asm _work.s --map _work.map -o axiom-whole.s
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  axiom-whole.s --output=axiom-whole.o --list=axiom-whole.list
	$(LWLINK) --output=axiom-whole.rom --map=axiom-whole.map --raw --script=$F/helper/axiom.script axiom-whole.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc

axiom-whole6k.decb: axiom-whole.rom
	$(LWLINK) --output=axiom-whole6k.decb --map=axiom-whole6k.map --decb --script=$F/helper/axiom6k.script axiom-whole.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc

axiom-whole.l3k: axiom-whole.rom
	$(GO) run $A/helper/shift-rom-to-3000/main.go < axiom-whole.rom > axiom-whole.l3k

axiom-gomar.rom: axiom.c commands.c dhcp3.c netlib3.c romapi3.c
	cat  $^ > _axiom_gomar.c
	gcc6809 -S -I$F/.. -Os -fwhole-program -fomit-frame-pointer --std=gnu99 -Wall -Werror -D'__GOMAR__' -D'NEED_STDLIB_IN_NETLIB3' _axiom_gomar.c
	cat $F/axiom/preboot3.asm _axiom_gomar.s > _work.s
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  _work.s --output=_work.o --list=_work.list
	$(LWLINK) --output=_work.rom --map=_work.map --raw --script=$F/helper/axiom.script _work.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc
	$(LWOBJDUMP) _work.o > _work.objdump
	$(GO) run $A/helper/insert-gap-in-asm/main.go  --asm _work.s --map _work.map -o axiom-gomar.s
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  axiom-gomar.s --output=axiom-gomar.o --list=axiom-gomar.list
	$(LWLINK) --output=axiom-gomar.rom --map=axiom-gomar.map --raw --script=$F/helper/axiom.script axiom-gomar.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc

###############################################

NET_CMDS =  f.arp.os9cmd f.config.os9cmd f.dhcp.os9cmd f.dig.os9cmd f.dump.os9cmd f.ntp.os9cmd f.ping.os9cmd f.recv.os9cmd f.send.os9cmd f.telnetd0.os9cmd f.tget.os9cmd f.wget.os9cmd f.ticks.os9cmd f.say.os9cmd r.os9cmd
all-net-cmds: $(NET_CMDS)

CDEFS = -DFOR_LEVEL2=1 -DBY_CMOC=1 -DMAX_VERBOSE=9
LIB_CDEFS = -D"BASENAME=\"$$(basename $@ .o)\"" $(CDEFS)
CMD_CDEFS = -D"BASENAME=\"$$(basename $@ .os9cmd)\"" $(CDEFS)

COMPILE_NET_LIB = $(CMOC) -i -c --os9 -I. -I$F/.. -I$(CMOCI) -L$(CMOCL) $(LIB_CDEFS) -o $@ $<

C_FILES_FOR_CMOC_ARCHIVE = $F/froblib/buf.c $F/froblib/flag.c $F/froblib/format.c $F/froblib/malloc.c $F/froblib/nylib.c $F/froblib/nystdio.c $F/froblib/std.c $F/wiz/wiz5100s.c $F/os9/frobos9.c


O_FILES_FOR_NET_CMDS = stack300.o buf.o flag.o format.o malloc.o nylib.o nystdio.o std.o wiz5100s.o frobos9.o

ifeq ($(strip $(CHOPPING)),)

COMPILE_NET_CMD = t=$$(basename $@ .os9cmd); $(CMOC) -i --os9 -I. -I$F/.. -I$(CMOCI) -L$(CMOCL) $(CMD_CDEFS) -o $$t $^ && mv -v $$t $@
CHOPPED_LIB=

else #### TODO: make -l_chopped work.

# LINKER="bash ../frobio/frob3/helper/lwlink-and-make-map.sh"
# COMPILE_NET_CMD = t=$$(basename $@ .os9cmd); MAPOUT="$$t.map" $(CMOC) --lwlink=$(LINKER) -i --os9 -I. -I$F/.. -I$(CMOCI) -L$(CMOCL) $(CMD_CDEFS) -o $$t $< -L. -l_chopped && mv -v $$t $@

COMPILE_NET_CMD = set -x; t=$$(basename $@ .os9cmd); rm -f $$t.map; $(CMOC) -i --os9 -I. -I$F/.. -I$(CMOCI) -L$(CMOCL) $(CMD_CDEFS) -o $$t $< -L. -l_chopped && mv -v $$t $@ && ( test -f $$t.map || mv -v $$(ls -t *.map | head -1) $$t.map )

lib_chopped.a: $(C_FILES_FOR_CMOC_ARCHIVE)
	bash $F/helper/cmoc-chopped.sh lib_chopped.a "$^" $(CDEFS) -I$F/..

CHOPPED_LIB=lib_chopped.a

endif

stack300.o: $F/froblib/stack300.asm
	$(LWASM) --obj -o $@ $<
buf.o: $F/froblib/buf.c
	$(COMPILE_NET_LIB)
flag.o: $F/froblib/flag.c
	$(COMPILE_NET_LIB)
format.o: $F/froblib/format.c
	$(COMPILE_NET_LIB)
malloc.o: $F/froblib/malloc.c
	$(COMPILE_NET_LIB)
nylib.o: $F/froblib/nylib.c
	$(COMPILE_NET_LIB)
nystdio.o: $F/froblib/nystdio.c
	$(COMPILE_NET_LIB)
std.o: $F/froblib/std.c
	$(COMPILE_NET_LIB)
wiz5100s.o: $F/wiz/wiz5100s.c
	$(COMPILE_NET_LIB)
frobos9.o: $F/os9/frobos9.c
	$(COMPILE_NET_LIB)

f.arp.os9cmd: $F/net-cmds/f-arp.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.config.os9cmd: $F/net-cmds/f-config.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.dhcp.os9cmd: $F/net-cmds/f-dhcp.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.dig.os9cmd: $F/net-cmds/f-dig.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.dump.os9cmd: $F/net-cmds/f-dump.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.ntp.os9cmd: $F/net-cmds/f-ntp.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.ping.os9cmd: $F/net-cmds/f-ping.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.recv.os9cmd: $F/net-cmds/f-recv.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.send.os9cmd: $F/net-cmds/f-send.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.say.os9cmd: $F/net-cmds/f-say.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

r.os9cmd: $F/net-cmds/r.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.telnetd0.os9cmd: $F/net-cmds/f-telnetd0.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.tget.os9cmd: $F/net-cmds/f-tget.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.wget.os9cmd: $F/net-cmds/f-wget.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

f.ticks.os9cmd: $F/net-cmds/f-ticks.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

all-fuse-daemons: fuse.n.os9cmd fuse.ramfile.os9cmd fuse.tftp.os9cmd

fuse.n.os9cmd: $F/fuse-daemons/fuse-n.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

fuse.ramfile.os9cmd: $F/fuse-daemons/fuse-ramfile.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

fuse.tftp.os9cmd: $F/fuse-daemons/fuse-tftp.c $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NET_CMD)

#######################################################################################

octet2.o: $F/octet/octet2.c $F/octet/octet2.h
	$(COMPILE_NET_LIB)

#######################################################################################

wc.os9cmd: /sy/gosub/demo/wc.go octet2.o
	d=$$(pwd) && cd /sy/gosub/ && $(GO) build gosub.go && cp gosub $$d/
	t=$$(basename $@ .os9cmd).gosub && rm -rf $$t && mkdir $$t $$t/runtime
	t=$$(basename $@ .os9cmd).gosub && cd $$t && ../gosub <"$<" --libdir /sy/gosub/lib > ___.defs.h 2>___.err
	t=$$(basename $@ .os9cmd).gosub && cp -av $$(ls /sy/gosub/runtime/*.[ch] | grep -v /unix_) $$t/runtime
	set -ex; t=$$(basename $@ .os9cmd).gosub && cd $$t && set -x; for x in *.c ; do $(CMOC) -i -I. -I../../frobio/ -S $$x ; done
	set -ex; t=$$(basename $@ .os9cmd).gosub && cd $$t && set -x; for x in runtime/*.c ; do $(CMOC) -i -I. -I../../frobio/ -S $$x ; done
	#-- t=$$(basename $@ .os9cmd).gosub && $(CMOC) -i --os9 --function-stack=0 -I../frobio/ $$t/*.s --#
	set -ex; t=$$(basename $@ .os9cmd).gosub && cd $$t && for x in *.s; do $(LWASM_C) -o $$(basename $$x .s).o $$x ; done
	# set -ex; t=$$(basename $@ .os9cmd).gosub && cd $$t && for x in *.s; do $(LWASM_C) $$x ; done
	set -ex; t=$$(basename $@ .os9cmd).gosub && ls . && ls $$t
	set -ex; t=$$(basename $@ .os9cmd).gosub && lwar -c lib$@.a $$t/*.o
	set -ex; $(LWLINK) $(O_FILES_FOR_NET_CMDS) octet2.o -L. -l$@  -L../frobio/frob3/../../share/cmoc/lib -L/home/strick/coco-shelf/share/cmoc/lib -lcmoc-crt-os9 -lcmoc-std-os9



#######################################################################################

# NCL is currently broken (link errors).
NCL_CMDS = f.ncl.os9cmd
_ncl_cmds: $(NCL_CMDS) _NOT_WORKING_YET_

COMPILE_NCL_CMD = t=$$(basename $@ .os9cmd); $(CMOC) -i --os9 -I$F/.. -I$(CMOCI) -L$(CMOCL) $(CMD_CDEFS) -o $$t $(NCL_C) && mv -v $$t $@
f.ncl.os9cmd:  $(NCL_C) $(NCL_H) $(O_FILES_FOR_NET_CMDS) $(CHOPPED_LIB)
	$(COMPILE_NCL_CMD)

#######################################################################################

all-fuse-modules: fuseman.os9mod fuser.os9mod fuse.os9mod

LWASM_FOR_FUSE_MODULE =	$(LWASM) --6809 --format=os9 -l'$@.list' -I'$F' -D'Level=2' -D'H6309=0' -o '$@' $<

_generated_from_fusec_.s : fusec.c fuse.h
	$(CMOC) -S -i --os9 -O2 --function-stack=0 --switch=ifelse -I'$F/..' $<
	sed -n '/SECTION.[cr]od[ea]/, /ENDSECTION/ p' < fusec.s | egrep -v 'SECTION|EXPORT' > _generated_from_fusec_.s

fuseman.os9mod: fuseman.asm defsfile _generated_from_fusec_.s
	: note : fuseman.asm will use _generated_from_fusec_.s
	$(LWASM_FOR_FUSE_MODULE) -I'.'

fuser.os9mod: fuser.asm defsfile fuse.h
	$(LWASM_FOR_FUSE_MODULE)

fuse.os9mod: fuse.asm defsfile fuse.h
	$(LWASM_FOR_FUSE_MODULE)

#######################################################################################

all-drivers: rblemma.os9mod dd.b0.os9mod b0.os9mod b1.os9mod b2.os9mod b3.os9mod n.fuse.os9mod boot.lemma.os9mod all-lemman

LWASM_FOR_DRIVER	= t=$$(basename $@ .os9mod); $(LWASM) -o $$t --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export -D'Level=2' -D'H6309=0' -I'.' -I'$F' -I'$F/wiz'  --format=os9 --list=$@.list $< && mv -v $$t $@

hdbdos_lemma.os9mod: $F/drivers/rblemmac.c $F/drivers/rblemmac.h $F/drivers/hdbdos_lemma.asm
_generated_from_hdbdos_lemmac.s: hdbdos_lemmac.c rblemmac.h hdbdos_lemma.asm
	$(CMOC)  -i --decb -O2 --function-stack=0 --switch=ifelse -S -I$F/.. -o hdbdos_lemmac.s $<
	sed -n '/SECTION.[cr]od[ea]/, /ENDSECTION/ p' < rblemmac.s | egrep -v 'SECTION|EXPORT|IMPORT' > _generated_from_rblemmac.s

rblemma.os9mod: $F/drivers/rblemmac.c $F/drivers/rblemmac.h $F/drivers/rblemma.asm
_generated_from_rblemmac.s: rblemmac.c rblemmac.h rblemma.asm
	$(CMOC)  -i --os9 -O2 --function-stack=0 --switch=ifelse -S -I$F/.. -o rblemmac.s $<
	sed -n '/SECTION.[cr]od[ea]/, /ENDSECTION/ p' < rblemmac.s | egrep -v 'SECTION|EXPORT|IMPORT' > _generated_from_rblemmac.s
rblemma.os9mod: rblemma.asm _generated_from_rblemmac.s
	$(LWASM_FOR_DRIVER)
dd.b0.os9mod: dd_b0.asm
	$(LWASM_FOR_DRIVER)
b0.os9mod: b0.asm
	$(LWASM_FOR_DRIVER)
b1.os9mod: b1.asm
	$(LWASM_FOR_DRIVER)
b2.os9mod: b2.asm
	$(LWASM_FOR_DRIVER)
b3.os9mod: b3.asm
	$(LWASM_FOR_DRIVER)
n.fuse.os9mod : n_fuse.asm
	$(LWASM_FOR_DRIVER)
boot.lemma.os9mod : boot_lemma.asm
	$(LWASM_FOR_DRIVER)

all-lemman: lemman.os9mod lemmer.os9mod lem.os9mod
	echo '*** WARNING *** UNTESTED ***'

_generated_from_lemmanc.s: lemmanc.c lemmanc.h lemman.asm
	$(CMOC)  -i --os9 -O2 --function-stack=0 --switch=ifelse -S -I$F/.. -o lemmanc.s $<
	sed -n '/SECTION.[cr]od[ea]/, /ENDSECTION/ p' < lemmanc.s | egrep -v 'SECTION|EXPORT|IMPORT' > _generated_from_lemmanc.s
	echo '*** WARNING *** UNTESTED ***'
lemman.os9mod: lemman.asm _generated_from_lemmanc.s $F/drivers/lemmanc.c $F/drivers/lemmanc.h $F/drivers/lemman.asm
	$(LWASM_FOR_DRIVER)
	echo '*** WARNING *** UNTESTED ***'

lemmer.os9mod: lemmer.asm defsfile
	$(LWASM_FOR_DRIVER)
	echo '*** WARNING *** UNTESTED ***'

lem.os9mod: lem.asm defsfile
	$(LWASM_FOR_DRIVER)
	echo '*** WARNING *** UNTESTED ***'

#######################################################################################

all-lemmings: burn-rom-fast.lem
	-test -s ../eou-h6309/63EMU.dsk && (cd ../frobio/frob3/lemmings && cp -vf EOU_H6309.new EOU_H6309.go)
	-test -s ../eou-m6809/68EMU.dsk && (cd ../frobio/frob3/lemmings && cp -vf EOU_M6809.new EOU_M6809.go)
	$(GO) run $A/lemmings/*.go --nitros9dir='$(NITROS9)' --shelf='$(SHELF)'
	mkdir -p results/LEMMINGS/
	ln *.lem results/LEMMINGS/
	ln *.dsk results/LEMMINGS/ || echo Ignore errors above, if some already existed.

#######################################################################################

# run-lemma
#   This target is not called by default.
#   If you do `make run-lemma` then it will build and run the lemma server.
#   It's a server, so it will run forever, as long as nothing goes wrong.
#   You can hit ^C to kill it.  `make` will then print an error message
#   that you can ignore.
server: all-without-gccretro
	cd $A/lemma/ && GOBIN=$(SHELF)/bin GOPATH=$(SHELF) $(GO) install -x server.go
run-server: run-lemma  # Alias.
run-lemma: server
	cp -fv ../frobio/built/wip-2023-04-22-cocofest/netboot3.dsk /tmp/disk0.dsk
	$(SHELF)/bin/server  -cards -ro results/LEMMINGS -lan=$(LAN) -config_by_dhcp=$(DHCP) --dos_root $F/../../../shelving/lemniscate/Coco-Disk-Tree/

##############  Old Junk Follows
#   For debugging with Gomar on Loopback.
run-lemma-L: all
	cd $A/lemma/ && GOBIN=$(SHELF)/bin GOPATH=$(SHELF) $(GO) install -x server.go
	(echo t; echo dir; echo date -t ; echo setime 2020/12/25 01:02:03 ; echo date -t ; \
      echo dir /lem; echo dir -e /lem; echo dir -e /lem/foo ; \
      echo dump /lem/tuesday.txt ; \
      echo list /lem/tuesday.txt ; \
      echo dump /lem/foo/bar.txt ; \
      echo list /lem/foo/bar.txt ; \
      echo list /lem/xattr.conf ; \
      echo 'dir > /lem/abc' ; \
      echo dir -e /lem ; \
      echo dump /lem/abc ; \
        )| \
          os9 copy -l -r /dev/stdin Nitros9_Coco3_M6809_Level2.dsk,startup
	$(SHELF)/bin/server --program ./Nitros9_Coco3_M6809_Level2_L.lem --block0 Nitros9_Coco3_M6809_Level2.dsk -lemman_fs=$F/testdata

run-axiom-L:
	cd /sy/doing_os9/gomar && go run --tags=d,coco3,level2,cocoio,hyper gomar.go  --rom_a000 /home/strick/6809/ROMS/color64bas.rom  --rom_8000 /home/strick/6809/ROMS/color64extbas.rom  -cart  ~/coco-shelf/build-frobio/axiom-gomar.rom -v=w   -basic_text  -v=d  2>/tmp/log

trace-axiom-L:
	go run /sy/doing_os9/gomar/borges/borges.go --outdir /sy/doing_os9/borges/ -glob '*.list,*.lst' .
	cd /sy/doing_os9/gomar && go run --tags=coco3,level2,cocoio gomar.go  --rom_a000 /home/strick/6809/ROMS/color64bas.rom  --rom_8000 /home/strick/6809/ROMS/color64extbas.rom  -cart  ~/coco-shelf/build-frobio/axiom-gomar.rom -v=  2>/tmp/log
	cd /sy/doing_os9/gomar && go run --tags=d,coco3,level2,cocoio,hyper,trace gomar.go  --rom_a000 /home/strick/6809/ROMS/color64bas.rom  --rom_8000 /home/strick/6809/ROMS/color64extbas.rom  -cart  ~/coco-shelf/build-frobio/axiom-gomar.rom -basic_text  -v=dmw  --borges ../borges --trigger_os9='(?i:fork.*file=.dir)'  2>/tmp/log
	# cd /sy/doing_os9/gomar && go run --tags=d,coco3,level2,cocoio,trace,hyper gomar.go  --rom_a000 /home/strick/6809/ROMS/color64bas.rom  --rom_8000 /home/strick/6809/ROMS/color64extbas.rom  -cart  ~/coco-shelf/build-frobio/axiom-gomar.rom -v=w   -basic_text  -v=d  2>/tmp/log

# Disable RCS & SCCS patterns.
%:: %,v
%:: RCS/%,v
%:: RCS/%
%:: s.%
%:: SCCS/s.%
# End.
