#!/bin/bash

set -eu

case $# in
  3)
    : good, $# args. ;;
  *)
    echo "Usage:  $0 [-f | -d] filename.dsk /dir/path/..." >&2
    exit 13 ;;
esac

case $1 in
  -d )
    PATTERN='d......r'
    ;;
  -f )
    PATTERN='-......r'
    ;;
  * )
    echo "Usage:  $0 [-f | -d] filename.dsk /dir/path/..." >&2
    exit 13 ;;
esac

os9 dir -e "$2","$3" | while read Owner    Last modified    Attributes Sector Bytecount Name junk
do
  if expr "X$Attributes" : "X$PATTERN" >/dev/null
  then
    ATTRS=''
    expr "X$Attributes" : 'X.s......' >/dev/null && ATTRS="$ATTRS -s"
    expr "X$Attributes" : 'X..e.....' >/dev/null && ATTRS="$ATTRS -pe"
    expr "X$Attributes" : 'X...w....' >/dev/null && ATTRS="$ATTRS -pw"
    expr "X$Attributes" : 'X....r...' >/dev/null && ATTRS="$ATTRS -pr"
    expr "X$Attributes" : 'X.....e..' >/dev/null && ATTRS="$ATTRS -e"
    expr "X$Attributes" : 'X......w.' >/dev/null && ATTRS="$ATTRS -w"
    expr "X$Attributes" : 'X.......r' >/dev/null && ATTRS="$ATTRS -r"
    echo $Name $ATTRS
  fi
done | LC_ALL=C sort --ignore-case
