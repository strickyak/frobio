all: a.out markup_test parseurl_test

a.out: _FORCE_
	gcc   -I../.. markup_test.c markup.c 
	./a.out | grep '<<<.*>>>'

markup_test: _FORCE_
	gcc -g -I../.. markup_test.c  markup.c ../ncl/buf.c
	./a.out
	cmoc --os9 -I../.. markup_test.c markup.c 
	os9 ident markup_test

parseurl_test: _FORCE_
	gcc -g -I../.. parseurl_test.c  parseurl.c ../ncl/buf.c
	./a.out
	cmoc --os9 -I../.. parseurl_test.c  parseurl.c ../ncl/buf.c ../ncl/malloc.c ../ncl/std.c ../os9call.c ../ncl/puthex.c
	os9 ident parseurl_test

clean:
	rm -f a.out *_test *.link *.map *.s

_FORCE_:
