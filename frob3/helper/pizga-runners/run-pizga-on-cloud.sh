#!/bin/sh
#
# This script will be copied into the top directory of a release.
# To run the lemma waiter server, it should be executed from there.
#
# This version is for running in the cloud.
# It does not have LAN discovery.
# NOTA BENE: the NAVROOT directory is a peer to this directory.
#
#    Usage:
#        mkdir -p $HOME/serving/NAVROOT
#        sh run-lemma-on-cloud.sh
#

set -ex
cd "`dirname "$0"`"/../

exec Internal/bin/lemma-waiter \
    --cards=true \
    --lemmings_root Internal/LEMMINGS/ \
    --nav_root . \
    --sideload_raw Internal/ETC/sideload.lwraw \
    --inkey_raw  Internal/ETC/inkey_trap.lwraw \
    --scan_keyboard=1 \
    ##
