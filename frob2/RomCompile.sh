#!/bin/bash

LOC=${LOC:-C0}
COMPILER=${COMPILER:-cmoc}
OPT=${OPT:--O0}
WHOLE=${WHOLE:-0}
FRAME=${FRAME:-1}
EXTRA=${EXTRA:-}

src=$1
p=$(basename $src .c)
entry=$(python3 -c "print('%x' % ((0x${LOC}) + 2))")

set -eux

case $WHOLE in
  0)
    lwasm --define="WHOLE_PROGRAM=$WHOLE" --obj -lpreboot.list -opreboot.o bootrom/preboot.asm
    ;;
  *)
    sed s/RomMain/main/g <bootrom/preboot.asm >/tmp/prebootmain.asm
    lwasm --define="WHOLE_PROGRAM=$WHOLE" --obj -lpreboot.list -opreboot.o /tmp/prebootmain.asm
    ;;
esac

cat >$p.decb.script <<HERE
define basesympat s_%s
define lensympat l_%s
section start load $LOC
section .text
section code
section *,!bss
section *,bss
entry $entry
HERE

case $WHOLE in
  0) wp= cs=checksum.o ;;
  *) wp=-fwhole-program cs=;;
esac

case $FRAME in
  0) frame=-fomit-frame-pointer ;;
  *) frame=-fno-omit-frame-pointer  ;;
esac

case $COMPILER in
  cmoc)
	  cmoc -i -c -o checksum.o bootrom/checksum.c

	  cmoc -i -S $wp -o $p.s $src
	  lwasm --obj -l$p.o.list -o $p.o $p.s

	  lwlink --entry=program_start --format=decb \
      --output=$p.decb \
      --script=$p.decb.script \
      --map=$p.decb.map \
      -L/opt/yak/cmoc-0.1.80/share/cmoc/lib \
      -lcmoc-crt-ecb -lcmoc-std-ecb \
      preboot.o $p.o checksum.o
    ;;

  gcc)
	  gcc6809 -Os  $frame -S --std=gnu99 bootrom/checksum.c
	  gcc6809 -O2  $frame -S --std=gnu99 bootrom/stdlib.c
	  gcc6809 $OPT $frame -S --std=gnu99 $EXTRA -D"WHOLE_PROGRAM=$WHOLE" $wp $src
	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l'checksum.list' -o'checksum.o' \
      checksum.s
	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l'stdlib.list' -o'stdlib.o' \
      stdlib.s
	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l"$p.o.list" -o"$p.o" \
      $p.s
	  lwlink --decb --entry=program_start --output=$p.decb --map=$p.map \
      --script=bootrom/bootrom-c000.script \
      preboot.o $p.o stdlib.o $cs \
      -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
      -lgcc
    ;;
esac

hd $p.decb | head -1
ls -l $p.decb
dd bs=1 skip=5 if=$p.decb of=$p.cart

# Change the loading address to $3000.
bootrom/decb-reposition-3000 $p.decb 
# Expand it to 2KB and add checksums at the end.
bootrom/expand2000 < $p.decb > $p.exp

# END
