set -eux
OS9=${OS9:-os9}
TMP=${TMP:-/tmp}

OS9BOOT=""
while true
do
    case "$1" in
        --os9boot ) OS9BOOT="$2"; shift ; shift ;;
        * ) break ;;
    esac
done

case $# in
  2 ) : good ;;
  * ) echo "Usage:   $0 SourceDiskFile DestDiskFile" >&2
     exit 13 ;;
esac

format_byte="`dd bs=1 skip=10 count=1 <$1 2>/dev/null | od | head -1`"
case $format_byte in
  *02) boot_sector=612 ;;
  *03) boot_sector=1224 ;;
  *) echo "Unknown format byte: $format_byte" >&2
     exit 3 ;;
esac

dd bs=512 skip=$boot_sector count=18 <$1 >$TMP/BootTrack.tmp

os9 format  -t250 -ds $2

if test -z "$OS9BOOT"
then
    os9 copy $1,/OS9Boot  $TMP/OS9Boot.tmp
else
    cp -v "$OS9BOOT" "$TMP/OS9Boot.tmp"
fi
os9 gen -b=$TMP/OS9Boot.tmp -t=$TMP/BootTrack.tmp $2

os9 copy $1,/sysgo $2,/sysgo
os9 attr -e -pe $2,/sysgo

os9 makdir $2,/CMDS
os9 makdir $2,/SYS
os9 dsave -e $1,/CMDS $2,/CMDS
os9 dsave -e $1,/SYS $2,/SYS

os9 free $2

echo "OKAY: FROM $1 CREATED $2" >&2
