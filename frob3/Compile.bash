#!/bin/bash
#
# Usage:
#    HOW=cmoc MAX_VERBOSE=9 bash Compile.bash object sources...

source ./config.bash

export FROB3=$(dirname $0)
export FROBIO=$(cd $FROB3/.. ; /bin/pwd)

OBJECT="$(basename $1 .os9)"
shift

MAX_VERBOSE=${MAX_VERBOSE:-9}
GCC6809=${GCC6809:-need-to-define-GCC6809}
CMOC=${CMOC:-need-to-define-CMOC}

set -ex
case $HOW in
    decb-gcc6809 )
        # HINT: HOW=decb-gcc6809 make -B x.decb0.decb
        cat $(ls "$@" | grep -v os9/ | grep -v [.]asm$ ) decb/frob-decb.c decb/std4gcc.c> __combined-$OBJECT.c

        $GCC6809 --std=gnu99 -funsigned-char -funsigned-bitfields -Os -S -fwhole-program -I.. -DMAX_VERBOSE="$MAX_VERBOSE" __combined-$OBJECT.c

        lwasm --obj --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource --list=combined.list --map=combined.map  -I.. -DFOR_DECB=1 -DBY_GCC=1 -DMAX_VERBOSE=9 -o __combined-x.decb0.decb.o  __combined-x.decb0.decb.s

        lwlink --format=decb --entry=_main -o decb0.bin --map=decb0.map __combined-x.decb0.decb.o -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ -lgcc

        echo HINT -- 'decb copy -2 -b -r -c ~/sy/frobio/frob2/decb0.bin /media/strick/APRIL3/YDECB.DSK,DECB0.BIN ; sync' -- HINT
        ;;
    decb-cmoc )
        $CMOC --org=2000 -i --decb -I$FROBIO -DFOR_DECB=1 -DBY_GCC=1 -DMAX_VERBOSE="$MAX_VERBOSE" -o "$OBJECT" $(ls "$@" | grep -v os9/) decb/frob-decb.c
        mv "$OBJECT" "$OBJECT.decb"
        hd "$OBJECT.decb" | head -4
        ls -l "$OBJECT.decb"
        echo HINT -- 'decb copy -2 -b -r -c ~/sy/frobio/frob2/f.ticks.decb /media/strick/APRIL3/YDECB.DSK,TICKS.BIN ; sync' -- HINT
        ;;
    "" | cmoc )
        CMOCI=$(dirname $CMOC)/../share/cmoc/include
        CMOCL=$(dirname $CMOC)/../share/cmoc/lib
        $CMOC $CMOC_PRE -i --os9 -I$FROBIO -I$CMOCI -L$CMOCL -DFOR_LEVEL2=1 -DBY_CMOC=1 -DMAX_VERBOSE="$MAX_VERBOSE" -o "$OBJECT" "$@"
        mv "$OBJECT" "$OBJECT.os9cmd"
        os9 ident "$OBJECT.os9cmd"
        ;;
    cmocly )
        /home/strick/go/bin/cmocly --cmoc_pre="-DFOR_LEVEL2=1 -DBY_CMOC=1 -DMAX_VERBOSE=$MAX_VERBOSE" -I=.. -o "$OBJECT" "$@"
        mv "$OBJECT" "$OBJECT.os9cmd"
        os9 ident "$OBJECT.os9cmd"
        ;;
    gcc )
        for x
        do
          case "$x" in 
            *.c )
                y=$(basename $x .c)
                $GCC6809 -DBY_GCC=1 -DFOR_LEVEL2=1 --std='gnu99' -f'pic' -f'no-builtin' -f'unsigned-char' -f'unsigned-bitfields' -I$FROBIO -Os -S "$x"
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
