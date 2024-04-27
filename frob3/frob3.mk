# This file `frob3.mk` lives in frobio/frob3 but is included
# from the directory where you actually build (whose Makefile
# is created and configured by ./configure).

LAN=127.0.0.1
DHCP=0
# This becomes the superdirectory in the release tarballs.
RELEASE=lemma

all: all-net-cmds all-fuse-modules all-fuse-daemons all-drivers all-axiom41 all-hdbdos results1 results2 all-lemmings results2b results3
	find $(RELEASE) -type f -print | LC_ALL=C sort
	sync

tarballs: all lemma-base.tar.bz2 lemma-eou.tar.bz2

#all-without-gccretro: all-net-cmds all-fuse-modules all-fuse-daemons all-drivers results1 all-lemmings results3-without-gccretro tarballs
#	find $(RELEASE) -type f -print | LC_ALL=C sort
#	sync

# Quick assertion that we have the right number of things.
# Change these when you add more things.
NUM_CMDS = 18
NUM_MODULES = 14

###############################################

VPATH = $F $F/axiom41 $F/froblib $F/drivers $F/fuse-modules $F/fuse-daemons $F/net-cmds $F/hdbdos $F/burning $F/lemma/waiter $F/net-games $F/metal

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
	rm -rf ./$(RELEASE)/
	mkdir -p $(RELEASE)/CMDS $(RELEASE)/MODULES $(RELEASE)/NAVROOT $(RELEASE)/NAVROOT/PUBLIC $(RELEASE)/NAVROOT/HOMES
	cp -vf $F/helper/release-runners/*.sh $(RELEASE)/
	set -x; for x in *.os9cmd; do cp -v $$x $(RELEASE)/CMDS/$$(basename $$x .os9cmd) ; done
	set -x; for x in *.os9mod; do cp -v $$x $(RELEASE)/MODULES/$$(basename $$x .os9mod) ; done
	n=$$(ls $(RELEASE)/CMDS/* | wc -l) ; set -x; test $(NUM_CMDS) -eq $$n
	n=$$(ls $(RELEASE)/MODULES/* | wc -l) ; set -x; test $(NUM_MODULES) -eq $$n

results2: results1 os9disks lemma-waiter broadcast-burn
	mkdir -p $(RELEASE)/OS9DISKS $(RELEASE)/LEMMINGS $(RELEASE)/BOOTING $(RELEASE)/bin
	ln -fv lemma-waiter $(RELEASE)/bin
	ln -fv broadcast-burn $(RELEASE)/bin

NOS9_6809_L1_coco1_80d.bigdup : ../nitros9/level1/coco1/NOS9_6809_L1_coco1_80d.dsk
	bash ../frobio/frob3/helper/make-hard-drive.sh $< $@
NOS9_6809_L1_coco1_80d.dsk : NOS9_6809_L1_coco1_80d.bigdup results1
	rm -f $@
	cp -vf $< $@
	sh ../frobio/frob3/helper/os9-install.sh -r $@,CMDS -pe -pr -e -r $(RELEASE)/CMDS/*
	sh ../frobio/frob3/helper/os9-install.sh -r $@,MODULES -pe -pr -e -r $(RELEASE)/MODULES/*

NOS9_6809_L2_coco3_80d.bigdup : ../nitros9/level2/coco3/NOS9_6809_L2_80d.dsk
	bash ../frobio/frob3/helper/make-hard-drive.sh $< $@
NOS9_6809_L2_coco3_80d.dsk : NOS9_6809_L2_coco3_80d.bigdup results1
	rm -f $@
	cp -vf $< $@
	sh ../frobio/frob3/helper/os9-install.sh -r $@,CMDS -pe -pr -e -r $(RELEASE)/CMDS/*
	sh ../frobio/frob3/helper/os9-install.sh -r $@,MODULES -pe -pr -e -r $(RELEASE)/MODULES/*

NOS9_6309_L2_coco3_80d.bigdup : ../nitros9/level2/coco3_6309/NOS9_6309_L2_80d.dsk 
	bash ../frobio/frob3/helper/make-hard-drive.sh $< $@
NOS9_6309_L2_coco3_80d.dsk : NOS9_6309_L2_coco3_80d.bigdup results1
	rm -f $@
	cp -vf $< $@
	bash ../frobio/frob3/helper/os9-install.sh -r $@,CMDS -pe -pr -e -r $(RELEASE)/CMDS/*
	bash ../frobio/frob3/helper/os9-install.sh -r $@,MODULES -pe -pr -e -r $(RELEASE)/MODULES/*

os9disks: NOS9_6809_L1_coco1_80d.dsk NOS9_6809_L2_coco3_80d.dsk NOS9_6309_L2_coco3_80d.dsk
	mkdir -p $(RELEASE)/OS9DISKS $(RELEASE)/LEMMINGS
	set -x; for x in $^; do rm -f $(RELEASE)/LEMMINGS/$$x; ln $$x $(RELEASE)/LEMMINGS/; done
	set -x; for x in $^; do rm -f $(RELEASE)/OS9DISKS/$$x; ln $$x $(RELEASE)/OS9DISKS/; done


results2b: results2 all-axiom41
	mkdir -p $(RELEASE)/BOOTING $(RELEASE)/ETC $(RELEASE)/LEMMINGS
	cp -vf $F/booting/README.md $(RELEASE)/BOOTING/
	cp -vf axiom41-c300.decb $(RELEASE)/ETC/
	cp -vf axiom41.rom $(RELEASE)/ETC/
	cp -vf axiom41.rom.list $(RELEASE)/ETC/
	cp -vf primes41.rom $(RELEASE)/ETC/
	cp -vf primes41.rom.list $(RELEASE)/ETC/
	cp -vf hdbdos.rom $(RELEASE)/ETC/
	cp -vf hdbdos.list $(RELEASE)/ETC/
	cp -vf *.lwraw $(RELEASE)/ETC/
	:
	decb dskini $(RELEASE)/BOOTING/netboot3.dsk
	decb copy -0 -b $F/booting/INSTALL4.BAS $(RELEASE)/BOOTING/netboot3.dsk,INSTALL4.BAS
	decb copy -0 -b $F/booting/INSTALL5.BAS $(RELEASE)/BOOTING/netboot3.dsk,INSTALL5.BAS
	decb copy -3 -a $F/booting/README.md $(RELEASE)/BOOTING/netboot3.dsk,README.md
	decb copy -2 -b axiom41.l3k $(RELEASE)/BOOTING/netboot3.dsk,NETBOOT.DEC
	decb dir $(RELEASE)/BOOTING/netboot3.dsk

results3: results2b
	cp -v $F/../built/wip-2023-03-29-netboot2/demo-dgnpeppr.coco1.loadm $(RELEASE)/LEMMINGS
	cp -v $F/../built/wip-2023-03-29-netboot2/demo-nyancat.coco3.loadm $(RELEASE)/LEMMINGS

#results3-without-gccretro: results2
#	cp -v $F/../built/wip-2023-03-29-netboot2/demo-dgnpeppr.coco1.loadm $(RELEASE)/LEMMINGS
#	cp -v $F/../built/wip-2023-03-29-netboot2/demo-nyancat.coco3.loadm $(RELEASE)/LEMMINGS

###############################################

clean: _FORCE_
	rm -f *.o *.map *.lst *.link *.os9 *.s *.os9cmd *.os9mod _*
	rm -f *.list *.loadm *.script *.decb *.rom *.l3k
	rm -f *.dsk *.lem *.a *.sym *.asmap *.bin *.bigdup *.raw *.lwraw
	rm -f utility-* burn
	rm -rf ./$(RELEASE)
	rm -f broadcast-burn  done  done-without-gccretro  lemma-base.tar.bz2  lemma-eou.tar.bz2  lemma-waiter  server  SPCWARIO.BIN


_FORCE_:

###############################################

# Todo: convert spacewario to metal dir.
all-net-games: spacewario.bin spcwario.dsk
all-metal: burn-hostname.bin show-secrets.bin

spcwario.dsk: spacewario.bin
	rm -f $@
	decb dskini -n'SPCWARIO' $@
	decb copy -2 -b $< $@,SPCWARIO.BIN

spacewario.bin: spacewario.c spacewar-ships.h _FORCE_
	gcc6809 -S $< -I$F/.. --std=gnu99 -Os -f'omit-frame-pointer' -f'whole-program'
	lwasm --format=obj --pragma=newsource --pragma=cescapes spacewario.s -ospacewario.o
	lwlink --format=decb --entry=_main --script=$F/net-games/spacewario.script spacewario.o -o$@

burn-hostname.bin: burn-hostname.c _FORCE_
	gcc6809 -S $< -I$F/.. --std=gnu99 -Os -f'omit-frame-pointer' -f'whole-program'
	lwasm --format=obj --pragma=newsource --pragma=cescapes burn-hostname.s -oburn-hostname.o
	lwlink --format=decb --entry=_main --script=$F/metal/burn-hostname.script burn-hostname.o -o$@

show-secrets.bin: show-secrets.c _FORCE_
	gcc6809 -S $< -I$F/.. --std=gnu99 -Os -f'omit-frame-pointer' -f'whole-program'
	lwasm --format=obj --pragma=newsource --pragma=cescapes show-secrets.s -oshow-secrets.o
	lwlink --format=decb --entry=_main --script=$F/metal/burn-hostname.script show-secrets.o -o$@

###############################################

all-hdbdos: hdbdos.rom hdbdos.lem sideload.lwraw inkey_trap.lwraw

%.lwraw : %.asm
	$(LWASM) --raw $< -o'$@' -I$HOME/coco-shelf/toolshed/hdbdos/ -I../wiz/ --pragma=condundefzero,nodollarlocal,noindex0tonone --list='$@.list' --map='$@.map'

hdbdos.lem: hdbdos.rom
	cat $F/../../toolshed/hdbdos/preload $< $F/../../toolshed/hdbdos/postload > $@
hdbdos.rom: ecb_equates.asm hdbdos.asm 
	$(LWASM) -D'LEMMA=1' -r $^ --output=$@ -I$F/../../toolshed/hdbdos/ -I$F/wiz/ --pragma=condundefzero,nodollarlocal,noindex0tonone --list=hdbdos.rom.list --map=hdbdos.rom.map

###############################################

all-axiom41: axiom41.rom axiom41.decb axiom41.l3k axiom41-c300.decb primes41.decb primes41-c300.decb burn-rom-fast.lem burn-primes-fast.lem 

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

axiom41.l3k: axiom41.rom
	$(GO) run $A/helper/shift-rom-to-3000/main.go < $< > $@

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
	t=$$(basename $@ .os9cmd).gosub && rm -rf ./$$t && mkdir $$t $$t/runtime
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

all-lemmings: burn-rom-fast.lem all-net-games all-metal
	$(GO) run $A/lemmings/*.go --nitros9dir='$(NITROS9)' --shelf='$(SHELF)' --results_dir='$(RELEASE)'
	mkdir -p $(RELEASE)/LEMMINGS/
	ln -fv *.lem $(RELEASE)/LEMMINGS/
	ln -fv *.bin $(RELEASE)/LEMMINGS/
	ln -fv *.dsk $(RELEASE)/LEMMINGS/

#######################################################################################

lemma-base.tar.bz2: _FORCE_
	rm -fv $@
	tar -cjf $@ --exclude='EOU_*.dsk' $(RELEASE)/
lemma-eou.tar.bz2: _FORCE_
	rm -fv $@
	tar -cjf $@ $(RELEASE)/LEMMINGS/EOU_*.dsk

#######################################################################################

# run-lemma
#   This target is not called by default.
#   If you do `make run-lemma-waiter` then it will build and run the lemma-waiter.
#   It's a server, so it will run forever, as long as nothing goes wrong.
#   You can hit ^C to kill it.  `make` will then print an error message
#   that you can ignore.
###lemma-waiter: lemma-waiter.go all-without-gccretro
lemma-waiter: lemma-waiter.go
	P=`pwd` && cd $A/lemma/waiter/ && GOBIN=$(SHELF)/bin GOPATH=$(SHELF) $(GO) build -o $$P/lemma-waiter -x lemma-waiter.go
	ln -fv lemma-waiter ../bin
###broadcast-burn: broadcast-burn.go all-without-gccretro
broadcast-burn: broadcast-burn.go
	P=`pwd` && cd $A/burning/ && GOBIN=$(SHELF)/bin GOPATH=$(SHELF) $(GO) build -o $$P/broadcast-burn -x broadcast-burn.go
	ln -fv broadcast-burn ../bin
run-server: run-lemma-waiter  # Alias.
run-lemma: run-lemma-waiter   # Alias.
run-lemma-waiter: lemma-waiter
	./lemma-waiter  -cards -lemmings_root $(RELEASE)/LEMMINGS -lan=$(LAN) -config_by_dhcp=$(DHCP) --nav_root $F/../../../shelving/nav-root/ $(FORCE)

##############################
# Disable RCS & SCCS patterns.
%:: %,v
%:: RCS/%,v
%:: RCS/%
%:: s.%
%:: SCCS/s.%
# End.
