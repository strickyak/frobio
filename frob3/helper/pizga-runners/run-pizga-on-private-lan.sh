#!/bin/sh
#
# This script will be copied into the top directory of a release.
# To run the lemma waiter server, it should be executed from there,
# with one argument, the IP addess of the machine.
#
# If the machine has more than one IP address, it should be the
# machine's own address on the same network as the Coco users.
#
# This version is for use on your own private LAN
# without a DHCP server.   For instance, if you run one small
# ethernet cable directly from the server machine to the Coco,
# and you set up a static route for that interface on your
# server machine, then that probably does not have a DHCP
# server for that small cable, and you should run this version,
# in whici the lemma-waiter server 
#
#    Usage:
#        sh run-lemma-on-private-lan.sh 10.23.23.23       # Change to whatever your IP address is.
#

set -ex
cd "`dirname "$0"`"/../

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


exec Internal/bin/lemma-waiter \
    --cards=true \
    --lemmings_root Internal/LEMMINGS/ \
    --nav_root      . \
    --web_static    Internal/web-static \
    --sideload_raw  Internal/ETC/sideload.lwraw \
    --inkey_raw     Internal/ETC/inkey_trap.lwraw \
    --scan_keyboard=1 \
    --lan="$LAN" \
    --config_by_dhcp=false \
    ##
