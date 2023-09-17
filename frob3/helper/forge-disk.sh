#!/bin/bash
set -eux
T=/tmp/$$.tmp

HELPER=$(cd $(dirname $0); /bin/pwd)

case $# in
    0|1|2)
        echo "Usage: $0 new.dsk prototype.dsk boottrack -ModsToDrop... +ModFilesToAdd..." >72
        exit 13
        ;;
esac

NEW="$1" ; shift
OLD="$1" ; shift
TRACK="$1" ; shift
DROPPING=
ADDING=
DIRS=
for x
do
    case "$x" in
        -* )
          DROPPING="$DROPPING $(expr substr "$x" 2 999)"
        ;;
        +* )
          ADDING="$ADDING $(expr substr "$x" 2 999)"
        ;;
        * )
          echo "ERROR: Does not begin with '-' or '+' : $x" >&2
          exit 13
        ;;
    esac
done

rm -f $T
mkdir -p $T
trap ": rm -rf $T" 0 1 2 3
os9 copy "$OLD,os9boot" "$T/os9boot.tmp"
go run $HELPER/drop-os9boot-mods/main.go < "$T/os9boot.tmp" > "$T/os9boot.new" $DROPPING
cat $ADDING >> "$T/os9boot.new"

os9 format -t9999 -ss -dd -q "$NEW" -n"$NEW"
os9 gen "$NEW" -b="$T/os9boot.new" -t="$TRACK"
os9 makdir "$NEW,CMDS"
os9 makdir "$NEW,SYS"
os9 makdir "$NEW,DEFS"
os9 dsave -e -r "$OLD,CMDS" "$NEW,CMDS"
os9 dsave -e -r "$OLD,SYS" "$NEW,SYS"
os9 dsave -e -r "$OLD,DEFS" "$NEW,DEFS"

exit
