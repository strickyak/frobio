package lib

// TODO: use ReadFull everywhere on the network conn.

// TODO: gc Life Generator goroutines.

import (
	"flag"
	"io"
	"io/ioutil"
	"log"
	"net"
	"os"
	"strings"
	"time"
)

var PORT = flag.Int("port", 2319, "Listen on this TCP port")
var PROGRAM = flag.String("program", "", "[one program mode] Program to upload to COCOs")
var BLOCK0 = flag.String("block0", "", "[one program mode] filename of block drive 0")
var DEMO = flag.String("demo", "", "[demo mode] run a demo")
var CARDS = flag.Bool("cards", false, "Preferred mode: Present numbered pages to choose what to boot.")
var READONLY = flag.String("ro", "", "read only resource directory, for -cards mode.")
var LEVEL0 = flag.String("level0", "", "[experimental Level0 mode] level0.bin (decb) to upload")
var LAN = flag.String("lan", "", "Local IP address of interface for LAN Discovery Response.")
var TESTHOST = flag.String("testhost", "", "Host that runs tests")

var Block0 *os.File

const (
	CMD_POKE = 0
	CMD_CALL = 255

	CMD_BEGIN_MUX = 196
	CMD_MID_MUX   = 197
	CMD_END_MUX   = 198

	CMD_LEVEL0 = 199 // when Level0 needs attention, it sends 199.

	CMD_LOG     = 200
	CMD_INKEY   = 201
	CMD_PUTCHAR = 202
	CMD_PEEK    = 203
	CMD_DATA    = 204
	CMD_SP_PC   = 205
	CMD_REV     = 206

	CMD_BLOCK_READ     = 207 // block device
	CMD_BLOCK_WRITE    = 208 // block device
	CMD_BLOCK_ERROR    = 209 // nack
	CMD_BLOCK_OKAY     = 210 // ack
	CMD_BOOT_BEGIN     = 211 // boot_lemma
	CMD_BOOT_CHUNK     = 212 // boot_lemma
	CMD_BOOT_END       = 213 // boot_lemma
	CMD_LEMMAN_REQUEST = 214 // LemMan
	CMD_LEMMAN_REPLY   = 215 // LemMan

	CMD_RTI = 216
	CMD_ECHO = 217  // reply with CMD_DATA, with high bits toggled.
	CMD_DW = 218
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
	log.Printf("WriteFive: %d.=%x %x %x", cmd, cmd, n, p)
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
	log.Printf("ReadFive: %d.=%02x %04x %04x", cmd, cmd, n, p)
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

func PeekRam(conn net.Conn, addr uint, n uint) []byte {
	_, err := conn.Write([]byte{CMD_PEEK, Hi(n), Lo(n), Hi(addr), Lo(addr)}) // == WriteFull
	if err != nil {
		panic(err)
	}

	peekHeader := ReadN(conn, 5)
	log.Printf("peekHeader: %#v", peekHeader)
	if peekHeader[0] != CMD_DATA {
		log.Panicf("Expected DATA")
	}

	z := ReadN(conn, n)
	log.Printf("peek: %#v", z)
	return z
}

func PokeRam(conn net.Conn, addr uint, data []byte) {
	n := uint(len(data))

	_, err := conn.Write([]byte{CMD_POKE, Hi(n), Lo(n), Hi(addr), Lo(addr)}) // == WriteFull
	if err != nil {
		panic(err)
	}

	_, err = conn.Write(data) // == WriteFull
	if err != nil {
		panic(err)
	}
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

var LemManNames = []string{
	"Lem_UNKNOWN_ZERO", // 0
	"Lem_Create",       // 1
	"Lem_Open",         // 2
	"Lem_MakDir",       // 3
	"Lem_ChgDir",       // 4
	"Lem_Delete",       // 5
	"Lem_Seek",         // 6
	"Lem_Read",         // 7
	"Lem_Write",        // 8
	"Lem_ReadLn",       // 9
	"Lem_WritLn",       // 10
	"Lem_GetStat",      // 11
	"Lem_SetStat",      // 12
	"Lem_Close",        // 13
}

func LemManCommandName(b byte) string {
	if int(b) >= len(LemManNames) {
		log.Panicf("Lem_UKNOWN_%d", b)
	}
	return LemManNames[b]
}
func RegsString(b []byte) string {
	return Format("cc=%02x D=%04x dp=%02x  X=%04x Y=%04x  U=%04x PC=%04x",
		b[0], HiLo(b[1], b[2]), b[3],
		HiLo(b[4], b[5]),
		HiLo(b[6], b[7]),
		HiLo(b[8], b[9]),
		HiLo(b[10], b[11]))
}

func ReadFiveLoop(conn net.Conn, ses *Session) {
	var block0 *os.File
	quint := make([]byte, 5)

	for {
		cmd, n, p := ReadFive(conn)
		log.Printf("ReadFive: cmd=%02x n=%04x p=%04x ...", cmd, n, p)

		switch cmd {

		case CMD_SP_PC: // MISUSES N
			log.Printf("DEPRECATED: CMD_SP_PC should not be used any more.")
			log.Printf("ReadFive: sp=%x pc=%x", n, p)

		case CMD_LOG:
			{
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("ReadFive: DATA: stopping due to error: %v", err)
				}
				log.Printf("ReadFive: LOG %q", data)
			}

		case CMD_REV:
			{
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("ReadFive: DATA: stopping due to error: %v", err)
				}
				log.Printf("ReadFive: REV %q", data)
			}

		case CMD_INKEY: // N=0, OR MISUSES N
			log.Printf("ReadFive: inkey $%02x %q", quint[4], quint[4:])

		case CMD_DATA: // Sort of a core dump?
			{
				log.Printf("ReadFive: DATA $%x @ $%x", n, p)
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("ReadFive: DATA: stopping due to error: %v", err)
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
				if err != io.EOF {
					if err != nil {
						log.Panicf("BLOCK_READ: Cannot Read for Block device 0 from LSN %d: %q: %v", lsn, block0.Name(), err)
					}
					if cc != 256 {
						log.Panicf("BLOCK_READ: Short Read Block device 0 from LSN %d: %q: only %d bytes", lsn, block0.Name(), cc)
					}
				}
				WriteFive(conn, CMD_BLOCK_OKAY, 0, 0)

				cc, err = conn.Write(buf) // == WriteFull
				if err != nil {
					log.Panicf("BLOCK_READ: Write256: network block write failed: %v", err)
				}
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
		case CMD_LEMMAN_REQUEST:
			{
				var buf []byte
				if n > 0 {
					buf = make([]byte, n)
					{
						cc, err := io.ReadFull(conn, buf)
						if err != nil {
							log.Panicf("CMD_LEMMAN_REQUEST: error reading %d bytes from conn: %v", n, err)
						}
						if uint(cc) != n {
							log.Panicf("CMD_LEMMAN_REQUEST: short read, got %d, want %d bytes from conn: %v", cc, n, err)
						}
					}
				}

				log.Printf("@@@@@@@@@@ LemMan %d = %q :: n=$%x p=$%x :: %q :: %02x", buf[0], LemManCommandName(buf[0]), n, p, RegsString(buf[1:13]), buf)
				const headerLen = 15
				if len(buf) > headerLen {
					DumpHexLines("@@@@@@@@@@PAYLOAD", 0, buf[headerLen:])
				}

				reply := DoLemMan(conn, buf, p)
				WriteFive(conn, CMD_LEMMAN_REPLY, uint(len(reply)), p)

				{
					cc, err := conn.Write(reply)
					if err != nil {
						log.Panicf("CMD_LEMMAN_REPLY: error reading %d bytes from conn: %v", n, err)
					}
					if cc != len(reply) {
						log.Panicf("CMD_LEMMAN_REPLY: short read, got %d, want %d bytes from conn: %v", cc, n, err)
					}
				}
			}

		case CMD_LEVEL0:
			Level0Control(conn, ses)

		case CMD_BEGIN_MUX: //  = 196
			pay := ReadN(conn, n)
			BeginMux(ses, p, pay)
		case CMD_MID_MUX: //  = 197
			pay := ReadN(conn, n)
			MidMux(ses, p, pay)
		case CMD_END_MUX: //    = 198
			pay := ReadN(conn, n)
			EndMux(ses, p, pay)

		case CMD_ECHO:  // echos back data with high bit toggled.
			pay := ReadN(conn, 4)
			WriteFive(conn, CMD_DATA, 4, p)
			for i, e := range pay {
				pay[i] = 128 ^ e  // toggle high bits in payload.
			}
			conn.Write(pay)

		case CMD_DW:  // echos back data with high bit toggled.
			log.Printf("DW [n=%d.]", n)
			panic("TODO")

		default:
			log.Panicf("ReadFive: BAD COMMAND $%x=%d.", cmd, cmd)
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
	hostbytes := PeekRam(conn, 0xDFE0, 8) // Hostname in ROM
	hostname := strings.TrimRight(string(hostbytes), " _\377\n\r\000")
	if hostname == "" {
		hostname = "EMPTY"
	}
	log.Printf("hostname = %q", hostname)
	log.Printf("*TESTHOST = %q", *TESTHOST)

	log.Printf("~~~~~~~~~~~~~~ a  ")
	if false && hostname == *TESTHOST {
	log.Printf("~~~~~~~~~~~~~~ b  ")
		ses := NewSession(conn)
		ses.Screen.PutStr(Format("TEST MODE for host '%q'\n", hostname))
		test_card := Cards[328] // Nitros9 Level 2 for M6809

		dur, _ := time.ParseDuration("5s")
		time.Sleep(dur)

		// From session.go, cards, if current.Block0 != "":
		{
			tail := strings.TrimPrefix(test_card.Block0, ".")
			log.Printf("TEST Block0: %q", tail)
			ses.Screen.PutStr(Format("TEST Block0: %q", tail))
			ses.Block0 = DuplicateFileToTemp(
				*READONLY+"/"+tail,
				"f.say STARTUP OKAY")
		}

		{
			tail := strings.TrimPrefix(test_card.Launch, ".")
			log.Printf("Upload: %q", tail)
			UploadProgram(ses.Conn, *READONLY+"/"+tail)
			ReadFiveLoop(ses.Conn, ses)
			return
		}

	} else if *DEMO != "" {
	log.Printf("~~~~~~~~~~~~~~ c  ")

		demo, ok := Demos[*DEMO]
		if !ok {
			for k := range Demos {
				log.Printf("known demo: %q", k)
			}
			log.Panicf("Unknown demo: %q", *DEMO)
		}
		demo(conn)

	} else if *LEVEL0 != "" {
	log.Printf("~~~~~~~~~~~~~~ d  ")

		go ReadFiveLoop(conn, nil)
		time.Sleep(time.Second) // Handle anything pushed, first.
		UploadProgram(conn, *LEVEL0)

	} else if *PROGRAM != "" {
	log.Printf("~~~~~~~~~~~~~~ e  ")

		ses := NewSession(conn)
		func() {
			defer func() {
				println("recover: ", recover())
			}()
			go Catch(func() { ReadFiveLoop(conn, ses) })
			time.Sleep(time.Second) // Handle anything pushed, first.
			Catch(func() { UploadProgram(conn, *PROGRAM) })
		}()
	} else {
	log.Printf("~~~~~~~~~~~~~~ f  ")
		ses := NewSession(conn)
		ses.Screen.PutStr(Format("host '%q' connected.\n", hostname))
		Run(ses)
		log.Panicf("Run Cards: quit")
	}
	log.Printf("~~~~~~~~~~~~~~ g  ")

	log.Printf("Serving: Sleeping.")

	done := make(chan bool, 0)
	<-done
}

func Catch(fn func()) {
	defer func() {
		log.Printf("catch: %v", recover())
	}()
	fn()
}

///////////////////////////////////////////////////

func Listen() {
	if *LAN != "" {
		go ListenForLan(*LAN)
	}

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

/*
	                   // Add this code to save a snapshot of Ram that we've seen:

			if p == 0xFE00 && n == 256 {
				const RamFile = "/tmp/coco.ram"
				err = ioutil.WriteFile(RamFile, LogicalRamImage[:], 0777)
				if err != nil {
					log.Panicf("ReadFive: DATA: writing %q: %v", RamFile, err)
				}
			}
*/
