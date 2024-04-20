package lib

import (
	"fmt"
	"io"
	"log"
	//"os"
	//PFP "path/filepath"

	//"github.com/strickyak/frobio/frob3/lemma/rsdos"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

// Struct
type TextNav struct {
	screen TextScreen
	Ses    *Session
}

// W, H
func (t *TextNav) W() uint { return t.screen.W() }
func (t *TextNav) H() uint { return t.screen.H() }

// Push
func (t *TextNav) Push() {
	t.screen.Push(t.Ses)
}

// InvertChar
func (t *TextNav) InvertChar(x, y uint) {
	t.screen.InvertChar(x, y)
}

// InvertLine
func (t *TextNav) InvertLine(y uint) {
	w := t.screen.W()
	for i := uint(0); i < w; i++ {
		t.InvertChar(i, y)
	}
}

// Put

func (t *TextNav) Put(x, y uint, ascii byte, fl TextFlags) {
	t.screen.Put(x, y, ascii, fl)
}

// Get

func (t *TextNav) Get(x, y uint) (ascii byte, fl TextFlags) {
	ascii, fl = t.screen.Get(x, y)
	return
}

// WriteLine
func (t *TextNav) WriteLine(y uint, format string, args ...any) {
	w, h := t.screen.W(), t.screen.H()
	AssertLT(y, h)
	s := fmt.Sprintf(format, args...)
	if Slen(s) > w {
		s = s[:w] // Trim to w bytes
	}
	for x := uint(0); x < Slen(s); x++ {
		t.screen.Put(x, y, s[x], 0)
	}
	for x := Slen(s); x < w; x++ {
		t.screen.Put(x, y, ' ', 0)
	}
}

// Render

func (t *TextNav) Render(c *Chooser, focus uint) {
	w, h := t.W(), t.H()
	for row := uint(0); row < h; row++ {
		for col := uint(0); col < w; col++ {
			t.Put(col, row, ' ', 0)
		}
	}
	path := c.Path()
	t.WriteLine(1, "(%s)", path)
	if c.Parent == nil {
		t.WriteLine(2, "/ (this is the top)")
	} else {
		t.WriteLine(2, "/ (parent directory)")
	}
	for row := uint(3); row < h-1; row++ {
		if row < Len(c.Kids)+3 {
			kid := c.Kids[row-3]
			kid.FillChooser()
			var codes string
			if kid.Disk != nil {
				codes = kid.Disk.Codes()
			}
			t.WriteLine(row, "%c %-2s %s", Decoration(t.Ses, kid), codes, kid.Name)
		} else {
			t.WriteLine(row, "...")
		}
	}
	t.InvertLine(focus)
}

// Inkey

func (t *TextNav) Inkey() byte {
	WriteQuint(t.Ses.Conn, CMD_INKEY, 0, nil) // request inkey

	var q Quint
	Value(io.ReadFull(t.Ses.Conn, q[:]))
	switch q.Command() {
	case CMD_INKEY:
		return byte(q.P())
	default:
		log.Panicf("TextNav.Inkey: Unexpected quint: %02x", q)
	}
	panic(0)
}

// NavChoice

func (t *TextNav) NavChoice(c *Chooser) *Chooser {
	focus := uint(2)
	for {
		c.FillChooser()
		t.Render(c, focus)
		t.Push()
		b := t.Inkey()
		log.Printf("NavChoice: GOT INKEY %d.", b)
		switch b {
		case 94: // Up arrow
			if focus > 2 {
				focus--
			}
		case 10: // Down arrow
			if focus < t.screen.H()-2 {
				focus++
			}
		case 8: // Left arrow
			if c.Parent != nil {
				return c.Parent
			}
		case 9, 13: // Right arrow, Enter
			kid := AtFocus(c, focus)
			kid.FillChooser()
			return kid
		case 12: // Clear (exit)
			return nil
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			driveNum := b - '0'
			kid := AtFocus(c, focus)
			kid.FillChooser()
			t.Ses.HdbDos.SetDrive(driveNum, kid)
		}
	}
}

// Extra...

func (t *TextNav) Loop() {
	c := TopChooser()
	for {
		c.FillChooser()
		log.Printf("nav at %q", c.Path())
		c = t.NavChoice(c)
		if c == nil {
			break // got Clear; exit.
		}
	}
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
	Push(Ses *Session)
}

func Decoration(ses *Session, c *Chooser) byte {
	h := ses.HdbDos
	if c.IsDir {
		return '/'
	}
	floppySized := (c.Size == FloppySize35 || c.Size == FloppySize40 || c.Size == FloppySize80)
	if !floppySized {
		return '-'
	}
	path := c.Path()
	for d := byte(0); d < NumDrives; d++ {
		if h.Drives[d] != nil && h.Drives[d].Path == path {
			return '0' + d
		}
	}
	switch c.Size {
	case FloppySize35:
		return '=' // not mounted
	case FloppySize40:
		return '+' // not mounted
	case FloppySize80:
		return '#' // not mounted
	}
	panic(c.Size)
}
