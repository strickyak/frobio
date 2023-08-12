package lib

// TODO: use ReadFull everywhere on the network conn.

// TODO: gc Life Generator goroutines.

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net"
	"os"
	"time"
)

var Format = fmt.Sprintf

var PORT = flag.Int("port", 2319, "Listen on this TCP port")
var PROGRAM = flag.String("program", "", "Program to upload to COCOs")
var BLOCK0 = flag.String("block0", "", "filename of block drive 0")
var DEMO = flag.String("demo", "", "run a demo")
var CARDS = flag.Bool("cards", false, "run the cards")
var READONLY = flag.String("ro", "", "read only resource directory")
var LEVEL0 = flag.String("level0", "", "level0.bin (decb) to upload")

var Block0 *os.File

const (
	CMD_POKE = 0
	CMD_CALL = 255

	CMD_LEVEL0 = 199 // when Level0 needs attention, it sends 199.

	CMD_LOG     = 200
	CMD_INKEY   = 201
	CMD_PUTCHAR = 202
	CMD_PEEK    = 203
	CMD_DATA    = 204
	CMD_SP_PC   = 205
	CMD_REV     = 206

	CMD_BLOCK_READ  = 207 // block device
	CMD_BLOCK_WRITE = 208 // block device
	CMD_BLOCK_ERROR = 209 // nack
	CMD_BLOCK_OKAY  = 210 // ack
	CMD_BOOT_BEGIN  = 211 // boot_lemma
	CMD_BOOT_CHUNK  = 212 // boot_lemma
	CMD_BOOT_END    = 213 // boot_lemma

	CMD_RTI = 214
)

var Demos map[string]func(net.Conn)

var LogicalRamImage [0x10000]byte // capture coco image.

func HiLo(a, b byte) uint {
	return (uint(a) << 8) | uint(b)
}
func Hi(x uint) byte {
	return 255 & byte(x>>8)
}
func Lo(x uint) byte {
	return 255 & byte(x)
}

func WriteFive(conn net.Conn, cmd byte, n uint, p uint) {
	log.Printf("WriteFive: %d.=%x=%q <<<<< %x %x", cmd, cmd, CmdToStr(cmd), n, p)
	_, err := conn.Write([]byte{cmd, Hi(n), Lo(n), Hi(p), Lo(p)}) // == WriteFull
	if err != nil {
		log.Panicf("writeFive: stopping due to error: %v", err)
	}
}
func ReadFive(conn net.Conn) (cmd byte, n uint, p uint) {
	quint := make([]byte, 5)
	log.Printf("ReadFive...")
	_, err := io.ReadFull(conn, quint)
	if err != nil {
		log.Panicf("ReadFive: stopping due to error: %v", err)
	}
	cmd = quint[0]
	n = HiLo(quint[1], quint[2])
	p = HiLo(quint[3], quint[4])
	log.Printf("ReadFive: %d.=%02x=%q >>>>> %04x %04x", cmd, cmd, CmdToStr(cmd), n, p)
	return
}

func ReadN(conn net.Conn, n uint) []byte {
	bb := make([]byte, n)
	log.Printf("ReadN...")
	_, err := io.ReadFull(conn, bb)
	if err != nil {
		log.Panicf("ReadN=%d.: stopping due to error: %v", n, err)
	}
	return bb
}

func PokeRam(conn net.Conn, addr uint, data []byte) {
	n := uint(len(data))

	_, err := conn.Write([]byte{CMD_POKE, Hi(n), Lo(n), Hi(addr), Lo(addr)}) // == WriteFull
	if err != nil {
		panic(err)
	}

	cc, err := conn.Write(data) // == WriteFull
	if err != nil {
		panic(err)
	}
	if cc != len(data) {
		panic("short write")
	}

    log.Printf("Poke(%x@%x) <- %x", len(data), addr, data)
}

