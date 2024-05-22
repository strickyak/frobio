package lemma

import (
	"bytes"
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
const CocoBreak = 3

////////////////// NAV

/*
type Location struct {
	// TODO: use this, so Menu can find stuff.
	// Or just give menu the path?
	Path  string
	Page  int
	Focus int
}
*/

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
	w, h, vdg := t.W(), t.H(), t.IsVDG()
	blueBar := Cond(vdg, byte(0xA3), byte(0x7F))

	navLines := h - HeaderSize - FooterSize

	up := mod.Parent()
	kids := mod.Kids()
	nk := LenSlice(kids)
	limit := nk + 2 // 2 = 1 parent + 1 this

	var chosen Model
	var page, slot uint
	if focus == 0 {
		// parent dir
		page, slot = 0, focus
		chosen = up
		if chosen == nil {
			chosen = mod
		}
	} else if focus == 1 {
		// this dir
		page, slot = 0, focus
		chosen = mod
	} else {
		page = (focus - 2) / (navLines - 2)
		slot = 2 + (focus-2)%(navLines-2)
		chosen = kids[focus-2]
	}

	nav.ClearScreen()

	nav.t.SetColor(T.SimpleBlue)
	what := Format("%q", "/" + chosen.Path())
	if LenStr(what) > w {
		what = what[LenStr(what)-w:]
	}
	pad := (w-LenStr(what)) / 2
	var bb bytes.Buffer
	for i:=uint(0); i<pad; i++ {
		bb.WriteByte(blueBar)
	}
	bb.WriteString(what)
	for bb.Len() < int(w) {
		bb.WriteByte(blueBar)
	}
	T.TextScreenWriteLine(t, 1, "%s", bb.String())

	nav.t.SetColor(T.SimpleYellow)
	for j := uint(0); j < navLines; j++ {
		switch j {
		case 0:
			if up != nil {
				T.TextScreenWriteLine(t, 2, "Parent Dir (%s)", up.Name())
			}
		case 1:
			T.TextScreenWriteLine(t, 3, "This Dir (%s)", mod.Name())

		default:
			at := j + (page * (navLines - 2))
			if at < limit {
				kid := mod.KidAtFocus(at)
				kid.Kids()
				T.TextScreenWriteLine(t, j+HeaderSize, "% 4s  %s", kid.Decoration(nav.ds), kid.Name())
			}
		}
	}
	T.TextScreenInvertLine(t, slot+HeaderSize)

	RecolorMenuBar(nav.t, TopMenu, nil) // turns White
	nav.t.SetColor(T.SimpleYellow)
	// bar := "VIEW EDIT QUIK MOUNT PREF BOOK GOTO ?"
	// inv := "I... I... I... I.... I... I... I... I"
	// T.TextScreenWriteLine(t, 0, bar)
	//for i, c := range inv {
	//if c == 'I' {
	//t.InvertChar(uint(i), 0)
	//}
	//}
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
		b := Up(T.GetCharFromKeyboard(t.Comm()))
		log.Printf("ZXC NavStep: GOT INKEY %d.", b)
		switch b {
		case CocoShiftUp:
			if focus > shiftSkip {
				focus -= shiftSkip
			} else {
				focus = 0
			}
		case CocoShiftDown:
			if focus < limit-shiftSkip {
				focus += shiftSkip
			} else {
				focus = limit-1
			}

		// Support WASD and HJKL as well as arrows.
		case 'W', 'K', CocoUp:
			if focus > 0 {
				focus -= 1
			}
		case 'S', 'J', CocoDown:
			if focus < limit-1 {
				focus += 1
			}
		case 'A', 'H', CocoLeft:
			if c.Parent != nil {
				return c.Parent()
			} // else do nothing if we are at the top.

		case 'D', 'L', CocoRight, CocoEnter:
			kid := c.KidAtFocus(focus) // KidAtFocus understands 0->parent; 1->this; 2..->kids.
			kid.Kids()
			return kid

		case CocoClear: // to exit UI
			return nil

		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			// Currently these mean mount -- but in the future, that will be
			// under the 'M' menu, and these will jump quickly to choice lines.
			driveNum := b - '0' // MAX 10 DRIVES
			kid := c.KidAtFocus(focus)
			kid.Kids()
			Log("NavStep: gonna SetDrive: %v ( %v , %v )", nav, driveNum, kid)
			nav.ds.SetDrive(driveNum, kid) // mounter.go

		default:
			ok := nav.TryMenu(c, focus, b)
			if !ok {
				log.Printf("keystroke rejected: %d.", b)
			}
		}
	}
}

