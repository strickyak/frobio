# frobio

The `frobio` repository is Network IO tools for NitrOS-9 with Wiznet W5100S,
like with the CocoIO board.  You need `cmoc` to compile, and `cmoc` needs
`lwtools` to assemble and link.  I am currently using cmoc 0.1.77
and lwasm 4.19.

## Current Work: frob2

frob2 is the current work, in the frob2 directory.

```
cd frob2/
make -B
```

The resulting commands end with an extension .os9 so you can find them easily.
Copy them into the CMDS/ directory of the OS-9 disk, leaving off the .os9 extension.

PROBLEM:  
Some linux tools didn't like "." in the filename, like "f.send.c".
So I changed to "-".
Now Some OS9 tools don't like "-" in the filenames, like "f-send".
So I need to change the binaries back to using "."
even if I keep the source files using "-".

See the README in the old1/ directory for info about using the commands,
until I update this, and fix the PROBLEM.

## Old Work: Version 1.

Version 1 is in the "old1" directory.
If you rename the directory from "old1" to "frobio",
it should still "make".
