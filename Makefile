all: frobio tftp

frobio: _FORCE_
	cmoc -i --os9 -I.. frobio.c w5100s.c
	sync

tftp: _FORCE_
	cmoc -i --os9 -I.. tftp.c w5100s.c nylib.c os9call.c
	sync
launch-tftp: tftp
	sh  ../doing_os9/gomar/launch.sh  ./tftp  /dev/null


test_nylib: _FORCE_
	cmoc -i --os9 -I.. test_nylib.c nylib.c
	sync
launch-test_nylib: test_nylib
	sh  ../doing_os9/gomar/launch.sh  ./test_nylib /dev/null



my: all
	os9 copy -r ./frobio /media/strick/APRIL3/MY.DSK,CMDS/frobio
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/frobio
	os9 copy -r ./tftp /media/strick/APRIL3/MY.DSK,CMDS/tftp
	os9 attr -per /media/strick/APRIL3/MY.DSK,CMDS/tftp
	sync

_FORCE_:
	: _force_

clean:
	rm -f frobio tftp test_nylib *.o *.s *.list *.lst *.map *.link

ci:
	mkdir -p RCS
	ci -l -m/dev/null -t/dev/null -q *.c *.h Makefile
