all: a.out markup_test parseurl_test

a.out: _FORCE_
	gcc   -I../.. markup_test.c markup.c 
	./a.out | grep '<<<.*>>>'

markup_test: _FORCE_
	gcc -fsanitize=address -static-libasan -g -I../.. markup_test.c  markup.c  ../nyformat.c ../ncl/buf.c  parseurl.c fetch.c
	./a.out
	cmoc --os9 -I../.. markup_test.c markup.c  ../nyformat.c ../nystdio.c ../ncl/buf.c  parseurl.c fetch.c ../ncl/malloc.c ../ncl/std.c ../os9call.c ../ncl/puthex.c
launch_markup_test: markup_test
	sh ../../doing_os9/gomar/launchx.sh markup_test /n

parseurl_test: _FORCE_
	gcc -g -I../.. parseurl_test.c  parseurl.c ../ncl/buf.c ../nyformat.c
	./a.out
	cmoc --os9 -I../.. parseurl_test.c  parseurl.c ../ncl/buf.c ../ncl/malloc.c ../ncl/std.c ../os9call.c ../ncl/puthex.c ../nyformat.c
	os9 ident parseurl_test
launch_parseurl_test: parseurl_test
	sh ../../doing_os9/gomar/launchx.sh parseurl_test /n

clean:
	rm -f a.out *_test *.link *.map *.s *.sym *.list *.asmap

_FORCE_:
