#!/bin/bash
#
# Usage:
#    sh Compile object sources...

OBJECT="$(basename $1 .os9)"
shift

set -ex
case $HOW in
    "" | cmoc )
        cmoc -i --os9 -I.. -DMAX_VERBOSE=9 -o "$OBJECT" "$@"
        mv "$OBJECT" "$OBJECT.os9"
        os9 ident "$OBJECT.os9"
        ;;
    cmocly )
        /home/strick/go/bin/cmocly --cmoc_pre="-DMAX_VERBOSE=9" -I=.. --o "$OBJECT" "$@" &&
        mv "$OBJECT" "$OBJECT.os9"
        os9 ident "$OBJECT.os9"
        ;;
    * )
        echo "Use environment HOW=cmoc or HOW=cmocly or edit $0" >&2
        exit 13
        ;;
esac
