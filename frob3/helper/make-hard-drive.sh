#!/bin/bash

set -eux

FORMAT_OPTS=-l'65000'
case $# in
  0 | 1)
    echo "Usage:  $0 source.dsk new-dest.dsk [os9 format options... (default is -l'65000')]" >&2
    exit 13
    ;;
  2)
    S="$1"; shift; D="$1"; shift
    ;;
  *)
    S="$1"; shift; D="$1"; shift
    FORMAT_OPTS="$*"
    ;;
esac

BIN=$(dirname $0)

rm -f "$D"
os9 format "$D" $FORMAT_OPTS

function deep_copy () {
  local src="$1"
  local dst="$2"
  local path="$3"
  local read name attrs

  bash $BIN/os9-dir.sh -d $src $path | while read name attrs
  do
    : name $name :: attrs $attrs
    os9 makdir $dst,$path/$name
    # os9 attr $dst,$path/$name $attrs # does not work on directories
    deep_copy $src $dst $path/$name
  done

  bash $BIN/os9-dir.sh -f $src $path | while read name attrs
  do
    : : name = $name : : attrs = $attrs
    os9 copy -r $src,$path/$name $dst,$path/$name
    os9 attr $dst,$path/$name $attrs
  done
}

function deep_copy_dirs () {
  local src="$1"
  local dst="$2"
  local path="$3"
  local read name attrs

  bash $BIN/os9-dir.sh -d $src $path | while read name attrs
  do
    : name $name :: attrs $attrs
    os9 makdir $dst,$path/$name
    # os9 attr $dst,$path/$name $attrs # does not work on directories
    deep_copy_dirs $src $dst $path/$name
  done
}

function deep_copy_files () {
  local src="$1"
  local dst="$2"
  local path="$3"
  local read name attrs

  bash $BIN/os9-dir.sh -f $src $path | while read name attrs
  do
    : : name = $name : : attrs = $attrs
    os9 copy -r $src,$path/$name $dst,$path/$name
    os9 attr $dst,$path/$name $attrs
  done

  bash $BIN/os9-dir.sh -d $src $path | while read name attrs
  do
    deep_copy_files $src $dst $path/$name
  done

}

: deep_copy $S $D /
deep_copy_dirs $S $D /
deep_copy_files $S $D /
