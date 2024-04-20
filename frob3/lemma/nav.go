package lemma

import (
	"log"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	"github.com/strickyak/frobio/frob3/lemma/finder"
	T "github.com/strickyak/frobio/frob3/lemma/text"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type Navigator struct {
	t   *T.TextNav
	ds  *finder.DriveSession
	com *coms.Comm
}

// Render

func (nav *Navigator) Render(c finder.Chooser, focus uint) {
	t := nav.t
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
		if row < Len(c.Kids())+3 {
			kid := c.Kids()[row-3]
			kid.Kids()
			var codes string
			/* todo
			if kid.Disk != nil {
				codes = kid.Disk.Codes()
			}
			*/
			t.WriteLine(row, "%3s %-2s %s", kid.Decoration(nav.ds), codes, kid.Name())
		} else {
			t.WriteLine(row, "...")
		}
	}
	t.InvertLine(focus)
}

// NavStep

func (nav *Navigator) NavStep(c finder.Chooser) finder.Chooser {
	t := nav.t
	focus := uint(2)
	for {
		c.Kids()
		Log("NavStep: gonna Render: %v ( %v , %v )", nav, c, focus)
		nav.Render(c, focus)
		t.Push()
		b := t.Inkey()
		log.Printf("NavStep: GOT INKEY %d.", b)
		switch b {
		case 94: // Up arrow
			if focus > 2 {
				focus--
			}
		case 10: // Down arrow
			if focus < t.H()-2 {
				focus++
			}
		case 8: // Left arrow
			if c.Parent != nil {
				return c.Parent()
			}
		case 9, 13: // Right arrow, Enter
			kid := c.KidAtFocus(focus)
			kid.Kids()
			return kid
		case 12: // Clear (exit)
			return nil
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			driveNum := b - '0'
			kid := c.KidAtFocus(focus)
			kid.Kids()
			Log("NavStep: gonna SetDrive: %v ( %v , %v )", nav, driveNum, kid)
			nav.ds.SetDrive(driveNum, kid) // mounter.go
		}
	}
}

// Extra...

func (t *Navigator) Loop() {
	var c finder.Chooser = finder.TopChooser()
	for {
		log.Printf("nav at %q", c.Path())
		c = t.NavStep(c)
		if c == nil {
			break // got Clear; exit.
		}
	}
}

// called by hdbdos HdbDosHijack
func TextChooserShell(com *coms.Comm, ds *finder.DriveSession) {
	t40 := &T.Text40{}

	nav := &Navigator{
		t:   T.NewTextNav(t40, com),
		com: com,
		ds:  ds,
	}

	nav.Loop() // Loop until we return from Hijack.
}
