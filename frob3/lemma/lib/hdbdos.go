package lib

import (
	"flag"
	"log"
	"os"
)

var FlagDosDisk = flag.String("dos_disk", "", "for HdbDos")

type HdbDosSession struct {
	NumReads	int64
}

func HdbDos(ses *Session, payload []byte) {
	if ses.HdbDos == nil {
		 ses.HdbDos = &HdbDosSession{
		 }
	}
	cmd := payload[0]
	drive := payload[1]
	lsn2 := uint(payload[2])
	lsn1 := uint(payload[3])
	lsn0 := uint(payload[4])
	lsn := (lsn2 << 16) | (lsn1 << 8) | lsn0

	log.Printf("HdbDos cmd=%x drive=%x lsn=%x paylen=%d.", cmd, drive, lsn, len(payload))
	DumpHexLines("payload", 0, payload)

	switch cmd {
	case 2: // Read
		ses.HdbDos.NumReads++
		if true && ses.HdbDos.NumReads == 1 {
			SendInitialInjections(ses)
		}

		fd := Value(os.Open(*FlagDosDisk))
		defer fd.Close()

		buf := make([]byte, 256+5)
		for i := 0; i < 5; i++ {
			buf[i] = payload[i]
		}
		Value(fd.Seek(256*int64(lsn), 0))
		Value(fd.Read(buf[5:]))
		log.Printf("HDB READ lsn=%d.: %02x", lsn, buf[5:])

		log.Printf("HdbDos Reply Packet: quint + %d.", len(buf))
		DumpHexLines("REPLY", 0, buf)
		WriteQuint(ses.Conn, CMD_HDBDOS, 0, buf)

	case 3: // Write
		fd := Value(os.OpenFile(*FlagDosDisk, os.O_RDWR, 0666))
		defer fd.Close()

		Value(fd.Seek(256*int64(lsn), 0))
		Value(fd.Write(payload[5:]))
		log.Printf("HDB WRITE lsn=%d.: %02x", lsn, payload[5:])
	default:
		panic(cmd)
	}
}

func SendInitialInjections(ses *Session) {
	// Currently, just send some semigraphics chars to the text screen.
	buf := make([]byte, 5+256)
	copy(buf[5:], SideLoadRaw)

	// Poke to VDG screen
	buf[5 + 0x38] = 0x05
	buf[5 + 0x39] = 0xC0

	// Do not exec anything else
	buf[5 + 0x3A] = 0
	buf[5 + 0x3B] = 0

	buf[5 + 0x3C] = 32  // One text line

	for i:=0; i < 32; i++ {
		// Splash some semigraphics chars
		buf[5 + 64 + i] = byte(256 - 32 + i)
	}

	WriteQuint(ses.Conn, CMD_HDBDOS+1, 0, buf)
	DumpHexLines("Injection", 0, buf)
}

var SideLoadRaw = []byte{
	0x34, 0x76, 0x10, 0xbe, 0x06, 0x38, 0x8e, 0x06, 0x40, 0xf6, 0x06, 0x3c, 0xa6, 0x80, 0xa7, 0xa0,
	0x5a, 0x26, 0xf9, 0xbe, 0x06, 0x3a, 0x27, 0x02, 0xad, 0x84, 0x35, 0xf6,
}

/*
     1	                      (     sideload.asm):00001         ;; sideload.asm
     2	                      (     sideload.asm):00002         ;;
     3	                      (     sideload.asm):00003         ;; lemma_hdb.asm adds the ability for a Lemma Server to send extra sectors
     4	                      (     sideload.asm):00004         ;; to the Coco to be executed, before it sends the actual sector.
     5	                      (     sideload.asm):00005         ;; This gives us a hook to load more code into RAM.
     6	                      (     sideload.asm):00006         ;; The limitation is that the extra sector loads at $0600 and executes there.
     7	                      (     sideload.asm):00007         ;;
     8	                      (     sideload.asm):00008         ;; This scrap of code at the beginning of the sector expects control data
     9	                      (     sideload.asm):00009         ;; from $0638 to $063F, and payload data from $0640 to $06FF.
    10	                      (     sideload.asm):00010         ;;
    11	                      (     sideload.asm):00011         ;; $0638:  word: destination to copy to
    12	                      (     sideload.asm):00012         ;; $063A:  word: destination to JSR to, if nonzero
    13	                      (     sideload.asm):00013         ;; $063C:  byte: number of bytes to copy
    14	                      (     sideload.asm):00014         ;; $063D:  3 bytes unused
    15	                      (     sideload.asm):00015         ;;;;;;;;;;;;;;;;;;;;;;;;;;;
    16	                      (     sideload.asm):00016
    17	     0600             (     sideload.asm):00017         SIDELOAD  equ $0600
    18	     0638             (     sideload.asm):00018         COPY_DEST equ $0638
    19	     063A             (     sideload.asm):00019         JSR_DEST  equ $063A
    20	     063C             (     sideload.asm):00020         COPY_LEN  equ $063C
    21	     0640             (     sideload.asm):00021         COPY_SRC_IMM  equ $0640
    22	                      (     sideload.asm):00022
    23	                      (     sideload.asm):00023                 ORG SIDELOAD
    24	                      (     sideload.asm):00024
    25	0600 3477             (     sideload.asm):00025                 pshs cc,d,x,y,u
    26	0602 1A50             (     sideload.asm):00026                 orcc #$50             ; disable interrupts while we patch stuff.
    27	                      (     sideload.asm):00027
    28	0604 10BE0638         (     sideload.asm):00028                 ldy COPY_DEST
    29	0608 8E0640           (     sideload.asm):00029                 ldx #COPY_SRC_IMM
    30	060B F6063C           (     sideload.asm):00030                 ldb COPY_LEN
    31	060E                  (     sideload.asm):00031         @loop
    32	060E A680             (     sideload.asm):00032                 lda ,x+               ; copy COPY_LEN bytes (at least 1).
    33	0610 A7A0             (     sideload.asm):00033                 sta ,y+
    34	0612 5A               (     sideload.asm):00034                 decb
    35	0613 26F9             (     sideload.asm):00035                 bne @loop
    36	                      (     sideload.asm):00036
    37	0615 BE063A           (     sideload.asm):00037                 ldx JSR_DEST
    38	0618 2702             (     sideload.asm):00038                 beq @skip
    39	061A AD84             (     sideload.asm):00039                 jsr ,x                ; call the JSR, if nonzero.
    40	061C                  (     sideload.asm):00040         @skip
    41	061C 35F7             (     sideload.asm):00041                 puls cc,d,x,y,u,pc
    42	                      (     sideload.asm):00042
    43	061E                  (     sideload.asm):00043         SIDELOAD_END
    44	                      (     sideload.asm):00044
    45	                      (     sideload.asm):00045                 end SIDELOAD
*/