func GetBlockDevice(ses *Session) *os.File {
	var err error
	if *BLOCK0 != "" {
		if Block0 == nil {
			log.Printf("Opening BLOCK0 == %q", *BLOCK0)
			Block0, err = os.OpenFile(*BLOCK0, os.O_RDWR, 0777)
			if err != nil {
				log.Panicf("GBD: Cannot OpenFile for Block device 0: %q: %v", *BLOCK0, err)
			}
		}
		log.Printf("GBD: returning %v", Block0)
		return Block0
	}
	log.Printf("GBD: not global BLOCK0")
	log.Printf("GBD: session %v", ses)
	log.Printf("GBD: session.Block0 %v", ses.Block0)
	log.Printf("GBD: session.Block0.Name(): %q", ses.Block0.Name())
	return ses.Block0
}

func SeekSectorReturnLSN(block *os.File, n uint, p uint) int64 {
	var err error
	lsn := (int64(n&255) << 16) | int64(p)
	log.Printf("block READ LSN %d. =$%x (%q)", lsn, lsn, block.Name())

	_, err = block.Seek(256*lsn, 0)
	if err != nil {
		log.Panicf("SSRL: Cannot Seek for Block device to LSN %d.: %q: %v", lsn, block.Name(), err)
	}
	return lsn
}

func ReadFiveLoop(conn net.Conn, ses *Session) {
	var block0 *os.File

	/*
		defer func() {
			r := recover()
			if r != nil {
				log.Printf("ReadfiveLoop terminating with recovered: %v", r)
			}
		}()
	*/

	quint := make([]byte, 5)
	for {
		log.Printf("====== for: ddt will Read5 ======")

		cmd, n, p := ReadFive(conn)

		switch cmd {

		case CMD_SP_PC:
			log.Printf("CMD_SP_PC: sp=%x pc=%x", n, p)

		case CMD_LOG:
			{
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("DATA: stopping due to error: %v", err)
				}
				log.Printf("LOG %q", data)
			}

		case CMD_REV:
			{
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("DATA: stopping due to error: %v", err)
				}
				log.Printf("REV %q", data)
			}

		case CMD_INKEY:
			log.Printf("inkey $%02x %q", quint[4], quint[4:])

		case CMD_DATA:
			{
				log.Printf("DATA $%x @ $%x", n, p)
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("DATA: stopping due to error: %v", err)
				}
				for i := uint(0); i < n; i++ {
					LogicalRamImage[p+i] = data[i]
				}
				DumpHexLines("M", p, data)
			}

		case CMD_BLOCK_READ:
			{
				// block0, lsn := prepareBlockDevice(ses, n, p)
				// lsn := prepareBlockDevice(n, p)
				block0 = GetBlockDevice(ses)
				lsn := SeekSectorReturnLSN(block0, n, p)
				var err error

				buf := make([]byte, 256)
				cc, err := block0.Read(buf)
				log.Printf("read %d bytes from %q", cc, block0.Name())
				if err != nil {
					log.Panicf("BLOCK_READ: Cannot Read for Block device 0 from LSN %d: %q: %v", lsn, block0.Name(), err)
				}
				if cc != 256 {
					log.Panicf("BLOCK_READ: Short Read Block device 0 from LSN %d: %q: only %d bytes", lsn, block0.Name(), cc)
				}
				WriteFive(conn, CMD_BLOCK_OKAY, 0, 0)
				log.Printf("sent Block Okay header")

				cc, err = conn.Write(buf) // == WriteFull
				if err != nil {
					log.Panicf("BLOCK_READ: Write256: network block write failed: %v", err)
				}
				// ddt -- added this panic and it broke?
				log.Printf("Should be 255 conn.Write, but wrote %d", cc)
				if false && cc != 256 {
					log.Panicf("BLOCK_READ: Short TCP Write Block device 0 from LSN %d: %q: only %d bytes", lsn, block0.Name(), cc)
				}
				log.Printf("sent buf: [%d]", len(buf))
			}
		case CMD_BLOCK_WRITE:
			{
				// block0, lsn := prepareBlockDevice(ses, n, p)
				// lsn := prepareBlockDevice(n, p)
				block0 = GetBlockDevice(ses)
				lsn := SeekSectorReturnLSN(block0, n, p)
				var err error

				buf := make([]byte, 256)
				cc, err := io.ReadFull(conn, buf)
				if err != nil {
					log.Panicf("BLOCK_WRITE: error reading 256 from conn: %v", err)
				}
				if cc != 256 {
					log.Panicf("BLOCK_WRITE: short read, got %d, want 256 from conn: %v", cc, err)
				}

				cc, err = block0.Write(buf)
				if err != nil {
					log.Panicf("BLOCK_WRITE: Cannot Write for Block device 0 at LSN %d: %q: %v", lsn, block0.Name(), err)
				}
				if cc != 256 {
					log.Panicf("BLOCK_WRITE: Short Write Block device 0 at LSN %d: %q: only %d bytes", lsn, block0.Name(), cc)
				}
				WriteFive(conn, CMD_BLOCK_OKAY, 0, 0)

			}
		case CMD_LEVEL0:
			Level0Control(conn, ses, n, p)

		default:
			log.Panicf("BAD COMMAND $%x: %#v", quint[0], quint)
		} // end switch
	} // end for
} // end ReadFiveLoop

