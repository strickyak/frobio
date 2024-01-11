## This directory has tools to burn the CocoIOr EEPROM.

The `burn` program is thought to work on either a coco1 or a coco2, with
as little as 4K RAM.  You need to use another card (or, if you're daring,
the same card) to boot Axiom4.  (I boot using a CocoSDC for Axiom4 from
slot @4.)

The product `burn` is a DECB BINARY that can upload from the waiter
(that is, from Lemma Server).  But to the end of it, add 3-byte records
("Triples") to specify bytes to be poked.  Format is (hi-addr, lo-addr,
data).  End with three zero bytes.  The program rom-triples reads from
standard input, which must be exactly 8192 bytes, and it writes the
triples for the burn program.  Append the output of rom-triples to the
burn binary, and that is what the Lemma should server.

To burn the rom, the address would be $C000, $C001, etc. up to $DFFF.
The data are the bytes to appear in the ROM.

I also have a demo that will poke a message to the VDG screen instead.
splash-triples generates those triples.  Append the output of
splash-triples to the end of the "burn" binary, and that is what
the Lemma should serve.
