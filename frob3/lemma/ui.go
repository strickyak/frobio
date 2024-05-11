package lemma

/*
  -- keeping the text directory
  -- moved finder/chooser in
  -- more finder/* files to go?
*/

import (
	"log"
	"os"
	PFP "path/filepath"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	T "github.com/strickyak/frobio/frob3/lemma/text"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

////////////////// NAV

type Navigator struct {
	t   T.TextScreen
	ds  *DriveSession
	com *coms.Comm
}

func (nav *Navigator) Render(c Model, focus uint) {
	t := nav.t
	w, h := t.W(), t.H()
	for row := uint(0); row < h; row++ {
		for col := uint(0); col < w; col++ {
			t.Put(col, row, ' ', 0)
		}
	}
	path := c.Path()
	T.TextScreenWriteLine(t, 1, "(%s)", path)
	if c.Parent == nil {
		T.TextScreenWriteLine(t, 2, "/ (this is the top)")
	} else {
		T.TextScreenWriteLine(t, 2, "/ (parent directory)")
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
			T.TextScreenWriteLine(t, row, "%3s %-2s %s", kid.Decoration(nav.ds), codes, kid.Name())
		} else {
			T.TextScreenWriteLine(t, row, "...")
		}
	}
	T.TextScreenInvertLine(t, focus)
}

// NavStep

func (nav *Navigator) NavStep(c Model) Model {
	t := nav.t
	focus := uint(2)
	for {
		c.Kids()
		Log("NavStep: gonna Render: %v ( %v , %v )", nav, c, focus)
		nav.Render(c, focus)
		t.Push()
		b := T.GetCharFromKeyboard(t.Comm())
		log.Printf("ZXC NavStep: GOT INKEY %d.", b)
		switch b {
		case 'K', 'k', 94: // Up arrow
			if focus > 2 {
				focus--
			}
		case 'J', 'j', 10: // Down arrow --- COMMENT OUT 10 FOR NOW
			if focus < t.H()-2 {
				focus++
			}
		case 'H', 'h', 8: // Left arrow
			if c.Parent != nil {
				return c.Parent()
			}
		case 'L', 'l', 9, 13: // Right arrow, Enter
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
	var c Model = DotModel()
	for {
		log.Printf("nav at %q", c.Path())
		c = t.NavStep(c)
		if c == nil {
			break // got Clear; exit.
		}
	}
}

// called by hdbdos HdbDosHijack
func TextChooserShell(com *coms.Comm, ds *DriveSession) {
	log.Printf("ZXC TextChooserShell")
	nav := &Navigator{
		t:   T.NewText40(com),
		com: com,
		ds:  ds,
	}

	nav.Loop() // Loop until we return from Hijack.
}

///////////////// VIEW

type Viewer interface {
	Nick() string
	Name() string
	Parent() Viewer
	MenuGroups() []*MenuGroup
	NumPlainLines() int
	NumChoiceLines() int
	PlainLine(i int) string
	Choice(i int) string
}

type MultiView struct {
}

///////////////// SCREEN

type Screener interface {
}

type GimeScreen struct {
}

type Model interface {
	Order() int
	Path() string
	Name() string
	Kids() []Model
	Parent() Model
	Decoration(ds *DriveSession) string
	KidAtFocus(focus uint) Model
	ShowMenuGroups() []*MenuGroup
	Hot() bool // turn ENTER or -> into Action.
	Size() int64
}

func (uc *BaseModel) Order() int    { return uc.order }
func (uc *BaseModel) Name() string  { return uc.name }
func (uc *BaseModel) Parent() Model { return uc.parent }

type BaseModel struct {
	order  int
	name   string
	kids   []Model
	parent Model

	isDir      bool
	size       int64
	kidsFilled bool
}

func (uc *BaseModel) String() string {
	return Format("{UC(%s)%d}", uc.name, len(uc.kids))
}

func (uc *BaseModel) ShowMenuGroups() []*MenuGroup {
	return MenuGroups
}

func (uc *BaseModel) Hot() bool   { return false }
func (uc *BaseModel) Size() int64 { return uc.size }

func (uc *BaseModel) KidAtFocus(focus uint) Model {
	if focus == 2 { // Parent
		if uc.parent != nil {
			return uc.parent
		} else {
			return uc
		}
	}
	return uc.kids[focus-3]
}

func DotModel() *BaseModel {
	return &BaseModel{
		order: 100,
		name:  ".",
	}
}

func (uc *BaseModel) UnixPath() string {
	if *FlagNavRoot == "" {
		log.Panicf("Missing --dos_root flag on server")
	}
	return PFP.Join(*FlagNavRoot, uc.Path())
}
func (uc *BaseModel) Path() string {
	if uc.parent == nil {
		return "."
	} else {
		return PFP.Join(uc.parent.Path(), uc.name)
	}
}

func (uc *BaseModel) Kids() []Model {
	if uc.kidsFilled {
		return uc.kids
	}
	uPath := uc.UnixPath()
	stat := Value(os.Stat(uPath))
	if stat.IsDir() {
		uc.isDir = true
		entries := Value(os.ReadDir(uPath))
		for _, e := range entries {
			name := e.Name()
			if name[0] == '.' {
				continue
			}
			uc.kids = append(uc.kids, &BaseModel{
				order:  100,
				name:   name,
				parent: uc,
			})
		}
	} else {
		// Not a directory.
		uc.size = stat.Size()
	}
	uc.kidsFilled = true
	return uc.kids
}

func (uc *BaseModel) MakeChoice(kid string) Model {
	kids := uc.Kids() // Forces filling kids.

	for _, e := range kids {
		if e.Name() == kid {
			return e
		}
	}

	log.Panicf("Kid %q not found in BaseModel %q", kid, uc.Path())
	panic(0)
}

func (uc *BaseModel) Decoration(ds *DriveSession) string {
	if uc.isDir {
		return "/"
	}
	floppySized := (uc.size == FloppySize35 || uc.size == FloppySize40 || uc.size == FloppySize80)
	if !floppySized {
		return "-"
	}
	path := uc.Path()
	for d := byte(0); d < NumDrives; d++ {
		if ds.drives[d] != nil && ds.drives[d].Path == path {
			return string('0' + d)
		}
	}
	switch uc.size {
	case FloppySize35:
		return "=" // not mounted
	case FloppySize40:
		return "+" // not mounted
	case FloppySize80:
		return "#" // not mounted
	}
	panic(uc.size)
}

const FloppySize35 = 161280
const FloppySize40 = 184320
const FloppySize80 = 368640

var FloppySizes = []int64{FloppySize35, FloppySize40, FloppySize80}

//////////////////////////////////////////////////////////
