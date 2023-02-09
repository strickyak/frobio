#!/bin/bash
#
# Usage:
#    HOW=... MAX_VERBOSE=9 sh Compile object sources...

OBJECT="$(basename $1 .os9)"
shift

MAX_VERBOSE=${MAX_VERBOSE:-9}
GCC_6809=${GCC_6809:-/opt/yak/fuzix/bin/m6809-unknown-gcc-4.6.4}

set -ex
case $HOW in
    decb-gcc6809 )
        # HINT: HOW=decb-gcc6809 make -B x.decb0.decb
        cat $(ls "$@" | grep -v os9/ | grep -v [.]asm$ ) decb/frob-decb.c decb/std4gcc.c> __combined-$OBJECT.c

        $GCC_6809 --std=gnu99 -funsigned-char -funsigned-bitfields -Os -S -fwhole-program -I.. -DMAX_VERBOSE="$MAX_VERBOSE" __combined-$OBJECT.c

        lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource --list=combined.list --map=combined.map  -I.. -DFOR_DECB=1 -DBY_GCC=1 -DMAX_VERBOSE=9 -o __combined-x.decb0.decb.o  __combined-x.decb0.decb.s

        lwlink --format=decb --entry=_main -o decb0.bin --map=decb0.map __combined-x.decb0.decb.o -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ -lgcc

        echo HINT -- 'decb copy -2 -b -r -c ~/sy/frobio/frob2/decb0.bin /media/strick/APRIL3/YDECB.DSK,DECB0.BIN ; sync' -- HINT
        ;;
    decb-cmoc )
        cmoc --org=2000 -i --decb -I.. -DFOR_DECB=1 -DBY_GCC=1 -DMAX_VERBOSE="$MAX_VERBOSE" -o "$OBJECT" $(ls "$@" | grep -v os9/) decb/frob-decb.c
        mv "$OBJECT" "$OBJECT.decb"
        hd "$OBJECT.decb" | head -4
        ls -l "$OBJECT.decb"
        echo HINT -- 'decb copy -2 -b -r -c ~/sy/frobio/frob2/f.ticks.decb /media/strick/APRIL3/YDECB.DSK,TICKS.BIN ; sync' -- HINT
        ;;
    "" | cmoc )
        cmoc -i --os9 -I.. -DFOR_LEVEL2=1 -DBY_CMOC=1 -DMAX_VERBOSE="$MAX_VERBOSE" -o "$OBJECT" "$@"
        mv "$OBJECT" "$OBJECT.os9"
        os9 ident "$OBJECT.os9"
        ;;
    cmocly )
        /home/strick/go/bin/cmocly --cmoc_pre="-DFOR_LEVEL2=1 -DBY_CMOC=1 -DMAX_VERBOSE=$MAX_VERBOSE" -I=.. -o "$OBJECT" "$@"
        mv "$OBJECT" "$OBJECT.os9"
        os9 ident "$OBJECT.os9"
        ;;
    gcc )
        for x
        do
          case "$x" in 
            *.c )
                y=$(basename $x .c)
                $GCC_6809 -DBY_GCC=1 -DFOR_LEVEL2=1 --std='gnu99' -f'pic' -f'no-builtin' -f'unsigned-char' -f'unsigned-bitfields' -I.. -Os -S "$x"
                lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource -o "$y.o" -l"$y.o.list" "$y.s"
                ;;
            *.asm )
                y=$(basename $x .asm)
                lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource -o "$y.o" -l"$y.o.list" "$y.asm"
                ;;
            * )
                echo "UNKNOWN COMPILE THING: $x" >&2
                exit 13
                ;;
          esac
        done
        ;;
    * )
        echo "Use environment HOW=cmoc or HOW=cmocly or edit $0" >&2
        exit 13
        ;;
esac
