all: unix_all os9_all launch_all

SANITIZE= -fsanitize=address -static-libasan -g 

unix_all: unix_markup_test unix_parseurl_test
os9_all: markup_test parseurl_test
launch_all: launch_markup_test launch_parseurl_test

unix_markup_test: _FORCE_
	gcc $(SANITIZE) -I../.. -o markup_test markup_test.c markup.c parseurl.c fetch.c ../froblib/*.c ../unix/*.c
	./markup_test

unix_parseurl_test: _FORCE_
	gcc $(SANITIZE) -I../.. -o parseurl_test parseurl*.c ../froblib/*.c ../unix/*.c
	./parseurl_test

markup_test: _FORCE_
	cd ..; sh Compile.sh markup.test tweb/markup_test.c tweb/markup.c tweb/parseurl.c tweb/fetch.c froblib/*.c os9/*.c stack300.asm
	os9 ident ../markup.test.os9
launch_markup_test: markup_test
	cp ../markup.test.os9 /tmp/markup.test
	sh ../../../doing_os9/gomar/launchx.sh /tmp/markup.test /dev/null

parseurl_test: _FORCE_
	cd ..; sh Compile.sh parseurl.test tweb/parseurl_test.c tweb/parseurl.c froblib/*.c os9/*.c stack300.asm
	os9 ident ../parseurl.test.os9
launch_parseurl_test: parseurl_test
	cp ../parseurl.test.os9 /tmp/parseurl.test
	sh ../../../doing_os9/gomar/launchx.sh /tmp/parseurl.test /dev/null

_FORCE_:
