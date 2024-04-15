package lib

import (
	"fmt"
	"io"
	"log"
	"os"
	PFP "path/filepath"

	"github.com/strickyak/frobio/frob3/lemma/rsdos"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type TextVGA struct {
	Screen [32 * 16]byte
	Ses    *Session
}

type Chooser struct {
	Order  int
	Name   string
	Kids   []*Chooser
	Parent *Chooser

	IsDir bool
	Size  int64
	Disk  rsdos.DiskRec

	Line int // in Parent's chooser
}

func (t *TextVGA) Loop() {
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

func AtFocus(c *Chooser, focus int) *Chooser {
	if focus == 2 { // Parent
		if c.Parent != nil {
			return c.Parent
		} else {
			return c
		}
	}
	return c.Kids[focus-3]
}
func (t *TextVGA) NavChoice(c *Chooser) *Chooser {
	focus := 2
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
			if focus < 14 {
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

func (t *TextVGA) Inkey() byte {
	WriteQuint(t.Ses.Conn, CMD_INKEY, 0, nil) // request inkey

	var q Quint
	Value(io.ReadFull(t.Ses.Conn, q[:]))
	switch q.Command() {
	case CMD_INKEY:
		return byte(q.P())
	default:
		log.Panicf("TextVGA.Inkey: Unexpected quint: %02x", q)
	}
	panic(0)
}

func (t *TextVGA) Push() {
	WriteQuint(t.Ses.Conn, CMD_POKE, 0x400, t.Screen[:])
}

func DisplayCode(b byte) byte {
	if b < 32 {
		return ' '
	}
	if b < 64 {
		return b
	}
	if b < 96 {
		return b - 64
	}
	if b < 128 {
		return b - 96
	}
	return b // Semigraphics
}

func (t *TextVGA) InvertLine(y int) {
	for i := 0; i < 32; i++ {
		t.Screen[y*32+i] |= 0x40
	}
}
func (t *TextVGA) WriteLine(y int, format string, args ...any) {
	s := fmt.Sprintf(format, args...)
	if len(s) > 32 {
		s = s[:32] // Trim to 32 bytes
	}
	for i := 0; i < len(s); i++ {
		t.Screen[y*32+i] = DisplayCode(s[i])
	}
	for i := len(s); i < 32; i++ {
		t.Screen[y*32+i] = ' ' // Clear rest of line
	}
}

func (t *TextVGA) Decoration(c *Chooser) byte {
	h := t.Ses.HdbDos
	if c.IsDir {
		return '/'
	}
	if c.Size != FloppySize {
		return '-'
	}
	path := c.Path()
	for d := byte(0); d < NumDrives; d++ {
		if h.Drives[d] != nil && h.Drives[d].Path == path {
			return '0' + d
		}
	}
	return '=' // not mounted
}
func (t *TextVGA) Render(c *Chooser, focus int) {
	for i := 0; i < 32; i++ {
		t.Screen[i] = 0xAC
		t.Screen[15*32+i] = 0xA3
	}
	path := c.Path()
	if len(path) > 32 {
		path = path[len(path)-32:]
	}
	t.WriteLine(1, "(%s)", path)
	if c.Parent == nil {
		t.WriteLine(2, "/ (this is the top)")
	} else {
		t.WriteLine(2, "/ (parent directory)")
	}
	for row := 3; row < 15; row++ {
		if row < len(c.Kids)+3 {
			kid := c.Kids[row-3]
			kid.FillChooser()
			var codes string
			if kid.Disk != nil {
				codes = kid.Disk.Codes()
			}
			t.WriteLine(row, "%c %-2s %s", t.Decoration(kid), codes, kid.Name)
		} else {
			t.WriteLine(row, "...")
		}
	}
	t.InvertLine(focus)
}

func TopChooser() *Chooser {
	return &Chooser{
		Order: 100,
		Name:  ".",
	}
}

func (c *Chooser) UnixPath() string {
	if *FlagPublicRoot == "" {
		log.Panicf("Missing --dos_root flag on server")
	}
	return PFP.Join(*FlagPublicRoot, c.Path())
}
func (c *Chooser) Path() string {
	if c.Parent == nil {
		return "."
	} else {
		return PFP.Join(c.Parent.Path(), c.Name)
	}
}

func (c *Chooser) FillChooser() {
	if c.Kids != nil {
		return
	}
	uPath := c.UnixPath()
	stat := Value(os.Stat(uPath))
	if stat.IsDir() {
		c.IsDir = true
		entries := Value(os.ReadDir(uPath))
		line := 0
		for _, e := range entries {
			name := e.Name()
			if name[0] == '.' {
				continue
			}
			c.Kids = append(c.Kids, &Chooser{
				Order:  100,
				Name:   name,
				Parent: c,
				Line:   line,
			})
			line++
		}
	} else {
		// Not a directory.
		c.Size = stat.Size()
		contents := Value(os.ReadFile(uPath))
		c.Disk = rsdos.DiskParse(contents)
	}
}

func (c *Chooser) MakeChoice(kid string) *Chooser {
	c.FillChooser()

	for _, e := range c.Kids {
		if e.Name == kid {
			return e
		}
	}

	log.Panicf("Kid %q not found in Chooser %q", kid, c.Path())
	panic(0)
}
