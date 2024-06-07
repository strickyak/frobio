#!/bin/sh
#
# This script will be copied into the pizga-base/Internal directory of a release.
# To run the lemma waiter server, it should be executed from there.
#
# This version is for running in the cloud.
# It does not have LAN discovery.
#
#    Usage:
#        sh serve-alpha-w2373-h8003/pizga-base/Internal/run-lemma-on-cloud.sh

set -eux
cd "`dirname "$0"`"/../

# Defaults:
waiterPort=2321
httpPort=8001

# Examine our Current Working Directory
# for a piece with a name like serve-alpha-w2373-h8003
# and if so, use that waiter and http port number.
for piece in $(pwd | tr '/' ' ')
do
	echo WOOT $piece
	case "X$piece" in
		X*-w[1-9]*-h[1-9]*[0-9] )   # e.g. serve-alpha-w2373-h8003
			waiterPort=$(expr "X$piece" : 'X.*-w\([1-9][0-9]*\).*')
			httpPort=$(expr "X$piece" : 'X.*-h\([1-9][0-9]*\).*')
		;;
	esac
done

exec Internal/bin/lemma-waiter \
    --cards=true \
    --lemmings_root Internal/LEMMINGS/ \
    --nav_root . \
    --sideload_raw Internal/ETC/sideload.lwraw \
    --inkey_raw  Internal/ETC/inkey_trap.lwraw \
    --scan_keyboard=1 \
    --port=$waiterPort \
    --http_port=$httpPort \
    --web_static Internal/web-static/ \
    ##

# TODO -- needs a "flavor" flag.