func UploadProgram(conn net.Conn, filename string) {
	bb, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Printf("CANNOT READ PROGRAM %q: %v", filename, err)
		return
	}

	_, err2 := conn.Write(bb)
	if err2 != nil {
		log.Printf("CANNOT UPLOAD PROGRAM %q: %v", filename, err2)
		return
	}

	log.Printf("Uploaded %q", filename)
}

func Serve(conn net.Conn) {
	defer func() {
		r := recover()
		if r != nil {
			log.Printf("Closing connection %q: Exception %v", conn.RemoteAddr().String(), r)
		} else {
			log.Printf("Done with connection %q", conn.RemoteAddr().String())
		}
		conn.Close()
	}()

	log.Printf("Serving: Poke to 0x400")
	PokeRam(conn, 0x400, []byte("IT'S A COCO SYSTEM! I KNOW THIS!"))

	if *DEMO != "" {
		demo, ok := Demos[*DEMO]
		if !ok {
			for k := range Demos {
				log.Printf("known demo: %q", k)
			}
			log.Panicf("Unknown demo: %q", *DEMO)
		}
		demo(conn)
	} else if *LEVEL0 != "" {
		go ReadFiveLoop(conn, nil)
		time.Sleep(time.Second) // Handle anything pushed, first.
		UploadProgram(conn, *LEVEL0)
	} else if *PROGRAM != "" {
		go ReadFiveLoop(conn, nil)
		time.Sleep(time.Second) // Handle anything pushed, first.
		UploadProgram(conn, *PROGRAM)
	} else {
		ses := NewSession(conn)
		Run(ses)
		log.Panicf("Run Cards: quit")
	}

	log.Printf("Serving: Sleeping.")

	done := make(chan bool, 0)
	<-done
}

///////////////////////////////////////////////////

func Listen() {
	l, err := net.Listen("tcp", Format(":%d", *PORT))
	if err != nil {
		log.Panicf("Cannot Listen(): %v", err)
	}
	defer l.Close()
	log.Printf("Listening on port %d", *PORT)

	for {
		conn, err := l.Accept()
		if err != nil {
			log.Panicf("Cannot Accept() connection: %v", err)
		}
		log.Printf("Accepted %q.", conn.RemoteAddr().String())
		go Serve(conn)
	}
}

func CmdToStr(cmd byte) string {
  switch cmd {
	case CMD_POKE: return "POKE"
	case CMD_CALL: return "CALL"

	case CMD_LEVEL0: return "LEVEL0"

	case CMD_LOG    : return "LOG"
	case CMD_INKEY  : return "INKEY"
	case CMD_PUTCHAR: return "PUTCHAR"
	case CMD_PEEK   : return "PEEK"
	case CMD_DATA   : return "DATA"
	case CMD_SP_PC  : return "SP_PC"
	case CMD_REV    : return "REV"

	case CMD_BLOCK_READ : return "BLOCK_READ"
	case CMD_BLOCK_WRITE: return "BLOCK_WRITE"
	case CMD_BLOCK_ERROR: return "BLOCK_ERROR"
	case CMD_BLOCK_OKAY : return "BLOCK_OKAY"
	case CMD_BOOT_BEGIN : return "BOOT_BEGIN"
	case CMD_BOOT_CHUNK : return "BOOT_CHUNK"
	case CMD_BOOT_END   : return "BOOT_END"

	case CMD_RTI: return "RTI"
  }
  log.Panicf("Bad CMD byte: %d.=$%x", cmd)
  panic(0)
}
