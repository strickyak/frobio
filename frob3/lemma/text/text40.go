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
type Text40 struct {
	Screen [2 * 40 * 24]byte
}

// W, H
func (t *Text40) W() uint { return 40 }
func (t *Text40) H() uint { return 24 }

// Push
func (t *Text40) Push(ses *Session) {
	WriteQuint(ses.Conn, CMD_POKE, 0x2000, t.Screen[:])
}

// InvertChar
func (t *Text40) InvertChar(x, y uint) {
	j := y*80 + x*2
	bg := 7 & t.Screen[j+1]        // old bg
	fg := 7 & (t.Screen[j+1] >> 3) // old fg
	c := FgBg(bg, fg)
	t.Screen[j+1] = c
}

// Put

func (t *Text40) Put(x, y uint, ascii byte, fl TextFlags) {
	t.Screen[y*80+2*x] = ascii
	t.Screen[y*80+2*x+1] = FgBg(Yellow, Black)
}

// Get

func (t *Text40) Get(x, y uint) (ascii byte, fl TextFlags) {
	ascii = t.Screen[y*80+2*x]
	fl = Cond((t.Screen[y*80+2*x+1] == FgBg(Yellow, Black)), 0, InverseFlag)
	return
}

// Render

// Inkey

// NavChoice

// Extra...

func FgBg(fg, bg byte) byte {
	return ((fg & 7) << 3) | (bg & 7)
}

func GimeDisplayCode(b byte) byte {
	switch {
	case b == 0x5E: // ascii hat
		return 0x60 // coco3 hat
	case b == 0x5F: // ascii under
		return 0x7F // coco3 under
	case b == 0x60: // ascii hat
		return 0x1E // coco3 degrees
	case b > 127:
		return 0x1F // forte 'f'
	default:
		return b
	}
}
