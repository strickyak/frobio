# The Axiom/Waiter TCP Protocol

**Version 41C -- Cocofest 2024**

**Henry Strickland -- strick@yak.net -- May 2024**

Axiom and Waiter are the client and the server designed for network boot of
[Cocos](https://www.google.com/search?q=Radio+Shack+Tandy+Color+Computers)
using the
[CocoIOr cartridges by Computer Conect](https://computerconect.com/products/cocoio-prom?variant=42556416196788)
and software from the
[Frobio](https://github.com/strickyak/frobio)
repository.

**Axiom** is software burned into the EEPROM in the Cartridge.

A **waiter** is the server that Axiom connects to.

In Frobio, the primary waiter is one named **Lemma**.
But Axiom doesn't require that you use the Lemma
waiter.  This document describes the protocol
supported by Axiom, if you wanted to write your
own waiter.

This does not describe Axiom versions before 41C,
which was released just before Cocofest in May 2024.
There are incompatibilits in earlier versions of Axiom.

This does not describe how Axiom discovers the Waiter.
That will be in another document.

## Quintuples and Commands.

If you are familiar with Radio Shack's
[DECB (Disk Extended Color Basic) protocol for binary programs](http://www.lwtools.ca/manual/x27.html),
you know that a 5-byte header format is used.
Axiom supports the same DECB binary format and extends it.
Axiom is based on similar 5-byte headers, which we call **Quintuples**.

```
+----------+----------+----------+----------+----------+
| Command  |  16-bit number N    |  16-bit number P    |
|   byte   |   (big endian)      |   (big endian)      |
+----------+----------+----------+----------+----------+
```

The first byte of a Quintuple is the "command" byte.
These commands are defined as constants:

```
  CMD_POKE = 0,    // SAME AS DECB
  CMD_HELLO = 1,   // Client says HELLO.
  CMD_GETCHAR = 192, // blocking, only returns valid char
  CMD_KEYBOARD = 193,  // nonblocking
  CMD_SUM = 194,
  CMD_INKEY = 201,  // nonblocking, can return 0
  CMD_PUTCHAR = 202,
  CMD_PEEK = 203,
  CMD_DATA = 204,
  CMD_JSR = 255,    // SAME AS DECB, but may return!
```

The second and third bytes are a parameter named N.
N is a 16-bit unsigned integer in big-endian format.
N is often (but not always) the size of a payload.

The final two bytes are a parameter named P.
P is also a 16-bit unsigned integer in big-endian format.
P is sometimes (but not always) an address in the
Coco's 64K address space.

## Axiom, the client, speaks first.

The waiter should listen for TCP connections and accept them.
On a new connection, it should read "HELLO" records from the
client until it gets a "HELLO" record with n=0 and p=0.
If no valid HELLO record is read, or if clearly invalid
bytes are read, the waiter can assume the client is
up to no good, and drop the connection.

HELLO records are a chance for the client to identify itself
to the server, and to give the server some hints what type
of a machine it is.

A HELLO record starts with a quintuple.  Following the
quintuple is a payload of N bytes, where N is the number N
from the quintuple.  The payload is selected contents of the coco's
memory, each starting at address P.

The waiter may ignore these payloads, or pay attention to some of the data.
One payload will begin at P=$DF00, and contain approximentally N=$F4
bytes.  This is the "identification" sector of the Bootrom,
containing the Hostname and some other fields.  This structure
defines some of the fields:

```
struct axiom4_rom_tail { // $DFC0..$DFFF
  byte rom_reserved_14[14];  // $DFC0
  word rom_waiter_port;
  byte rom_waiter[4];   // $DFD0
  byte rom_dns[4];
  byte rom_hailing[8];
  byte rom_hostname[8];  // $DFE0
  byte rom_reserved_3[3];
  byte rom_mac_tail[5];  // After initial $02 byte, 5 random bytes!
  byte rom_secrets[16];  // For Challenge/Response Authentication Protocols.
};
```

So if the hostname is at address $DFE0 in the EEPROM,
and the HELLO payload begins at address $DF00, then the
hostname will appear at offset $E0 in that payload.

Other fields:  The "hailing" record is initially the 8 bytes in
"SWI4SWI5".  The "mac_tail" is 5 random bytes, which should be
unique to the client.   If the payload starts at $DF00 and
has length $F4, its last four bytes are the first four bytes
of the "secrets" field.   These are the least secure of the
16 secret bytes, because they are passed in plaintext.
The length is $F4 to avoid sending the other 12 secret bytes.

The final HELLO record has N=0 and P=0, and is followed by
no bytes of payload.  After the final HELLO record, the
client waits for the server to issue commands.

## Waiter to Axiom commands

There are 8 commands the waiter can send to Axiom.
The waiter can send these in any order it likes.

The eight commands are GETCHAR, INKEY, KEYBOARD,
PUTCHAR, POKE, PEEK, SUM, and JSR.  Each begins with
a quintuple from the waiter to Axiom, and the
quintuple begins with the command.  Whether a payload
is also sent depends on the command.  Whether there
is a reply depends on the command.

### GETCHAR (blocking)

```
Waiter to Axiom  ...   Axiom to Waiter

GETCHAR n=0 p=0  ....  GETCHAR n=0 p=AsciiCode.
```

Command quintuple:  GETCHAR n=0 p=0

The coco waits for someone to hit a character
on the keyboard (using the BASIC ROM routine POLCAT).
When this happens, the client sends back a quintuple
with the ASCII code (or special Coco code) of the character
in the P slot.


### INKEY (nonblocking)

```
Waiter to Axiom  ...   Axiom to Waiter

INKEY n=0 p=0  ...  INKEY n=0 p=AsciiCode.
```

INKEY is just like GETCHAR except the coco does not wait.
If no key is sent, then P=0 is returned.

### KEYBOARD (nonblocking)

```
Waiter to Axiom  ...   Axiom to Waiter

INKEY n=0 p=0  ...  INKEY n=8 p=0,  8-byte payload.
```

KEYBOARD is similar to INKEY, except the response has
an 8 byte payload.  The 64 bits in the payload include
all 56 intersections of the Coco's Keyboard Matrix.
This can be used to detect things like the SHIFT key
being pressed, which does not generate a GETCHAR or INKEY
code of its own.  The BASIC ROM routine POLCAT is not used.

### PUTCHAR

```
Waiter to Axiom  ...   Axiom to Waiter

INKEY n=0 p=AsciiCode  ...  (no reply)
```

This calls the BASIC ROM routine to print one character
on the Coco's screen.

### POKE

```
Waiter to Axiom  ...   Axiom to Waiter

POKE n=SIZE p=ADDRESS, SIZE-byte payload ...  (no reply)
```

The payload, which has N bytes, is poked to the Coco's memory space
starting at ADDRESS.

Notice that POKEs are not limited to program code.
The waiter may also poke bytes onto text or graphics screens,
poke video modes, poke MMU maps, etc.

### PEEK

```
Waiter to Axiom  ...   Axiom to Waiter

PEEK n=SIZE p=ADDRESS  ...   DATA n=SIZE p=ADDRESS, SIZE-byte payload.
```

The waiter is requesting Axiom to return bytes from the Coco's
memory space.


### SUM

```
Waiter to Axiom  ...   Axiom to Waiter

SUM n=SIZE p=ADDRESS  ...   SUM n=0 p=CheckSum
```

Instead of PEEKing bytes from the Coco, the waiter asks the
Coco to compute a checksum of a region of its memory
and to return it.  The size must be an even number.
The CheckSum is computed by considering the region of
memory to be composed of 16-bit unsigned integers
in big-endian order.  The CheckSum is merely the 16-bit
sum of these integers, with carry bits ignored.

### JSR

```
Waiter to Axiom  ...   Axiom to Waiter

JSR n=0 p=ADDRESS  ...   (no reply)
```

The coco is requested to call ADDRESS with a subroutine call.
If the subroutine returns, the waiter can send more commands.

This is the same Quintuple that a DECB binary uses to JUMP
to a program, but DECB does not allow the program to return.

There is no mechanism in this protocol to know if the
subroutine returned to axiom.  If the subroutine is quick
and always returns immediately, this is not a problem.

But if the subroutine may run a long time, and may use the
TCP connection for other application-dependant communications
with the waiter, it will have to provide some other way to
tell the waiter that it is finished.

## WizNet W5100S Socket 1

The TCP connection between the waiter and Axiom is on
Socket 1 in the
[WizNet W5100S](https://docs.wiznet.io/Product/iEthernet/W5100S/overview)
chip on the CocoIOr board.
This is the socket with control registers starting at
WizNet register $0500.

Pieces of code that are executed on the Coco may use
that TCP socket for more communications with the waiter,
if the waiter understands these extra protocols.
The socket is already open, and it probably should not
be closed, or communiation with the waiter will be lost.

Lemma does understand some extra protocols, including these:

*   BOOTing initial modules into RAM for Nitros-9

*   Block device for remote drives in Nitros-9, using RBLemma driver.

*   Block device protocl for HDB-DOS.

But Axiom does not know those protocols.  The Client protocols are
in those operating systems.

## Trivial Write-Only Waiter

The simplest waiter is a TCP server that sends one DECB Binary to
every connection.  It does not have to read the HELLO packets.

For instance, this is a waiter (good for only one connection)
using the unix NETCAT utility:

```
$  nc -l 2321 <frobio/built/wip-2023-03-29-netboot2/demo-nyancat.coco3.loadm >/dev/null
```

## END
