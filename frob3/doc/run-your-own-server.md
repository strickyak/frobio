# How to run your own Server

When your Coco boots up with the CocoIOr cartridge
and the Axiom bootrom code in the EEPROM, its goal is to
connect to a TCP server, called its Waiter.

The primary waiter that Frobio supports is
a program named Lemma.

To run your own Lemma, you could compile your own,
using the
[coco-shelf](https://github.com/strickyak/coco-shelf)
and 
[frobio](https://github.com/strickyak/frobio)
packages from GitHub, or you can run a
pre-built release.

## Downloading prebuilt packages

Download one or more of these tarballs
from http://pizga.net:8001/releases/ :

  * pizga-base.tar.bz2 (Required)
  * pizga-eou.tar.bz2 (for Nitros-9 Ease Of Use edition)
  * pizga-media.tar.bz2 (for demo audio/video movies)
  * pizga-sdc.tar.bz2 (for the SDC repository of Coco software)

You can do this in a directory of your choosing.
For simplicity, it may be your home directory;
everything will be in appropriately-named subdirectories.

You must download pizga-base.tar.bz2 and this command
will expand it (change the final word to the path
to your download):

```
tar -xjf Downloads/pizga-base.tar.bz2
```

That will create a directory named `pizga-base`.

Optionally, download any or all of the other three
and use the same `tar -xjf` command to expand them.

If you are upgrading to new versions, you can delete the
old directory, and then expand the tarball:

```
rm -rf pizga-base
tar -xjf Downloads/pizga-base.tar.bz2
```

## Running Lemma on Linux `X86_64` on a network with DHCP

If your home network has a DHCP server (usually inside
your home broadband router) and your router will allow
traffic from one machine in your home to another (at
home this is almost always enabled -- at a hotel
or coffeeshop it may not be), you can run Lemma
this way on your Linux laptop or workstation:

```
sh pizga-base/Internal/run-pizga-on-dhcp-lan.sh 192.168.86.235
```

Replace the final word with the IP address that DHCP gave
your laptop.  You can find this out with this command:

```
ifconfig
```

Look for the interface that is your home network with DHCP.
In the following sample output, that is the Wifi interface,
usually starting with the letter "w".  In my case, the interface
is named `wlp0s20f3` and on the next line it says
`inet 192.168.86.235`.   So my IP address is `192.168.86.235`,
and that is what is used for the last word on the sh command.

Here is my sample `ifconfig` command and its output:

```
$ ifconfig
enx5c857e3f5a6c: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 10.23.23.23  netmask 255.255.255.0  broadcast 10.23.23.255
        inet6 fe80::ba5c:710d:e748:7031  prefixlen 64  scopeid 0x20<link>
        ether 5c:85:7e:3f:5a:6c  txqueuelen 1000  (Ethernet)
        RX packets 15759  bytes 1418395 (1.4 MB)
        RX errors 0  dropped 2  overruns 0  frame 0
        TX packets 4277  bytes 3491275 (3.4 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 68  bytes 5634 (5.6 KB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 68  bytes 5634 (5.6 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

wlp0s20f3: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.86.235  netmask 255.255.255.0  broadcast 192.168.86.255
        inet6 fe80::c5:a516:8e95:338a  prefixlen 64  scopeid 0x20<link>
        ether c0:a5:e8:5a:76:2f  txqueuelen 1000  (Ethernet)
        RX packets 35067  bytes 33293813 (33.2 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 17410  bytes 6992117 (6.9 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```


## Running Lemma on Linux `X86_64` on a private network without DHCP

In the above `ifconfig` output, notice there is another interface named
`enx5c857e3f5a6c`.  That is a statically-configured wired network on
my Linux laptop.  I can connect a Cat5 ethernet cable directly between
my laptop and the CocoIOr board.  Since my home router
does not know anything about this wired network, and it is not
running a DHCP server, I can use this command to run the 
Lemma waiter:

```
sh pizga-base/Internal/run-pizga-on-private-lan.sh 10.23.23.23
```

The time the last word is the IP address of the Linux machine
on the private interface, which was found on the line following
`enx5c857e3f5a6c`.

The only difference between this command and the previous one is that
this server includes the ability to give an IP address to your
coco, whereas the previous command asked the Coco to use the address
it got from DHCP.  The private network should be at least as big
as a "/24" or `netmask 255.255.255.0` network, for the IP addresses
created by the server to work.

More than one Coco may connect to the Linux server.  There is a
small chance of "IP address collision" -- if it doesn't work,
just reboot the Cocos and try again.


## Compiling your own server

Using an `X86_64` Linux machine, for instance running Ubuntu Linux,
follow instructions at 
[coco-shelf](https://github.com/strickyak/coco-shelf)
and type "make".

If that succeeds, then chdir into the coco-shelf/build-frobio
directory and run one of these two commands:

For cocos to use your home DHCP, use this command,
changing the LAN= address to your Linux machine's DHCP-given
IP address:

```
make run-lemma LAN=192.168.86.235  DHCP=1
````

For cocos to use a private LAN with your server giving out
IP addresses, use this command,
changing the LAN= address to your Linux machine's DHCP-given
IP address:

```
make run-lemma LAN=192.168.86.235
````

# How to run the `Conduit` caching proxy

Instead of running your own server,
you may usually prefer to use the global Lemma server
in the internet clouds.

This is usually very easy:  You just let your Coco boot,
let it use DHCP, and let it connect to the global Lemma.

But for some use cases, you will find this is a bit slow.
Running Nitros-9 with the remote Block disk devices is slow,
the Nitros-9 Ease-Of-Use edition is especially slow,
and watching videos is usually too laggy to be tolerable.

Most of those problems can be solved by using a caching proxy
named `Conduit`.

Install only the `pizga-base.tar.bz2` tarball and expand it,
following the instructions above.

Then run this:

```
pizga-base/Internal/bin/conduit -config_by_dhcp -lan 192.168.86.235
```

Substitue your own IP address for the last word.

To run on a private interface without DHCP, leave out the
`-config_by_dhcp` flag, and use your private interface address:

```
pizga-base/Internal/bin/conduit -lan 10.23.23.23
```

What happens is your Coco connects to the conduit program,
as if it were the Lemma server.  But it's not; it connects out
to the Global Lemma Server in the Cloud on your behalf.

It speeds up your Lemma usage in several ways:

  * Since Conduit is on your LAN, the round-trip latency from
  your Coco to Conduit is much shorte than the
  latency to the Global server in the cloud.

  * The TCP stack on your Linux machine can buffer a lot more
  data than the WizNet chip in the CocoIOr on your Coco can.

  * Conduit prefetches disk contents, to prevent round trips
  to the Global server in the cloud.

  * Conduit remembers disk contents, to prevent round trips
  to the Global server in the cloud.

Except for speeding things up, the Conduit cache is designed to
be invisible to the Coco.  Everything should function just the same.

### Typical Speedups with Cache

The Life viewer on page 13 is sped up considerably,
from under 3 frames per second, to over 12 fps.

Running #343 (Ease Of Use 1.0.1 for H6309) direct from the cloud, it takes
3:24 to boot up, and 2:12 to run the command "dir -e -x".

Running #343 using Conduit, the same thing takes
0:58 to boot up, and 1:28 to run the command "dir -e -x".

Running #343 from a Linux server on a private lan, the same thing takes
0:49 to boot up, and 0:37 to run the command "dir -e -x".

So you can see that it's fastest to run your own Lemma server,
but you can get a lot of the speedup by just running Conduit.
