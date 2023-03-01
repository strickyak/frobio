#!/bin/bash

LOC=${LOC:-C000}
COMPILER=${COMPILER:-cmoc}
OPT=${OPT:--O0}
# WHOLE=${WHOLE:-0}
FRAME=${FRAME:-1}
EXTRA=${EXTRA:-}

src=$1 ; shift
others="$@"
p=$(basename $src .c)
entry=$(python3 -c "print('%x' % ((0x${LOC}) + 2))")

set -eux

# case $WHOLE in
#   0)
#     lwasm --define="WHOLE_PROGRAM=$WHOLE" --obj -lpreboot.list -opreboot.o bootrom/preboot.asm
#     ;;
#   *)
#     sed s/RomMain/main/g <bootrom/preboot.asm >/tmp/prebootmain.asm
#     lwasm --define="WHOLE_PROGRAM=$WHOLE" --obj -lpreboot.list -opreboot.o /tmp/prebootmain.asm
#     ;;
# esac

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

#case $WHOLE in
#  0) wp=
#    ;;
#  *) wp=-fwhole-program
#    ;;
#esac

case $FRAME in
  0) frame=-fomit-frame-pointer ;;
  *) frame=-fno-omit-frame-pointer  ;;
esac

case $COMPILER in
  cmoc)
    lwasm --obj -lpreboot.list -opreboot.o   bootrom/preboot.asm

	  cmoc -i -S -I.. -o $p.s $src
	  lwasm --obj -l$p.o.list -o $p.o $p.s
    ofiles=$p.o

    for x in $others ; do
      y=$(basename $x .c)
	    cmoc -i -S -I.. -o $y.o $x
	    lwasm --obj -l$y.o.list -o $y.o $y.s
      ofiles="$ofiles $y.o"
    done

	  lwlink --entry=program_start --format=decb \
      --output=$p.decb \
      --script=$p.decb.script \
      --map=$p.decb.map \
      -L/opt/yak/cmoc/share/cmoc/lib \
      -lcmoc-crt-ecb -lcmoc-std-ecb \
      preboot.o $ofiles
    ;;

  gcc)
    lwasm --obj -lpreboot.list -opreboot.o   bootrom/preboot.asm
	  gcc6809 -I.. $OPT  $frame -S --std=gnu99 bootrom/stdlib.c
	  gcc6809 -I.. $OPT  $frame -S --std=gnu99 bootrom/abort.c
	  gcc6809 -I.. $OPT $frame -S --std=gnu99 $EXTRA -D"WHOLE_PROGRAM=0" $src
    ofiles="$p.o"

    for x in $others ; do
	    gcc6809 -I.. -Os  $frame -S --std=gnu99 $x
    done
    for x in $others ; do
      y=$(basename $x .c)
	    lwasm --format=obj \
        --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
        -l"$y.o.list" -o"$y.o" $y.s
      ofiles="$ofiles $y.o"
    done

	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l'stdlib.list' -o'stdlib.o' \
      stdlib.s
	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l"$p.o.list" -o"$p.o" \
      $p.s
	  lwlink --decb --entry=program_start --output=$p.decb --map=$p.map \
      --script=$p.decb.script \
      preboot.o $ofiles stdlib.o \
      -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
      -lgcc abort.o
    ;;

  whole-gcc)
    sed s/RomMain/main/g <bootrom/preboot.asm >__prebootmain.s
    lwasm --obj -lpreboot.list -opreboot.o __prebootmain.s

	  gcc6809 -I.. $OPT  $frame -S --std=gnu99 bootrom/stdlib.c
	  gcc6809 -I.. $OPT  $frame -S --std=gnu99 bootrom/abort.c

    cat $src $others > __whole.c

	  gcc6809 -fwhole-program -I.. $OPT $frame -S --std=gnu99 $EXTRA -D"WHOLE_PROGRAM=1" __whole.c

	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l'stdlib.list' -o'stdlib.o' \
      stdlib.s

	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l"__whole.o.list" -o"__whole.o" \
      __whole.s

	  lwlink --decb --entry=program_start --output=$p.decb --map=$p.map \
      --script=$p.decb.script \
      preboot.o __whole.o stdlib.o \
      -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
      -lgcc abort.o
    ;;
esac

hd $p.decb | head -1
ls -l $p.decb
dd bs=1 skip=5 if=$p.decb of=$p.cart

# Expand it to 2KB and add checksums at the end.
bootrom/expand2000 < $p.decb > $p.exp
# Change the loading address to $3000.
bootrom/decb-reposition-3000 $p.exp
# Result in in $p.exp
hd $p.exp | head -1
ls -l $p.exp

# END
      # --script=bootrom/bootrom-c000.script \
