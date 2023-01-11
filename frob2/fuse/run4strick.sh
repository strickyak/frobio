#!/bin/bash
#
# This script is how strick runs his emulator.
# This certainly won't work for you unless modified.

set -ex

WHAT=${WHAT:-80d}
WHAT=${WHAT:-cocosdc}

cd $(dirname $0)

GH=$HOME/go/src/github.com
T=$GH/strickyak/doing_os9/gomar/drive/disk2

(
  cd modules/
  make -B LWASM=lwasm-orig
  cp -f *.os9 /tmp/
)
(
  cd $GH/strickyak/doing_os9/gomar/borges/
  go install borges.go
)

$HOME/go/bin/borges -outdir "$GH/strickyak/doing_os9/gomar/borges/" -glob '*.os9' .

(
  cd $GH/n6il/nitros9/level2/coco3 
  export NITROS9DIR=$GH/n6il/nitros9 
  make clean 
  make 
  make NOS9_6809_L2_v030300_coco3_$WHAT.dsk
  cp -v NOS9_6809_L2_v030300_coco3_$WHAT.dsk $GH/strickyak/doing_os9/gomar/drive/disk2
)

(
  cd daemons/demos
  make -B
  os9 copy -r fuse.ramfile.os9 $T,CMDS/fuse.ramfile
  os9 attr -per $T,CMDS/fuse.ramfile
  os9 copy -r fuse.twice.os9 $T,CMDS/fuse.twice
  os9 attr -per $T,CMDS/fuse.twice
)
(
  cd ..
  make -B x.cat.os9
  os9 copy -r x.cat.os9 $T,CMDS/x.cat
  os9 attr -per $T,CMDS/x.cat
)

# echo 'fuse.twice & sleep 5 ; x.cat -o /fuse/twice/boo startup ; dir ; x.cat /fuse/twice > z9 ; list z9; proc; date; date > /fuse/twice; list /fuse/twice > z1; list z1 ; date > /fuse/twice; list /fuse/twice > z1; list z1 ;  ' | os9 copy -r -l /dev/stdin $GH/strickyak/doing_os9/gomar/drive/disk2,/startup

os9 copy -r -l /dev/stdin $GH/strickyak/doing_os9/gomar/drive/disk2,/startup <<\HERE 
-t
-p
-x
* fuse.twice >>>/w1 &
fuse.ramfile >>>/w1 &
sleep 10
* date > /fuse/twice/foo
date > /fuse/ramfile/short
date -t > /fuse/ramfile/long
dir > /fuse/ramfile/dir
echo ========
* list /fuse/twice/foo
* echo ========
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

echo 'echo Nando' | os9 copy -r -l /dev/stdin $GH/strickyak/doing_os9/gomar/drive/disk2,/startup

(
  cd $GH/strickyak/doing_os9/gomar 

  # go run -x --tags=coco3,level2 gomar.go -boot drive/boot2coco3 -disk drive/disk2 2>/dev/null

  go run -x --tags=coco3,level2,trace gomar.go -boot drive/boot2coco3 -disk drive/disk2 --borges "$GH/strickyak/doing_os9/gomar/borges/" --trigger_os9='(?i:fork.*file=.sleep)' 2>/tmp/_
)
