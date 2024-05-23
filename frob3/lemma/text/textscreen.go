package text

import (
	"fmt"
	"log"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type TextFlags uint

const (
	InverseFlag TextFlags = 1 << iota
)

type TextScreen interface {
	W() uint
	H() uint
	IsVDG() bool

	Put(x, y uint, ch byte, fl TextFlags)
	Get(x, y uint) (ch byte, fl TextFlags)
	InvertChar(x, y uint)
	Flush()
	Comm() *coms.Comm

	SetColor(byte) byte // might be ignored, if mono.

	Save() any
	Restore(any)
}

// InvertLine
func TextScreenInvertLine(t TextScreen, y uint) {
	w := t.W()
	for i := uint(0); i < w; i++ {
		t.InvertChar(i, y)
	}
}

// WriteLine
func TextScreenWriteLine(t TextScreen, y uint, format string, args ...any) {
	w, h := t.W(), t.H()
	AssertLT(y, h)
	s := fmt.Sprintf(format, args...)
	log.Printf("TextScreenWriteLine w,h=%d,%d. y=%d. s=%q", w, h, y, s)
	if LenStr(s) > w {
		s = s[:w] // Trim to w bytes
	}
	for x := uint(0); x < LenStr(s); x++ {
		t.Put(x, y, s[x], 0)
	}
	for x := LenStr(s); x < w; x++ {
		t.Put(x, y, ' ', 0)
	}
}

// Inkey

// DirectKeyboard should talk to Axiom, but not to Hijack.
func DirectKeyboard(com *coms.Comm) byte {
	com.WriteQuint(coms.CMD_KEYBOARD, 0, nil) // request inkey

	var q coms.Quint
	com.ReadFull(q[:])
	if q.Command() != coms.CMD_KEYBOARD {
		log.Panicf("DirectKeyboard: Unexpected quint: % 3x", q)
	}
	if q.N() != 8 {
		log.Panicf("DirectKeyboard: Unexpected N: % 3x", q)
	}

	var keybits [8]byte
	com.ReadFull(keybits[:])

	log.Printf("DIRECT KEYBOARD: % 3x", keybits[:])
	return 0
}

func GetCharFromKeyboard(com *coms.Comm) byte {
	log.Printf("ZXC Calling GetCharFromKeyboard... send quint GETCHAR....")
	com.WriteQuint(coms.CMD_GETCHAR, 0, nil) // request inkey

	var q coms.Quint
	com.ReadFull(q[:])
	log.Printf("ZXC Got response: %v", q)

	if q.Command() != coms.CMD_GETCHAR {
		log.Panicf("DirectKeyboard: Unexpected quint: % 3x", q)
	}

	z := byte(q.P())
	switch z {
	case 27: // Escape
		z = 3 // becomes BREAK
	}
	return z
}
