#!/bin/bash
#
# This script is how strick runs his emulator.
# This certainly won't work for you unless modified.

set -ex

WHAT=${WHAT:-80d}
WHAT=${WHAT:-cocosdc}

cd $(dirname $0)
FUSE=$(pwd)         # Fuse Dir, absolute
FM=$FUSE/modules    # Fuse Modules, absolute

GH=$HOME/go/src/github.com
T=$GH/strickyak/doing_os9/gomar/drive/disk2

(
  cd modules/
  make -B LWASM=lwasm-orig
)
(
  cd $GH/strickyak/doing_os9/gomar/borges/
  go install borges.go
)

$HOME/go/bin/borges -outdir "$GH/strickyak/doing_os9/borges/" -glob '*.os9' .


(
  cd $GH/n6il/nitros9/level2/coco3 
  export NITROS9DIR=$GH/n6il/nitros9 
  make
  make -C bootfiles clean
  make -C bootfiles
  cat $FM/fuseman.os9mod $FM/fuser.os9mod $FM/fuse.os9mod >> bootfiles/bootfile_$WHAT
  rm -f NOS9_6809_L2_v030300_coco3_$WHAT.dsk
  # This `make` depends on *not* re-making bootfiles.
  make  NOS9_6809_L2_v030300_coco3_$WHAT.dsk
  cp -v NOS9_6809_L2_v030300_coco3_$WHAT.dsk $T
)

(
  cd daemons/demos
  make -B
  for x in *.os9cmd
  do
    y=$(basename $x .os9cmd)
    os9 copy -r $x $T,CMDS/$y
    os9 attr -per $T,CMDS/$y
  done
)
(
  cd ..
  make

  make f.ncl.os9
  os9 copy -r -l ncl/nclrc.tcl $T,SYS/nclrc.tcl

  #make yadd.os9
  #os9 copy -r yadd.os9 $T,CMDS/yadd
  #os9 attr -per $T,CMDS/yadd
  #make yadd2.os9
  #os9 copy -r yadd2.os9 $T,CMDS/yadd2
  #os9 attr -per $T,CMDS/yadd2
  #make yadd2all.os9
  #os9 copy -r yadd2all.os9 $T,CMDS/yadd2all
  #os9 attr -per $T,CMDS/yadd2all

  for x in [fx].*.os9
  do
    y=$(basename $x .os9)
    os9 copy -r $x $T,CMDS/$y
    os9 attr -per $T,CMDS/$y
  done
  for x in [fx].*.shell
  do
    y=$(basename $x .shell)
    os9 copy -r -l $x $T,CMDS/$y
    os9 attr -per $T,CMDS/$y
  done
)

: os9 copy -r -l /dev/stdin $T,/s.5 <<'HERE'
t
-p
-x
fuse.ramfile >>>/w1 &
sleep 10
date > /fuse/ramfile/short
date -t > /fuse/ramfile/long
dir > /fuse/ramfile/dir
echo ========
sleep 5
list /fuse/ramfile/short
echo ========
sleep 5
list /fuse/ramfile/long
echo ========
sleep 5
list /fuse/ramfile/dir
echo ========
HERE
os9 copy -r -l /dev/stdin $T,/s.5 <<'HERE'
t
fuse.ramfile &
sleep 5
date -t > /fuse/ramfile/aaa
list /fuse/ramfile/aaa > zaaa
list zaaa
date -t > /fuse/ramfile/bbb
list /fuse/ramfile/bbb > zbbb
list zbbb
date -t > /fuse/ramfile/ccc
list /fuse/ramfile/ccc > zccc
list zccc
HERE
os9 attr -per $T,s.5

echo 'echo Nando' | os9 copy -r -l /dev/stdin $GH/strickyak/doing_os9/gomar/drive/disk2,/startup

(
  cd $GH/strickyak/doing_os9/gomar 

  case $WHAT in
    80d )
      go run -x --tags=cocoio,coco3,level2 gomar.go -boot drive/boot2coco3 -disk drive/disk2 2>/dev/null

      # go run -x --tags=cocoio,coco3,level2,trace,d gomar.go -boot drive/boot2coco3 -disk drive/disk2 --borges "$GH/strickyak/doing_os9/borges/" --trigger_os9='(?i:fork.*file=.sleep)' 2>/tmp/_
      ;;
    cocosdc )
      test -d /media/strick/APRIL3/ && cp $T /media/strick/APRIL3/z ||
          scp $T root@yak.net:/tmp/
      sync
      ;;
  esac

)
