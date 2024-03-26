package lib

import (
	"flag"
	"log"
	"os"
)

var FlagDosDisk = flag.String("dos_disk", "", "for HdbDos")

func HdbDos(ses *Session, payload []byte) {
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
