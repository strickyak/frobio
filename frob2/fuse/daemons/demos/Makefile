all: fuse.twice.os9 fuse.ramfile.os9

FROB2DIR = ../../..

SUPPORT = $(FROB2DIR)/froblib/*.c $(FROB2DIR)/os9/*.c $(FROB2DIR)/stack300.asm

fuse.twice.os9 : fuse-twice.c
	cmoc -i --os9 -I../../../.. -DMAX_VERBOSE=9 -o fuse.twice fuse-twice.c $(SUPPORT)
	mv fuse.twice fuse.twice.os9

fuse.ramfile.os9 : fuse-ramfile.c
	cmoc -i --os9 -I../../../.. -DMAX_VERBOSE=9 -o fuse.ramfile fuse-ramfile.c $(SUPPORT)
	mv fuse.ramfile fuse.ramfile.os9

clean:
	rm -f *.o *.s *.list *.lst *.os9 *.map *.link
