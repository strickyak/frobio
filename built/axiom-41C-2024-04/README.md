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

## DIY:  Burning your CocoIOr EEPROM with `netcat` and `burn-rom-fast.lem`

Here's a third way you can upgrade.  If you don't have access to the internet,
or your DHCP is broken, you can statically configure the wired Ethernet interface
on your Linux laptop, and run an Ethernet cable directly from your laptop
to the CocoIOr cartridge.  I recommend statically configuring for IP
address 10.23.23.23 with a "/24" or "255.255.255.0" netmask and no Gateway.

Follow the "With some risk, how to upgrade a CocoIOr to Axiom 41C using itself"
instructions, but when you boot with Axiom, hit the SPACE bar as soon as
it starts the countdown.

Use the `A <Enter>` command to put your Coco on Axiom's default Class A
network (which will be a random IP address in `10.23.23.*`).
Use the command `W 10.23.23.23:1234 <Enter>` to set the IP address
and port number that the laptop will be listening on (your "Waiter").
Verify these with the Show command: `S <Enter>`.

On your Ubuntu Laptop, tell the firewall to allow access to port 1234,
and run the `netcat` command `nc` to serve port 1234,
with standard input redirected from the file `burn-rom-fast.lem`:

```
$ sudo ufw allow 1234

$ nc -l 1234 < burn-rom-fast.lem >/dev/null
```

Now tell Axiom to launch `@ <Enter>` and instead of connectiong to a
Lemma server, it connects directly to the `nc` command, which feeds
it the `burn-rom-fast.lem` file.   That `nc` command is only
good for one TCP connection.   Hit ^C and re-run the command
for another connection.

(You can use this technique to have the Coco load and run any
"DECB Binary" file (one that uses the `00` five-byte header
and `FF` five-byte trailer).  If there is more data after the
five-byte trailer, your binary can read it from WizNet socket 1
which is already Opened and running.)
