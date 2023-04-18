package waiter

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"net"
	"strconv"
	"strings"
)

type Number interface {
	~byte | ~rune | ~int | ~uint | ~int64
}

func Assert(b bool) {
	if b {
		log.Panic("Assert Fail")
	}
}

func AssertLT[N Number](a, b N) {
	if a >= b {
		log.Panicf("AssertLT Fail: %v < %v", a, b)
	}
}

func Check(err error) {
	if err != nil {
		log.Panicf("Check Fail: %v", err)
	}
}

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
	//Width() int
	//Height() int
	//Push(*Session)
	//Redraw()
}

type AxScreen struct {
	Ses *Session
}

func (ax *AxScreen) Clear() {
	for i := 0; i < 16; i++ {
		ax.PutChar(13)
	}
}
func (ax *AxScreen) PutChar(ch byte) {
	q := NewQuint(CMD_PUTCHAR, 0, uint(ch))
	_, err := ax.Ses.Conn.Write(q[:])
	Check(err)
}
func (ax *AxScreen) PutStr(s string) {
	for _, r := range s {
		AssertLT(r, 127)
		ax.PutChar(byte(r))
	}
}

/*
type TextScreen struct {
	Old  []byte // screen RAM already sent.
	New  []byte // screen RAM not yet sent.
	W, H int    // screen dimensions
	P    int    // cursor
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
		P:    0,
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
	if ch == 10 || ch == 13 {
		PutChar(' ')
		for (t.P & (t.W - 1)) != 0 {
			PutChar(' ')
		}
		return
	}

	if ' ' <= ch && ch <= '~' {
		t.New[t.P] = ch
		t.P++
		for t.P >= t.W*t.H {
			for i := t.W; i < t.W*t.H; i++ {
				t.New[i-t.W] = t.New[i]
			}
		}
	}
}
*/

type LineBuf struct {
	Ses *Session
}

func (lb *LineBuf) GetLine() string {
	var buf []byte
	for {
		var q Quint
		_, err := io.ReadFull(lb.Ses.Conn, q[:])
		Check(err)
		cmd := q.Command()

		switch cmd {
		case CMD_INKEY:
			ch := byte(q.P())
			switch ch {
			case 10, 13:
				lb.Ses.Screen.PutChar(ch)
				return string(buf)
			case 8:
				if len(buf) > 0 {
					buf = buf[:len(buf)-1] // trim last
					lb.Ses.Screen.PutChar(ch)
				}
			default:
				if ' ' <= ch && ch <= '~' {
					buf = append(buf, ch)
					lb.Ses.Screen.PutChar(ch)
				} else {
					// log.Panicf("LineBuf: weird char: %d", ch)
				}
			} // switch ch
		default:
			log.Panicf("bad packet to GetLine: cmd=%d", cmd)
		} // switch cmd
	}
}

type Session struct {
	ID      string
	Conn    net.Conn
	Screen  Screen
	LineBuf *LineBuf
	// Env     map[string]string // for whatever
	// User    *User             // logged in user
	// Card *Card // Current card.
}

// Run() is called with "go" to manage a connection session.
var nextID int = 1001

func NewSession(conn net.Conn) *Session {
	ses := &Session{
		ID:   fmt.Sprintf("ses%d", nextID),
		Conn: conn,
	}
	ses.Screen = &AxScreen{ses}
	ses.LineBuf = &LineBuf{ses}
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

var Cards = make(map[int]*Card)

type Card struct {
	Num  int
	Name string
	Moms []int
	Kids map[int]*Card

	Text    string
	Actions map[string]*Card
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
}

func init() {
	Add(0, 0, "Home", &Card{
		Text: `Welcome to Lemma.
This is the home card.
You can return here by typing 0.
`,
	})

	Add(10, 0, "CocoIO-Tests", &Card{
		Text: `These are some tests you can try
to stress your CocoIO Card.
`,
	})

	Add(20, 0, "Demos", &Card{})

	Add(30, 0, "Nitros-9", &Card{})
	Add(31, 30, "Level1", &Card{})
	Add(32, 30, "Level2", &Card{})
	Add(33, 30, "EOU", &Card{})
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
	current := Cards[0]

	for {
		ses.Screen.Clear()
		ses.Screen.PutStr(fmt.Sprintf("== %d == %q ==\n", BoldInt(current.Num), Bold(current.Name)))
		ses.Screen.PutStr(current.Text + "\n")
		for num, kid := range current.Kids {
			ses.Screen.PutStr(fmt.Sprintf("[%d] %s\n", num, kid.Name))
		}

		line := ses.LineBuf.GetLine()
		line = strings.Trim(line, " \t\r\n")
		aNum, err := strconv.Atoi(line)
		if err == nil {
			// It was a number.
			card, ok := Cards[aNum]
			if ok {
				current = card
			} else {
				log.Panicf("Unknown card number: %d", aNum)
			}
		} else {
			// It was something else.
			log.Panicf("Not a number: %q", line)
		}
	}
}
