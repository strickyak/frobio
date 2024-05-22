package text

import (
	//"fmt"
	//"io"
	//"log"
	//"os"
	//PFP "path/filepath"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type Text40Guts [2 * 40 * 24]byte

// Struct
type Text40 struct {
	Screen Text40Guts
	Color  byte
	Com    *coms.Comm
}

func NewText40(com *coms.Comm) *Text40 {
	return &Text40{
		Color: SimpleYellow,
		Com:   com,
	}
}

func (t *Text40) SetColor(c byte) byte {
	z := t.Color
	t.Color = c
	return z
}

// Save, Restore.
func (t *Text40) Save() any {
	return *t
}

func (t *Text40) Restore(a any) {
	*t = a.(Text40)
}

// W, H
func (t *Text40) IsVDG() bool { return false }
func (t *Text40) W() uint          { return 40 }
func (t *Text40) H() uint          { return 24 }
func (t *Text40) Comm() *coms.Comm { return t.Com }

// Flush
func (t *Text40) Flush() {
	t.Com.WriteQuint(coms.CMD_POKE, 0x2000, t.Screen[:])
}

// InvertChar
func (t *Text40) InvertChar(x, y uint) {
	j := y*80 + x*2
	bg := 7 & t.Screen[j+1]        // old bg
	fg := 7 & (t.Screen[j+1] >> 3) // old fg
	t.Screen[j+1] = FgBg(bg, fg)
}

// Put

func (t *Text40) Put(x, y uint, ascii byte, fl TextFlags) {
	if 40 <= x || 24 <= y {
		return
	}
	t.Screen[y*80+2*x] = ascii
	t.Screen[y*80+2*x+1] = FgBg(t.Color, SimpleBlack)
}

// Get

func (t *Text40) Get(x, y uint) (ascii byte, fl TextFlags) {
	if 40 <= x || 24 <= y {
		return ' ', 0
	}
	ascii = t.Screen[y*80+2*x]
	fl = Cond((t.Screen[y*80+2*x+1] == FgBg(t.Color, SimpleBlack)), 0, InverseFlag)
	return
}
