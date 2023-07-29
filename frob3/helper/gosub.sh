#!/bin/bash
set -eux

S=$HOME/coco-shelf/
export PATH=$S/bin:/usr/bin:/bin
CMD=$(basename $1 .go)
WORK=$PWD

GOSUB_DIR=/sy/gosub

( cd $GOSUB_DIR &&
  go build gosub.go
  mv gosub $WORK
)

case $CMD in
  wc ) : ok  ;;
  * ) echo WRONG CMD >&2 ; exit 1 ;;
esac

rm -rf "$CMD.sub"
mkdir "$CMD.sub"
cd "$CMD.sub"

mkdir -p runtime
cp /sy/gosub/runtime/*.[ch] runtime/

../gosub < "$1" --libdir /sy/gosub/lib > ___.defs.h 2>___.err || {
  cat ___.err >&2
  echo Stopping due to error. >&2
  exit 13
}

##########################################

todo gcc here
:w


##########################################

for x in *.c
do
  $HOME/coco-shelf/bin/cmoc -i -I. -I../../frobio/ -S $x
done

echo 'void opanic() {}' > runtime/opanic.c
for x in runtime/*.c
do
  case $(basename $x) in
    unix_* ) : skip ;;
    * )
      $HOME/coco-shelf/bin/cmoc -i -I. -I../../frobio/ -S $x ;;
  esac
done

for x in *.s
do
  lwasm --obj -o $(basename $x .s).o $x
done

lwar -c lib'all'.a *.o

O_FILES_FOR_NET_CMDS='stack300.o buf.o flag.o format.o malloc.o nylib.o nystdio.o std.o wiz5100s.o frobos9.o octet2.o'
OFFNC=$( for x in $O_FILES_FOR_NET_CMDS; do echo ../$x ; done)

echo 'void initvars() {}' > initvars.c
echo 'void markvars() {}' > markvars.c
cmoc -i --os9 -o a.out $OFFNC initvars.c markvars.c -L'.' -l'all'

