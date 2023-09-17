package level0

import (
    "fmt"
	"bytes"
	"encoding/binary"
	"flag"
	"io/ioutil"
	"log"
	"net"
	"path/filepath"
	"strconv"
	"strings"
    "time"

    "github.com/strickyak/frobio/frob3/lemma/sym"
    . "github.com/strickyak/frobio/frob3/lemma/lib"
)

var F = fmt.Sprintf
var L = log.Printf

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

type Regs6809 struct {
	CC byte
	D  Word
	DP byte
	X  Word
	Y  Word
	U  Word
	PC Word // S starts here for RTI.
}

var Cur *L0Proc

type L0Proc struct {
	pid      byte
	stack   Word // downward
	params  Word
	data    Word // data address
	uid     Word
	state   Word
	modaddr Word
    icptCall Word  // F$ICPT
    icptData Word  // F$ICPT

	parent    *L0Proc
	deadKids  []*L0Proc

	module  *L0Module
	parambb []byte

	files [16]*L0File
    cwd *L0File
    cxd *L0File
}

type L0Device interface {
    Open(mode byte, pathname string) (fd byte, errnum byte)
}

var Devices = make(map[string]L0Device)

type L0File struct {
	name string
    mode byte
    isDir bool
    isTerm bool
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
func (m L0Module) Exec() Word     { return HiLo(m.contents[9], m.contents[10]) }
func (m L0Module) DataSize() Word { return HiLo(m.contents[11], m.contents[12]) }

func (p L0Proc) String() string     { return F("L0Proc<%d,%s>", p.pid, p.module.name) }
func (f L0File) String() string     { return F("L0File<%s>", f.name) }
func (m L0Module) String() string     { return F("L0Module<%s>", m.name) }

func (r * Regs6809) A() byte { return Hi(r.D) }
func (r * Regs6809) B() byte { return Lo(r.D) }
func (r * Regs6809) SetA(a byte) { r.D = HiLo(a, r.B()) }
func (r * Regs6809) SetB(b byte) { r.D = HiLo(r.A(), b) }

var ModuleMap map[string]*L0Module
var nextPid = byte(0)

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

func FindModule(name string) (*L0Module, error) {
	m, ok := ModuleMap[strings.ToUpper(name)]
	if !ok {
		return m, Errorf("E_MNF cannot find module %q", name)
	}
	return m, nil
}

func (p *L0Proc) DoExit(status byte) {
    if p.parent == nil {
        log.Panicf("Orphan Death status %x", status)
    }
    panic("exit TODO")
}

func Launch(conn net.Conn, module string, params string, parent *L0Proc) (*L0Proc, error) {
	log.Printf("LLL HELLO Launch %q %q %#v", module, params, parent)

    cwd := &L0File{
        name: "/DD",
        isDir: true,
    }

    cxd := &L0File{
        name: "/DD/CMDS",
        isDir: true,
    }

    term := &L0File{
        name: "/TERM",
        isTerm: true,
    }

	mod, err := FindModule(module)
	if err != nil {
	    log.Printf("LLL CANNOT FIND MODULE: Launch %q -> %#v %v", module, nil, err)
		return nil, err
	}
	nextPid++  // todo -- overflow? pick new pid?
	p := &L0Proc{
		pid:     nextPid,
		parent:    parent,
		stack:   Word(Usr_Stack - len(params)),
		parambb: []byte(params),
		params:  Word(Usr_Stack - len(params)),
		data:    Usr_Data,
		state:   Ready,
        cwd: cwd,
        cxd: cxd,

		module:  mod,
		modaddr: Usr_Load,

        files: [16]*L0File{term, term, term, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil},
	}

	log.Printf("LLL launch: %v", *p)
	PokeRam(conn, Usr_Load, mod.contents) // Upload program module to RAM.
	log.Printf("LLL uploaded")

	regs := &Regs6809{
		CC: 0xD0, /* sets interrupt disables and entire flag */
		D:  0x1234,
		DP: byte(p.data >> 8),
		X:  p.stack,
		Y:  Usr_Stack,
		U:  p.data,
		PC: Word(Usr_Load + mod.Exec()),
	}

	bb := RegsToBytes(regs)
	AssertEQ(len(bb), 12)

	log.Printf("LLL regs12...")
	PokeRam(conn, p.stack-12, bb)
	log.Printf("LLL sending RTI")
	WriteFive(conn, CMD_RTI, 0, p.stack-12)
    Cur = p
	return p, nil
}

func RegsToBytes(regs *Regs6809) []byte {
	var buf bytes.Buffer
	binary.Write(&buf, binary.BigEndian, regs)
	bb := buf.Bytes()
	AssertEQ(len(bb), 12)
    log.Printf("R2B %#v --> %x", *regs, bb)
    return bb
}

func BytesToRegs(bb []byte) *Regs6809 {
	regs := &Regs6809{}
    r := bytes.NewReader(bb)
	binary.Read(r, binary.BigEndian, regs)
    log.Printf("B2R %x --> %#v", bb, *regs)
    return regs
}

func ExtractName(bb []byte, i Word, filename string) string {
	var buf bytes.Buffer
	for {
		ch := 127 & bb[i]
		AssertLE('-', ch, filename, i, ch)
		AssertLE(ch, 'z', filename, i, ch)
		buf.WriteByte(ch)
		if 128 <= bb[i] {
			break
		}
		i++
	}
	return buf.String()
}

func ExtractFileName(bb []byte) string {
	var buf bytes.Buffer
    i := 0
	for {
        // stop at \0 \n \r ...
		ch := 127 & bb[i]
        if ch < '!' { break }
		buf.WriteByte(ch)
        // stop when high bit was set.
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

func Peek(conn net.Conn, addr Word, n Word) []byte {
	WriteFive(conn, CMD_PEEK, n, addr)
	cmd, m, p := ReadFive(conn)
	if cmd != CMD_DATA || m != n || p != addr {
		log.Panicf("bad globals data quint")
	}
	z := ReadN(conn, n)
	log.Printf("Peek(%x@%x) -> %x", n, addr, z)
	return z
}

func ChopLine(a []byte) []byte {
    var z bytes.Buffer
    for _, b := range a {
        if b == 0 {
            return z.Bytes()
        }
        z.WriteByte(b & 127)
        if b == '\r' || b == '\n' {
            return z.Bytes()
        }
        if (b & 128) != 0 {
            return z.Bytes()
        }
    }
    return z.Bytes()
}

func NextFileDesc() (fd byte, errnum byte) {
    for i := byte(0); i < 16; i++ {
        if Cur.files[i] == nil {
            return i, 0
        }
    }
    return 0, sym.E_PthFul
}

func internalOpen(mode byte, filename string) *L0File {
    return &L0File{
        mode: mode,
        name: filename,
    }
}

func Do_I_Read(conn net.Conn, ses *Session, regs *Regs6809) (errnum byte) {
/*
    f := Cur.files[regs.A()]
*/
    L("I_Read: %v", regs)
    panic("TODO read")
}

func Do_I_Open(conn net.Conn, ses *Session, regs *Regs6809) (fd byte, errnum byte) {
    mode := regs.A()
    pathBegin := regs.X
    bb := Peek(conn , pathBegin, 80 )
    filename := ExtractFileName(bb)
    log.Printf("Open($%x, %q)", mode, filename)

    fd, errnum = NextFileDesc()
    if errnum > 0 {
        return 0, errnum
    }

    Cur.files[fd] = internalOpen(mode, filename)
    return fd, errnum
}

func Do_I_WritLn(conn net.Conn, ses *Session, regs *Regs6809) byte {
    block := Peek(conn, regs.X, regs.Y)
    line := ChopLine(block)
    log.Printf("WritLn(%d.) ===>>> %q", Hi(regs.D), line)
    regs.Y = Word(len(line))
    return 0
}

func Do_F_Time(conn net.Conn, ses *Session, regs *Regs6809) byte {
    // Magic layout example is "Mon Jan  2 15:04:05 2006"
    s := time.Now().Format("2006 1 2 15 4 5")
    log.Printf("TIME STRING: %q", s)
    v := strings.Split(s, " ")
    AssertEQ(len(v), 6)
    var bb []byte
    for i, numstr := range v {
        n, err := strconv.ParseInt(numstr, 10, 16)
        Check(err)
        if i==0 {
            bb = append(bb, byte(n-1900))
        } else {
            bb = append(bb, byte(n))
        }
    }
    PokeRam(conn, regs.X, bb)
    return 0
}

func   ReturnFromOs9Call(conn net.Conn, ses *Session, userSP Word, regs *Regs6809, errbyte byte) {
    const carryBit = 0x01
    if errbyte==0 {
        regs.CC &^= carryBit // clear the carry bit
    } else {
        regs.CC |= carryBit // set the carry bit
    }

    bb := RegsToBytes(regs)
    PokeRam(conn, userSP, bb)

	WriteFive(conn, CMD_RTI, 0, userSP)
}

func Level0Control(conn net.Conn, ses *Session, n Word, p Word) {
	// First read the globals.
	const LEN = 16

	log.Printf("Level0Entryx")

	globals := Peek(conn, 0x07F0, LEN)
	log.Printf("globals: %x %v", globals, globals)

	userSP := HiLo(globals[12], globals[13])
    AssertEQ(userSP, p)
	whichInterrupt := globals[11]
	log.Printf("user_stack: %04x which %04x", userSP, whichInterrupt)

	stacked := Peek(conn, userSP, LEN)
	log.Printf("stacked: %x %v", stacked, stacked)

    interrupt := Lo(n)
    postbyte := Hi(n)
    AssertEQ(whichInterrupt, interrupt)

    regs := BytesToRegs(stacked)

    switch interrupt {
    case 2:
        switch postbyte {
        case 0x06: // F$Exit
            Cur.DoExit(regs.B())

        case 0x09: // F$ICPT
            Cur.icptCall = regs.X
            Cur.icptData = regs.U
            ReturnFromOs9Call(conn, ses, userSP, regs, 0)

        case 0x0C: // F$ID
            regs.SetA ( Cur.pid )
            regs.Y = 0 // userid
            ReturnFromOs9Call(conn, ses, userSP, regs, 0)

        case 0x15: // F$Time
            errbyte := Do_F_Time(conn, ses, regs)
            ReturnFromOs9Call(conn, ses, userSP, regs, errbyte)

        case 0x84: // I$Open
            fd, errbyte := Do_I_Open(conn, ses, regs)
            if errbyte == 0 {
                regs.SetA(fd)
            }
            ReturnFromOs9Call(conn, ses, userSP, regs, errbyte)
        case 0x89: // I$Read
            errbyte := Do_I_Read(conn, ses, regs)
            if errbyte == 0 {
                //regs.SetA(fd)
            }
            ReturnFromOs9Call(conn, ses, userSP, regs, errbyte)

        case 0x8c: // I$WritLn
            errbyte := Do_I_WritLn(conn, ses, regs)
            ReturnFromOs9Call(conn, ses, userSP, regs, errbyte)

        case 0x8d: // I$GetStt
            ReturnFromOs9Call(conn, ses, userSP, regs, 255)

        case 255: // Initial program launch.
	        log.Printf("gonna Launch First...")
	        Launch(conn, "SHELL" /*module*/, "\r" /*params*/, nil /*parent*/)
	        log.Printf("...done Launch First.")
            return // Explicit return to skip extra RTI.

        default:
            log.Panicf("Unimplemented OS9 Call %d.=$%02x", postbyte, postbyte)
        }
    default:
        log.Panicf("Unknown interrupt vector %x", interrupt)
    }

    // TODO move here. ReturnFromOs9Call(conn, ses, bb, errbyte)
}

func init() {
    Dispatch[CMD_LEVEL0] = Level0Control
    DispatchInit[CMD_LEVEL0] = L0Init
}
