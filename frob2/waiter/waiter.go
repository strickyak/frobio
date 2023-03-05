package main

import (
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"time"
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

func ReadFive(conn net.Conn) {
    quint := make([]byte, 5)
    for {
        _, err := io.ReadFull(conn, quint)
        if err != nil {
            log.Printf("ReadFive: stopping due to error: %v", err);
            break;
        }

        switch quint[0] {
        case 78:
            log.Printf("ReadFive: message %q", quint)
        case 201:
            log.Printf("ReadFive: inkey $%02x %q", quint[4], quint[4:])
        default:
            log.Fatalf("ReadFive: BAD COMMAND %d.", quint[0]);
        }
    }
}

func Serve(conn net.Conn) {
    log.Printf("gonna ReadFive");
    go ReadFive(conn);

    log.Printf("Serving: Poke to 0x400");
	PokeRam(conn, 0x400, []byte("IT'S A COCO SYSTEM! I KNOW THIS!"))

    log.Printf("Serving: Sleeping.");
    time.Sleep(3600 * time.Second)
    conn.Close()
    log.Printf("Serving: Closed.");
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
	Listen()
}
