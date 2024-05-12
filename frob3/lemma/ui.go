package lemma

import (
	"log"
	"os"
	PFP "path/filepath"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	T "github.com/strickyak/frobio/frob3/lemma/text"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

const HeaderSize = 2
const FooterSize = 2

const CocoShiftUp = 95    // '_'
const CocoShiftDown = 91  // '['
const CocoShiftLeft = 21  // ^U
const CocoShiftRight = 93 // ']'

const CocoUp = 94
const CocoDown = 10
const CocoLeft = 8
const CocoRight = 9

const CocoClear = 12
const CocoEnter = 13

////////////////// NAV

type Navigator struct {
	t   T.TextScreen
	ds  *DriveSession
	com *coms.Comm
}

func (nav *Navigator) ClearScreen() {
	t := nav.t
	w, h := t.W(), t.H()

	for row := uint(0); row < h; row++ {
		for col := uint(0); col < w; col++ {
			t.Put(col, row, ' ', 0)
		}
	}
}
func (nav *Navigator) Render(mod Model, focus uint) {
	t := nav.t
	_, h := t.W(), t.H()

	navLines := h - HeaderSize - FooterSize

	var page, slot uint
	if focus > 1 {
		page = (focus - 2) / (navLines - 2)
		slot = 2 + (focus-2)%(navLines-2)
	} else {
		page, slot = 0, focus
	}

	nav.ClearScreen()

	path := mod.Path()
	T.TextScreenWriteLine(t, 1, "[ %s ]", Cond(path == ".", "(TOP)", path))

	up := mod.Parent()
	nk := uint(len(mod.Kids()))
	limit := nk + 2 // 2 = 1 parent + 1 this

	for j := uint(0); j < navLines; j++ {
		switch j {
		case 0:
			if up != nil {
				T.TextScreenWriteLine(t, 2, "PARENT DIR (..)")
				// T.TextScreenWriteLine(t, 2, "% 4s% 4s  %s", "UP", "..", up.Path())
			}
		case 1:
			T.TextScreenWriteLine(t, 3, "THIS DIR (.)")
			// T.TextScreenWriteLine(t, 3, "% 4s% 4s  %s", "THIS", ".", Cond(up==nil, "(TOP)", mod.Path()))

		default:
			at := j + (page * (navLines - 2))
			if at < limit {
				kid := mod.KidAtFocus(at)
				kid.Kids()
				// T.TextScreenWriteLine(t, j+HeaderSize, "% 4s% 4s  %s", Format("K%d", at), kid.Decoration(nav.ds), kid.Name())
				T.TextScreenWriteLine(t, j+HeaderSize, "% 4s  %s", kid.Decoration(nav.ds), kid.Name())
			}
		}
	}
	T.TextScreenInvertLine(t, slot+HeaderSize)

	bar := "VIEW EDIT TOOL MOUNT"
	inv := "I....I....I....I...."
	T.TextScreenWriteLine(t, 0, bar)
	for i, c := range inv {
		if c == 'I' {
			t.InvertChar(uint(i), 0)
		}
	}
}

// NavStep

func (nav *Navigator) NavStep(c Model) Model {
	t := nav.t
	navLines := t.H() - HeaderSize - FooterSize
	shiftSkip := navLines * 3 / 4 // how far if SHIFT with Up/Down
	focus := uint(1)
	for {
		nk := uint(len(c.Kids()))
		limit := nk + 2 // 2 = 1 parent + 1 this
		Log("NavStep: gonna Render: nav=%v ( c=%v , f=%v )", nav, c, focus)
		nav.Render(c, focus)
		t.Flush()
		b := T.GetCharFromKeyboard(t.Comm())
		log.Printf("ZXC NavStep: GOT INKEY %d.", b)
		switch b {
		case CocoShiftUp:
			focus -= shiftSkip
			focus = Cond(focus < 0, 0, focus)
		case CocoShiftDown:
			focus += shiftSkip
			focus = Cond(focus >= limit, limit-1, focus)

		case 'W', 'w', 'K', 'k', CocoUp:
			focus -= 1
			focus = Cond(focus < 0, 0, focus)
		case 'S', 's', 'J', 'j', CocoDown:
			focus += 1
			focus = Cond(focus >= limit, limit-1, focus)
		case 'A', 'a', 'H', 'h', CocoLeft:
			if c.Parent != nil {
				return c.Parent()
			} // else do nothing if we are at the top.

		case 'D', 'd', 'L', 'l', CocoRight, CocoEnter:
			kid := c.KidAtFocus(focus) // KidAtFocus understands 0->parent; 1->this; 2..->kids.
			kid.Kids()
			return kid

		case CocoClear: // to exit UI
			return nil

		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			driveNum := b - '0' // MAX 10 DRIVES
			kid := c.KidAtFocus(focus)
			kid.Kids()
			Log("NavStep: gonna SetDrive: %v ( %v , %v )", nav, driveNum, kid)
			nav.ds.SetDrive(driveNum, kid) // mounter.go
		default:
			// fn := nav.FindMenuGroup(b)
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

/*
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
*/

///////////////// SCREEN

/*
type Screener interface {
}

type GimeScreen struct {
}
*/

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

func (mod *BaseModel) Order() int    { return mod.order }
func (mod *BaseModel) Name() string  { return mod.name }
func (mod *BaseModel) Parent() Model { return mod.parent }

type BaseModel struct {
	order  int
	name   string
	kids   []Model
	parent Model

	isDir      bool
	size       int64
	kidsFilled bool
}

func (mod *BaseModel) String() string {
	return Format("{mod(%s)%d}", mod.name, len(mod.kids))
}

func (mod *BaseModel) ShowMenuGroups() []*MenuGroup {
	return MenuGroups
}

func (mod *BaseModel) Hot() bool   { return false }
func (mod *BaseModel) Size() int64 { return mod.size }

// KidAtFocus understands 0->parent; 1->this; 2.. are kids.
func (mod *BaseModel) KidAtFocus(focus uint) Model {
	switch focus {
	case 0: // parent
		if mod.parent != nil {
			return mod.parent
		} else {
			return mod
		}
	case 1: // this
		return mod

	default:
		i := int(focus - 2)
		if 0 <= i && i < len(mod.kids) {
			return mod.kids[focus-2] // 2 = 1 parent + 1 this
		} else {
			log.Printf("EEEEEEEEEEEEEERROR -- KidAtFocus(%d) vs len=%d", i, len(mod.kids))
			return nil
		}
	}
}

func DotModel() *BaseModel {
	return &BaseModel{
		order: 100,
		name:  ".",
	}
}

func (mod *BaseModel) UnixPath() string {
	if *FlagNavRoot == "" {
		log.Panicf("Missing --dos_root flag on server")
	}
	return PFP.Join(*FlagNavRoot, mod.Path())
}
func (mod *BaseModel) Path() string {
	if mod.parent == nil {
		return "."
	} else {
		return PFP.Join(mod.parent.Path(), mod.name)
	}
}

func (mod *BaseModel) Kids() []Model {
	if mod.kidsFilled {
		return mod.kids
	}
	uPath := mod.UnixPath()
	stat := Value(os.Stat(uPath))
	if stat.IsDir() {
		mod.isDir = true
		entries := Value(os.ReadDir(uPath))
		for _, e := range entries {
			name := e.Name()
			if name[0] == '.' {
				continue
			}
			mod.kids = append(mod.kids, &BaseModel{
				order:  100,
				name:   name,
				parent: mod,
			})
		}
	} else {
		// Not a directory.
		mod.size = stat.Size()
	}
	mod.kidsFilled = true
	return mod.kids
}

func (mod *BaseModel) MakeChoice(kid string) Model {
	kids := mod.Kids() // Forces filling kids.

	for _, e := range kids {
		if e.Name() == kid {
			return e
		}
	}

	log.Panicf("Kid %q not found in BaseModel %q", kid, mod.Path())
	panic(0)
}

func (mod *BaseModel) Decoration(ds *DriveSession) string {
	if mod.isDir {
		return "/"
	}
	path := mod.Path()
	for d := byte(0); d < NumDrives; d++ {
		if ds.drives[d] != nil && ds.drives[d].Path == path {
			return string('0' + d) // LIMIT 10 drives!
		}
	}

	switch mod.size {
	case FloppySize35:
		return "=" // not mounted
	case FloppySize40:
		return "+" // not mounted
	case FloppySize80:
		return "#" // not mounted
	default:
		return "-" // Not a floppy
	}
}

const FloppySize35 = 161280
const FloppySize40 = 184320
const FloppySize80 = 368640

var FloppySizes = []int64{FloppySize35, FloppySize40, FloppySize80}

//////////////////////////////////////////////////////////
