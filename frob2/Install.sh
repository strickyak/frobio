#!/bin/bash
#
# Install files on an OS-9 disk image, using the "os9" command.
#
# Set the filename of the disk image with the DISK environment variable.
#
# One of the following extensions is required, and it will be removed from the copy:
#
# If extension is ".os9", copy the file into CMDS and make it executable.
# If extension is ".shell", copy the file as text into CMDS and make it executable.
# If extension is ".bin", copy the file into the root directory and make it readable.
# If extension is ".text", copy the file as text into the root directory and make it readable.

set -ex

DISK="${DISK:-/media/strick/APRIL3/MY.DSK}"

for x; do
    case "$x" in
        *.os9 )
            B=$(basename "$x" .os9 | tr A-Z a-z)
            os9 copy -r "$x" "${DISK},CMDS/$B"
            os9 attr -per "${DISK},CMDS/$B"
            ;;
        *.shell )
            B=$(basename "$x" .shell | tr A-Z a-z)
            os9 copy -l -r "$x" "${DISK},CMDS/$B"
            os9 attr -per "${DISK},CMDS/$B"
            ;;
        *.bin )
            B=$(basename "$x" .bin | tr A-Z a-z)
            os9 copy -r "$x" "${DISK},$B"
            os9 attr -pr "${DISK},$B"
            ;;
        *.txt )
            B=$(basename "$x" .txt | tr A-Z a-z)
            os9 copy -l -r "$x" "${DISK},$B"
            os9 attr -pr "${DISK},$B"
            ;;
        * )
            echo "$0: FATAL: Unknown extension: $x" >&2
            exit 13
            ;;
    esac
done
sync
