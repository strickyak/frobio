package lemma

import (
	"bytes"
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"os/exec"
	"sort"
	"strconv"
	"strings"
	"sync"
	"text/template"
	"time"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	T "github.com/strickyak/frobio/frob3/lemma/text"

	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var ForcePage = flag.Int("force_page", 0, "Always launch this page")

func (ses *Session) ReplyOnChannel(cmd byte, p uint, pay []byte) {
	n := uint(len(pay))
	log.Printf("PAY OUT [%d.] %v", n, pay)
	log.Printf("        [$%x] %q", n, pay)
	buf := make([]byte, 5+n)
	buf[0] = cmd
	buf[1], buf[2] = Hi(n), Lo(n)
	buf[3], buf[4] = Hi(p), Lo(p)
	copy(buf[5:], pay)

	log.Printf("SENDING %02x", buf)
	cc, err := ses.Conn.Write(buf)
	if err != nil {
		log.Panicf("SendQandP(%q) got err: %v", ses.ID, err)
	}
	if uint(cc) != 5+n {
		log.Panicf("SendQandP(%q) short write: %v", ses.ID, cc)
	}
}

type IScreen interface {
	Clear()
	PutChar(ch byte)
	PutStr(s string)
	Flush()
	//Width() int
	//Height() int
}

/*
type AxScreen struct {
	Ses *Session
}

func (ax *AxScreen) Flush() {
}

func (ax *AxScreen) Clear() {
	for i := 0; i < 16; i++ {
		ax.PutChar(13)
	}
}
func (ax *AxScreen) PutChar(ch byte) {
	q := coms.NewQuint(CMD_PUTCHAR, 0, uint(ch))
	_, err := ax.Ses.Conn.Write(q[:])
	Check(err)
}
func (ax *AxScreen) PutStr(s string) {
	for _, r := range s {
		AssertLT(r, 127)
		ax.PutChar(byte(r))
	}
}
*/

// TODO: convert to new Screen.
// TODO: requires dealing with LineBuf::GetLine
// this one depends on CMD_PUTCHAR.
type XTextScreen struct {
	B         []byte // screen RAM not yet sent.
	W, H      int    // screen dimensions
	P         int    // cursor
	Addr      int
	Majuscule bool // Use 0..63
	Ses       *Session
}

func NewTextScreen(ses *Session, w, h int, addr int) *XTextScreen {
	o := make([]byte, w*h)
	n := make([]byte, w*h)
	for i := 0; i < w*h; i++ {
		o[i] = ' '
		n[i] = ' '
	}

	return &XTextScreen{
		B:         n,
		W:         w,
		H:         h,
		P:         0,
		Addr:      addr,
		Majuscule: true,
		Ses:       ses,
	}
}

func (ax *XTextScreen) PutStr(s string) {
	for _, r := range s {
		ax.PutChar(byte(r))
	}
}
func (t *XTextScreen) Flush() {
	AssertEQ(len(t.B), 512)
	AssertEQ(t.Addr, 0x0400)
	coms.Wrap(t.Ses.Conn).WriteQuintAndPayload(coms.CMD_POKE, uint(t.Addr), t.B)
}
func (t *XTextScreen) Clear() {
	t.B = make([]byte, t.W*t.H)
	for i := range t.B {
		t.B[i] = 32
	}
}
func (t *XTextScreen) PutChar(ch byte) {
	//println("putchar", ch)
	if ch == 10 || ch == 13 {
		t.PutChar(' ')
		for (t.P & (t.W - 1)) != 0 {
			//println(t.P, t.W, (t.P & (t.W - 1)))
			t.PutChar(' ')
		}
		return
	}

	if ch == 1 && t.P > 0 { // Invert previous char
		p := t.P - 1
		t.B[p] |= 0x40
		return
	}

	if ch == 8 && t.P > 0 { // Delete previous char
		t.B[t.P] = 32 // wipe out old curser
		t.P--         // new cursor will be put here.
	}

	if ' ' <= ch && ch <= '~' {
		d := ch
		if t.Majuscule {
			if 64 <= ch && ch <= 127 {
				d = ch & 31 // becomes uppercase 0..31
			}
		}
		t.B[t.P] = d
		t.P++
		for t.P >= t.W*t.H {
			for i := t.W; i < t.W*t.H; i++ {
				t.B[i-t.W] = t.B[i]
			}
			for i := t.W * (t.H - 1); i < t.W*t.H; i++ {
				t.B[i] = 32
			}
			t.P -= t.W
		}
	}
	t.B[t.P] = 0xA3 // Blue Bar for Cursor
}

type LineBuf struct {
	Ses *Session
}

func (lb *LineBuf) GetLine() string {
	var buf []byte
	for {
		log.Printf("NANDO @@@ askchar")
		coms.Wrap(lb.Ses.Conn).WriteQuintAndPayload(coms.CMD_GETCHAR, 0, nil) // request inkey
		log.Printf("NANDO @@@ read q")

		var q coms.Quint
		_, err := io.ReadFull(lb.Ses.Conn, q[:])
		Check(err)
		cmd := q.Command()
		log.Printf("NANDO @@@ cmd %d", cmd)

		switch cmd {
		case coms.CMD_GETCHAR:
			ch := byte(q.P())
			log.Printf("NANDO @@@ GETCHAR %d", ch)
			switch ch {
			case 10, 13:
				lb.Ses.IScreen.PutChar(ch)
				lb.Ses.IScreen.Flush()
				log.Printf("NANDO @@@ Pushed (10, 13)")
				return string(buf)
			case 8:
				if len(buf) > 0 {
					buf = buf[:len(buf)-1] // trim last
					lb.Ses.IScreen.PutChar(ch)
					lb.Ses.IScreen.Flush()
					log.Printf("NANDO @@@ Pushed (8)")
				}
			default:
				if ' ' <= ch && ch <= '~' {
					buf = append(buf, ch)
					lb.Ses.IScreen.PutChar(ch)
					lb.Ses.IScreen.Flush()
					log.Printf("NANDO @@@ Pushed $d.", ch)
				} else {
					log.Printf("LineBuf: weird char: %d", ch)
				}
			} // switch ch
		default:
			log.Panicf("bad packet to GetLine: cmd=%d", cmd)
		} // switch cmd
	}
}

type Session struct {
	ID        int
	Conn      net.Conn
	IScreen   IScreen
	LineBuf   *LineBuf
	Hostname  string
	RomID     []byte
	AxiomVars []byte
	Hellos    map[uint][]byte

	Block0 *os.File
	Block1 *os.File

	Procs map[uint]*Proc // mux.go

	HdbDos *HdbDosSession

	Cleanups []func()

	InitGimeRegs  []byte
	BasicGimeRegs []byte

	T T.TextScreen

	// Env     map[string]string // for whatever
	// User    *User             // logged in user
	// Card *Card // Current card.
}

func (ses *Session) String() string {
	return Format("Session(%d)", ses.ID)
}

// RunCards() is called with "go" to manage a connection session.
var nextSerial int = 1000
var nextSerialMutex sync.Mutex

func GetSerial() int {
	nextSerialMutex.Lock()
	defer nextSerialMutex.Unlock()
	nextSerial++
	return nextSerial
}

func NewSession(conn net.Conn, hostname string) *Session {
	id := GetSerial()

	ses := &Session{
		ID:       id,
		Conn:     conn,
		Procs:    make(map[uint]*Proc), // mux.go
		Hostname: hostname,
	}
	// ses.IScreen = &AxScreen{ses}
	ses.IScreen = NewTextScreen(ses, 32, 16, 0x0400)
	ses.LineBuf = &LineBuf{ses}

	return ses
}

var Cards = make(map[int]*Card)

type Card struct {
	Num  int
	Name string
	Moms []int
	Kids map[int]*Card

	Text     string
	Template *template.Template

	ReconnectPort int
	Launch        string
	Block0        string
	Block1        string
}

func Add(num int, parent int, name string, tc *Card) {
	tc.Num = num
	tc.Name = name
	tc.Moms = append(tc.Moms, parent)
	tc.Kids = make(map[int]*Card)

	Cards[num] = tc
	if num > 0 {
		for _, mom := range tc.Moms {
			parent, ok := Cards[mom]
			if !ok {
				log.Panicf("Add Card #%d=%q: cannot find mom %d", num, name, mom)
			}
			parent.Kids[num] = tc
		}
	}

	tc.Template = template.Must(template.New(Str(tc.Num)).Parse(tc.Text))
}

func DuplicateFileToTemp(filename string, startup string) *os.File {
	r, err := os.Open(filename)
	Check(err)
	tmp, err := os.CreateTemp("/tmp", "tmp.waiter.*.tmp")
	Check(err)
	_, err = io.Copy(tmp, r)
	Check(err)

	err = r.Close()
	Check(err)

	log.Printf("Duplicated %q => %q", filename, tmp.Name())
	if startup != "" {
		shell := Format("set -x; echo '%s' | os9 copy -l -r /dev/stdin '%s,startup' ", startup, tmp.Name())
		cmd := exec.Command("/bin/sh", "-c", shell)
		err := cmd.Run()
		if err != nil {
			log.Panicf("Installing startup failed: %q => %v", shell, err)
		}
	}

	err = os.Remove(tmp.Name())
	Check(err)
	return tmp
}

func NonduplicatingDuplicateFileToTemp(filename string) *os.File {
	r, err := os.OpenFile(filename, os.O_RDWR, 0777)
	Check(err)
	return r
}

func Bold(s string) string {
	var bb bytes.Buffer
	for _, r := range s {
		bb.WriteByte(byte(r))
		bb.WriteByte(1)
	}
	return bb.String()
}

func BoldInt(x int) string {
	return Bold(fmt.Sprintf("%d", x))
}

type NumStr struct {
	Num int
	Str string
}
type NumStrSlice []NumStr

func (o NumStrSlice) Len() int           { return len(o) }
func (o NumStrSlice) Less(i, j int) bool { return o[i].Num < o[j].Num }
func (o NumStrSlice) Swap(i, j int)      { o[i], o[j] = o[j], o[i] }

func RunCards(ses *Session) (notice string) {
	f := Flip13AfterFlipMark

	defer func() {
		r := recover()
		if r != nil {
			ses.IScreen.PutStr(fmt.Sprintf("\n\n(session RunCards) FATAL ERROR: %v\n", r))
			ses.IScreen.Flush()
			panic(r)
		}
	}()

	current := Cards[0]
	if *ForcePage != 0 {
		current = Cards[*ForcePage]
	}

CARD:
	for {
		log.Printf("== %d == %q ==", current.Num, f(current.Name))
		log.Printf("%#v", *current)
		ses.IScreen.Clear()
		ses.IScreen.PutStr(fmt.Sprintf("== %s == %s ==\n", BoldInt(current.Num), Bold(f(current.Name))))

		// ses.IScreen.PutStr("(*" + current.Text + "*)\n")

		var buf bytes.Buffer
		err := current.Template.Execute(&buf, ses)
		if err != nil {
			log.Printf("Template Error p%d: %v", current.Num, err)
			ses.IScreen.PutStr("Template Error: " + Str(err))
		} else {
			ses.IScreen.PutStr(f(buf.String()))
		}

		if current.Launch != "" {
			ses.IScreen.PutStr("[@] Launch!\n")
		}

		// Sort kids and show them.
		var vec NumStrSlice
		for num, kid := range current.Kids {
			vec = append(vec, NumStr{num, fmt.Sprintf("[%d] %s\n", num, f(kid.Name))})
		}
		sort.Sort(vec)
		for _, ns := range vec {
			ses.IScreen.PutStr(ns.Str)
		}
		ses.IScreen.PutStr("> ")
		ses.IScreen.Flush()

		line := "@"
		if *ForcePage == 0 {
			line = ses.LineBuf.GetLine()
			line = strings.Trim(line, " \t\r\n")
		}

		log.Printf("GetLine => %q", line)
		aNum, err := strconv.Atoi(line)
		if err == nil {
			// It was a number.
			card, ok := Cards[aNum]
			if ok {
				current = card
			} else {
				ses.IScreen.PutStr(fmt.Sprintf("\nError: Not a number: %q", line))
				goto DELAY
			}
		} else if line == "@" {
			if current.ReconnectPort != 0 {
				ReconnectPort(ses, current.ReconnectPort)
				return "Left two goroutines copying"
			}

			if current.Launch == "" {
				ses.IScreen.PutStr("There is no \"@\" command on this page.")
				goto DELAY
			}

			if current.Block0 != "" {
				tail := strings.TrimPrefix(current.Block0, ".")
				// tail := strings.TrimPrefix(current.Block0 + ".dsk", ".")
				diskname := *LEMMINGS_ROOT + "/" + tail
				log.Printf("Block0: %q Name: %q tail: %q diskname: %q", current.Block0, current.Name, tail, diskname)
				ses.IScreen.PutStr(fmt.Sprintf("Block0: %q", tail))
				ses.Block0 = DuplicateFileToTemp(diskname, "")
				log.Printf(".... ses.Block0 = %q", ses.Block0.Name())
			}

			if strings.HasPrefix(current.Launch, "@") {
				name := current.Launch[1:]
				demo, ok := Demos[name]
				if !ok {
					for k := range Demos {
						log.Printf("known demo: %q", k)
					}
					log.Printf("Unknown demo: %q", name)
					ses.IScreen.PutStr(fmt.Sprintf("\nUnknown Demo: %q", name))
					goto DELAY
				}
				demo(ses.Conn)
				return "Demo finished"
			}

			if strings.HasPrefix(current.Launch, "%1.") {
				// media/video1: Must flip.
				log.Printf("Before flip: %q", Flip13(current.Launch))
				flipped := strings.TrimPrefix(current.Launch, "%1.") + "---flip"
				log.Printf("Upload: %q", flipped)
				UploadProgram(ses.Conn, *LEMMINGS_ROOT+"/../../../pizga-media/video1/"+flipped)
				ReadFiveLoop(ses.Conn, ses)
				return "Launch finished"
			}

			tail := strings.TrimPrefix(current.Launch, ".")
			log.Printf("Upload: %q", tail)
			UploadProgram(ses.Conn, *LEMMINGS_ROOT+"/"+tail)
			ReadFiveLoop(ses.Conn, ses)
			return "Launch finished"
		} else {
			// It was something else.
			ses.IScreen.PutStr(fmt.Sprintf("\nError: Not a number: %q", line))
			goto DELAY
		}
		continue CARD
	DELAY:
		ses.IScreen.Flush()
		time.Sleep(3 * time.Second)
	}
	return "RunCards Finish"
}

func ReconnectPort(ses *Session, port int) {
	reconn, err := net.Dial("tcp", Format("127.0.0.1:%d", port))
	if err != nil {
		log.Panicf("ReconnectPort: Cannot net.dial TCP 127.0.0.1:%d", port)
	}

	for p, payload := range ses.Hellos {
		WriteFive(reconn, coms.CMD_HELLO, LenSlice(payload), p)
		_, err := reconn.Write(payload)
		if err != nil {
			log.Panicf("ReconnectPort: error sending hello payload: %v", err)
		}
	}
	WriteFive(reconn, coms.CMD_HELLO, 0, 0)

	// fmt.Fprintf(conn, "GET / HTTP/1.0\r\n\r\n")
	// status, err := bufio.NewReader(conn).ReadString('\n')

	onFinish := make(chan bool, 2)
	go CopyTcp(onFinish, reconn, ses.Conn, "reconn to ses.Conn")
	go CopyTcp(onFinish, ses.Conn, reconn, "ses.Conn to reconn")
	<-onFinish
	<-onFinish
}

func CopyTcp(onFinish chan bool, r, w net.Conn, what string) {
COPY:
	for {
		// Start with 1 byte buffer.  TODO more.
		one := make([]byte, 1)
		n, err := r.Read(one)
		if n == 1 {
			n2, err2 := w.Write(one)
			if n2 != 1 {
				log.Printf("STOP CopyTcp %q on write: %v", what, err2)
				break COPY
			}
			continue COPY
		}

		log.Printf("STOP CopyTcp %q on read: %v", what, err)
		break COPY
	}
	onFinish <- true
}
