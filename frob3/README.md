To build, please use https://github.com/strickyak/coco-shelf

The README there tells you how to set up an Ubuntu machine
for building frobio and all its dependant packages.

What coco-shelf actually does:

This frob3 directory is designed to be built in a peer directory
(usually named `build-frobio`) to the `frobio` directory.
That is, `build-frobio/` and `frobio/` both have `coco-shelf/`
as their parent directory.

After you make that `build-frobio/` directory and `cd` into it,
run `../frobio/frob3/configure` which will build a small Makefile,
which largely depends on rules in `../frobio/frob3/frob3.mk` .

Then `make all` will do the job.

To run a Lemma waiter on your LAN, `make run-lemma LAN=10.23.23.23 DHCP=1`

Put the IP address of your server, if it is not 10.23.23.23.
Put `DHCP` if the coco should get its address from DHCP,
or leave that out, if the coco should get its address from
the LAN Waiter.

You probably have to unblock TCP port number 2321
and UDP port number 12114 in your server's filewall:

```
$ sudo ufw allow 2321
$ sudo ufw allow 12114
```

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
