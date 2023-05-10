#!/bin/bash
set -eux

case "$*" in
  EMU*)
    function gcc() {
      gcc6809 -S -I$F/.. -Os -fomit-frame-pointer --std=gnu99 -Wall -DEMULATED=1 "$@"
    }
    function asm() {
      lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource "$@"
    }
  ;;
  *)
    function gcc() {
      gcc6809 -S -I$F/.. -Os -fomit-frame-pointer --std=gnu99 -Wall -Werror "$@"
    }
    function asm() {
      lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource "$@"
    }
  ;;
esac

asm $F/rom/preboot3.asm --output=preboot3.o --list=preboot3.list

gcc $F/rom/romapi3.c -o __romapi3.s
sed -e 's/[.]area[ \t][.]data$/.area api.data/' < __romapi3.s > romapi3.s
asm romapi3.s --output=romapi3.o --list=romapi3.list

gcc $F/rom/netlib3.c -o __netlib3.s
sed -e 's/[.]area[ \t][.]text$/.area net.text/' < __netlib3.s > netlib3.s
asm netlib3.s --output=netlib3.o --list=netlib3.list

gcc $F/rom/dhcp3.c
asm dhcp3.s --output=dhcp3.o --list=dhcp3.list

gcc $F/rom/commands.c
asm commands.s --output=commands.o --list=commands.list

gcc $F/rom/netboot3.c
asm netboot3.s --output=netboot3.o --list=netboot3.list

gcc $F/rom/abort.c
asm abort.s --output=abort.o --list=abort.list

########################################################
cat >'exoboot3.script' <<\HERE
section preboot          load 0x6000
section api.data         load 0x6100
section net.text         load 0x6200
section .text.startup    load 0x6900
section .text
section rom.data
section rom.bss
section .data
section .bss
HERE
lwlink --decb --script=exoboot3.script preboot3.o romapi3.o netlib3.o netboot3.o commands.o dhcp3.o \
    -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ -lgcc abort.o \
    --output=exoboot3.loadm --map=exoboot3.map --entry=_main

ls -l exoboot3.loadm

########################################################
cat >'netboot3.script' <<\HERE
section preboot          load 0xC000
section api.data         load 0xC100
section net.text         load 0xC200
section .text.startup    load 0xC900
section .text
section rom.data
section rom.bss
section .data
section .bss
HERE
lwlink --decb --script=netboot3.script preboot3.o romapi3.o netlib3.o netboot3.o commands.o dhcp3.o \
    -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ -lgcc abort.o \
    --output=netboot3.decb --map=netboot3.map

(cc -g -o utility-decb-to-rom $F/rom/utility-decb-to-rom.c )
./utility-decb-to-rom < netboot3.decb > netboot3.rom
(cc -g -o utility-rom-to-decb3000 $F/rom/utility-rom-to-decb3000.c )
./utility-rom-to-decb3000 < netboot3.rom > netboot3.l3k

ls -l netboot3.decb netboot3.rom netboot3.l3k exoboot3.loadm
