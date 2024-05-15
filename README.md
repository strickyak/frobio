# Frobio

Frobio is software written for Computer Conect's `CocoIOr` series
of ethernet cartridges (
https://computerconect.com/products/cocoio-prom?variant=42556416196788 
) for the Radio Shack (Tandy) color computers
(affectionately called "coco"s).  The primary goal is to make the
`CocoIOr` cartridges as useful as possible for the Coco community.

The current frobio sources are all in the `frob3` directory.

This frobio software is mostly written by Henry Strickland,
`strickyak` on github or `strick9` on the Coco Discord,
although it certainly builds on lots of software built
by others.

If you want to build the Frobio software yourself,
it is highly recommended to use the Coco-Shelf repository (
https://github.com/strickyak/coco-shelf ).
That fetches and builds the software that Frobio
depends on, as well as Frobio, and at least one
system test!  I found it to be much cleaner to assume
one location in the filesystem for each dependency,
than to make the locations of everything depend on
lots of variables.  Coco-shelf puts things in the
right place for you.  The current Ubuntu Linux (64-bit OS on `x86_64`)
is the primary platform, but it should build on Raspberry
Pi as well, with the latest Raspberry Pi OS.
Most will also build on MacOS (gcc6809 being the problem).

## AXIOM BOOTROM

One big part of frobio is the bootrom code called Axiom.
This is installed as a "DK"-style external cartridge ROM.
It is called Axiom becuase it only does a small number of
things -- like Peek RAM, Poke RAM, get Keystrokes, display
characters on the screen, and JSR to a location in memory.
Everything else is built on these few things.

It is capable of connecting to a "Waiter", which is its generic
name for any TCP server that understands the Axiom protocols.
The waiter to which it connects determines what software
is available to be run.

## LEMMA SERVER

Another big part of Frobio is the waiter (TCP server) named Lemma.
Most people will use Lemma as their waiter.

It lets you remote boot and execute a wide variety of software,
including Nitros-9 Operating System, HDB-DOS Disk Basic programs,
and "bare metal" programs that just run on the bare 6809 processor
in the Coco (possibly using the network).

## Mount PIZGA

Pizga is the (future) codename for the collection of software available via Lemma.

If someone doesn't really understand Axiom or Lemma, that's OK.
But Pizga is what they will see once those are running.

TODO: Design the Mount Pizga collection, and figure out how to integrate it.

(It's actually named for Mount Pisgah in western North Carolina,
but with "modernized" spelling so that it is more unique to Search Engines.
The origin of the name is Biblical, where in Hebrew it just means "summit" --
but important things happen at summits!)

Note: There are a number of projects that collect lots of existing
coco software, like the CocoSDC collection, or the CocoPI collection,
and probably the FujiNet project.
Pizga will borrow from those, and probably contribute to them.
Maybe there will be a grand convergance!

## Future Intergrations

We hope to tunnel these protocols through our Ethernet Cartridge
and provide support for these networking projects:
 
*  DriveWire
*  FujiNet
*  TNFS

We would also like to support more operating systems, like these:

*  Fuzix
*  Flex
*  Other environments like Forth

Finally, the Lemma server and the Pigza repository should be paired
with a webserver that makes upload and download between Cocos and
other computers easier.

## F-Dot Commands

Historically, the first things written in Frobio were the
so-called F-Dot commands, which naturally begin with a
prefix of "f.".  These predate Axiom and Lemma, are totally independant
of them, and have nothing to do with network booting:

f.arp f.config f.dhcp f.dig f.dump f.ntp f.ping f.recv f.resolv f.send f.tget f.ticks ...

These are for OS-9/Nitros-9 operating systems, especially as running
on a Coco with the CocoIO(r) cartridge.

The current sources for these commands are under `frobio/frob3/net-cmds/`.
These are small tools, each focused on doing one thing, and are similiar
to common Unix software that does the same thing.

The OS9 binaries from the first working version are available in
the directory `frobio/built/v1/CMDS/`.  You can continue to use them
for now.  Copy them to your CMDS directory and set the execute bits.

The OS9 binaries from the second working version are available in
the directory `frobio/built/v2/CMDS/`.

### How to use the F-Dot Commands

If you are plugged into the internet using DHCP,
try the script "f.1" as a demo.  It runs f.dhcp

```
FOR DYNAMIC IP ADDR:
$ f.dhcp coco
    -- Resets the Wiznet chip, and uses DHCP to set the ipv4 address, netmask, & gateway.
    -- The argument must be four letters, like "coco".
    -- That will determine your MAC address and your DHCP hostname.
    -- Always use the same name for each computer, to avoid IP address exhaustion.
    -- Does not automatically renew the lease, but it lasts long enough to play around.
    -- Just run this command again to renew the lease.

OR FOR STATIC IP ADDR:
$ f.config 10.1.2.3 255.0.0.0 10.3.3.3
    -- Reset the Wiznet chip and set the ipv4 address (10.1.2.3),
    -- the netmask (255.0.0.0), and the gateway (10.3.3.3).

$ f.ticks
31159
    -- Get the 16-bit "ticks counter" (in hundreds of microseconds) from
    -- the Wiznet chip.  This can be a source of arbitrary numbers.
    -- Add "-x" for output in Hex.

$ f.ping 10.2.2.2
Ping 10.2.2.2: OK
    -- ping host 10.2.2.2

$ f.arp 10.2.2.2
F0:DE:F1:81:EC:94
    -- use Address Resolution Protocol to get the MAC address for host 10.2.2.2

$ f.dump
    -- like TCPDUMP.  Does not show broadcast, multicast, or ipv6.

$ f.tget 10.2.2.2:69 remote-filename local-filename
    -- Fetch 'remote-filename' from TFTP server 10.2.2.2:69 and save it locally
    -- with local-filename.
    -- BUG: You may have to re-initialize the Wiznet chip (with f.config)
    -- before you can run f.tget again.  Something to do with the Ring Buffer
    -- is left in a bad state by f.tget.

$ date | f.send 10.2.2.2:7777
    -- Known to work if input is ASCII.

$ f.recv -p6666 | ...
    -- Receives UDP packets on given port.
    -- Known to work if packet is ASCII and the string is NL or CR terminated.

$ f.ntp [-s] 10.2.2.2
    -- Displays the time (UTC) from given NTP daemon.
    -- If "-s", then also sets the system time (UTC).

$ f.dig [-a] [-tN] 10.2.2.2 www.mit.edu
    -- Domain Internet Groper, like "dig" in Linux.
    -- "-a" means query all records (or as many as server want to give).
    -- "-tN" (for integer N) queries type N (see RFCs).
    -- first arg "10.2.2.2" is recursive nameserver to use.
    -- second arg "www.mit.edu" is the domain to grope.
```

All of the above programs will take a "-wXXXX" flag to change the Wiznet Port Address
on the 6809 CPU.    The default is "-wFF68".  Use hex.

All of the above programs will take a "-v" flag to enable verbosity.
This is just for debugging.

The above programs do not read any configuration files.
All parameters are on the command line.

## WIFI EXTENDER

You don't have wired Ethernet at the location of your Coco?

This "extender" will connect to your Wifi network (2.4 GHz only)
and make it available on a wired ethernet port.
(It also puts out more Wifi, which might be useful, but is not why I recommend it.)
This is $16 on Amazon and is quite easy to set up with a cell phone web browser.
(You do not need the App or to make an account, even if they encourage it.)

https://www.amazon.com/gp/product/B08DHLCLCY/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1

TP-Link N300 WiFi Extender(RE105), Wall Plug Design, 2.4Ghz only
