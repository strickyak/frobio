# This file `frob3.mk` lives in frobio/frob3 but is included
# from the directory where you actually build (whose Makefile
# is created and configured by ./configure).

all: all-the-things
	find results -type f -print | LC_ALL=C sort

all-the-things: all-gen-api all-net-cmds all-fuse-modules all-fuse-daemons all-drivers all-axiom results1 all-disks all-lemmings results2

# Quick assertion that we have the right number of things.
# Change these when you add more things.
NUM_CMDS = 17
NUM_MODULES = 11

###############################################

VPATH = $F $F/axiom $F/froblib $F/drivers $F/fuse-modules $F/fuse-daemons $F/net-cmds

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

NOMOD = GO111MODULE=off

###############################################

results1:
	rm -rf results
	mkdir -p results/CMDS results/MODULES
	set -x; for x in *.os9cmd; do cp -v $$x results/CMDS/$$(basename $$x .os9cmd) ; done
	set -x; for x in *.os9mod; do cp -v $$x results/MODULES/$$(basename $$x .os9mod) ; done
	n=$$(ls results/CMDS/* | wc -l) ; set -x; test $(NUM_CMDS) -eq $$n
	n=$$(ls results/MODULES/* | wc -l) ; set -x; test $(NUM_MODULES) -eq $$n

results2: results1
	mkdir -p results/BOOTING results/OS9DISKS results/LEMMINGS
	ln *.dsk results/OS9DISKS
	ln *.lem results/LEMMINGS
	cp -v $F/booting/README.md results/BOOTING/README.md
	decb dskini results/BOOTING/netboot3.dsk
	decb copy -0 -b $F/booting/INSTALL4.BAS results/BOOTING/netboot3.dsk,INSTALL4.BAS
	decb copy -3 -a $F/booting/README.md results/BOOTING/netboot3.dsk,README.md
	decb copy -2 -b axiom-whole.l3k results/BOOTING/netboot3.dsk,NETBOOT.DEC
	decb copy -2 -b axiom-whole6k.decb results/BOOTING/netboot3.dsk,EXOBOOT.BIN
	decb dir results/BOOTING/netboot3.dsk
	ls -l results/BOOTING/


###############################################

clean: _FORCE_
	rm -f *.o *.map *.lst *.link *.os9 *.s *.os9cmd *.os9mod _*
	rm -f *.list *.loadm *.script *.decb *.rom *.l3k
	rm -f *.dsk *.lem *.a *.sym *.asmap *.bin
	rm -f utility-*
	rm -rf results

_FORCE_:

###############################################

all-gen-api: _generated_os9api_for_cmoc.h _generated_os9api_for_cmoc.asm

_generated_os9api_for_cmoc.h:
	$(GO) run $A/helper/api-generator/*.go

_generated_os9api_for_cmoc.asm: _generated_os9api_for_cmoc.h
  

all-axiom: axiom-whole.rom axiom-whole.l3k axiom-whole6k.decb

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
	$(NOMOD) $(GO) run $A/helper/insert-gap-in-asm/main.go  --asm _work.s --map _work.map -o axiom-gomar.s
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  axiom-gomar.s --output=axiom-gomar.o --list=axiom-gomar.list
	$(LWLINK) --output=axiom-gomar.rom --map=axiom-gomar.map --raw --script=$F/helper/axiom.script axiom-gomar.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc

###############################################

NET_CMDS =  f.arp.os9cmd f.config.os9cmd f.dhcp.os9cmd f.dig.os9cmd f.dump.os9cmd f.ntp.os9cmd f.ping.os9cmd f.recv.os9cmd f.send.os9cmd f.telnetd0.os9cmd f.tget.os9cmd f.wget.os9cmd f.ticks.os9cmd
all-net-cmds: $(NET_CMDS)

O_FILES_FOR_NET_CMDS = stack300.o buf.o flag.o format.o malloc.o nylib.o nystdio.o std.o wiz5100s.o frobos9.o _generated_os9api_for_cmoc.o

CDEFS = -DFOR_LEVEL2=1 -DBY_CMOC=1 -DMAX_VERBOSE=9
LIB_CDEFS = -D"BASENAME=\"$$(basename $@ .o)\"" $(CDEFS)
CMD_CDEFS = -D"BASENAME=\"$$(basename $@ .os9cmd)\"" $(CDEFS)

COMPILE_NET_LIB = $(CMOC) -i -c --os9 -I. -I$F/.. -I$(CMOCI) -L$(CMOCL) $(LIB_CDEFS) -o $@ $<

COMPILE_NET_CMD = t=$$(basename $@ .os9cmd); $(CMOC) -i --os9 -I. -I$F/.. -I$(CMOCI) -L$(CMOCL) $(CMD_CDEFS) -o $$t $^ && mv -v $$t $@

C_FILES_FOR_CMOC_ARCHIVE = $F/froblib/buf.c $F/froblib/flag.c $F/froblib/format.c $F/froblib/malloc.c $F/froblib/nylib.c $F/froblib/nystdio.c $F/froblib/std.c $F/wiz/wiz5100s.c $F/os9/frobos9.c
_chopped.a: $(C_FILES_FOR_CMOC_ARCHIVE)
	sh $F/helper/cmoc-chopped.sh _chopped.a "$^" $(CDEFS) -I$F/..

_generated_os9api_for_cmoc.o: _generated_os9api_for_cmoc.asm
	$(LWASM) --obj -o $@ $<
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

f.arp.os9cmd: $F/net-cmds/f-arp.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.config.os9cmd: $F/net-cmds/f-config.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.dhcp.os9cmd: $F/net-cmds/f-dhcp.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.dig.os9cmd: $F/net-cmds/f-dig.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.dump.os9cmd: $F/net-cmds/f-dump.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.ntp.os9cmd: $F/net-cmds/f-ntp.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.ping.os9cmd: $F/net-cmds/f-ping.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.recv.os9cmd: $F/net-cmds/f-recv.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.send.os9cmd: $F/net-cmds/f-send.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.telnetd0.os9cmd: $F/net-cmds/f-telnetd0.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.tget.os9cmd: $F/net-cmds/f-tget.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.wget.os9cmd: $F/net-cmds/f-wget.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

f.ticks.os9cmd: $F/net-cmds/f-ticks.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

all-fuse-daemons: fuse.n.os9cmd fuse.ramfile.os9cmd fuse.tftp.os9cmd fuse.tnfs.os9cmd

fuse.n.os9cmd: $F/fuse-daemons/fuse-n.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

fuse.ramfile.os9cmd: $F/fuse-daemons/fuse-ramfile.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

fuse.tftp.os9cmd: $F/fuse-daemons/fuse-tftp.c $(O_FILES_FOR_NET_CMDS)
	$(COMPILE_NET_CMD)

fuse.tnfs.os9cmd: $F/fuse-daemons/fuse-tnfs.c $(O_FILES_FOR_NET_CMDS)
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
f.ncl.os9cmd:  $(NCL_C) $(NCL_H) $(O_FILES_FOR_NET_CMDS)
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

all-drivers: rblemma.os9mod dd.b0.os9mod b0.os9mod b1.os9mod b2.os9mod b3.os9mod n.fuse.os9mod boot.lemma.os9mod

LWASM_FOR_DRIVER	= t=$$(basename $@ .os9mod); $(LWASM) -o $$t --pragma=pcaspcr,condundefzero,undefextern,dollarnotlocal,noforwardrefmax,export -D'Level=2' -D'H6309=0' -I'.' -I'$F' -I'$F/wiz'  --format=os9 --list=$@.list $< && mv -v $$t $@

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

#######################################################################################

all-lemmings:
	$(NOMOD) $(GO) run $A/lemmings/*.go --nitros9dir='$(NITROS9)'

# # These should have already been done by the Shelf....
# #cd '$(NITROS9)' && make PORTS=coco1 dsk
# #cd '$(NITROS9)' && make PORTS=coco2 dsk
# #cd '$(NITROS9)' && make PORTS=coco3 dsk
# #cd '$(NITROS9)' && make PORTS=coco3_6309 dsk
# # .....................................................
all-disks:
	$(GO) run $A/helper/drop-os9boot-mods/main.go < '$(NITROS9)/level2/coco3/bootfiles/bootfile_80d' > bootfile_80fuse pipeman piper pipe
	cat fuseman.os9mod fuser.os9mod fuse.os9mod >> bootfile_80fuse
	bash $F/helper/make-big-floppy.sh --os9boot bootfile_80fuse '$(NITROS9)/level2/coco3/NOS9_6809_L2_80d.dsk' Nitros9_Coco3_M6809_Level2_80d_big.dsk
	bash $F/helper/add-cmds.sh Nitros9_Coco3_M6809_Level2_80d_big.dsk *.os9cmd
	exit

#	cp -v '$(NITROS9)/level2/coco3_6309/NOS9_6309_L2_cocosdc.dsk' Nitros9_Coco3_H6309_Level2.dsk
#	bash /frob3/helper/make-big-floppy.sh '$(NITROS9)/level2/coco3_6309/NOS9_6309_L2_80d.dsk' Nitros9_Coco3_H6309_Level2_80d_big.dsk
#	: : : make --directory=. install-on-disk DISK=Nitros9_Coco3_M6809_Level2.dsk
#	make --directory=. install-on-disk DISK=Nitros9_Coco3_M6809_Level2_80d_big.dsk
#	: : : make --directory=. install-on-disk DISK=Nitros9_Coco3_H6309_Level2.dsk
#	make --directory=. install-on-disk DISK=Nitros9_Coco3_H6309_Level2_80d_big.dsk
#	: : :
#	: : : bash $F/helper/forge-disk.sh Nitros9_Coco3_M6809_Level2_80d_big_fuse.dsk  '$(NITROS9)/level2/coco3/NOS9_6809_L2_80d.dsk' '$(NITROS9)/level2/coco3/bootfiles/kernel_1773' -pipeman -piper -pipe +fuseman.os9mod +fuser.os9mod +fuse.os9mod 
#X#: : :
#X#os9 copy -r '$(NITROS9)/level2/coco3/NOS9_6809_L2_d80.dsk,os9boot' m_d80.os9boot
#X#os9 ident m_d80.os9boot
#X#$(NOMOD) $(GO) run $A/helper/drop-os9boot-mods/main.go <m_d80.os9boot >m_d80_fuse.os9boot pipeman piper pipe
#X#cat fuseman.os9mod fuser.os9mod fuse.os9mod >>m_d80_fuse.os9boot

#######################################################################################


install-on-disk: _FORCE_
	test ! -z '$(DISK)' || { echo You must define the DISK varaiable >&2 ; exit 13 ; }
	: test -s '$(DISK)' || os9 format -l'65000' '$(DISK)'
	$(OS9) dir '$(DISK),CMDS' 2>/dev/null || $(OS9) makdir '$(DISK),CMDS'
	set -x; for x in results/CMDS/* ; do $(OS9) copy -r "$$x" "$(DISK),CMDS/$$(basename $$x)" ; done
	set -x; for x in results/CMDS/* ; do $(OS9) attr -e -pe "$(DISK),CMDS/$$(basename $$x)" ; done
	$(OS9) dir '$(DISK),MODULES' 2>/dev/null || $(OS9) makdir '$(DISK),MODULES'
	set -x; for x in results/MODULES/* ; do $(OS9) copy -r "$$x" "$(DISK),MODULES/$$(basename $$x)" ; done
	set -x; for x in results/MODULES/* ; do $(OS9) attr -e -pe "$(DISK),MODULES/$$(basename $$x)" ; done
	$(OS9) copy -r $(NITROS9)/level2/coco3/cmds/telnet '$(DISK),CMDS/telnet'
	$(OS9) attr -e -pe '$(DISK),CMDS/telnet'

#######################################################################################
# Disable RCS & SCCS patterns.
%:: %,v
%:: RCS/%,v
%:: RCS/%
%:: s.%
%:: SCCS/s.%
#######################################################################################
# Two Bugs:
# FIXED We're leaving out stack300.
# FIXED We're leaving out FUSE.  And PIPE.  What about the SDC?
# Needed Soon:
#   3.  BigBoot Technique, replacing REL, boot_lemma, and Boot2 chunks.  Regain their space.
#        (boot with all code and data and stack in $2000-$3fff page.  That can clear
#        the $0000-$1fff page, and even initialize the memory map (AnteNitros).
#   4.  Non-PIC compilation of Kernel Modules, for much tighter GCC optimizations,
#        without encountering the PIC bugs.

# Experimental level0:
level0.loadm: $F/level0/level0c.c $F/level0/lib0.c $F/level0/level0.asm $F/level0/abort.asm
	cat  $F/level0/level0c.c $F/level0/lib0.c > _level0_whole.c
	gcc6809 -S -I$F/.. -Os -fwhole-program -fomit-frame-pointer --std=gnu99 -Wall -Werror -D'NEED_STDLIB_IN_NETLIB3' _level0_whole.c
	(echo '  .area .text'; echo '  jmp _main'; sed 's/.text.startup/.text/' < _level0_whole.s) > level0.s
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  -I. $F/level0/level0.asm --output=level0.o --list=level0.list
	$(LWASM) --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource  -I. $F/level0/abort.asm --output=abort.o --list=abort.list
	$(LWLINK) --entry=Boot0 --format=decb --output=level0.bin --map=level0.map --script=$F/helper/level0.script level0.o  -L$F/../../lib/gcc/m6809-unknown/4.6.4/  -lgcc abort.o


#cmoc_level0.loadm: ../level0/level0c.c ../axiom/netlib3.c
#	$(CMOC) -c -i -I../.. $^
#	$(LWLINK) --format=decb --output=cmoc_level0.bin  --map=cmoc_level0.map -L$F/../../share/cmoc/lib -lcmoc-crt-ecb -lcmoc-std-ecb level0c.o netlib3.o
