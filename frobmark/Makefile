all: a.out markup_test

a.out: _FORCE_
	gcc   -I../.. markup_test.c markup.c 
	./a.out | grep '<<<.*>>>'

markup_test: _FORCE_
	cmoc --os9 -I../.. markup_test.c markup.c 

clean:
	rm -f a.out markup_test *.link *.map *.s

_FORCE_:
