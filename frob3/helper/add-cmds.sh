#!/bin/bash
#
#  Usage:
#    bash $0 disk_to_modify *.os9cmd
#
set -ex

case $# in
    0 | 1 )
        echo "Usage:  bash $0 disk_to_modify *.os9cmd" >&2
        exit 7
        ;;
esac

DISK="$1"
shift

for x
do
    case $x in
        *.os9cmd )
            y=$(basename $x .os9cmd)
            os9 copy -r "$x" "$DISK,/CMDS/$y"
            os9 attr -e -pe "$DISK,/CMDS/$y"
            ;;

         * )
            echo "Not a .os9cmd file: $x" >&2
            exit 13
            ;;
    esac
done 
