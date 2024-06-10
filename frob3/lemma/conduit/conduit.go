package main

import (
	"flag"
	"io"
	"log"
	"net"

	C "github.com/strickyak/frobio/frob3/lemma/coms"
	"github.com/strickyak/frobio/frob3/lemma/hex"
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
	// go ConduitCopyBytes(client, upstream, done)
	// go ConduitCopyBytes(upstream, client, done)
	go CopyFromServer(client, upstream, done)
	go CopyFromClient(upstream, client, done)

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

var CommandsFromServerWithNoPayload = []byte{
	C.CMD_PEEK,
	C.CMD_SUM,
	C.CMD_BOOT_BEGIN,
	C.CMD_BOOT_END,
}

func CopyFromServer(client, server net.Conn, done chan<- bool) {
	log.Printf("Started copying server %s to client %s", ShowConn(server), ShowConn(client))
	for {
		var q C.Quint
		Value(io.ReadFull(server, q[:]))

		cmd, n, p := q.Command(), q.N(), q.P()
		log.Printf("<< %s (n=%d p=%d)", C.CmdName(cmd), n, p)
		Value(client.Write(q[:]))

		if cmd == C.CMD_BLOCK_OKAY {
			n, p = 256, 0
			log.Printf("Corrected << %s (n=%d p=%d)", C.CmdName(cmd), n, p)
		}

		var payload []byte
		if n > 0 && !InSlice(cmd, CommandsFromServerWithNoPayload) {
			payload = make([]byte, n)
			Value(io.ReadFull(server, payload))
			hex.DumpHexLines("<<<<", 0, payload)
			Value(client.Write(payload))
		}
	}
}

var CommandsFromClientWithNoPayload = []byte{}

func CopyFromClient(server, client net.Conn, done chan<- bool) {
	log.Printf("Started copying client %s to server %s", ShowConn(client), ShowConn(server))
	for {
		var q C.Quint
		Value(io.ReadFull(client, q[:]))

		cmd, n, p := q.Command(), q.N(), q.P()
		log.Printf(">> %s (n=%d p=%d)", C.CmdName(cmd), n, p)
		Value(server.Write(q[:]))

		// Work around Axiom 41C bug:
		if cmd == C.CMD_KEYBOARD {
			n, p = 8, 0 // Instead they might have been (n=0 p=2048).
			log.Printf("Corrected >> %s (n=%d p=%d)", C.CmdName(cmd), n, p)
		}

		var payload []byte
		if n > 0 && !InSlice(cmd, CommandsFromClientWithNoPayload) {
			payload = make([]byte, n)
			Value(io.ReadFull(client, payload))
			hex.DumpHexLines(">>>>", 0, payload)
			Value(server.Write(payload))
		}
	}
}
