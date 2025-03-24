package lemma

// TODO: use ReadFull everywhere on the network conn.

// TODO: gc Life Generator goroutines.

import (
	"bytes"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net"
	"os"
	"runtime/debug"
	"strings"
	"time"

	"github.com/strickyak/nekot-coco-microkernel/mcp"

	"github.com/strickyak/frobio/frob3/lemma/canvas"
	C "github.com/strickyak/frobio/frob3/lemma/coms"
	"github.com/strickyak/frobio/frob3/lemma/hex"
	"github.com/strickyak/frobio/frob3/lemma/lan"

	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var SCAN_KEYBOARD = flag.Bool("scan_keyboard", true, "flag flip for CMD_KEYBOARD (to detect SHIFT, etc)")

var PRINT_VERSION = flag.Bool("version", false, "Just print protocol version")
var PORT = flag.Int("port", 2321, "Listen on this TCP port (V41)")
var PROGRAM = flag.String("program", "", "[one program mode] Program to upload to COCOs")
var BLOCK0 = flag.String("block0", "", "[one program mode] filename of block drive 0")
var DEMO = flag.String("demo", "", "[demo mode] run a demo")
var CARDS = flag.Bool("cards", false, "Preferred mode: Present numbered pages to choose what to boot.")
var LEMMINGS_ROOT = flag.String("lemmings_root", "LEMMINGS/", "read only resource directory, for -cards mode.")
var LEVEL0 = flag.String("level0", "", "[experimental Level0 mode] level0.bin (decb) to upload")
var TESTHOST = flag.String("testhost", "", "Host that runs tests")
var FLAVOR = flag.String("flavor", "", "what flavor the server (e.g. alpha, beta, prod)")

var Block0 *os.File

var Demos map[string]func(net.Conn)

var LogicalRamImage [0x10000]byte // capture coco image.

func WriteFive(conn net.Conn, cmd byte, n uint, p uint) {
	log.Printf("WriteFive: %d.=%x %x %x", cmd, cmd, n, p)
	_, err := conn.Write([]byte{cmd, Hi(n), Lo(n), Hi(p), Lo(p)}) // == WriteFull
	if err != nil {
		log.Panicf("writeFive: stopping due to error: %v", err)
	}
}
func ReadFive(conn net.Conn) (cmd byte, n uint, p uint) {
	quint := make([]byte, 5)
	log.Printf("===================================")
	_, err := io.ReadFull(conn, quint)
	// err := MyReadFull(conn, quint)
	if err != nil {
		log.Panicf("ReadFive: stopping due to error: %v", err)
	}
	cmd = quint[0]
	n = HiLo(quint[1], quint[2])
	p = HiLo(quint[3], quint[4])
	log.Printf("ReadFive: %d.=%02x %04x %04x", cmd, cmd, n, p)
	return
}

/*
func MyReadFull(conn net.Conn, bb []byte) error {
    n := len(bb)
    done := 0
    for done < n {
        cc, err := conn.Read(bb[done:])
        if err != nil {
            return err
        }
        done += cc
    }
    return nil
}
*/

func ReadN(conn net.Conn, n uint) []byte {
	bb := make([]byte, n)
	log.Printf("ReadN...")
	_, err := io.ReadFull(conn, bb)
	// err := MyReadFull(conn, bb)
	if err != nil {
		log.Panicf("ReadN=%d.: stopping due to error: %v", n, err)
	}
	return bb
}

func CheckSum16Ram(conn net.Conn, addr uint, n uint) uint {
	_, err := conn.Write([]byte{C.CMD_SUM, Hi(n), Lo(n), Hi(addr), Lo(addr)}) // == WriteFull
	if err != nil {
		panic(err)
	}

	sumReply := ReadN(conn, 5)
	log.Printf("sumReply: % 3x", sumReply)
	if sumReply[0] != C.CMD_SUM {
		log.Panicf("Expected DATA, got %d.", sumReply[0])
	}

	return HiLo(sumReply[3], sumReply[4])
}

func Peek2Ram(conn net.Conn, addr uint, n uint) []byte {
	n1, n2 := byte(n>>8), byte(n)
	p1, p2 := byte(addr>>8), byte(addr)
	sending := []byte{C.CMD_PEEK2, 0, 4, 0, 0, n1, n2, p1, p2}
	log.Printf("peek2ram sending: % 3x", sending)
	_, err := conn.Write(sending)
	if err != nil {
		panic(err)
	}
	i := 0 // for i := 0; i < 3; i++ {
	recvHeader := ReadN(conn, 5)
	log.Printf("[%d] peek2Header: % 3x", i, recvHeader)
	if recvHeader[0] != C.CMD_PEEK2 {
		log.Panicf("Peek2Ram: Expected PEEK2, got %d", recvHeader[0])
	}
	sezN := (uint(recvHeader[1]) << 8) | uint(recvHeader[2])
	if sezN != n {
		log.Panicf("Peek2Ram: Expected recvHeader N=%d., but got N=%d.", n, sezN)
	}
	// }

	log.Printf("Peek2Ram -- reading reply payload %d. bytes", n)
	z := ReadN(conn, n)
	log.Printf("peek2: % 3x", z)
	return z
}

func PeekRam(conn net.Conn, addr uint, n uint) []byte {
	_, err := conn.Write([]byte{C.CMD_PEEK, Hi(n), Lo(n), Hi(addr), Lo(addr)}) // == WriteFull
	if err != nil {
		panic(err)
	}

	peekHeader := ReadN(conn, 5)
	log.Printf("peekHeader: %#v", peekHeader)
	if peekHeader[0] != C.CMD_DATA {
		log.Panicf("Expected DATA, got $%2x", peekHeader)
	}

	z := ReadN(conn, n)
	log.Printf("peek: %#v", z)
	return z
}

func PokeRam(conn net.Conn, addr uint, data []byte) {
	n := uint(len(data))

	_, err := conn.Write([]byte{C.CMD_POKE, Hi(n), Lo(n), Hi(addr), Lo(addr)}) // == WriteFull
	if err != nil {
		panic(err)
	}

	_, err = conn.Write(data) // == WriteFull
	if err != nil {
		panic(err)
	}
}

func ScanKeyboard(conn net.Conn) (keybits [8]byte) {
	_, err := conn.Write([]byte{C.CMD_KEYBOARD, 0, 0, 0, 0}) // == WriteFull
	if err != nil {
		panic(err)
	}
	peekHeader := ReadN(conn, 5)
	log.Printf("peekHeader: %#v", peekHeader)
	if peekHeader[0] != C.CMD_KEYBOARD {
		log.Panicf("Expected KEYBOARD, got $% 3x", peekHeader)
	}

	if peekHeader[1] != 0 && peekHeader[2] != 8 {
		log.Panicf("Expected KEYBOARD n=8, got $% 3x", peekHeader)
	}

	z := ReadN(conn, 8)
	log.Printf("scan keyboard keybits: $% 3x", z)
	copy(keybits[:], z)
	return
}

func GetBlockDevice(ses *Session) *os.File { // Nitros9 RBLemma devices
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
	// log.Printf("GBD: not global BLOCK0")
	// log.Printf("GBD: session %v", ses)
	// log.Printf("GBD: session.Block0 %v", ses.Block0)
	log.Printf("GBD: session.Block0.Name(): %q", ses.Block0.Name())
	return ses.Block0
}

func SeekSectorReturnLSN(block *os.File, n uint, p uint) int64 {
	var err error
	lsn := (int64(n&255) << 16) | int64(p)
	log.Printf("block Seek LSN %d. =$%x (%q)", lsn, lsn, block.Name())

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
	defer func() {
		// Do Cleanup functions in backwards order.
		n := len(ses.Cleanups)
		for i := range ses.Cleanups {
			Catch("ses.Cleanups", ses.Cleanups[n-1-i])
		}
	}()

	var block0 *os.File

	for {
		cmd, n, p := ReadFive(conn)
		log.Printf("===== ReadFive: cmd=%02x n=%04x p=%04x ........", cmd, n, p)

		switch cmd {
		case C.CMD_HELLO:
			{
				// Very much like CMD_DATA.
				log.Printf("ReadFive: HELLO $%x @ $%x", n, p)
				if n > 0 {
					data := make([]byte, n)
					_, err := io.ReadFull(conn, data)
					if err != nil {
						log.Panicf("ReadFive: HELLO: stopping due to error: %v", err)
					}
					for i := uint(0); i < n; i++ {
						LogicalRamImage[p+i] = data[i]
					}
					hex.DumpHexLines("M", p, data)
				}
			}

		case C.CMD_LOG:
			{
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("ReadFive: DATA: stopping due to error: %v", err)
				}
				log.Printf("ReadFive: LOG %q", data)
			}

		case C.CMD_DATA: // Sort of a core dump?
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
				hex.DumpHexLines("M", p, data)
			}

		case C.CMD_BLOCK_READ, // Nitros9 RBLemma devices
			C.CMD_CACHE_READ: // Nitros9 RBLemma devices
			{
				block0 = GetBlockDevice(ses)
				lsn := SeekSectorReturnLSN(block0, n, p)
				var err error

				buf := make([]byte, 256)
				cc, err := block0.Read(buf)
				log.Printf("read %d bytes from %q", cc, block0.Name())
				if err != io.EOF {
					if err != nil {
						log.Panicf("%s: Cannot Read for Block device 0 from LSN %d: %q: %v", C.CmdName(cmd), lsn, block0.Name(), err)
					}
					if cc != 256 {
						log.Panicf("%s: Short Read Block device 0 from LSN %d: %q: only %d bytes", C.CmdName(cmd), lsn, block0.Name(), cc)
					}
				}
				replyCmd := Cond(cmd == C.CMD_BLOCK_READ, byte(C.CMD_BLOCK_OKAY), byte(C.CMD_CACHE_OKAY))
				WriteFive(conn, replyCmd, n, p)

				cc, err = conn.Write(buf) // == WriteFull
				if err != nil {
					log.Panicf("%s: Write256: network block write failed: %v", C.CmdName(cmd), err)
				}
			}
		case C.CMD_BLOCK_WRITE: // Nitros9 RBLemma devices
			{
				block0 = GetBlockDevice(ses)
				lsn := SeekSectorReturnLSN(block0, n, p)
				var err error

				buf := make([]byte, 256)
				// log.Printf("C.CMD_BLOCK_WRITE nando Going to read 256")
				cc, err := io.ReadFull(conn, buf)
				if err != nil {
					log.Panicf("BLOCK_WRITE: error reading 256 from conn: %v", err)
				}
				if cc != 256 {
					log.Panicf("BLOCK_WRITE: short read, got %d, want 256 from conn: %v", cc, err)
				}
				// log.Printf("C.CMD_BLOCK_WRITE nando Did read 256: %v", buf)

				cc, err = block0.Write(buf)
				if err != nil {
					log.Panicf("BLOCK_WRITE: Cannot Write for Block device 0 at LSN %d: %q: %v", lsn, block0.Name(), err)
				}
				if cc != 256 {
					log.Panicf("BLOCK_WRITE: Short Write Block device 0 at LSN %d: %q: only %d bytes", lsn, block0.Name(), cc)
				}
				// log.Printf("C.CMD_BLOCK_WRITE nando Going to write C.CMD_BLOCK_OKAY")
				WriteFive(conn, C.CMD_BLOCK_OKAY, ^uint(0), ^uint(0))
				// log.Printf("C.CMD_BLOCK_WRITE nando Wrote C.CMD_BLOCK_OKAY")

			}
		case C.CMD_LEMMAN_REQUEST:
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
					hex.DumpHexLines("@@@@@@@@@@PAYLOAD", 0, buf[headerLen:])
				}

				reply := DoLemMan(conn, buf, p)
				WriteFive(conn, C.CMD_LEMMAN_REPLY, uint(len(reply)), p)

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

		case C.CMD_LEVEL0:
			Level0Control(conn, ses)

		case C.CMD_BEGIN_MUX: //  = 196
			pay := ReadN(conn, n)
			BeginMux(ses, p, pay)
		case C.CMD_MID_MUX: //  = 197
			pay := ReadN(conn, n)
			MidMux(ses, p, pay)
		case C.CMD_END_MUX: //    = 198
			pay := ReadN(conn, n)
			EndMux(ses, p, pay)

		case C.CMD_ECHO: // echos back payload with CMD_DATA
			pay := ReadN(conn, n)
			WriteFive(conn, C.CMD_DATA, n, p)
			conn.Write(pay)

		case C.CMD_DW:
			log.Printf("DW [n=%d. p=%d.]", n, p)
			panic("TODO")

		case C.CMD_HDBDOS_SECTOR:
			log.Printf("HDBDOS [n=%d. p=%d.]", n, p)
			pay := ReadN(conn, n)
			HdbDosSector(ses, pay)

		case C.CMD_HDBDOS_HIJACK:
			log.Printf("HDBDOS [n=%d. p=%d.]", n, p)
			pay := ReadN(conn, n)
			HdbDosHijack(ses, pay)

		case C.CMD_HELLO_NEKOT:
			pay := make([]byte, n)
			_, err := io.ReadFull(conn, pay)
			if err != nil {
				log.Panicf("ReadFive: pay: stopping due to error: %v", err)
			}

			// For CocoIOr to request Nekot:
			log.Printf("CMD_HELLO_NEKOT %d: %q", p, pay)
			mcp.MCP(conn, p, pay, ses.Hellos)

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

func UpperCleanName(s []byte, maxLen int) string {
	var buf bytes.Buffer
	for _, b := range s {
		if 'A' <= b && b <= 'Z' {
			buf.WriteByte(b)
		} else if '0' <= b && b <= '9' {
			buf.WriteByte(b)
		} else if 'a' <= b && b <= 'z' {
			buf.WriteByte(b - 32)
		} else {
			buf.WriteByte('_')
		}
	}
	z := strings.TrimRight(buf.String(), "_")
	if z == "" {
		z = "EMPTY"
	}
	if !('A' <= z[0] && z[0] <= 'Z') {
		z = "X" + z
	}
	if len(z) > maxLen {
		z = z[:maxLen]
	}
	return z
}

func GetHellos(conn net.Conn) map[uint][]byte {
	dict := make(map[uint][]byte)
	for {
		cmd, n, p := ReadFive(conn)
		log.Printf("GetHellos: $%x $%x $%x", cmd, n, p)
		if cmd != C.CMD_HELLO {
			log.Panicf("Expected CMD_HELLO, got $%x $%x $%x", cmd, n, p)
		}

		var payload []byte
		if n > 0 {
			payload = ReadN(conn, n)
		}
		dict[p] = payload

		if p == 0 {
			return dict
		}
	}
}

func Serve(conn net.Conn) {
	defer func() {
		r := recover()
		if r != nil {
			log.Printf("Recovered: %q", r)
			debug.PrintStack()
			log.Printf("Closing connection %q: Exception %v", conn.RemoteAddr().String(), r)
		} else {
			log.Printf("Done with connection %q", conn.RemoteAddr().String())
		}
		conn.Close()
	}()

	// You have 10 seconds to say Hello.
	timeoutDuration := 10 * time.Second
	conn.SetReadDeadline(time.Now().Add(timeoutDuration))
	hellos := GetHellos(conn)
	// You pass the test.  No more time limits.
	var noMoreDeadline time.Time // the "zero" value.
	conn.SetReadDeadline(noMoreDeadline)

	greeting, ok := hellos[0]
	if !ok {
		log.Panicf("Missing HELLO for 0")
	}

	if string(greeting) == "bonobo-nekot1" {
		// For Bonobo to request Nekot:
		log.Printf("GREETING Bonobo %d: %q", 0, greeting)
		mcp.MCP(conn, 0, nil, hellos)
		return
	}

		// hellos[0xDF00] = append(make([]byte, 128), hellos[0xDF80]...)
		romID, ok := hellos[0xDF00]
		if !ok {
			log.Panicf("Missing HELLO for 0xDF00")
		}

		axiomVars, ok := hellos[0x01DA]
		if !ok {
			log.Panicf("Missing HELLO for 0x01DA")
		}

	if string(greeting) != "nekot1" {
		if *SCAN_KEYBOARD {
			keybits := ScanKeyboard(conn)
			log.Printf("Scan Keyboard: Keybits: $% 3x", keybits)
		}
	}

		// romID := PeekRam(conn, 0xDF00, 256-12)  // Identity Last Half Sector, less 12 bytes of secret.
		// axiomVars := PeekRam(conn, 0x01DA, 128) // CASBUF contains Axiom's struct axiom4_vars

		// sum8 := CheckSum16Ram(conn, 0x8000, 0x2000)
		// sumA := CheckSum16Ram(conn, 0xA000, 0x2000)
		// sumC := CheckSum16Ram(conn, 0xC100, 0x0700)
		// sumE := CheckSum16Ram(conn, 0xE000, 0xF000)
		stack := HiLoBy(axiomVars[0:2])
		main := HiLoBy(axiomVars[2:4])
		sum8 := HiLoBy(axiomVars[4:6])   // CheckSum16Ram(conn, 0x8000, 0x2000)
		sumA := HiLoBy(axiomVars[6:8])   // CheckSum16Ram(conn, 0xA000, 0x2000)
		sumC := HiLoBy(axiomVars[8:10])  // CheckSum16Ram(conn, 0xC100, 0x0700)
		sumE := HiLoBy(axiomVars[10:12]) // CheckSum16Ram(conn, 0xE000, 0xF000)
		log.Printf("CheckSum16s: %04x %04x %04x %04x stack=%04x main=%04x", sum8, sumA, sumC, sumE, stack, main)

		hostRaw := romID[0xE0:0xE8]
		hostName := UpperCleanName(hostRaw, 8)

		hex.DumpHexLines("romID", 0xDF00, romID)
		hex.DumpHexLines("axiomVars", 0, axiomVars)

		log.Printf("hostRaw = % 3x | %q", hostRaw, hostRaw)
		log.Printf("hostName = %q", hostName)
		log.Printf("*TESTHOST = %q", *TESTHOST)


		log.Printf("~~~~~~~~~~~~~~ a  ")
	if *DEMO != "" {
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

		ses := NewSession(conn, hostName)
		func() {
			defer func() {
				println("recover: ", recover())
			}()
			go Catch("ReadFiveLoop", func() { ReadFiveLoop(conn, ses) })
			time.Sleep(time.Second) // Handle anything pushed, first.  // TODO: wait for Hello packet.
			Catch("Upload", func() { UploadProgram(conn, *PROGRAM) })
		}()
	} else {
		log.Printf("~~~~~~~~~~~~~~ f  ")
		ses := NewSession(conn, hostName)
		ses.RomID = romID
		ses.AxiomVars = axiomVars
		ses.Hellos = hellos
		ses.IScreen.PutStr(Format("host '%q' connected.\n", hostName))
		notice := RunCards(ses)
		log.Panicf("Run Cards: quit: %q", notice)
	}
	log.Printf("~~~~~~~~~~~~~~ g  ")

	log.Printf("Serving: Sleeping.")

	done := make(chan bool, 0)
	<-done
}

func Catch(label string, fn func()) (err string) {
	defer func() {
		r := recover()
		if r != nil {
			err = fmt.Sprintf("%v", r)
			log.Printf("Catch %q: %q", label, err)
		}
	}()
	fn()
	return ""
}

///////////////////////////////////////////////////

func InitDemos() {
	api := &canvas.DemosAPI{
		WriteFive: WriteFive,
		CMD_POKE:  C.CMD_POKE,
		CMD_PEEK:  C.CMD_PEEK,
		CMD_DATA:  C.CMD_DATA,
		// CMD_REV:   C.CMD_REV,
		// CMD_SP_PC: C.CMD_SP_PC,
	}

	// Some server-side demos live in canvas/life.go.
	Demos = make(map[string]func(net.Conn))
	canvas.Init(Demos, api)
}

func Listen() {
	InitDemos()
	go RunWeb()

	go lan.ListenForLan()

	l, err := net.Listen("tcp", Format(":%d", *PORT))
	if err != nil {
		log.Panicf("Cannot Listen(): %v", err)
	}
	defer l.Close()
	log.Printf("Waiter listening on TCP port %d", *PORT)

	for {
		conn, err := l.Accept()
		if err != nil {
			log.Panicf("Cannot Accept() connection: %v", err)
		}
		log.Printf("Accepted %q.", conn.RemoteAddr().String())
		go Serve(conn)
	}
}
