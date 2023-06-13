all: fuse.ramfile.os9 fuse.tftp.os9 fuse.n.os9

FROB2DIR = ../../..

SUPPORT = $(FROB2DIR)/froblib/*.c $(FROB2DIR)/os9/frobos9.c $(FROB2DIR)/stack300.asm

fuse.n.os9 : fuse-n.c
	cmoc -DMAX_VERBOSE=9 -i --os9 -I../../../.. -o fuse.n fuse-n.c $(SUPPORT)
	mv fuse.n fuse.n.os9cmd

fuse.ramfile.os9 : fuse-ramfile.c
	cmoc -DMAX_VERBOSE=9 -i --os9 -I../../../.. -o fuse.ramfile fuse-ramfile.c $(SUPPORT)
	mv fuse.ramfile fuse.ramfile.os9cmd

fuse.tftp.os9 : fuse-tftp.c
	cmoc -i --os9 -I../../../.. -o fuse.tftp fuse-tftp.c $(FROB2DIR)/wiz/*.c $(SUPPORT)
	mv fuse.tftp fuse.tftp.os9cmd

clean:
	rm -f *.o *.s *.list *.lst *.os9cmd *.map *.link
