#!/bin/bash
set -euxo pipefail

thisdir=$(dirname $0)
cd $thisdir
shelf_home="$(cd ../../../ && pwd)"
shelf_bin="$(cd ../../../bin/ && pwd)"

export PATH="$shelf_bin:/usr/bin:/bin"
export HOME="$shelf_home"
export GOPATH="$shelf_home"

test -s /tmp/temp.droplet.name
test -s /tmp/temp.droplet.ip
test ! -z $(cat /tmp/temp.droplet.name)
test ! -z $(cat /tmp/temp.droplet.ip)
head /tmp/temp.droplet.name /tmp/temp.droplet.ip
# Create a new droplet in Digital Ocean.
(
	cd $HOME/godo-client &&
	which go >&2 &&
	go env >&2 &&
	go get github.com/digitalocean/godo &&
	go run godo-client.go -f=d $(cat /tmp/temp.droplet.name)
)
rm -f /tmp/temp.droplet.name /tmp/temp.droplet.ip

echo $0 OKAY >&2
