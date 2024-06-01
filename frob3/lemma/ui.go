package lemma

import (
	"bytes"
	"fmt"
	"log"
	"os"
	P "path"
	PFP "path/filepath"
	"runtime/debug"

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

type Navigator struct {
	t         T.TextScreen
	ds        *DriveSession
	com       *coms.Comm
	Bookmarks [10]string
	GotoPath  string
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
	log.Printf("RENDERING @@@@@@ %#v ;;; %q ; %d", mod, mod.Path(), focus)

	t := nav.t
	w, h, vdg := t.W(), t.H(), t.IsVDG()
	blueBar := Cond(vdg, byte(0xA3), byte(0x7F))

	navLines := h - HeaderSize - FooterSize

	up := mod.Parent()
	kids := mod.ReKids()
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
		if focus > 1 && focus-2 < LenSlice(kids) {
			chosen = kids[focus-2]
		} else {
			// Reset to the mod, if the focus is bogus.
			chosen = mod
			focus = 1
			kids = mod.Kids()
		}
	}

	nav.ClearScreen()

	nav.t.SetColor(T.SimpleBlue)
	what := Format("%q", chosen.Path())
	if LenStr(what) > w {
		what = what[LenStr(what)-w:]
	}
	pad := (w - LenStr(what)) / 2
	var bb bytes.Buffer
	for i := uint(0); i < pad; i++ {
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
				T.TextScreenWriteLine(t, 2, "Parent (%s)", up.Name())
			}
		case 1:
			T.TextScreenWriteLine(t, 3, "Self (%s)", mod.Name())

		default:
			at := j + (page * (navLines - 2))
			if at < limit {
				kid := mod.KidAtFocus(at)
				kid.ReKids()
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
				focus = limit - 1
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

			/*
				case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
					// Currently these mean mount -- but in the future, that will be
					// under the 'M' menu, and these will jump quickly to choice lines.
					driveNum := b - '0' // MAX 10 DRIVES
					kid := c.KidAtFocus(focus)
					kid.Kids()
					Log("NavStep: gonna SetDrive: %v ( %v , %v )", nav, driveNum, kid)
					nav.ds.SetDrive(driveNum, kid) // mounter.go
			*/
		default:
			if 'A' <= b && b <= 'Z' || b == '?' {
				nav.TryMenu(TopMenu, c, focus, b)
				if nav.GotoPath != "" {
					c = NewModel(nav.GotoPath)
					focus = 1 // Focus on Self
					c.ReKids()
				}
			} else {
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

func (nav *Navigator) DoAction(chosen *Menu, c Model, focus uint, boxX, boxY uint) {
	action := chosen.Action // TODO: Should make a copy, or pass nav & path.
	// action.Set(nav, c.KidAtFocus(focus).Path())
	kid := c.KidAtFocus(focus)
	fpath := kid.Path()

	defer func() {
		r := recover()
		if r != nil {
			log.Printf("Caught in Action %q: %v", action, r)
			debug.PrintStack()
			ErrorAlertChop(nav.t, Format("ERROR: %v", r))
		}
	}()

	/*
		confirmation := []string{
			"",
			Format("Action: %q", action.String(nav, kid, fpath)),
			"",
			"Hit ENTER to confirm,",
			"or BREAK to cancel.",
			"",
		}
	*/
	confirmation := []string{""}

	confirmation = append(confirmation,
		Chop(nav.t, Format("Action: %q", action.String(nav, kid, fpath)))...)

	confirmation = append(confirmation,
		"",
		"Hit ENTER to confirm,",
		"or BREAK to cancel.",
		"",
	)

	DrawBoxed(nav.t, boxX, boxY, confirmation, false)
	nav.t.Flush()

	log.Printf("Starting: %q", action)
	nav.GotoPath = ""
	action.Do(nav, kid, fpath)
	log.Printf("Finished: %q", action)
	/*
		for {
			ch := Up(T.GetCharFromKeyboard(nav.t.Comm()))
			switch ch {
			case 'N', 3, 32:
				return
			case 'Y', 10, 13:
				// TODO: call action
				log.Printf("Starting: %q", action)
				action.Do(nav, kid, fpath)
				log.Printf("Finished: %q", action)
				return
			}
		}
	*/
}

// Returns true if the input char ch is consumed.
// If returns false, you can look elsewhere for a handler.
func (nav *Navigator) TryMenu(menu *Menu, c Model, focus uint, ch byte) {
	var boxX, boxY uint = 2, 2 // for walking submenus

	saved := nav.t.Save()
	defer nav.t.Restore(saved) // How necessary is this?  Won't we redraw anyway?

	for {
		var chosen *Menu
		for _, it := range menu.Items {
			if Up(it.Name[0]) == Up(ch) {
				chosen = it
				break
			}
		}
		if chosen == nil {
			ErrorAlert(nav.t, []string{
				Format("'%c' is not a menu option.", ch),
			})
			return
		}

		Log("TryMenu: nav=%v ( c=%v , f=%v ) '%c' -> %q", nav, c, focus, ch, chosen.Name)
		RecolorMenuBar(nav.t, TopMenu, chosen)

		if chosen.Action != nil {
			nav.DoAction(chosen, c, focus, boxX, boxY)
			return
		}

		// No Action, so make sure there is a next menu.
		if chosen.Items == nil {
			ErrorAlert(nav.t, []string{
				"Dead End",
				"",
				"  (should not happen)",
			})
			return
		}

		// Draw the next menu
		nav.t.SetColor(T.SimpleCyan)
		lines := chosen.Lines()
		DrawBoxed(nav.t, boxX, boxY, lines, true)
		nav.t.Flush()

		ch = Up(T.GetCharFromKeyboard(nav.t.Comm()))
		log.Printf("ZXC TryMenu: GOT CHAR %d.", ch)
		// Space cancels, like Break does.
		if ch == CocoBreak || ch == ' ' {
			return
		}

		menu = chosen
		boxX += 2
		boxY += 2

		/*
			var chosen *Menu
			for _, it := range menu.Items {
				if it.Shortcut() == ch2 {
					chosen = it
					break
				}
			}
			if chosen == nil {
				ErrorAlert(nav.t, []string{
					Format("'%c' is not a menu option.", ch2),
				})
				return
			}
		*/

	}
	return
}

func SplitTextAsLines(text []byte) (lines []string) {
	var bb bytes.Buffer
	for _, b := range text {
		switch {
		case b == 10 || b == 13: // newlines
			lines = append(lines, bb.String())
			bb.Reset()
		case ' ' <= b && b <= '~': // printable ASCII chars
			bb.WriteByte(b)
		default:
			fmt.Fprintf(&bb, "{%d}", b)
		}
		if bb.Len() > 150 {
			lines = append(lines, bb.String())
			bb.Reset()
		}
	}
	if bb.Len() > 0 || len(lines) == 0 {
		lines = append(lines, bb.String())
	}
	return
}

func Chop(t T.TextScreen, line string) (lines []string) {
	w := t.W() - 5 // effective width is reduced by \\ and borders.
	for LenStr(line) > w {
		lines = append(lines, line[:w]+"\\")
		line = line[w:]
	}
	if len(line) > 0 || len(lines) == 0 {
		lines = append(lines, line)
	}
	return
}

func ErrorAlertChop(t T.TextScreen, line string) {
	ErrorAlert(t, Chop(t, line))
}

func ViewBoxedText(t T.TextScreen, text []byte, color byte) {
	lines := SplitTextAsLines(text)
	ViewBoxedLines(t, lines, color)
}
func ViewBoxedLines(t T.TextScreen, lines []string, color byte) {
	t.SetColor(color)
	DrawBoxed(t, 0, 6, lines, false)
	t.Flush()
	WaitForBreak(t)
}

func ViewFullScreenText(t T.TextScreen, text []byte, color byte) {
	lines := SplitTextAsLines(text)
	ViewFullScreenLines(t, lines, color)
}
func ViewFullScreenLines(t T.TextScreen, lines []string, color byte) {
	h1 := t.H() - 1
	lines = ContinueLongLines(t, lines)
	/*
		tmp := ContinueLongLines(t, lines)
		lines = []string {
			"-- Hit SPACE for more. --",
			"-- Hit BREAK to abort. --",
			"-----------------------",
		}
		lines = append(lines, tmp...)
	*/

	n := LenSlice(lines)
	t.SetColor(color)
	i := uint(0) // line index (0 based) to start with
LOOP:
	for {
		title := "SPACE for more, BREAK to exit."
		if i > 0 {
			title = Format("=== Page %d ===", 1+i/h1)
		}
		DrawFullScreenLines(t, lines[i:], title)
		t.Flush()

		ch := Up(T.GetCharFromKeyboard(t.Comm()))
		switch ch {
		case CocoBreak, 'Q':
			break LOOP
		case ' ', CocoDown, 'J', 'S':
			if i+h1 < n {
				i += h1
			}
		case CocoUp, 'K', 'W':
			if i >= h1 {
				i -= h1
			}
		}
	}
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
	DrawBoxed(t, 0, 6, lines, false)
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

func ContinueLongLines(t T.TextScreen, lines []string) (result []string) {
	w := t.W()
	for i, line := range lines {
		println(LenStr(line), w, line, 888, i)
		if LenStr(line) == 0 {
			result = append(result, "")
			continue
		}
		for LenStr(line) > w {
			println(LenStr(line), w, line)
			result = append(result, line[:w-1]+"\\")
			line = line[w-1:]
			if LenStr(line) > 0 {
				line = "\\" + line
			}
		}
		if LenStr(line) > 0 {
			result = append(result, line)
		}
	}
	return
}

func DrawFullScreenLines(t T.TextScreen, lines []string, title string) {
	n := LenSlice(lines)
	w, h := t.W(), t.H()
	for y := uint(0); y < h; y++ {
		line := ""
		switch {
		case y == 0:
			line = title
			t.SetColor(T.SimpleYellow)
		case y-1 < n:
			line = lines[y-1]
			t.SetColor(T.SimpleWhite)
		}

		ll := LenStr(line)

		for x := uint(0); x < w; x++ {
			if x >= ll {
				t.Put(x, y, ' ', 0)
			} else {
				c := line[x]
				if 32 <= c && c <= 126 {
					t.Put(x, y, c, 0)
				} else {
					t.Put(x, y, '#', 0)
				}
			}
		}
	}
}

func DrawBoxed(t T.TextScreen, x, y uint, lines []string, invertHeadChar bool) {
	// BorderChar := byte('#')
	vdg := t.IsVDG()
	Blue := byte(0xA0)

	W := Cond(vdg, Blue+0xA, '|')
	E := Cond(vdg, Blue+0x5, '|')
	N := Cond(vdg, Blue+0xC, '-')
	NW := Cond(vdg, N|W, '+')
	NE := Cond(vdg, N|E, '+')
	S := Cond(vdg, Blue+0x3, '-')
	SW := Cond(vdg, S|W, '+')
	SE := Cond(vdg, S|E, '+')

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
		case wid + 3:
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
	var c Model = NewModel("/")
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
	//Order() int
	Path() string
	Name() string
	Kids() []Model
	ReKids() []Model
	Parent() Model
	Decoration(ds *DriveSession) string
	KidAtFocus(focus uint) Model
	ShowMenus() *Menu
	Hot() bool // turn ENTER or -> into Action.
	Size() int64
}

// func (mod *BaseModel) Order() int    { return mod.order }
func (mod *BaseModel) Name() string  { return mod.name }
func (mod *BaseModel) Parent() Model { return mod.parent }

type BaseModel struct {
	// order  int
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

func NewModel(path string) *BaseModel {
	path = P.Clean("/" + path)

	if path == "/" || path == "." {
		z := &BaseModel{
			name: path,
		}
		z.ReKids()
		return z
	}

	z := &BaseModel{
		name:   path,
		parent: NewModel(P.Dir(path)),
	}
	z.ReKids()
	return z
}

func (mod *BaseModel) UnixPath() string {
	if *FlagNavRoot == "" {
		log.Panicf("Missing --dos_root flag on server")
	}
	return PFP.Join(*FlagNavRoot, mod.Path())
}
func (mod *BaseModel) Path() string {
	if mod.parent == nil {
		return "/"
	} else {
		return PFP.Join(mod.parent.Path(), mod.name)
	}
}

func (mod *BaseModel) ReKids() []Model {
	mod.kidsFilled = false
	return mod.Kids()
}

func (mod *BaseModel) Kids() []Model {
	if mod.kidsFilled {
		return mod.kids
	}
	mod.kids = nil
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
				// order:  100,
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
