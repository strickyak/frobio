all: f.config f.ticks f.ping f.arp f.tget

f.config: FORCE
	cmoc -i --os9 -I.. f.config.c wiz5100s.c nylib.c os9call.c

f.ticks: FORCE
	cmoc -i --os9 -I.. f.ticks.c wiz5100s.c nylib.c os9call.c

f.ping: FORCE
	cmoc -i --os9 -I.. f.ping.c wiz5100s.c nylib.c os9call.c

f.arp: FORCE
	cmoc -i --os9 -I.. f.arp.c wiz5100s.c nylib.c os9call.c

f.tget: FORCE
	cmoc -i --os9 -I.. f.tget.c wiz5100s.c nylib.c os9call.c

test.nylib: FORCE
	cmoc -i --os9 -I.. test.nylib.c nylib.c
launch.test.nylib: test.nylib
	sh  ../doing_os9/gomar/launch.sh  ./test.nylib /dev/null


my: all
	os9 copy -r ./f.config /media/strick/APRIL3/MY.DSK,CMDS/f.config
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.config
	os9 copy -r ./f.ticks /media/strick/APRIL3/MY.DSK,CMDS/f.ticks
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.ticks
	os9 copy -r ./f.ping /media/strick/APRIL3/MY.DSK,CMDS/f.ping
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.ping
	os9 copy -r ./f.arp /media/strick/APRIL3/MY.DSK,CMDS/f.arp
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.arp
	os9 copy -r ./f.tget /media/strick/APRIL3/MY.DSK,CMDS/f.tget
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.tget
	sync
	(echo "echo f.config 10.1.2.3 255.0.0.0 6.6.6.6" ; echo "f.config 10.1.2.3 255.0.0.0 6.6.6.6"; echo "echo f.arp 10.2.2.2"; echo "f.arp 10.2.2.2";  echo "echo f.arp 10.2.2.8"; echo "f.arp 10.2.2.8";  echo "echo f.ping 10.2.2.2"; echo "f.ping 10.2.2.2";  echo "echo f.ping 10.2.2.8"; echo "f.ping 10.2.2.8";  ) > _zz_
	os9 copy -l -r _zz_ /media/strick/APRIL3/MY.DSK,CMDS/zz
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/zz
	sync

FORCE:

clean:
	rm -f f.config f.ticks f.ping f.arp f.tget
	rm -f test.nylib *.o *.s *.list *.lst *.map *.link

ci:
	mkdir -p RCS
	ci -l -m/dev/null -t/dev/null -q *.c *.h Makefile
	sync
