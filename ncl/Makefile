all:
	cmoc --os9 -i -I../.. ncl.c buf.c std.c util.c puthex.c re.c ../os9call.c malloc.c
	os9 ident ncl


it:
	(cd /home/strick/go/src/github.com/strickyak/doing_os9/gomar/cmocly && go install -x cmocly.go)
	/home/strick/go/bin/cmocly -cmoc `which cmoc` -cmoc_pre='-I../..' -o ncl  ncl.c buf.c std.c util.c puthex.c re.c ../os9call.c malloc.c
	os9 ident ncl


clean:
	-rm ncl *.o *.s *.s-orig *.list *.map *.link 
