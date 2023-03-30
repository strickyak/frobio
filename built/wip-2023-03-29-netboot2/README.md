# DEMO of netboot2 (2023-03-29)

Prerequisites:

  * coco3 with 512K RAM. -- Or, for coco1 and coco2, see the bottom.

  * CocoIO cart with W5100S ethernet chip at the default address $FF68.

  * CocoSDC cart with an SD card, and nothing you need to keep in it's Flash ROM slot @4.

  * MPI or MiniMPI or a Y cable that will let you use CocoIO and CocoSDC together.

  * Any Linux machine (like a laptop or a Raspberry Pi) with golang 1.18 or later,
    and the ability to mount the SD card.

  * Private LAN (e.g. just an ethernet cable between the Linux machine and the CocoIO Cart)
    with static IP config.

Steps:

Configure your linux machine's ethernet interface like this,
and plug it into the CocoIO.

    IP address  10.2.2.2
    Mask        255.0.0.0
    Gateway     none (or 0.0.0.0)

Set up your coco3 with a MPI and CocoSDC in slot 1 and CocoIO in slot 2.
Configure it to boot from slot 1.

On a linux box, mount the SD card, and add the file `netboot2.dsk` to it.
You'll find `netboot2.dsk` in this repo directory.

Type `sync`.

Remove the SD card, put it in the CocoSDC, and boot the coco3 using CocoSDC.

Mine is configured to automatically run the GUI.  Use the GUI to select
the `netboot.dsk` and execute the file `INSTALL4.BAS` in it.
THIS WILL OVERWRITE FLASH ROM SLOT NUMBER 4 IN YOUR COCOSDC.
IT WILL NOT ASK IF YOU ARE SURE.  IT DOES IT IMMEDIATELY.

It also reboots your coco3 from ROM slot 4, which runs the netboot2 bootloader.
You should see a banner on the screen like this:

    -- STRICKYAK FROBIO NETBOOT --

and then an error message because we're not running the daemon on Linux yet.

## On Linux

Install golang 1.18 or later.

    $ apt install golang-1.18
    $ go version

Make directories in your $HOME directory like this:

    $ cd
    $ mkdir go
    $ mkdir go/src
    $ mkdir go/src/github.com
    $ mkdir go/src/github.com/strickyak

Clone this archive into that directory.

    $ cd go/src/github.com/strickyak
    $ git clone https://github.com/strickyak/frobio.git

Chdir into `frobio/frob2/waiter/cli` and run the program `server.go`:

    $ cd frobio/frob2/waiter/cli
    $ go run server.go --help

You should see this help message:

    Usage of /tmp/go-build2733676161/b001/exe/waiter:
      -block0 string
    	    filename of block drive 0
      -port int
    	    Listen on this TCP port (default 14511)
      -program string
    	    Program to upload to COCOs

A valid example:

    $ go run server.go --block0 63SDC.VHD -program nitros9-EOU100-h6309.lemma

Don't change the `-port`.
The netboot2 ROM expects the default.

The `-program` flag tells the program or operating system to boot.
Use one of the `*.loadm` or `*.lemma` files in this repo directory:

    demo-dgnpeppr.coco1.loadm
    demo-nyancat.coco3.loadm
    nitros9-EOU100-h6309.lemma
    nitros9-stock330-level1.lemma
    nitros9-stock330-level2-h6309.lemma
    nitros9-stock330-level2.lemma.bad

The `*.loadm` files are single-file binary programs that would
work with the DECB `loadm` command, but don't use any disk
operations after they are loaded.  Try your favorite!

The `*.lemma` files use extensions to the `loadm` format
so that the typical Nitros9 booting mechanism can work.

The `-block0` flag is for an OS9 filesystem disk image
to be the Default Drive /DD.
(None of the boot track or OS9Boot file will be used from it;
those all come from the -program file.)
So you can select the "cocosdc" version, because it will be
large enough to have lots of free space.
Select one that matches the OS and CPU type of the -program flag.
You only need to use `-block0` if you are booting OS9 and
want to have a disk.

    Examples:
    NOS9_6309_L2_cocosdc.dsk
    EOU_Version1_0_0_6309_ONLY_12-02-2022/63SDC.VHD

You can make Stock Nitros9 disks using the nitros9 repo like this:

    $ cd
    $ git clone https://github.com/n6il/nitros9.git
    $ cd nitros9
    $ make PORTS=coco3 dsk
    $ find . -name "*.dsk"

So now, while you are running server.go on the Linux box,
power up your coco3.   If you get the CocoSDC GUI,
hit Break.   Once you have the HDB-DOS BASIC prompt, type `RUN @4`.
That should reboot with the netboot2 ROM code.

You can take the SD card out.  It's not used any more.
It was just for programming the Flash ROM.

You can also change the DIP switches to the binary pattern
for 4, which is `0100` (or: down, up, down, down), to skip
going into DOS and directly boot with Flash ROM slot 4.

# coco1s and coco2s

You can boot Nitros9 Level 1 on a coco1 or coco2 if it has 64K.
It may be a little flaky?

You can probably boot demo-dgnpeppr.coco1.loadm on a coco1.

You can boot demo-nyancat.coco3.loadm on a coco3.

Both of those demos work on a coco2.
