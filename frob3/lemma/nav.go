package lemma

//Ximport (
//X	"log"
//X
//X	"github.com/strickyak/frobio/frob3/lemma/coms"
//X	"github.com/strickyak/frobio/frob3/lemma/finder"
//X	T "github.com/strickyak/frobio/frob3/lemma/text"
//X	. "github.com/strickyak/frobio/frob3/lemma/util"
//X)
//X
//Xtype Navigator struct {
//X	t   *T.TextNav
//X	ds  *finder.DriveSession
//X	com *coms.Comm
//X}
//X
//X// Render
//X
//Xfunc (nav *Navigator) Render(c finder.Chooser, focus uint) {
//X	t := nav.t
//X	w, h := t.W(), t.H()
//X	for row := uint(0); row < h; row++ {
//X		for col := uint(0); col < w; col++ {
//X			t.Put(col, row, ' ', 0)
//X		}
//X	}
//X	path := c.Path()
//X	t.WriteLine(1, "(%s)", path)
//X	if c.Parent == nil {
//X		t.WriteLine(2, "/ (this is the top)")
//X	} else {
//X		t.WriteLine(2, "/ (parent directory)")
//X	}
//X	for row := uint(3); row < h-1; row++ {
//X		if row < Len(c.Kids())+3 {
//X			kid := c.Kids()[row-3]
//X			kid.Kids()
//X			var codes string
//X			/* todo
//X			if kid.Disk != nil {
//X				codes = kid.Disk.Codes()
//X			}
//X			*/
//X			t.WriteLine(row, "%3s %-2s %s", kid.Decoration(nav.ds), codes, kid.Name())
//X		} else {
//X			t.WriteLine(row, "...")
//X		}
//X	}
//X	t.InvertLine(focus)
//X}
//X
//X// NavStep
//X
//Xfunc (nav *Navigator) NavStep(c finder.Chooser) finder.Chooser {
//X	t := nav.t
//X	focus := uint(2)
//X	for {
//X		c.Kids()
//X		Log("NavStep: gonna Render: %v ( %v , %v )", nav, c, focus)
//X		nav.Render(c, focus)
//X		t.Flush()
//X		b := t.Inkey()
//X		log.Printf("NavStep: GOT INKEY %d.", b)
//X		switch b {
//X		case 94: // Up arrow
//X			if focus > 2 {
//X				focus--
//X			}
//X		case 10: // Down arrow
//X			if focus < t.H()-2 {
//X				focus++
//X			}
//X		case 8: // Left arrow
//X			if c.Parent != nil {
//X				return c.Parent()
//X			}
//X		case 9, 13: // Right arrow, Enter
//X			kid := c.KidAtFocus(focus)
//X			kid.Kids()
//X			return kid
//X		case 12: // Clear (exit)
//X			return nil
//X		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
//X			driveNum := b - '0'
//X			kid := c.KidAtFocus(focus)
//X			kid.Kids()
//X			Log("NavStep: gonna SetDrive: %v ( %v , %v )", nav, driveNum, kid)
//X			nav.ds.SetDrive(driveNum, kid) // mounter.go
//X		}
//X	}
//X}
//X
//X// Extra...
//X
//Xfunc (t *Navigator) Loop() {
//X	var c finder.Chooser = finder.TopChooser()
//X	for {
//X		log.Printf("nav at %q", c.Path())
//X		c = t.NavStep(c)
//X		if c == nil {
//X			break // got Clear; exit.
//X		}
//X	}
//X}
//X
//X// called by hdbdos HdbDosHijack
//Xfunc TextChooserShell(com *coms.Comm, ds *finder.DriveSession) {
//X	t40 := &T.Text40{}
//X
//X	nav := &Navigator{
//X		t:   T.NewTextNav(t40, com),
//X		com: com,
//X		ds:  ds,
//X	}
//X
//X	nav.Loop() // Loop until we return from Hijack.
//X}
