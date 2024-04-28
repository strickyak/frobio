#!/bin/sh
#
# This script will be copied into the top directory of a release.
# To run the lemma waiter server, it should be executed from there,
# with one argument, the IP addess of the machine.
#
# If the machine has more than one IP address, it should be the
# machine's own address on the same network as the Coco users.
#
# This version is for use on your own LAN when you have
# a DHCP server (built into your router, perhaps) to
# give out IP Addresses to machines.
#
#    Usage:
#        sh run-lemma-on-dhcp-lan.sh 10.23.23.23       # Change to whatever our IP address is.
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


exec bin/lemma-waiter \
    --cards=true \
    --lemmings_root LEMMINGS/ \
    --nav_root NAVROOT/ \
    --sideload_raw ETC/sideload.lwraw \
    --inkey_raw  ETC/inkey_trap.lwraw \
    --scan_keyboard=1 \
    --lan="$LAN" \
    --dhcp=true \
    ##
