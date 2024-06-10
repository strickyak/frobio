package main

import (
	"flag"
	"io"
	"log"
	"net"

	"github.com/strickyak/frobio/frob3/lemma/lan"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var LISTEN = flag.Int("listen", 2321, "listen for connetions here")
var DIAL = flag.String("dial", "pizga.net:2321", "dial upstream TCP server")

func main() {
	flag.Parse()
	go lan.ListenForLan()
	Listen()
}

func Listen() {
	l, err := net.Listen("tcp", Format(":%d", *LISTEN))
	if err != nil {
		log.Panicf("Conduit cannot Listen(): %v", err)
	}
	defer l.Close()
	log.Printf("Conduit listening on TCP port %d", *LISTEN)

	for {
		client, err := l.Accept()
		if err != nil {
			log.Panicf("Cannot Accept() connection: %v", err)
		}
		log.Printf("Accepted %q.", client.RemoteAddr().String())
		go Serve(client)
	}
}

func Serve(client net.Conn) {
	var upstream net.Conn
	var err error

	defer func() {
		r := recover()
		if r != nil {
			log.Printf("Serve(%v) shutting down; CAUGHT %v", client, r)
			if client != nil {
				client.Close()
				client = nil
			}
			if upstream != nil {
				upstream.Close()
				upstream = nil
			}
		}
	}()

	upstream, err = net.Dial("tcp", *DIAL)
	if err != nil {
		log.Panicf("Cannot dial %q: %v", *DIAL, err)
	}
	log.Printf("Dialed %v", upstream)

	done := make(chan bool, 3)
	go ConduitCopyBytes(client, upstream, done)
	go ConduitCopyBytes(upstream, client, done)

	<-done // Wait for either to finish.

	if client != nil {
		client.Close()
		client = nil
	}
	if upstream != nil {
		upstream.Close()
		upstream = nil
	}
}

func ShowConn(c net.Conn) string {
	return Format("(%s--%s)", c.LocalAddr().String(), c.RemoteAddr().String())
}

func ConduitCopyBytes(to, from net.Conn, done chan<- bool) {
	defer func() {
		done <- true
	}()

	log.Printf("Started copying %s to %s", ShowConn(from), ShowConn(to))

	// bb = make([]byte, 1)
	for {
		n, err := io.Copy(to, from)
		if n == 0 {
			break
		}
		if err != nil {
			break
		}
	}
	log.Printf("Finished copying %s to %s", ShowConn(to), ShowConn(from))
}

func CopyFromServer(client, server net.Conn, done chan<- bool) {
}

func CopyFromClient(server, client net.Conn, done chan<- bool) {
}
