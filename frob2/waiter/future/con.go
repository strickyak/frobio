package lemma

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"net"
	"strconv"
	"strings"

	. "github.com/strickyak/frobio/frob2/waiter"
)

/*
const (
	POKE = 0
	CALL = 255
)

func HiLo(a, b byte) int {
	return int((uint(a) << 8) | uint(b))
}
func Hi(x int) byte {
	return 255 & byte(uint(x)>>8)
}
func Lo(x int) byte {
	return 255 & byte(uint(x))
}
*/

type Quint [5]byte // Quintabyte commands.

func NewQuint(cmd byte, n uint, p uint) Quint {
	var q Quint
	q[0] = cmd
	q[1] = Hi(n)
	q[2] = Lo(n)
	q[3] = Hi(p)
	q[4] = Lo(p)
	return q
}

func (q Quint) Command() byte {
	return q[0]
}
func (q Quint) N() uint {
	return HiLo(q[1], q[2])
}
func (q Quint) P() uint {
	return HiLo(q[3], q[4])
}

func (ses *Session) SendQuint(q Quint) {
	cc, err := ses.Conn.Write(q[:])
	if err != nil {
		log.Panicf("SendQuint(%q) got err: %v", ses.ID, err)
	}
	if cc != 5 {
		log.Panicf("SendQuint(%q) short write: %v", ses.ID, cc)
	}
}

func (ses *Session) RecvQuint() Quint {
	var q Quint
	cc, err := io.ReadFull(ses.Conn, q[:])
	if err != nil {
		log.Panicf("RecvQuint(%q) got err: %v", ses.ID, err)
	}
	if cc != 5 {
		log.Panicf("RecvQuint(%q) short read: %v", ses.ID, cc)
	}
	return q
}

type Screen interface {
	Clear()
	PutChar(ch byte)
	PutStr(s string)
	Width() int
	Height() int
	Push(*Session)
	Redraw()
}

type TextScreen struct {
	Old  []byte // screen RAM already sent.
	New  []byte // screen RAM not yet sent.
	W, H int    // screen dimensions
	X, Y int    // cursor
	Addr int
}

func NewTextScreen(w, h int, addr int) *TextScreen {
	o := make([]byte, w*h)
	n := make([]byte, w*h)
	for i := 0; i < w*h; i++ {
		o[i] = ' '
		n[i] = ' '
	}

	return &TextScreen{
		Old:  o,
		New:  n,
		W:    w,
		H:    h,
		X:    0,
		Y:    0,
		Addr: addr,
	}
}

func (t *TextScreen) Clear() {
	t.New = make([]byte, t.W*t.H)
	for i := range t.New {
		t.New[i] = 32
	}
}
func (t *TextScreen) PutChar(ch byte) {
	TODO()
}

type LineBuf struct {
	buf []byte
}

type Session struct {
	ID      string
	Conn    net.Conn
	Screen  Screen
	LineBuf *LineBuf
	// Env     map[string]string // for whatever
	// User    *User             // logged in user
	Card Card // Current card.
}

// Run() is called with "go" to manage a connection session.
var nextID int = 1001

func NewSession(conn net.Conn, initialCard Card) *Session {
	ses := &Session{
		ID:      fmt.Sprintf("ses%d", nextID),
		Conn:    conn,
		Screen:  NewTextScreen(32, 16, 0x0400),
		LineBuf: nil,
	}
	nextID++

	/*
		for {
			q := ses.RecvQuint()
			switch q.Command() {
			case CMD_INKEY:
				ses.LineBuf.Inkey(q.P())
			default:
				fmt.Panicf("Unexpected RecvQuint Command: %d", q.Command())
			}
		}
	*/
	return ses
}

type Card interface {
	// Activate(ses *Session)
	Name() string
}

var Cards = make(map[int]Card)

type TCard struct {
	Num  int
	name string
	Moms []int
	Kids map[int]Card

	Text    string
	Actions map[string]Card
}

/*
func (tc *TCard) Activate(ses *Session) {
	ses.Screen.Clear()
	ses.Screen.PutStr(tc.Text)
	for num, kid := range tc.Kids {
		ses.Screen.PutStr(fmt.Sprintf("[%d] %s\n", num, kid.name))
	}
	ses.Screen.Push(ses)

	line := ses.GetLine()
	line = strings.Trim(line, " \t\r\n")
	aNum, err := strconv.Atoi(line)
	if err == nil {
		// It was a number.
		card, ok := Cards[aNum]
		card.Activate(ses)
	} else {
		// It was something else.
		log.Panicf("Not a number: %q", line)
	}
}
*/

type ActionCard struct {
	Func func(ses *Session)
}

func (ac *ActionCard) Activate(ses *Session) {
	ac.Func(ses)
}

var RunNitros9Card = &ActionCard{
	Func: func(ses *Session) {
		ToDo()
	},
}

func Add(num int, parent int, name string, tc *TCard) {
	tc.Num = num
	tc.name = name
	tc.Moms = append(tc.Moms, parent)
	tc.Kids = make(map[int]Card)

	Cards[num] = tc
	if num > 0 {
		for _, mom := range tc.Moms {
			p, ok := Cards[mom]
			if !ok {
				log.Panicf("Add TCard #%d=%q: cannot find mom %d", num, name, mom)
			}
			ptc, ok := p.(*TCard)
			if !ok {
				log.Panicf("Add TCard #%d=%q: mom not a TCard %T :: %v", num, name, p, p)
			}
			ptc.Kids[num] = tc
		}
	}
}

func init() {
	Add(0, 0, "Home", &TCard{
		Text: `Welcome to Lemma.
This is the home card.
You can return here by typing 0.
`,
	})

	Add(10, 0, "CocoIO-Tests", &TCard{
		Text: `These are some tests you can try
to stress your CocoIO Card.
`,
	})

	Add(20, 0, "Demos", &TCard{})

	Add(30, 0, "Nitros-9", &TCard{})
	Add(31, 30, "Level1", &TCard{})
	Add(32, 30, "Level2", &TCard{})
	Add(33, 30, "EOU", &TCard{})
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

func Run(ses *Session) {
	current := Cards[0].(*TCard)

	for {
		ses.Screen.Clear()
		ses.Screen.PutStr(fmt.Sprintf("%d = %q\n\n", BoldInt(current.Num), current.name))
		ses.Screen.PutStr(current.Text + "\n")
		for num, kid := range current.Kids {
			ses.Screen.PutStr(fmt.Sprintf("[%d] %s\n", num, kid.Name()))
		}
		ses.Screen.Push(ses)

		line := ses.GetLine()
		line = strings.Trim(line, " \t\r\n")
		aNum, err := strconv.Atoi(line)
		if err == nil {
			// It was a number.
			card, ok := Cards[aNum]
			if ok {
				tcard, ok := card.(*TCard)
				if ok {
					current = tcard
				}
			} else {
			}
		} else {
			// It was something else.
			log.Panicf("Not a number: %q", line)
		}
	}
}
