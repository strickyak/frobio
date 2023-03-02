package main

import (
	"flag"
	"fmt"
	"io"
	"log"
	"net"
)

var Format = fmt.Sprintf

var PORT = flag.Int("p", 14511, "Listen on this TCP port")

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

func ReadFive(conn net.Conn) []byte {
	buf := make([]byte, 5)
	_, err := io.ReadFull(conn, buf)
	if err != nil {
		panic(err)
	}
	return buf
}

func Serve(conn net.Conn) {
	PokeRam(conn, 0x500, []byte("IT'S A UNIX SYSTEM!  I KNOW THIS!"))
	// buf := ReadFive(conn)
}

func Listen() {
	l, err := net.Listen("tcp", Format(":%d", *PORT))
	if err != nil {
		log.Fatalf("Cannot Listen(): %v", err)
	}
	defer l.Close()

	for {
		conn, err := l.Accept()
		if err != nil {
			log.Fatalf("Cannot Accept() connection: %v", err)
		}
		go Serve(conn)
	}
}

func main() {
	Listen()
}
