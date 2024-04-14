#!/bin/bash
set -euxo pipefail

thisdir=$(dirname $0)
cd $thisdir
shelf_home="$(cd ../../../ && pwd)"
shelf_bin="$(cd ../../../bin/ && pwd)"

export PATH="$shelf_bin:/usr/bin:/bin"
export HOME="$shelf_home"
export GOPATH="$shelf_home"

# Find IP ADDY of the droplet in Digital Ocean.
(
	cd $HOME/godo-client &&
	which go >&2 &&
	go env >&2 &&
	go get github.com/digitalocean/godo &&
	go run godo-client.go -f=n $(cat /tmp/temp.droplet.name) > /tmp/temp.droplet.ip
)
head /tmp/temp.droplet.name /tmp/temp.droplet.ip

H=`cat /tmp/temp.droplet.ip`
ssh -o "UserKnownHostsFile=/dev/null" -o "StrictHostKeyChecking=no" -n root@$H '
	set -euxo pipefail
	export DEBIAN_FRONTEND=noninteractive

	apt -y update < /dev/null
	apt -y upgrade < /dev/null

	apt -y install gcc make flex bison gdb build-essential < /dev/null
	apt -y install git golang zip curl python3-serial < /dev/null
	apt -y install libgmp-dev libmpfr-dev libmpc-dev libfuse-dev < /dev/null

	test -d /home/coco || useradd --create-home --shell /bin/bash coco 
'
echo $0 OKAY >&2
