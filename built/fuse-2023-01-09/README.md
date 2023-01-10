Here are prebuilt previews of FUSE modules and daemons.
The functionality is incomplete, but ReadLn and WritLn work.

```
.../built/fuse-2023-01-09/daemons/fuse.ramfile.os9
.../built/fuse-2023-01-09/daemons/fuse.twice.os9
.../built/fuse-2023-01-09/modules/fuseman.os9
.../built/fuse-2023-01-09/modules/fuse.os9
.../built/fuse-2023-01-09/modules/fuser.os9
```

Build your BOOTFILE with the three modules.

Copy the daemons into your CMDS (removing the .os9 suffix).
Run them in background before using the /Fuse device.

See the README.md in `../../frob2/fuse/` for example commands.
