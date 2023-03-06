package main

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

func ReadFive(conn net.Conn) {
	quint := make([]byte, 5)
	for {
		_, err := io.ReadFull(conn, quint)
		if err != nil {
			log.Printf("ReadFive: stopping due to error: %v", err)
			break
		}

		switch quint[0] {
		case 78:
			log.Printf("ReadFive: message %q", quint)
		case 201:
			log.Printf("ReadFive: inkey $%02x %q", quint[4], quint[4:])
		default:
			log.Fatalf("ReadFive: BAD COMMAND %d.", quint[0])
		}
	}
}

func UploadProgram(conn net.Conn) {
	if *PROGRAM == "" {
		return
	}

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
	go ReadFive(conn)

	log.Printf("Serving: Poke to 0x400")
	PokeRam(conn, 0x400, []byte("IT'S A COCO SYSTEM! I KNOW THIS!"))

	/*
		    var b []byte
		    for i := 128+3; i < 256; i += 4 {
		      b = append(b, byte(i))
		    }
			PokeRam(conn, 0x600 - 64, b)
	*/

	UploadProgram(conn)

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

func main() {
	flag.Parse()
	Listen()
}
