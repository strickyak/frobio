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
    decb-cmoc )
        cmoc -i --decb -I.. -DFROB_DECB_CMOC -DMAX_VERBOSE="$MAX_VERBOSE" -o "$OBJECT" $(ls "$@" | grep -v os9/) decb/frob-decb.c
        mv "$OBJECT" "$OBJECT.decb"
        hd "$OBJECT.decb" | head -4
        ls -l "$OBJECT.decb"
        echo HINT -- 'decb copy -2 -b -r -c ~/sy/frobio/frob2/f.ticks.decb /media/strick/APRIL3/YDECB.DSK,TICKS.BIN' -- HINT
        ;;
    "" | cmoc )
        cmoc -i --os9 -I.. -DMAX_VERBOSE="$MAX_VERBOSE" -o "$OBJECT" "$@"
        mv "$OBJECT" "$OBJECT.os9"
        os9 ident "$OBJECT.os9"
        ;;
    cmocly )
        /home/strick/go/bin/cmocly --cmoc_pre="-DMAX_VERBOSE=$MAX_VERBOSE" -I=.. -o "$OBJECT" "$@"
        mv "$OBJECT" "$OBJECT.os9"
        os9 ident "$OBJECT.os9"
        ;;
    gcc )
        # $GCC_6809 --std='gnu99' -f'no-builtin' -I.. -Os -o "$OBJECT" "$@"
        for x
        do
          case "$x" in 
            *.c )
                y=$(basename $x .c)
                $GCC_6809 --std='gnu99' -f'pic' -f'no-builtin' -f'unsigned-char' -f'unsigned-bitfields' -I.. -Os -S "$x"
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
        false
        ;;
    * )
        echo "Use environment HOW=cmoc or HOW=cmocly or edit $0" >&2
        exit 13
        ;;
esac
