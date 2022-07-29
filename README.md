# frobio

The `frobio` repository is Network IO tools for NitrOS-9 with Wiznet W5100S,
like with the CocoIO board.  You need `cmoc` to compile, and `cmoc` needs
`lwtools` to assemble and link.  I am currently using cmoc 0.1.77
and lwasm 4.19.

## Summary of programs

```
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

## How to make

```
$ cd frobio
$ make clean
rm -f f.config f.ticks f.ping f.arp f.tget
rm -f test.nylib *.o *.s *.list *.lst *.map *.link
$ make all
cmoc -i --os9 -I.. f.config.c wiz5100s.c nylib.c os9call.c
cmoc -i --os9 -I.. f.ticks.c wiz5100s.c nylib.c os9call.c
cmoc -i --os9 -I.. f.ping.c wiz5100s.c nylib.c os9call.c
cmoc -i --os9 -I.. f.arp.c wiz5100s.c nylib.c os9call.c
cmoc -i --os9 -I.. f.tget.c wiz5100s.c nylib.c os9call.c
$
```
