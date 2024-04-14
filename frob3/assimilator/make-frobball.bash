#!/bin/bash
set -eux -o pipefail

function die() {
	echo "FATAL: $*" >&2
	exit 13
}

test ! -d "$1" || die "The target directory already exists: $1"

test -d coco-shelf/ || die "Where is coco-shelf/ ?"
test -d shelving/ || die "Where is shelving/ ?"
test -d shelving/lemniscate/ || die "Where is shelving/lemniscate/ ?"
test -d shelving/lemniscate/Coco-Disk-Tree/ || die "Where is shelving/lemniscate/Coco-Disk-Tree/ ?"
test -d shelving/lemniscate/Coco-Disk-Tree/APPS/ || die "Where is shelving/lemniscate/Coco-Disk-Tree/APPS/ ?"
test -d shelving/borges-library/ || die "Where is shelving/borges-library/ ?"

mkdir -p $1
mkdir -p $1/public
mkdir -p $1/homes
mkdir -p $1/secrets
mkdir -p $1/frobio-bin
mkdir -p $1/frobio-data
mkdir -p $1/frobio-raw

rsync -av shelving/lemniscate/ $1/public/
rsync -av coco-shelf/build-frobio/results/ $1/frobio-data/

cp -av coco-shelf/bin/* $1/frobio-bin/
cp -av coco-shelf/bin/server $1/frobio-bin/lemma-waiter-server
cp -av coco-shelf/bin/broadcast-burn $1/frobio-bin/rescue-broadcaster

cp -av $(find coco-shelf/build-frobio/* -prune -type f) $1/frobio-raw
cp -av coco-shelf/conf.mk $1/frobio-raw/shelf-conf-mk
cp -av coco-shelf/Makefile $1/frobio-raw/shelf-makefile

cat >$1/lemma-waiter.sh <<'EOF' 
#!/bin/sh
set -ex
cd `dirname $0`

case $1 in
  [0-9]*.[0-9]*.[0-9]*.*[0-9] )
    BIND="$1"
    shift
  ;;
  * )
    echo "ERROR: $0 requires one argument, the IP address of this machine (on the same interface as the coco will use)." >&2
    exit 13
  ;;
esac

exec frobio-bin/lemma-waiter-server -cards=1 -ro frobio-data/LEMMINGS -lan="$1" -config_by_dhcp=1 --dos_root public/Coco-Disk-Tree/
EOF

cat >$1/rescue-broadcast.sh <<'EOF' 
#!/bin/sh
set -ex
cd `dirname $0`

case $1 in
  [0-9]*.[0-9]*.[0-9]*.*[0-9] )
    BIND="$1"
    shift
  ;;
  * )
    echo "ERROR: $0 requires one argument, the IP address of this machine (on the same interface as the coco will use)." >&2
    exit 13
  ;;
esac

exec frobio-bin/rescue-broadcaster -b="$1" -d="((( AXIOM41 EEPROM IMAGE )))" < frobio-raw/axiom41.decb
EOF
