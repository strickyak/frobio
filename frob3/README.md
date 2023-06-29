First you should run these make commands in the $NITROS9DIR:
```
$ cd $NITROS9DIR
$ make PORTS=coco3  dsk
$ make PORTS=coco3_6309  dsk
```

Then these should be in your $PATH:
*  `cmoc`
*  `lwasm`
*  `lwlink`
*  `m6809-unknown-gcc` (version 4.6.4 (gcc6809lw pl9))
*  `os9`
*  `decb`
*  `go` (version 1.18 or later)

Then build the frobio tools, starting from this frob3 directory:

```
$ cd .../frobio/frob3
$ mkdir build
$ cd build
$ ../configure --nitros9=$NITROS9DIR
$ make
```

The results will be in `build/results/CMDS` and `build/results/MODULES`.
Instead of `build`, you can use any directory, anywhere, as long as you
change directory to it, and then run the configure program that is in
this frob3 directory.

To add the results to a disk image, do this, defining DISK to be
where your disk image is, or will be:

```
$ make install-to-disk DISK=/tmp/disk.image
```

TODO: Build Lemma files for distros.
