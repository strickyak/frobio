#!/bin/bash
set -eux

alias gcc='gcc6809 -S -I.. -Os -fomit-frame-pointer --std=gnu99'

alias asm='lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource'

asm bootrom/preboot3.asm --output=preboot3.o --list=preboot3.list

gcc bootrom/romapi3.c -o __romapi3.s
sed -e 's/[.]area [.]data$/.area api.data/' < __romapi3.s > romapi3.s
asm romapi3.s --output=romapi3.o --list=romapi3.list

gcc bootrom/netlib3.c -o __netlib3.s
sed -e 's/[.]area [.]text$/.area net.text/' < __netlib3.s > netlib3.s
asm netlib3.s --output=netlib3.o --list=netlib3.list

gcc bootrom/netboot3.c
asm netboot3.s --output=netboot3.o --list=netboot3.list

gcc bootrom/abort.c
asm abort.s --output=abort.o --list=abort.list

cat >'netboot3.script' <<\HERE
section preboot  load 0xC000
section api.data load 0xC100
section net.text load 0xC200
section .text    load 0xC900
section rom.data
section rom.bss
section .data
section .bss
HERE
lwlink --decb --script=netboot3.script preboot3.o romapi3.o netlib3.o netboot3.o \
    -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ -lgcc abort.o \
    --output=netboot3.decb --map=netboot3.map

(cc -g -o decb-to-rom bootrom/decb-to-rom.c )
./decb-to-rom < netboot3.decb > netboot3.rom
(cc -g -o rom-to-decb3000 bootrom/rom-to-decb3000.c )
./rom-to-decb3000 < netboot3.rom > netboot3.l3k

ls -l netboot3.decb netboot3.rom netboot3.l3k

exit 0

false <<'END'
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LOC=${LOC:-C000}
COMPILER=${COMPILER:-cmoc}
OPT=${OPT:--O0}
# WHOLE=${WHOLE:-0}
FRAME=${FRAME:-1}
PIC=${PIC:-}

src=$1 ; shift
others="$@"
p=$(basename $src .c)
entry=$(python3 -c "print('%x' % ((0x${LOC}) + 2))")

set -eux

# case $WHOLE in
#   0)
#     lwasm --define="WHOLE_PROGRAM=$WHOLE" --obj -lpreboot3.list -opreboot3.o bootrom/preboot3.asm
#     ;;
#   *)
#     sed s/RomMain/main/g <bootrom/preboot3.asm >/tmp/preboot3main.asm
#     lwasm --define="WHOLE_PROGRAM=$WHOLE" --obj -lpreboot3.list -opreboot3.o /tmp/preboot3main.asm
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
    lwasm --obj -lpreboot3.list -opreboot3.o   bootrom/preboot3.asm

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
      preboot3.o $ofiles
    ;;

  gcc)
    lwasm --obj -lpreboot3.list -opreboot3.o   bootrom/preboot3.asm
	  gcc6809 -I.. $OPT $frame -S --std=gnu99 $PIC -D"WHOLE_PROGRAM=0" $src
    ofiles="$p.o"

    for x in $others bootrom/abort.c ; do
	    gcc6809 -I.. $OPT  $frame -S --std=gnu99 $PIC $x
    done
    for x in $others bootrom/abort.c ; do
      y=$(basename $x .c)
	    lwasm --format=obj \
        --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
        -l"$y.o.list" -o"$y.o" $y.s
      ofiles="$ofiles $y.o"
    done

	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l"$p.o.list" -o"$p.o" \
      $p.s
	  lwlink --decb --entry=program_start --output=$p.decb --map=$p.map \
      --script=$p.decb.script \
      preboot3.o $ofiles \
      -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
      -lgcc abort.o
    ;;

  whole-gcc)
    sed s/RomMain/main/g <bootrom/preboot3.asm >__preboot3main.s
    lwasm --obj -lpreboot3.list -opreboot3.o __preboot3main.s

	  gcc6809 -I.. $OPT  $frame -S --std=gnu99 $PIC bootrom/stdlib.c
	  gcc6809 -I.. $OPT  $frame -S --std=gnu99 $PIC bootrom/abort.c

    cat $src $others > __whole.c

	  gcc6809 -fwhole-program -I.. $OPT $frame -S --std=gnu99 $PIC -D"WHOLE_PROGRAM=1" __whole.c

	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l'stdlib.list' -o'stdlib.o' \
      stdlib.s
	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l'abort.list' -o'abort.o' \
      abort.s

	  lwasm --format=obj \
      --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
      -l"__whole.o.list" -o"__whole.o" \
      __whole.s

	  lwlink --decb --entry=program_start --output=$p.decb --map=$p.map \
      --script=$p.decb.script \
      preboot3.o __whole.o stdlib.o \
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

END
