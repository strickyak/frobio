# Fuse File Mananger

TODO: document.

For now, look in subdirectory `modules` for the FUSE 
file manager, driver, and descriptor.

Look in subdirectory `daemons/demos` for two demo usermode filesystems.

## Quick Demo

First the command `fuse.ramfile &` needs to be running
in the background.

Then we can output text info /fuse/ramfile/FILENAME
for some FILENAME, and later we can read that text
from the same filename.

This is just a demo, so it can handle only a small number of files and
a small amount of data per file (like under 200 bytes).

```
{Term|02}/DD:proc

 ID Prnt User Pty  Age  Tsk  Status  Signal   Module    I/O Paths
___ ____ ____ ___  ___  ___  _______ __  __  _________ __________________
  1   0    0  255  255   00  sTimOut  0  00  System    <Term >Term >>Term
  2   1    0  128  131   00  s        0  00  Shell     <Term >Term >>Term
  3   2    0  128  128   02  s        0  00  Proc      <Term >Term >>Term
  5   0    0  128  128   00  s        0  00  fuse.twice <DD   >W1   >>W1
  6   0    0  128  128   00  s        0  00  fuse.ramfile <DD   >W1   >>W1

{Term|02}/DD:date > /fuse/ramfile/f1
{Term|02}/DD:date -t > /fuse/ramfile/f2
{Term|02}/DD:list /fuse/ramfile/f1
January 09, 2023

{Term|02}/DD:list /fuse/ramfile/f2
January 09, 2023  11:44:08

{Term|02}/DD:dir > /fuse/ramfile/f2
{Term|02}/DD:list /fuse/ramfile/f2

OS9Boot         CMDS            SYS             DEFS            ccbkrn
sysgo           startup         NITROS9         ZZ1             ZZ2


{Term|02}/DD:dir /fuse/ramfile

ERROR #203

{Term|02}/DD:dir /fuse

ERROR #203

{Term|02}/DD:
```
