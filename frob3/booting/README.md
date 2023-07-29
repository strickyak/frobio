# Axiom Network Boot Program & ROM

The file named `netboot3.dsk` is a "decb" floppy filesystem image.

1. Copy it into your SD Card for your CocoSDC.

2. You can run the `EXOBOOT.BIN` command to temporarily try the
   netboot Axiom program.  It should run from a casette, from a
   floppy disk, or from the CocoSDC.

3. Or you can burn the `NETBOOT.DEC` program into one of the
   flash rom slots on your CocoSDC.  The BASIC program
   `INSTALL4.BAS` will burn it into slot 4, zapping whatever was
   already in slot 4 (which by default is nothing). 

   To do netboot from slot 4, either type `RUN@4`,
   or set the DIP switches to 4 (down, up, down, down)
   to automatically use netboot.  (Set them back to 0
   (down down down down) to go back to HDB DOS.)

4. When you use the network boot, your machine will use DHCP
   to get an address and discover the gateway to the internet.
   Then it will connect to lemma.yak.net and you can load
   software from there.

5. If you don't want it to run DHCP right away, hit the space bar
   as soon as the Network Boot code takes over.   You can adjust
   your network and other settings from there.
