#!/bin/bash

LOC=${LOC:-C0}
COMPILER=${COMPILER:-cmoc}
OPT=${OPT:--O0}
WHOLE=${WHOLE:-0}
NOFP=${NOFP:-0}
EXTRA=${EXTRA:-}

SRC=$1
P=$(basename $SRC .c)
ENTRY=$(python3 -c "print('%x' % ((0x${LOC}) + 2))")

set -eux

lwasm -D"WHOLE_PROGRAM=$WHOLE" --obj -lpreboot.list -opreboot.o bootrom/preboot.asm

cat >$P.decb.script <<HERE
define basesympat s_%s
define lensympat l_%s
section start load $LOC
section .text
section code
section *,!bss
section *,bss
entry $ENTRY
HERE

case $COMPILER in
  cmoc)
	  cmoc -i -c -o checksum.o bootrom/checksum.c

	  cmoc -i -S -o $P.s $SRC
	  lwasm --obj -l$P.o.list -o $P.o $P.s

	  lwlink --entry=program_start --format=decb \
      --output=$P.decb \
      --script=$P.decb.script \
      --map=$P.decb.map \
      -L/opt/yak/cmoc-0.1.80/share/cmoc/lib \
      -lcmoc-crt-ecb -lcmoc-std-ecb \
      preboot.o $P.o checksum.o
    ;;

  gcc)
	  gcc6809 -O2 -S --std=gnu99 bootrom/checksum.c
	  gcc6809 -O2 -S --std=gnu99 bootrom/stdlib.c
	  gcc6809 $OPT -S --std=gnu99 $EXTRA $SRC
	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l'stdlib.list' -o'stdlib.o' \
      stdlib.s
	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l"$P.o.list" -o"$P.o" \
      $P.s
	  lwlink --decb --entry=program_start --output=$P.decb --map=$P.map \
      --script=bootrom/bootrom-c000.script \
      preboot.o $P.o stdlib.o checksum.o \
      -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
      -lgcc
    ;;
esac

hd $P.decb | head -1
ls -l $P.decb
dd bs=1 skip=5 if=$P.decb of=$P.cart


