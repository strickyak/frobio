#!/bin/bash
set -eux

# -- coco-shelf locations --
F=../frobio/frob3
CMOC=../bin/cmoc
CMOCI=../share/cmoc/include/
CMOCL=../share/cmoc/lib/
LWAR=../bin/lwar

# -- command line parameters --
AR_OUT="$1" ; shift
C_IN="$1" ; shift

python3 $F/helper/chopper.py $C_IN
rm -f __[0-9][0-9][0-9][0-9][0-9]_*.o

for x in __[0-9][0-9][0-9][0-9][0-9]_*.c
do
  b=$(basename $x .c)
  $CMOC -c --intermediate -DBASENAME="\"$b\"" -I$CMOCI -L$CMOCL "$@" "$x"
done

rm -f "$AR_OUT"
$LWAR -c "$AR_OUT" __[0-9][0-9][0-9][0-9][0-9]_*.o
