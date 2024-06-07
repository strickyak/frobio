package text

import (
	//"fmt"
	//"io"
	"log"
	//"os"
	//PFP "path/filepath"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

// Struct
type TextVDG struct {
	Screen [32 * 16]byte
	Com    *coms.Comm
}

func NewTextVDG(com *coms.Comm) *TextVDG {
	t := &TextVDG{
		Com: com,
	}
	for i := range t.Screen {
		t.Screen[i] = 32 // space  (green on black)
	}
	return t
}

// SetColor cannot really be supported.
func (t *TextVDG) SetColor(byte) byte { return 0 }

// W, H
func (t *TextVDG) IsVDG() bool      { return true }
func (t *TextVDG) W() uint          { return 32 }
func (t *TextVDG) H() uint          { return 16 }
func (t *TextVDG) Comm() *coms.Comm { return t.Com }

// Flush
func (t *TextVDG) Flush() {
	log.Printf("Flushing % 3x", t.Screen[:])
	t.Com.WriteQuintAndPayload(coms.CMD_POKE, 0x400, t.Screen[:])
}

// InvertChar
func (t *TextVDG) InvertChar(x, y uint) {
	if 32 <= x || 16 <= y {
		return
	}
	t.Screen[y*32+x] |= 0x40
}

// Put

func (t *TextVDG) Put(x, y uint, ascii byte, fl TextFlags) {
	if 32 <= x || 16 <= y {
		return
	}
	t.Screen[y*32+x] = VDGDisplayCode(ascii)
}

// Get

func (t *TextVDG) Get(x, y uint) (ascii byte, fl TextFlags) {
	if 32 <= x || 16 <= y {
		return 32, 0
	}
	ascii = t.Screen[y*32+x]
	ascii, fl = 63&ascii, Cond((x&0x40) != 0, InverseFlag, 0)
	if ascii < 32 {
		ascii += 64
	}
	return
}

// Save, Restore.
func (t *TextVDG) Save() any {
	return *t
}

func (t *TextVDG) Restore(a any) {
	*t = a.(TextVDG)
}

const VDGYellowBox = 0x9F
const VDGBlueBox = 0xAF
const VDGWhiteBox = 0xCF
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
		return b // VDGOrangeBox // Hyper-ASCII
	}
}
