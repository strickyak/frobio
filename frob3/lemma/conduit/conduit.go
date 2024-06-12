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

var LISTEN = flag.Int("listen", 2321, "listen for connetions here")
var DIAL = flag.String("dial", "pizga.net:2321", "dial upstream TCP server")
var CACHE_BLOCKS = flag.Int("cache_blocks", 0, "how many extra blocks to cache")

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

/*
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
*/

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
		// log.Printf("CFS: 2222*")
		var q C.Quint
		log.Printf("CFS: nando: going to read full 5")
		Value(io.ReadFull(server, q[:]))
		// log.Printf("CFS: nando: Did read full 5: %v", q[:])

		cmd, n, p := q.Command(), q.N(), q.P()
		log.Printf("<< %s (n=%d p=%d)", C.CmdName(cmd), n, p)
		// log.Printf("CFS: 11111")
		if cmd != C.CMD_CACHE_OKAY {
			// log.Printf("CFS: 11112")
			Value(client.Write(q[:]))
			// log.Printf("CFS: 11113")
			log.Printf("CFS: nando: Did write to client: %v", q[:])
		}
		// log.Printf("CFS: 11114")

		count := n
		if cmd == C.CMD_BLOCK_OKAY || cmd == C.CMD_CACHE_OKAY {
			count = 256
			// log.Printf("CFS: 11114a")
		}
		if n == 0xFFFF && p == 0xFFFF { // Special case for WRITE confirmation.
			count = 0
		}
		// log.Printf("CFS: 11114b  count=%d", count)

		var payload []byte
		// log.Printf("CFS: 11115")
		if count > 0 && !InSlice(cmd, CommandsFromServerWithNoPayload) {
			payload = make([]byte, count)
			// log.Printf("CFS: 11116 count=%d", count)
			Value(io.ReadFull(server, payload))
			// log.Printf("CFS: 11117")
			if cmd == C.CMD_CACHE_OKAY {
				// log.Printf("CFS: 11118")
				hex.DumpHexLines("..<<", 0, payload)
				key := (uint32(n) << 16) | uint32(p)
				var tmp [256]byte
				copy(tmp[:], payload)
				conduit.PutCache(key, &tmp)
			} else if cmd == C.CMD_BLOCK_OKAY {
				// log.Printf("CFS: 11119")
				hex.DumpHexLines("<<<<", 0, payload)
				key := (uint32(n) << 16) | uint32(p)
				var tmp [256]byte
				copy(tmp[:], payload)
				conduit.PutCache(key, &tmp)
				Value(client.Write(payload))
			} else {
				// log.Printf("CFS: 1111*")
				hex.DumpHexLines("<<<<", 0, payload)
				Value(client.Write(payload))
			}
			// log.Printf("CFS: 22222")
		}
		// log.Printf("CFS: 22223")
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
		log.Printf("CFC: nando: going to read full 5")
		var q C.Quint
		Value(io.ReadFull(client, q[:]))
		log.Printf("CFC: nando: Did read full 5: %v", q[:])

		cmd, n, p := q.Command(), q.N(), q.P()
		log.Printf(">> %s (n=%d p=%d)", C.CmdName(cmd), n, p)

		if cmd == C.CMD_BLOCK_READ {
			key := (uint32(n) << 16) | uint32(p)
			if tmp, ok := conduit.GetCache(key); ok {
				q2 := C.NewQuint(C.CMD_BLOCK_OKAY, n, p)
				client.Write(q2[:])
				client.Write(tmp[:])
				log.Printf("<. %s (n=%d p=%d)", C.CmdName(q2.Command()), n, p)
				hex.DumpHexLines("<<..", 0, tmp[:])
				continue LOOP
			}
		}

		log.Printf("CFC: nando: Copy to server: %v", q[:])
		Value(server.Write(q[:]))

		// Work around Axiom 41C bug:
		if cmd == C.CMD_KEYBOARD {
			n, p = 8, 0 // Instead they might have been (n=0 p=2048).
			log.Printf("Corrected >> %s (n=%d p=%d)", C.CmdName(cmd), n, p)
		}

		var payload []byte
		count := n
		if cmd == C.CMD_BLOCK_WRITE {
			count = 256
		}
		if count > 0 && !InSlice(cmd, CommandsFromClientWithNoPayload) {
			payload = make([]byte, count)
			Value(io.ReadFull(client, payload))
			hex.DumpHexLines(">>>>", 0, payload)
			Value(server.Write(payload))
			log.Printf("CFC: nando: Copy to server: %v", payload)
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
