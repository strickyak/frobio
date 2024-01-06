# frobio

In case you have problems running `make` in the `frob2` directory:

The OS9 binaries from the first working version are available in
the directory `built/v1/CMDS/`.  You can continue to use them
for now.  Copy them to your CMDS directory and set the execute bits.

The OS9 binaries from the second working version are available in
the directory `built/v2/CMDS/` ...


```
github.com/strickyak/frobio$ ls built/v1/CMDS/
f.1 f.arp f.config f.dhcp f.dig f.dump f.ntp f.ping f.recv f.resolv f.send f.tget f.ticks
```

# frobio

The `frobio` repository is Network IO tools for NitrOS-9 with Wiznet W5100S,
like with the CocoIO board.  You need `cmoc` to compile, and `cmoc` needs
`lwtools` to assemble and link.  I am currently using cmoc 0.1.79
and lwasm 4.19.

## Summary of programs

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

## How to make: Version 2.

frob2 is the current work, in the frob2 directory.

```
cd frob2/
make -B
make install DISK=/tmp/my-os9-disk-image.dsk
```

The resulting Nitros9 commands end with an extension .os9 so you can find them easily.
Copy them into the CMDS/ directory of the OS-9 disk, leaving off the .os9 extension.

## Old Work: Version 1.

Version 1 is in the "old1" directory.
If you rename the directory from "old1" to "frobio",
it should still "make".
test-alligator
test-bird
