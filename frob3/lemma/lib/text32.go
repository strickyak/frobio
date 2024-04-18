package lib

import (
	//"fmt"
	//"io"
	//"log"
	//"os"
	//PFP "path/filepath"

	//"github.com/strickyak/frobio/frob3/lemma/rsdos"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

// Struct
type TextVDG struct {
	Screen [32 * 16]byte
}

// W, H
func (t *TextVDG) W() uint { return 32 }
func (t *TextVDG) H() uint { return 16 }

// Push
func (t *TextVDG) Push(ses *Session) {
	WriteQuint(ses.Conn, CMD_POKE, 0x400, t.Screen[:])
}

// InvertChar
func (t *TextVDG) InvertChar(x, y uint) {
	t.Screen[y*32+x] |= 0x40
}

// Put

func (t *TextVDG) Put(x, y uint, ascii byte, fl TextFlags) {
	t.Screen[y*32+x] = VDGDisplayCode(ascii)
}

// Get

func (t *TextVDG) Get(x, y uint) (ascii byte, fl TextFlags) {
	ascii = t.Screen[y*32+x]
	ascii, fl = 63&ascii, Cond((x&0x40) != 0, InverseFlag, 0)
	if ascii < 32 {
		ascii += 64
	}
	return
}

// Render

// Inkey

// NavChoice

// Extra...

const VDGMagentaBox = 0xEF
const VDGOrangeBox = 0xFF

func VDGDisplayCode(b byte) byte {
	switch {
	case b < 32:
		return VDGMagentaBox // control char
	case b < 64:
		return b
	case b < 96:
		return b - 64
	case b < 128:
		return b - 96
	default:
		return VDGOrangeBox // Hyper-ASCII
	}
	panic(0)
}
