package main

// Debugging Hints
//
// ./lemma-waiter  -cards -lemmings_root pizga-base/Internal/LEMMINGS -config_by_dhcp=0 --nav_root pizga --web_static pizga-base/Internal/web-static/ --port=12345
//
// - pizga-base/Internal/bin/conduit --lan=10.23.23.23 -dial=127.0.0.1:12345 --cache_blocks=8

import (
	"flag"
	"io"
	"log"
	"net"
	"sync"

	C "github.com/strickyak/frobio/frob3/lemma/coms"
	"github.com/strickyak/frobio/frob3/lemma/hex"
	"github.com/strickyak/frobio/frob3/lemma/lan"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var LISTEN = flag.Int("listen", 2321, "listen for connections on this port")
var DIAL = flag.String("dial", "pizga.net:2321", "dial upstream TCP server")
var CACHE_BLOCKS = flag.Int("cache_blocks", 7, "how many extra blocks to cache")
var Q = flag.Bool("q", false, "be extra quiet")
var V = flag.Bool("v", false, "be extra verbose (show packets)")

func main() {
	defer func() {
		r := recover()
		if r != nil {
			log.Fatalf("Conduit FATAL: %v", r)
		}
	}()

	flag.Parse()
	if *V {
		*Q = false // can't be quiet, if being verbose.
	}

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

	conduit := &Conduit{
		blockCache: make(map[uint32]*[256]byte),
		done:       make(chan bool, 3),
	}
	// go ConduitCopyBytes(client, upstream, done)
	// go ConduitCopyBytes(upstream, client, done)
	go CopyFromServer(client, upstream, conduit)
	go CopyFromClient(upstream, client, conduit)

	<-conduit.done // Wait for either to finish.

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

type Conduit struct {
	blockCache map[uint32]*[256]byte
	done       chan bool
	mutex      sync.Mutex
}

func (o *Conduit) GetCache(key uint32) (block *[256]byte, ok bool) {
	o.mutex.Lock()
	defer o.mutex.Unlock()
	block, ok = o.blockCache[key]
	return
}

func (o *Conduit) PutCache(key uint32, block *[256]byte) {
	o.mutex.Lock()
	defer o.mutex.Unlock()
	o.blockCache[key] = block
}

var CommandsFromServerWithNoPayload = []byte{
	C.CMD_PEEK,
	C.CMD_SUM,
	C.CMD_BOOT_BEGIN,
	C.CMD_BOOT_END,
}

func CopyFromServer(client, server net.Conn, conduit *Conduit) {
	defer func() {
		conduit.done <- true
		r := recover()
		if r != nil {
			log.Printf("CopyFromServer: Dies after catching: %q", r)
		}
	}()

	log.Printf("Started copying server %s to client %s", ShowConn(server), ShowConn(client))
	for {
		var q C.Quint
		Value(io.ReadFull(server, q[:]))

		cmd, n, p := q.Command(), q.N(), q.P()
		Say("<< %s (n=%d p=%d)", C.CmdName(cmd), n, p)
		if cmd != C.CMD_CACHE_OKAY {
			Value(client.Write(q[:]))
		}

		count := n
		if cmd == C.CMD_BLOCK_OKAY || cmd == C.CMD_CACHE_OKAY {
			count = 256
		}
		if n == 0xFFFF && p == 0xFFFF { // Special case for WRITE confirmation.
			count = 0
		}

		var payload []byte
		if count > 0 && !InSlice(cmd, CommandsFromServerWithNoPayload) {
			payload = make([]byte, count)
			Value(io.ReadFull(server, payload))
			if cmd == C.CMD_CACHE_OKAY {
				VerboseDumpHexLines("..<<", 0, payload)
				key := (uint32(n) << 16) | uint32(p)
				var tmp [256]byte
				copy(tmp[:], payload)
				conduit.PutCache(key, &tmp)
			} else if cmd == C.CMD_BLOCK_OKAY {
				VerboseDumpHexLines("<<<<", 0, payload)
				key := (uint32(n) << 16) | uint32(p)
				var tmp [256]byte
				copy(tmp[:], payload)
				conduit.PutCache(key, &tmp)
				Value(client.Write(payload))
			} else {
				VerboseDumpHexLines("<<<<", 0, payload)
				Value(client.Write(payload))
			}
		}
	}
}

var CommandsFromClientWithNoPayload = []byte{
	C.CMD_BLOCK_READ,
	C.CMD_CACHE_READ, // does not actually come from coco, yet.
}

func CopyFromClient(server, client net.Conn, conduit *Conduit) {
	defer func() {
		conduit.done <- true
		r := recover()
		if r != nil {
			log.Printf("CopyFromClient: Dies after catching: %q", r)
		}
	}()

	log.Printf("Started copying client %s to server %s", ShowConn(client), ShowConn(server))
LOOP:
	for {
		Verbose("CFC: nando: going to read full 5")
		var q C.Quint
		Value(io.ReadFull(client, q[:]))
		Verbose("CFC: nando: Did read full 5: %v", q[:])

		cmd, n, p := q.Command(), q.N(), q.P()
		Say(">> %s (n=%d p=%d)", C.CmdName(cmd), n, p)

		if cmd == C.CMD_BLOCK_READ {
			key := (uint32(n) << 16) | uint32(p)
			if tmp, ok := conduit.GetCache(key); ok {
				q2 := C.NewQuint(C.CMD_BLOCK_OKAY, n, p)
				client.Write(q2[:])
				client.Write(tmp[:])
				Say("<. %s (n=%d p=%d)", C.CmdName(q2.Command()), n, p)
				VerboseDumpHexLines("<<..", 0, tmp[:])
				continue LOOP
			}
		}

		Value(server.Write(q[:]))

		// Work around Axiom 41C bug:
		if cmd == C.CMD_KEYBOARD {
			n, p = 8, 0 // Instead they might have been (n=0 p=2048).
			Say("Corrected >> %s (n=%d p=%d)", C.CmdName(cmd), n, p)
		}

		var payload []byte
		count := n
		if cmd == C.CMD_BLOCK_WRITE {
			count = 256
		}
		if count > 0 && !InSlice(cmd, CommandsFromClientWithNoPayload) {
			payload = make([]byte, count)
			Value(io.ReadFull(client, payload))
			VerboseDumpHexLines(">>>>", 0, payload)
			Value(server.Write(payload))
		}

		// Writes must update cache.
		if cmd == C.CMD_BLOCK_WRITE {
			key := (uint32(n) << 16) | uint32(p)
			var tmp [256]byte
			copy(tmp[:], payload)
			conduit.PutCache(key, &tmp)
		}

		if *CACHE_BLOCKS > 0 {
			switch cmd {
			case C.CMD_BLOCK_READ:
			CACHE_REQUESTS:
				for i := 1; i <= *CACHE_BLOCKS; i++ {
					n2, p2 := n, p
					p2 += uint(i)
					if p2 < p { // overflowed
						n2++
					}
					key := (uint32(n) << 16) | uint32(p)
					if _, ok := conduit.GetCache(key); ok {
						continue CACHE_REQUESTS // we already have it cached.
					}
					q2 := C.NewQuint(C.CMD_CACHE_READ, n2, p2)
					Value(server.Write(q2[:]))
				}
			}
		}
	}
}

// VerboseDumpHexLines: Dump Hex if Verbose.
func VerboseDumpHexLines(label string, offset uint, bb []byte) {
	if *V {
		hex.DumpHexLines(label, offset, bb)
	}
}

// Verbose: Log if Verbose
func Verbose(format string, args ...any) {
	if *V {
		log.Printf(format, args...)
	}
}

// Say: Log unless Quiet.
func Say(format string, args ...any) {
	if !*Q {
		log.Printf(format, args...)
	}
}
