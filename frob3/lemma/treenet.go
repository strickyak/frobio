package lemma

import (
	"flag"
)

var KIDSPORT = flag.Int("kidsport", 20185, "Lemma kids can connect here (for treenet)")
var PARENT = flag.String("parent", "", "Connect to a parent lemma host:port (for treenet)")
var NAME = flag.String("name", "", "Name to use (for treenet)")

var KnownNodes map[string]*TreeNode // closure Parents and immediate Kids

type TreeNode struct {
	Serial  int
	Version int
	Flags   []byte
	Host    string
	Mac     [5]byte
	Secret  []byte

	Parent *TreeNode
	Kids   map[string]*TreeNode

	User   string
	Groups []string
	Where  *TreeNode // which host authenticated User & Group namespace

	FanMax    int
	ZeroNet   int
	ZeroSlots int

	// Encrypt *Encryption
	// First   []byte // e.g. the Challenge
	// Second  []byte // e.g. the Reply
}

/*
func TreeListen() {
	l, err := net.Listen("tcp", Format(":%d", *KIDSPORT))
	if err != nil {
		log.Panicf("Tree: Cannot Listen(): %v", err)
	}
	defer l.Close()
	log.Printf("Tree: Listening on port %d", *PORT)

	for {
		conn, err := l.Accept()
		if err != nil {
			log.Panicf("Tree: Cannot Accept() connection: %v", err)
		}
		log.Printf("Tree: Accepted %q.", conn.RemoteAddr().String())
		go TreeServe(conn)
	}
}

func TreeServe(conn net.Conn) {
	defer func() {
		r := recover()
		if r != nil {
			log.Printf("Tree: Closing connection %q: Exception %v", conn.RemoteAddr().String(), r)
		} else {
			log.Printf("Tree: Done with connection %q", conn.RemoteAddr().String())
		}
		conn.Close()
	}()

  var q Quint

TODO
*/
