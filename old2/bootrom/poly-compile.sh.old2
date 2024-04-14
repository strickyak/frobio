#!/bin/bash
#
# Compile polynetboot.c with lots of compilers and options, and print the sizes to stdout.
# Lots of verbosity goes to stderr.
#
# Usage:
#   sh poly-compile.sh | tee /tmp/sizes
#   cat /tmp/sizes

set -ex

BOOT=${BOOT:-polynetboot}
i=1

for mode in cmoc,-O0, cmoc,-O1, cmoc,-O2, \
            gcc,-O0, gcc,-O1, gcc,-O2, gcc,-Os, \
            gcc,-O2,-fomit-frame-pointer \
            gcc,-Os,-fomit-frame-pointer \
            gcc,-O2,-fwhole-program \
            gcc,-O2,-fomit-frame-pointer,-fwhole-program \
            gcc,-Os,-fwhole-program \
            gcc,-Os,-fomit-frame-pointer,-fwhole-program \

do
  set $(echo $mode | tr "," " ")
  # read Compiler Opt Extra
  Compiler="$1" ; shift
  Opt="$1" ; shift
  Extra="$*"

  (
    case $Compiler in
    cmoc )
     cmoc -o $BOOT.decb $Opt -D'RomMain=main' $BOOT.c
     ;;
    gcc)
     gcc6809 $Opt -S --std=gnu99 stdlib.c
     gcc6809 $Opt -S --std=gnu99 -D'RomMain=main' $Extra $BOOT.c
     ;;
     esac

     case $Compiler in
     cmoc )
       ;;
     gcc )
       lwasm --format=obj \
          --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
          -l'stdlib.list' -o'stdlib.o' \
          stdlib.s

       lwasm --format=obj \
          --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
          -l"$BOOT.list" -o"$BOOT.o" \
          $BOOT.s

       lwlink --decb --entry=_main --output=$BOOT.decb --map=$BOOT.map \
         --script=bootrom.script \
         $BOOT.o stdlib.o \
         -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
         -lgcc
         ;;
     esac
  ) >&2

  echo $(wc -c < $BOOT.decb) :: $i. $Compiler $Opt $Extra
  mv $BOOT.decb $BOOT.decb.$i
  i=$((1 + $i))
done
sleep 1
:
:
