set -ex
- lwasm -D'LEMMA=1' -r ecb_equates.asm hdbdos.asm -o'hdbdos.rom' -I$HOME/coco-shelf/toolshed/hdbdos/ -I../wiz/ --pragma=condundefzero,nodollarlocal,noindex0tonone --list=hdbdos.list --map=hdbdos.map

cat ~/coco-shelf/toolshed/hdbdos/preload ~/coco-shelf/toolshed/hdbdos/preload2 hdbdos.rom ~/coco-shelf/toolshed/hdbdos/postload > ~/coco-shelf/build-frobio/results/LEMMINGS/test92.lem
ls -l  ~/coco-shelf/build-frobio/results/LEMMINGS/test92.lem

