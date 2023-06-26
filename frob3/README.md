How to build:

```
$ mkdir build
$ cd build
$ ../configure
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
