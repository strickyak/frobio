all: f.config f.ticks f.ping f.arp f.tget f.ntp f.send f.recv f.dump f.dig f.dhcp

nyformat_test: FORCE
	gcc -g -I.. nyformat_test.c nyformat.c ncl/buf.c
	./a.out
	cmoc --os9 -I.. nyformat_test.c nyformat.c nylib.c nystdio.c ncl/std.c ncl/buf.c ncl/malloc.c os9call.c ncl/puthex.c 
	echo TODO -- MAKE launchx.sh STOP IF SUCCESS.
	sh ../doing_os9/gomar/launchx.sh nyformat_test /dev/null

f.config: FORCE
	cmoc -i --os9 -I.. f.config.c wiz5100s.c nylib.c os9call.c stack300.asm

f.ticks: FORCE
	cmoc -i --os9 -I.. f.ticks.c wiz5100s.c nylib.c os9call.c stack300.asm

f.ping: FORCE
	cmoc -i --os9 -I.. f.ping.c wiz5100s.c nylib.c os9call.c stack300.asm

f.arp: FORCE
	cmoc -i --os9 -I.. f.arp.c wiz5100s.c nylib.c os9call.c stack300.asm

f.tget: FORCE
	cmoc -i --os9 -I.. f.tget.c wiz5100s.c nylib.c os9call.c stack300.asm

f.ntp: FORCE
	cmoc -i --os9 -I.. f.ntp.c wiz5100s.c nylib.c os9call.c stack300.asm

f.send: FORCE
	cmoc -i --os9 -I.. f.send.c wiz5100s.c nylib.c os9call.c stack300.asm

f.recv: FORCE
	cmoc -i --os9 -I.. f.recv.c wiz5100s.c nylib.c os9call.c stack300.asm

f.dump: FORCE
	cmoc -i --os9 -I.. f.dump.c wiz5100s.c nylib.c os9call.c stack300.asm

f.dig: FORCE
	cmoc -i --os9 -I.. f.dig.c wiz5100s.c nylib.c os9call.c stack300.asm

f.dhcp: CMOCLY FORCE
	cmoc -i --os9 -I.. f.dhcp.c wiz5100s.c nylib.c os9call.c stack300.asm

CMOCLY:
	cd ../doing_os9/gomar/cmocly && GO111MODULE=off go build cmocly.go && GO111MODULE=off go install cmocly.go

x.mallox: CMOCLY FORCE
	/home/strick/go/bin/cmocly -incr 300 -cmoc `which cmoc` -cmoc_pre='-I..' -o x.mallox x.mallox.c nystdio.c os9call.c ncl/malloc.c ncl/puthex.c

x.fgets: CMOCLY FORCE
	/home/strick/go/bin/cmocly -incr 300 -cmoc `which cmoc` -cmoc_pre='-I..' -o x.fgets x.fgets.c nystdio.c os9call.c ncl/malloc.c ncl/puthex.c

x.fputs: CMOCLY FORCE
	/home/strick/go/bin/cmocly -incr 300 -cmoc `which cmoc` -cmoc_pre='-I..' -o x.fputs x.fputs.c nystdio.c os9call.c ncl/malloc.c ncl/puthex.c

x.sprint: CMOCLY FORCE
	/home/strick/go/bin/cmocly -incr 300 -cmoc `which cmoc` -cmoc_pre='-I..' -o x.sprint x.sprint.c

x.inkey: CMOCLY FORCE
	/home/strick/go/bin/cmocly -incr 300 -cmoc `which cmoc` -cmoc_pre='-I..' -o x.inkey x.inkey.c os9call.c

shmem: FORCE
	cmoc -i --os9 -I.. shmem.c wiz5100s.c nylib.c os9call.c

shmem2: FORCE
	cmoc -i --os9 -I.. shmem2.c wiz5100s.c nylib.c os9call.c

queue-test: FORCE
	/home/strick/go/bin/cmocly -cmoc `which cmoc` -cmoc_pre='-I..' -o queue-test  queue_test.c queue.c

test.nylib: FORCE
	cmoc -i --os9 -I.. test.nylib.c nylib.c
launch.test.nylib: test.nylib
	sh  ../doing_os9/gomar/launch.sh  ./test.nylib /dev/null


my: all ci
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
	os9 copy -r ./f.ntp /media/strick/APRIL3/MY.DSK,CMDS/f.ntp
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.ntp
	os9 copy -r ./f.send /media/strick/APRIL3/MY.DSK,CMDS/f.send
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.send
	os9 copy -r ./f.recv /media/strick/APRIL3/MY.DSK,CMDS/f.recv
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.recv
	os9 copy -r ./f.dump /media/strick/APRIL3/MY.DSK,CMDS/f.dump
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.dump
	os9 copy -r ./f.dig /media/strick/APRIL3/MY.DSK,CMDS/f.dig
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.dig
	os9 copy -r ./f.dhcp /media/strick/APRIL3/MY.DSK,CMDS/f.dhcp
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/f.dhcp
	: os9 copy -r ./shmem /media/strick/APRIL3/MY.DSK,CMDS/shmem
	: os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/shmem
	sync
	(echo "echo f.config 10.1.2.3 255.0.0.0 10.2.2.2" ; echo "f.config 10.1.2.3 255.0.0.0 10.2.2.2"; echo "echo f.arp 10.2.2.2"; echo "f.arp 10.2.2.2";  echo "echo f.ping 10.2.2.2"; echo "f.ping 10.2.2.2"; echo "echo f.dig -a 8.8.8.8 yak.net"; echo "f.dig -a 8.8.8.8 yak.net"; echo "echo f.ntp -s 216.239.35.8" ; echo "f.ntp -s 216.239.35.8"; echo "echo date -t" ; echo "date -t" ) > _zz_
	os9 copy -l -r _zz_ /media/strick/APRIL3/MY.DSK,CMDS/zz
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/zz
	make ci
	sync

#	(echo "echo f.config 10.1.2.3 255.0.0.0 10.2.2.2" ; echo "f.config 10.1.2.3 255.0.0.0 10.2.2.2"; echo "echo f.arp 10.2.2.2"; echo "f.arp 10.2.2.2";  echo "echo f.ping 10.2.2.2"; echo "f.ping 10.2.2.2"; echo "echo f.ntp -s 10.2.2.2"; echo "f.ntp -s 10.2.2.2") > _zz_
#	(echo "echo f.config 10.1.2.3 255.0.0.0 10.2.2.2" ; echo "f.config 10.1.2.3 255.0.0.0 10.2.2.2"; echo "echo f.arp 10.2.2.2"; echo "f.arp 10.2.2.2";  echo "echo f.ping 10.2.2.2"; echo "f.ping 10.2.2.2") > _zz_

FORCE:

clean:
	rm -f f.config f.ticks f.ping f.arp f.tget f.ntp f.send f.recv f.dump f.dig
	rm -f shmem queue-test x.fgets x.fputs x.sprint x.inkey
	rm -f _zz* test.nylib *.o *.s *.s-orig *.list *.lst *.map *.link *.asmap *.sym

ci:
	mkdir -p RCS
	ci -l -m/dev/null -t/dev/null -q *.[ch] Makefile */*.[ch] */Makefile
	sync
