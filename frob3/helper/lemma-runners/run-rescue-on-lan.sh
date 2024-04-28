#!/bin/sh
#
# This script will be copied into the top directory of a release.
# To run the rescue burner broadcast, it should be executed from there,
# with one argument, the IP addess of the machine.
#
# If the machine has more than one IP address, it should be the
# machine's own address on the same network as the Coco users.
#
#    Usage:
#        sh run-rescue-on-lan.sh 10.23.23.23       # Change to whatever our IP address is.
#
set -ex
cd "`dirname "$0"`"

case $1 in
  [0-9]*.[0-9]*.[0-9]*.*[0-9] )
    LAN="$1"
    shift
  ;;
  * )
    echo "ERROR: $0 requires one argument, the IP address of this machine (on the same interface as the coco will use)." >&2
    exit 13
  ;;
esac


exec bin/broadcast-burn \
    -b "$LAN" \
    -d '((( rescue axiom41-c300.decb )))' \
     < ETC/axiom41-c300.decb
