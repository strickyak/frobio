all: parseurl_test
# all: unix_all markup_test parseurl_test

unix_all: unix_markup_test unix_parseurl_test unix_web

unix_web: _FORCE_
	gcc -fsanitize=address -static-libasan -g -I../.. f.web.c  markup.c  ../nyformat.c ../ncl/buf.c  parseurl.c fetch.c
	./a.out fetch.c

unix_markup_test: _FORCE_
	gcc -fsanitize=address -static-libasan -g -I../.. markup_test.c  markup.c  ../nyformat.c ../ncl/buf.c  parseurl.c fetch.c
	./a.out

unix_parseurl_test: _FORCE_
	gcc -I../.. parseurl*.c ../froblib/*.c ../unix/unix9.c
	./a.out

markup_test: _FORCE_
	gcc -fsanitize=address -static-libasan -g -I../.. markup_test.c  markup.c  ../nyformat.c ../ncl/buf.c  parseurl.c fetch.c
	./a.out
	cmoc --os9 -I../.. markup_test.c markup.c  ../nyformat.c ../nystdio.c ../ncl/buf.c  parseurl.c fetch.c ../ncl/malloc.c ../ncl/std.c ../os9call.c ../ncl/puthex.c
launch_markup_test: markup_test
	sh ../../doing_os9/gomar/launchx.sh markup_test /n

parseurl_test: _FORCE_
	gcc -I../.. -o parseurl_test parseurl*.c ../froblib/*.c ../unix/unix9.c
	./parseurl_test
	cmoc --os9 -I../.. parseurl_test.c  parseurl.c ../froblib/*.c ../os9/*.c ../stack300.asm
	cd ..; sh Compile.sh parseurl.test tweb/parseurl_test.c tweb/parseurl.c froblib/*.c os9/*.c stack300.asm
	os9 ident ../parseurl.test.os9
launch_parseurl_test: parseurl_test
	sh ../../doing_os9/gomar/launchx.sh ../parseurl.test.os9 /n

clean:
	rm -f a.out *_test *.link *.map *.s *.sym *.list *.asmap

_FORCE_:
