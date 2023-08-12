package lib

import (
	"bytes"
	"encoding/binary"
	"flag"
	"io/ioutil"
	"log"
	"net"
	"path/filepath"
	"strings"
)

var FlagL0ModuleDirPath = flag.String("l0_module_dir", "", "where to find commands for Level0")

const (
	Sys_Data = 0x7E00 // and upward
	Sys_Load = 0x7000 // and upward

	Usr_Load  = 0x3000 // and upward
	Usr_Stack = 0x3000 // downward
	Usr_Data  = 0x0800 // and upward
)

const ( // state:
	Ready   = 1
	Waiting = 2
	Blocked = 3
)

type word uint16

type Regs6809 struct {
	CC byte
	D  word
	DP byte
	X  word
	Y  word
	U  word
	PC word // S starts here for RTI.
}

type L0Proc struct {
	pid     word
	ppid    word
	stack   word // downward
	params  word
	data    word // data address
	uid     word
	state   word
	modaddr word

	module  *L0Module
	parambb []byte

	files [16]*L0File
}

type L0File struct {
	num  word
	name string
}

type L0Module struct {
	name     string
	filename string
	contents []byte
}

func (m L0Module) Kind() byte     { return m.contents[5] >> 4 }
func (m L0Module) Lang() byte     { return m.contents[5] & 15 }
func (m L0Module) Attr() byte     { return m.contents[6] >> 4 }
func (m L0Module) Rev() byte      { return m.contents[6] & 15 }
func (m L0Module) Exec() uint     { return HiLo(m.contents[9], m.contents[10]) }
func (m L0Module) DataSize() uint { return HiLo(m.contents[11], m.contents[12]) }

var ModuleMap map[string]*L0Module
var nextPid = word(0)

func FindModule(name string) (*L0Module, error) {
	m, ok := ModuleMap[strings.ToUpper(name)]
	if !ok {
		return m, Errorf("E_MNF cannot find module %q", name)
	}
	return m, nil
}

func Launch(conn net.Conn, module string, params string, parent *L0Proc) (*L0Proc, error) {
	log.Printf("XYX HELLO Launch %q %q %#v", module, params, parent)
	log.Printf("XYX HELLO Launch %q %q %#v", module, params, parent)
	log.Printf("XYX HELLO Launch %q %q %#v", module, params, parent)

	mod, err := FindModule(module)
	if err != nil {
	    log.Printf("XYX CANNOT FIND MODULE: Launch %q -> %#v %v", module, nil, err)
		return nil, err
	}
	nextPid++
	ppid := word(0)
	if parent != nil {
		ppid = parent.pid
	}
	p := &L0Proc{
		pid:     nextPid,
		ppid:    ppid,
		stack:   word(Usr_Stack - len(params)),
		parambb: []byte(params),
		params:  word(Usr_Stack - len(params)),
		data:    Usr_Data,
		state:   Ready,

		module:  mod,
		modaddr: Usr_Load,
	}

	log.Printf("XYX launch: %#v", *p)
	PokeRam(conn, Usr_Load, mod.contents)
	log.Printf("XYX uploaded")

	regs := Regs6809{
		CC: 0xD0, /* sets interrupt disables and entire flag */
		D:  0x1234,
		DP: byte(p.data >> 8),
		X:  p.stack,
		Y:  Usr_Stack,
		U:  p.data,
		PC: word(Usr_Load + mod.Exec()),
	}

	var buf bytes.Buffer
	binary.Write(&buf, binary.BigEndian, regs)
	bb := buf.Bytes()
	AssertEQ(len(bb), 12)
	log.Printf("XYX block...")
	PokeRam(conn, uint(p.stack-12), bb)
	log.Printf("XYX sent RTI block")
	WriteFive(conn, CMD_RTI, 0, uint(p.stack-12))
	return p, nil
}

func ExtractName(bb []byte, i uint, filename string) string {
	var buf bytes.Buffer
	for {
		ch := 127 & bb[i]
		AssertLE('-', ch, filename, i, ch)
		AssertLE(ch, 'z', filename, i, ch)
		buf.WriteByte(127 & bb[i])
		if 128 <= bb[i] {
			break
		}
		i++
	}
	return buf.String()
}

func AttemptToLoadModule(filename string) {
	bb, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Fatalf("cannot Read module file %q: %v", filename, err)
	}
	if len(bb) > 12 && bb[0] == 0x87 && bb[1] == 0xCD {
		hlen := int(bb[2])*256 + int(bb[3])
		if hlen != len(bb) {
			log.Fatal("Bad module length: header wants %d, got %d: %q", hlen, len(bb), filename)
		}
		hname := HiLo(bb[4], bb[5])
		name := ExtractName(bb, hname, filename)
		upname := strings.ToUpper(name)

		ModuleMap[upname] = &L0Module{
			name:     name,
			filename: filename,
			contents: bb,
		}
		log.Printf("XYX Loaded module %q len %d named %q.", filename, hlen, name)
	} else {
		log.Printf("XYX Not a module %q (wrong header)", filename)
	}
}

func Peek(conn net.Conn, addr uint, n uint) []byte {
	log.Printf("Peek1 %x %x", addr, n)
	WriteFive(conn, CMD_PEEK, n, addr)
	log.Printf("Peek2")
	cmd, m, p := ReadFive(conn)
	log.Printf("Peek3")
	if cmd != CMD_DATA || m != n || p != addr {
		log.Panicf("bad globals data quint")
	}
	log.Printf("Peek4")
	z := ReadN(conn, n)
	log.Printf("Peek5")
	return z
}

func Level0Control(conn net.Conn, ses *Session) {
	// First read the globals.
	const LEN = 16

	log.Printf("Level0Entryx")

	oldglobals := Peek(conn, 0x7E00, LEN)
	log.Printf("oldglobals: %x %v", oldglobals, oldglobals)

	globals := Peek(conn, 0x07F0, LEN)
	log.Printf("globals: %x %v", globals, globals)

	userSP := HiLo(globals[12], globals[13])
	whichInterrupt := globals[11]
	log.Printf("user_stack: %04x which %04x", userSP, whichInterrupt)

	stacked := Peek(conn, userSP, LEN)
	log.Printf("stacked: %x %v", stacked, stacked)

	Launch(conn, "date" /*module*/, "\r" /*params*/, nil /*parent*/)
	log.Printf("done Launch")
}

func L0Init() {
	ModuleMap = make(map[string]*L0Module)
	if *FlagL0ModuleDirPath != "" {
		matches, err := filepath.Glob(filepath.Join(*FlagL0ModuleDirPath, "*"))
		log.Printf("matches: %v", matches)
		if err != nil {
			log.Fatalf("Error globbing l0_module_dir %q: %v", *FlagL0ModuleDirPath, err)
		}
		for _, m := range matches {
			AttemptToLoadModule(m)
		}
	}
}
