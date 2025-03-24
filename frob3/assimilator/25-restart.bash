#!/bin/bash
set -euxo pipefail

thisdir=$(dirname $0)
cd $thisdir
shelf_home="$(cd ../../../ && pwd)"
shelf_bin="$(cd ../../../bin/ && pwd)"

export PATH="$shelf_bin:/usr/bin:/bin"
export HOME="$shelf_home"
export GOPATH="$shelf_home"

H=`cat /tmp/temp.droplet.ip`
ssh -o "UserKnownHostsFile=/dev/null" -o "StrictHostKeyChecking=no" -n root@$H '
	set -euxo pipefail
	( sync; sleep 1; sync; sync; sleep 1 ; reboot ) &
	exit
'
echo $0 OKAY >&2
