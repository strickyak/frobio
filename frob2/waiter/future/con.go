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
	return int(uint(a)<<8) | uint(b)
}
func Hi(x int) byte {
	return 255 & byte(uint(x)>>8)
}
func Lo(x int) byte {
	return 255 & byte(uint(x))
}

type Quint [5]byte // Quintabyte commands.

func NewQuint(cmd byte, count int, addr int) Quint {
	var q Quint
	q[0] = cmd
	q[1] = Hi(count)
	q[2] = Lo(count)
	q[3] = Hi(addr)
	q[4] = Lo(addr)
	return q
}

type Network struct {
	Conn *net.Conn
	ses  *Session
}

func (nw *Network) SendQuint(q Quint) {
	cc, err := nw.Conn.Write(q[:])
	if err != nil {
		log.Panicf("SendQuint(%q) got err: %v", nw.ses.ID, err)
	}
	if cc != 5 {
		log.Panicf("SendQuint(%q) short write: %v", nw.ses.ID, cc)
	}
}

func (nw *Network) RecvQuint() Quint {
	var q Quint
	cc, err := nw.Conn.ReadFull(q[:])
	if err != nil {
		log.Panicf("RecvQuint(%q) got err: %v", nw.ses.ID, err)
	}
	if cc != 5 {
		log.Panicf("RecvQuint(%q) short read: %v", nw.ses.ID, cc)
	}
	return q
}

type Screen interface {
	Clear()
	PutChar(ch byte)
	Width() int
	Height() int
	Push()
	Redraw()
}

type Text struct {
	Old  []byte // screen RAM already sent.
	New  []byte // screen RAM not yet sent.
	W, H int    // screen dimensions
	X, Y int    // cursor
	nw   *Network
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

type Session struct {
	Screen  *Screen
	LineBuf []byte
	nw      *Network
	ID      string
}
