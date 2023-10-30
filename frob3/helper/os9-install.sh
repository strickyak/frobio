#!/bin/bash
#
#  bash os9-install.sh [-l] [-r] dest-dsk,/dest-dir [-attrs...] src-files...
#
#  Removes ".os9cmd" or ".os9mod" from filenames.
#  Makes destination dir (just the final part).

set -eu

case $# in
  0)
    echo "Usage:  $0 dest-dsk,/dest-dir src-files..." >&2
    exit 13 ;;
esac

OPTS=
while true
do
  case $1 in
    -*) OPTS="$OPTS $1"; shift ;;
    *) break ;;
  esac
done

DEST="$1" ; shift

ATTRS='-r'
while true
do
  case $1 in
    -*) ATTRS="$ATTRS $1"; shift ;;
    *) break ;;
  esac
done

set -x
for x
do
  case $x in
    *.os9cmd )
      b=$(basename $x .os9cmd)
      os9 makdir $DEST || : ignore makdir error
      os9 copy $OPTS $x $DEST/$b
      os9 attr $DEST/$b -e $ATTRS
      ;;
    *.os9mod )
      b=$(basename $x .os9mod)
      os9 makdir $DEST || : ignore makdir error
      os9 copy $OPTS $x $DEST/$b
      os9 attr $DEST/$b -e $ATTRS
      ;;
    * )
      b=$(basename $x)
      os9 makdir $DEST || : ignore makdir error
      os9 copy $OPTS $x $DEST/$b
      os9 attr $DEST/$b $ATTRS
      ;;
  esac
done
