package text

import (
	"fmt"
	"log"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

// Struct
type TextNav struct {
	glass TextScreen
	com   *coms.Comm
}

func NewTextNav(glass TextScreen, com *coms.Comm) *TextNav {
	return &TextNav{
		glass: glass,
		com:   com,
	}
}

// W, H
func (t *TextNav) W() uint { return t.glass.W() }
func (t *TextNav) H() uint { return t.glass.H() }

// Push
func (t *TextNav) Push() {
	t.glass.Push(t.com)
}

// InvertChar
func (t *TextNav) InvertChar(x, y uint) {
	t.glass.InvertChar(x, y)
}

// InvertLine
func (t *TextNav) InvertLine(y uint) {
	w := t.glass.W()
	for i := uint(0); i < w; i++ {
		t.InvertChar(i, y)
	}
}

// Put

func (t *TextNav) Put(x, y uint, ascii byte, fl TextFlags) {
	t.glass.Put(x, y, ascii, fl)
}

// Get

func (t *TextNav) Get(x, y uint) (ascii byte, fl TextFlags) {
	ascii, fl = t.glass.Get(x, y)
	return
}

// WriteLine
func (t *TextNav) WriteLine(y uint, format string, args ...any) {
	w, h := t.glass.W(), t.glass.H()
	AssertLT(y, h)
	s := fmt.Sprintf(format, args...)
	if Slen(s) > w {
		s = s[:w] // Trim to w bytes
	}
	for x := uint(0); x < Slen(s); x++ {
		t.glass.Put(x, y, s[x], 0)
	}
	for x := Slen(s); x < w; x++ {
		t.glass.Put(x, y, ' ', 0)
	}
}

// Inkey

func (t *TextNav) Inkey() byte {
	t.com.WriteQuint(coms.CMD_GETCHAR, 0, nil) // request inkey

	var q coms.Quint
	t.com.ReadFull(q[:])
	switch q.Command() {
	case coms.CMD_GETCHAR:
		return byte(q.P())
	default:
		log.Panicf("TextNav.Inkey: Unexpected quint: %02x", q)
	}
	panic(0)
}

type TextFlags uint

const (
	InverseFlag TextFlags = 1 << iota
)

type TextScreen interface {
	W() uint
	H() uint
	Put(x, y uint, ch byte, fl TextFlags)
	Get(x, y uint) (ch byte, fl TextFlags)
	InvertChar(x, y uint)
	Push(com *coms.Comm)
}