func Up(x byte) byte {
	if 'a' <= x && x <= 'z' {
		return x - 32
	}
	return x
}
func (nav *Navigator) TryMenu(c Model, focus uint, ch byte) bool {
	saved := nav.t.Save()
	defer nav.t.Restore(saved)

	var menu *Menu
	for _, m := range TopMenu.Items {
		if Up(m.Name[0]) == Up(ch) {
			menu = m
			break
		}
	}
	if menu == nil {
		return false // did not accept the char
	}

	// We chose this menu menu.
	Log("TryMenu: nav=%v ( c=%v , f=%v ) '%c' -> %q", nav, c, focus, ch, menu.Name)

	RecolorMenuBar(nav.t, TopMenu, menu)
	nav.t.SetColor(T.SimpleCyan)
	lines := menu.Lines()
	DrawBoxed(nav.t, 4, 2, lines, true)
	nav.t.Flush()

	ch2 := Up(T.GetCharFromKeyboard(nav.t.Comm()))
	log.Printf("ZXC TryMenu: GOT CHAR %d.", ch2)

	// Space cancels, like Break does.
	if ch2 == CocoBreak || ch2 == ' ' {
		return true // Cancelling is one way of handling.
	}

	var item *Menu
	for _, it := range menu.Items {
		if it.Shortcut() == ch2 {
			item = it
			break
		}
	}
	if item == nil {
		ErrorAlert(nav.t, []string{
			Format("'%c' is not a menu option.", ch2),
		})
	}

	return (item != nil)
}

func ErrorAlert(t T.TextScreen, lines []string) {
	t.SetColor(T.SimpleMagenta)
	lines = CatSlices(
		[]string{""},
		lines,
		[]string{
			"",
			"Hit BREAK to close.",
			"",
		},
	)
	DrawBoxed(t, 6, 6, lines, false)
	t.Flush()
	WaitForBreak(t)
}

// WaitForBreak also allows SPACE.
func WaitForBreak(t T.TextScreen) {
	for {
		ch3 := Up(T.GetCharFromKeyboard(t.Comm()))
		if ch3 == CocoBreak || ch3 == ' ' {
			break
		}
	}
}

func DrawBoxed(t T.TextScreen, x, y uint, lines []string, invertHeadChar bool) {
	BorderChar := byte('#')
	vdg := t.IsVDG()

	Blue := byte(0xA0)
	W := Cond(vdg, Blue + 0xA, BorderChar)
	E := Cond(vdg, Blue + 0x5, BorderChar)
	N := Cond(vdg, Blue + 0xC, BorderChar)
	NW := Cond(vdg, N|W, BorderChar)
	NE := Cond(vdg, N|E, BorderChar)
	S := Cond(vdg, Blue + 0x3, BorderChar)
	SW := Cond(vdg, S|W, BorderChar)
	SE := Cond(vdg, S|E, BorderChar)

	wid, hei := uint(0), LenSlice(lines)
	for _, s := range lines {
		if LenStr(s) > wid {
			wid = LenStr(s)
		}
	}
	for i := uint(0); i < wid+4; i++ {
	    switch i {
	    case 0:
		t.Put(x+i, y, NW, T.InverseFlag)
		t.Put(x+i, y+1+hei, SW, T.InverseFlag)
	    case wid+3:
		t.Put(x+i, y, NE, T.InverseFlag)
		t.Put(x+i, y+1+hei, SE, T.InverseFlag)
	    default:
		t.Put(x+i, y, N, T.InverseFlag)
		t.Put(x+i, y+1+hei, S, T.InverseFlag)
	    }
	}
	for i := uint(0); i < hei; i++ {
		n := LenStr(lines[i])
		for j := uint(0); j < wid+3; j++ {
			if j < n {
				t.Put(x+j+2, y+i+1, lines[i][j], 0)
			} else {
				t.Put(x+j+2, y+i+1, ' ', 0)
			}
		}
	}

	for i := uint(0); i < hei; i++ {
		t.Put(x+0, y+i+1, W, 0)
		t.Put(x+1, y+i+1, ' ', 0)
		if invertHeadChar {
			t.InvertChar(x+2, y+i+1)
		}
		t.Put(x+2+wid, y+i+1, ' ', 0)
		t.Put(x+3+wid, y+i+1, E, 0)
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
func TextChooserShell(com *coms.Comm, ds *DriveSession, t T.TextScreen) {
	log.Printf("ZXC TextChooserShell")
	nav := &Navigator{
		t:   t,
		com: com,
		ds:  ds,
	}

	nav.Loop() // Loop until we return from Hijack.
}

type Model interface {
	Order() int
	Path() string
	Name() string
	Kids() []Model
	Parent() Model
	Decoration(ds *DriveSession) string
	KidAtFocus(focus uint) Model
	ShowMenus() *Menu
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

func (mod *BaseModel) ShowMenus() *Menu {
	return TopMenu
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
