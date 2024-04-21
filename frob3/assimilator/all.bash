#!/bin/bash
set -euxo pipefail

thisdir=$(dirname $0)
cd $thisdir
shelf_home="$(cd ../../../ && pwd)"
shelf_bin="$(cd ../../../bin/ && pwd)"

export PATH="$shelf_bin:/usr/bin:/bin"
export HOME="$shelf_home"
export GOPATH="$shelf_home"


bash 10-create.bash
sleep 120
bash 20-upgrade.bash
bash 25-restart.bash
sleep 60
bash 20-upgrade.bash
bash 30-build.bash

head /tmp/temp.droplet.name /tmp/temp.droplet.ip
echo $0 OKAY >&2
