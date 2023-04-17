package lemma

import (
	"log"
	"net"
)

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

type Quint [5]byte // Quintabyte commands.

func NewQuint(cmd byte, n int, p int) Quint {
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
func (q Quint) N() int {
	return HiLo(q[1], q[2])
}
func (q Quint) P() int {
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
	cc, err := ses.Conn.ReadFull(q[:])
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

type Text struct {
	Old  []byte // screen RAM already sent.
	New  []byte // screen RAM not yet sent.
	W, H int    // screen dimensions
	X, Y int    // cursor
	Addr int
}

func NewText(w, h int, addr int) *Text {
	o := make([]byte, w*h)
	n := make([]byte, w*h)
	for i := 0; i < w_h; i++ {
		o[i] = ' '
		n[i] = ' '
	}

	return &Text{
		Old:  o,
		New:  n,
		W:    w,
		H:    h,
		X:    0,
		Y:    0,
		Addr: addr,
	}
}

func (t *Text) Clear() {
	t.Screen = make([]byte, t.W*t.H)
}

type LineBuf struct {
	buf []byte
}

type Session struct {
	ID      string
	Conn    *net.Conn
	Screen  *Screen
	LineBuf *LineBuf
	Env     map[string]string // for whatever
	User    *User             // logged in user
	Card    Card              // Current card.
}

func (ses *Session) GoTo(card Card) {
	ses.Card = card
	card.Activate(ses)
}

// Run() is called with "go" to manage a connection session.
var nextID int = 101

func Run(conn *net.Conn, initialCard Card) {
	ses := &Session{
		ID:      fmt.Sprintf("ses%d", nextID),
		Conn:    conn,
		Screen:  NewScreen(),
		LineBuf: nil,
	}
	nextID++

	ses.GoTo(initialCard)

	for {
		q := ses.RecvQuint()
		switch q.Command() {
		case INKEY:
			ses.LineBuf.Inkey(q.P())
		default:
			fmt.Panicf("Unexpected RecvQuint Command: %d", q.Command())
		}
	}
}

type Card interface {
	Activate(ses *Session)
	// Accept(ses *Session, submission string) Card
}

type PromptCard struct {
	Prompt  string
	Actions map[string]*Card
}

func (pc *PromptCard) Activate(ses *Session) {
	ses.Screen.Clear()
	ses.Screen.PutStr(pc.Prompt)
	ses.Screen.Push(ses)
	ses.LineHandler = func() {
		s := string(ses.LineBuf.buf)
		s = strings.TrimSpace(s)
		next, ok := pc.Actions[s]
		if ok {
			next.Activate(ses)
		} else {
			pc.Activate(ses)
		}
	}
}

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

var HomeCard = &Card{
	Prompt: `Welcome to Lemma.
  1) run nitros9
  2) run nyancat
  3) login user
  4) create user
`,
	Actions: map[string]*Card{
		{"1", RunNitros9Card},
		{"2", RunNitros9Card},
		{"3", RunNitros9Card},
		{"4", RunNitros9Card},
	},
}
