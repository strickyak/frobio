package waiter

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net"
	"time"
)

var Format = fmt.Sprintf

var PORT = flag.Int("port", 14511, "Listen on this TCP port")
var PROGRAM = flag.String("program", "", "Program to upload to COCOs")

const (
	POKE = 0
	CALL = 255

	LOG      = 200
	INKEY    = 201
	PUTCHARS = 202
	PEEK     = 203
	DATA     = 204
	SP_PC    = 205
	REV      = 206
)

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

func WriteFive(conn net.Conn, cmd byte, a uint, b uint) {
	log.Printf("WriteFive: %x %x %x", cmd, a, b)
	_, err := conn.Write([]byte{cmd, Hi(a), Lo(a), Hi(b), Lo(b)}) // == WriteFull
	if err != nil {
		log.Fatalf("writeFive: stopping due to error: %v", err)
	}
}

func PokeRam(conn net.Conn, addr uint, data []byte) {
	n := uint(len(data))

	_, err := conn.Write([]byte{POKE, Hi(n), Lo(n), Hi(addr), Lo(addr)}) // == WriteFull
	if err != nil {
		panic(err)
	}

	_, err = conn.Write(data) // == WriteFull
	if err != nil {
		panic(err)
	}
}

func ReadFiveLoop(conn net.Conn) {
	defer func() {
		r := recover()
		if r != nil {
			log.Printf("ReadfiveLoop terminating with panic: %v", r)
		}
	}()

	quint := make([]byte, 5)
	for {
		_, err := io.ReadFull(conn, quint)
		if err != nil {
			log.Panicf("ReadFive: stopping due to error: %v", err)
		}

		cmd, n, p := quint[0], HiLo(quint[1], quint[2]), HiLo(quint[3], quint[4])
		log.Printf("ReadFive: cmd=%02x n=%04x p=%04x ...", cmd, n, p)

		switch cmd {

		case SP_PC:
			log.Printf("ReadFive: sp=%x pc=%x", n, p)

		case LOG:
			{
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("ReadFive: DATA: stopping due to error: %v", err)
				}
				log.Printf("ReadFive: LOG %q", data)
			}

		case REV:
			{
				data := make([]byte, n)
				_, err := io.ReadFull(conn, data)
				if err != nil {
					log.Panicf("ReadFive: DATA: stopping due to error: %v", err)
				}
				log.Printf("ReadFive: REV %q", data)
			}

		case INKEY: // INKEY
			log.Printf("ReadFive: inkey $%02x %q", quint[4], quint[4:])

		case DATA: // DATA
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
				/*
					                   // Add this code to save a snapshot of Ram that we've seen:

							if p == 0xFE00 && n == 256 {
								const RamFile = "/tmp/coco.ram"
								err = ioutil.WriteFile(RamFile, LogicalRamImage[:], 0777)
								if err != nil {
									log.Fatalf("ReadFive: DATA: writing %q: %v", RamFile, err)
								}
							}
				*/
			}

		default:
			log.Panicf("ReadFive: BAD COMMAND $%x: %#v", quint[0], quint)
		}
	}
}

func UploadProgram(conn net.Conn) {
	bb, err := ioutil.ReadFile(*PROGRAM)
	if err != nil {
		log.Printf("CANNOT READ PROGRAM %q: %v", *PROGRAM, err)
		return
	}

	_, err2 := conn.Write(bb)
	if err2 != nil {
		log.Printf("CANNOT UPLOAD PROGRAM %q: %v", *PROGRAM, err2)
		return
	}
	log.Printf("Uploaded %q", *PROGRAM)
}

func Serve(conn net.Conn) {
	log.Printf("gonna ReadFive")
	go ReadFiveLoop(conn)

	log.Printf("Serving: Poke to 0x400")
	PokeRam(conn, 0x400, []byte("IT'S A COCO SYSTEM! I KNOW THIS!"))

	if *PROGRAM != "" {
		UploadProgram(conn)
	}

	// WriteFive(conn, PEEK, 256, 0xC000)
	// WriteFive(conn, PEEK, 256, 0xC800)
	/*
		  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

			hub := NewHub()
			for {
				select {
				case x := <-hub.queue
				}
			}

		  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	*/
	log.Printf("Serving: Sleeping.")
	time.Sleep(3600 * time.Second)
	conn.Close()
	log.Printf("Serving: Closed.")
}

///////////////////////////////////////////////////

func Listen() {
	l, err := net.Listen("tcp", Format(":%d", *PORT))
	if err != nil {
		log.Fatalf("Cannot Listen(): %v", err)
	}
	defer l.Close()
	log.Printf("Listening on port %d", *PORT)

	for {
		conn, err := l.Accept()
		if err != nil {
			log.Fatalf("Cannot Accept() connection: %v", err)
		}
		log.Printf("Accepted.")
		go Serve(conn)
	}
}