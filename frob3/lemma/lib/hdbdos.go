package lib

import (
	"flag"
	"os"
	"log"
)

var FlagDosDisk = flag.String("dos_disk", "", "for HdbDos")

func HdbDos(ses *Session, payload []byte) {
	DumpHexLines("payload", 0, payload)

	cmd := payload[0]
	drive := payload[1]
	lsn2 := uint(payload[2])
	lsn1 := uint(payload[3])
	lsn0 := uint(payload[4])
	lsn := (lsn2<<16) | (lsn1<<8) | lsn0
	log.Printf("HdbDos cmd=%x drive=%x lsn=%x", cmd, drive, lsn)

	fd := Value(os.Open(*FlagDosDisk))
	buf := make([]byte, 256+5)
	for i:=0; i<5; i++ {
		buf[i] = 250  // not really used $FA...
	}
	Value(fd.Seek(256*int64(lsn), 0))
	Value(fd.Read(buf[5:]))

	log.Printf("HdbDos Reply Packet: quint + %d.", len(buf))
	DumpHexLines("REPLY", 0, buf)
	WriteQuint(ses.Conn, CMD_HDBDOS, 0, buf)
}
