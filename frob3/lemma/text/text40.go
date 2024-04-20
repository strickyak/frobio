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

// Struct
type Text40 struct {
	Screen [2 * 40 * 24]byte
}

// W, H
func (t *Text40) W() uint { return 40 }
func (t *Text40) H() uint { return 24 }

// Push
func (t *Text40) Push(com *coms.Comm) {
	com.WriteQuint(coms.CMD_POKE, 0x2000, t.Screen[:])
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
	t.Screen[y*80+2*x] = ascii
	t.Screen[y*80+2*x+1] = FgBg(SimpleYellow, SimpleBlack)
}

// Get

func (t *Text40) Get(x, y uint) (ascii byte, fl TextFlags) {
	ascii = t.Screen[y*80+2*x]
	fl = Cond((t.Screen[y*80+2*x+1] == FgBg(SimpleYellow, SimpleBlack)), 0, InverseFlag)
	return
}

// Render

// Inkey

// NavChoice

// Extra...
