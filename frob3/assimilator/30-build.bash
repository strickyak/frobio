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
	export DEBIAN_FRONTEND=noninteractive

	echo "
		set -euxo pipefail
		export DEBIAN_FRONTEND=noninteractive

		cat -n /proc/cpuinfo
		env | cat -n
		test -d coco-shelf || git clone https://github.com/strickyak/coco-shelf.git

		cd coco-shelf
		time make ANON=1 KEEP=1 2>&1 | tee /tmp/make.log

		cd build-frobio
		make tarballs
	" | su - coco >&2

'

echo $0 : OKAY >&2 
