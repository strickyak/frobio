#!/bin/bash
#
# Compile polynetboot.c with lots of compilers and options, and print the sizes to stdout.
# Lots of verbosity goes to stderr.
#
# Usage:
#   sh poly-compile.sh | tee /tmp/sizes
#   cat /tmp/sizes

set -ex

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
     cmoc -o polynetboot.decb $Opt -D'RomMain=main' polynetboot.c
     ;;
    gcc)
     gcc6809 $Opt -S --std=gnu99 stdlib.c
     gcc6809 $Opt -S --std=gnu99 -D'RomMain=main' $Extra polynetboot.c
     ;;
     esac

     case $Compiler in
     cmoc )
       ;;
     gcc )
       lwasm --format=obj \
          --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
          -l'stdlib'.list -o'stdlib'.o \
          stdlib.s

       lwasm --format=obj \
          --pragma=undefextern --pragma=cescapes --pragma=importundefexport --pragma=newsource \
          -l'polynetboot'.list -o'polynetboot'.o \
          polynetboot.s

       lwlink --decb --entry=_main --output=polynetboot.decb --map=polynetboot.map \
         --script=bootrom.script \
         polynetboot.o stdlib.o \
         -L/opt/yak/fuzix/lib/gcc/m6809-unknown/4.6.4/ \
         -lgcc
         ;;
     esac
  ) >&2

  echo $(wc -c < polynetboot.decb) :: $Compiler $Opt $Extra
done
sleep 1
:
:
