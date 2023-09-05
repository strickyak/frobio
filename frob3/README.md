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

## Running your own waiter

If you run your own waiter on the piece of ethernet connected directly
(or though a simple hub) to the CocoIO card, staticly configure the IPv4
settings for that network interface to one of the following:

```
   (Class A)  10.23.23.23   (netmask 255.255.255.0)
or (Class B)  176.23.23.23  (netmask 255.255.255.0)
or (Class C)  192.168.23.23 (netmask 255.255.255.0)
```

When you boot the Coco with the Axiom bootrom, use the Axiom command "A",
"B", or "C" to tell the bootrom which of those three your server is on.

If your ethernet needs to be configured to something else, you can use
the Axiom "I" command to set a different IP address and netmask and router
for the CocoIO card, and the "W" command to set the IP address to use
for the waiter.

After any of those commands, you can use "S" to confirm what settings the
Axiom bootrom will use.

### Using an alternate waiter.

A waiter is any program that runs on a server (like a Linux box)
and listens on port 2319.  The "lemma" program included in frob3
is designed to be a flexable waiter program.  But a waiter can be
as simple as the "netcat" command listening on port 2319 and serving
a single DECB-style binary, like this:

```
$ netcat -l 2319 < frobio/built/wip-2023-03-29-netboot2/demo-nyancat.coco3.loadm
```

But you'll have to kill and restart the "netcat" command after each use,
because it doesn't know how to rewind and reuse its input.

TODO: Document the extra packets the waiter can use, beyond the normal 00 and FF
packets of DECB.  Especially keyboard and terminal packets.
