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
)

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
	quint := make([]byte, 5)
	for {
		_, err := io.ReadFull(conn, quint)
		if err != nil {
			log.Fatalf("ReadFive: stopping due to error: %v", err)
		}

		if 'A' <= quint[0] && quint[0] <= 'Z' {
			log.Printf("ReadFive: message %q", quint)
		} else {
			cmd, n, p := quint[0], HiLo(quint[1], quint[2]), HiLo(quint[3], quint[4])
			log.Printf("ReadFive: cmd=%x n=%x p=%x ...", cmd, n, p)

			switch cmd {

			case SP_PC:
				log.Printf("ReadFive: sp=%x pc=%x", n, p)

			case LOG:
				{
					data := make([]byte, n)
					_, err := io.ReadFull(conn, data)
					if err != nil {
						log.Fatalf("ReadFive: DATA: stopping due to error: %v", err)
					}
					log.Printf("ReadFive: LOG %q", data)
				}

			case INKEY: // INKEY
				log.Printf("ReadFive: inkey $%02x %q", quint[4], quint[4:])

			case DATA: // DATA
				{
					log.Printf("ReadFive: DATA $%x @ $%x", n, p)
					data := make([]byte, n)
					_, err := io.ReadFull(conn, data)
					if err != nil {
						log.Fatalf("ReadFive: DATA: stopping due to error: %v", err)
					}
					DumpHexLines("M", p, data)
				}

			default:
				log.Fatalf("ReadFive: BAD COMMAND $%x", quint[0])
			}
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
	ReadFiveLoop(conn)
	// go ReadFiveLoop(conn)

	log.Printf("Serving: Poke to 0x400")
	PokeRam(conn, 0x400, []byte("IT'S A COCO SYSTEM! I KNOW THIS!"))

	if *PROGRAM != "" {
		UploadProgram(conn)
	}

	WriteFive(conn, PEEK, 0xC000, 256)
	WriteFive(conn, PEEK, 0xC800, 256)

	log.Printf("Serving: Sleeping.")
	time.Sleep(3600 * time.Second)
	conn.Close()
	log.Printf("Serving: Closed.")
}

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
