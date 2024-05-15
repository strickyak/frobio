# Axiom(41C) Network Boot Program & ROM

## For booting with Lemma over CocoIO using CocoSDC as the EEPROM.

This is one way to boot your CocoIO Ethernet cartridge (without EEPROM)
or to bootstrap the ROM on your CocoIOr (with EEPROM)
using a CocoSDC for its EEPROM.

1.  Unzip `netboot3.zip` to attain `netboot3.dsk`.
    The file named `netboot3.dsk` is a "decb" floppy filesystem image.

2. Copy it into your SD Card for your CocoSDC.

3. Burn the `NETBOOT.DEC` program into one of the
   flash ROM slots on your CocoSDC.  The BASIC program
   `INSTALL4.BAS` will burn it into slot 4, zapping whatever was
   already in slot 4 (which by default is nothing). 
   (Similarly, `INSTALL5.BAS` will burn it into slot 5.
   Conventionally, we use slot 5 when testing new versions.)

   To do netboot from slot 4, either type `RUN@4`,
   or set the DIP switches to 4 (down, up, down, down)
   to automatically use netboot.  (Set them back to 0
   (down down down down) to go back to HDB DOS.)

4. When you use the network boot, your machine will use DHCP
   to get an address and discover the gateway to the internet.
   Then it will connect to pizga.yak.net and you can load
   software from there.

5. If you don't want it to run DHCP right away, hit the space bar
   as soon as the Network Boot code takes over.   You can adjust
   your network and other settings from there (Type H for HELP).

## Summary for those wanting to upgrade a CocoIOr to Axiom 41C using a CocoSDC & MPI

1.  Burn the new version of AXIOM into the CocoSDC, either slot 4 or 5.

2.  Use an MPI to hold both CocoSDC and your CocoIOr,
    with the "boot" switch set to boot from the CocoSDC.
    Flip the switch or the jumper inside the CocoIOr to
    the "wr"/"write" setting.

2.  Boot the CocoSDC, let it connect to the Lemma server (text
    screen turns from orange to green), and go to page 4243
    (command "4243 <Enter>").    Launch it (command "@ <Enter>").

3.  Hit "X" to start the burn.

## With some risk, how to upgrade a CocoIOr to Axiom 41C using itself.

If this goes wrong, you may end up with a CocoIOr that cannot rescue itself.
But if you have a CocoSDC and MPI, you can rescue it that way (the above
instructions).

1.  Install your working (pre-version-41C) CocoIOr in the cartridge port.
    No MPI needed.  It needs to have the Read/Write switch visible (if it has
    a switch) or have its plastic cover off (so you can get to the jumper).

2.  Set the switch (or jumper) for Read.  Boot up and let it connect to
    its (old) Lemma server.

3.  Navigate to page 4243 and Launch ("@ <Enter>").

4.  Before pressing "X", change the switch or jumper to the Write position.

If it's a switch, that's easy, but do it gently so you don't rock the connection.

If it's a jumper, it's harder to be gentle.   Here's what I do:  With my left hand,
steady the card by holding it by the metal Ethernet connector, which is
grounded.  With my dominant hand, I use needle-nose pliers to pull the jumper
off and gently set it back into the Write position.

5.  Hit the "X" key to burn.   When it finishes (in about 15 seconds), turn the
    computer off.   Then restore the switch or jumper to the Read position,
    and you should be good.

Personally, I've never had this procedure fail in such a way to leave the
CocoIOr unbootable.  But if (say) the power went out during the middle of
booting, or you shook the connector in such a way that the coco freezes,
it could be unbootable.
