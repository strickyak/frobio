package lemma

import (
	"bytes"
	"fmt"
	"log"
	"os"

	T "github.com/strickyak/frobio/frob3/lemma/text"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var TopMenu = &Menu{
	Name: "", // Top level has empty name.
	Items: []*Menu{
		View_Menu,
		Edit_Menu,
		Quik_Menu,
		Pref_Menu,
		Mount_Menu,
		Book_Menu,
		Go_Menu,
		Help_Menu,
	},
}

type Menu struct {
	Name   string // first char is shortcut
	Items  []*Menu
	Action MenuAction
}

type MenuAction interface {
	// Set(nav *Navigator, mod Model, path string)
	Do(nav *Navigator, mod Model, path string)
	Undo(nav *Navigator, mod Model, path string)
	String(nav *Navigator, mod Model, path string) string
}

/*
type MenuActionBase struct {
	nav  *Navigator
	path string
}

func (o *MenuActionBase) Set(nav *Navigator, mod Model, path string) {
	o.nav = nav
	o.path = path
}
*/

type MenuActionHistory struct {
	History []*MenuAction
}

func (m *Menu) Shortcut() byte {
	return Up(m.Name[0])
}

func (m *Menu) Lines() (lines []string) {
	for _, it := range m.Items {
		lines = append(lines, it.Name)
	}
	return
}

func RecolorMenuBar(t T.TextScreen, group *Menu, chosen *Menu) {
	w := t.W()
	x := uint(0)
	for _, m := range group.Items {
		if m == chosen {
			t.SetColor(T.SimpleCyan)
		} else {
			t.SetColor(T.SimpleWhite)
		}
		n := LenStr(m.Name)
		for i := uint(0); i < n; i++ {
			t.Put(x+i, 0, m.Name[i], 0)
			invert := (chosen == nil) && (i == 0)
			if invert {
				t.InvertChar(x+i, 0)
			}
		}
		if w >= 40 {
			t.Put(x+n, 0, ' ', 0)
			x += LenStr(m.Name) + 1
		} else {
			x += LenStr(m.Name)
		}
	}
	for ; x < w; x++ {
		t.Put(x, 0, ' ', 0)
	}
}

var View_Menu = &Menu{
	Name: "View",
	Items: []*Menu{
		&Menu{
			Name:   "Info",
			Action: nil,
		},
		&Menu{
			Name:   "Text View",
			Action: &ViewTextAction{},
		},
		&Menu{
			Name:   "Hex View",
			Action: &ViewHexAction{},
		},
	},
}

var Edit_Menu = &Menu{
	Name: "Edit",
	Items: []*Menu{
		&Menu{
			Name:   "X  Cut",
			Action: nil,
		},
		&Menu{
			Name:   "C  Copy",
			Action: nil,
		},
		&Menu{
			Name:   "V  Paste",
			Action: nil,
		},
	},
}

var Quik_Menu = &Menu{
	Name: "Quik",
	Items: []*Menu{
		&Menu{
			Name:   "Fast 1.8 MHz Poke",
			Action: nil,
		},
		&Menu{
			Name:   "Native 6309 Mode",
			Action: nil,
		},
		&Menu{
			Name:   "Reboot!",
			Action: nil,
		},
	},
}

var Pref_Menu = &Menu{
	Name: "Pref",
	Items: []*Menu{
		&Menu{
			Name:   "User Settings",
			Action: nil,
		},
		&Menu{
			Name:   "Host Settings",
			Action: nil,
		},
		/*
			&Menu{
				Name:     "Environment Variables",
				Action:   nil,
			},
		*/
	},
}

var Book_Menu = &Menu{
	Name: "Book",
	Items: []*Menu{
		&Menu{
			Name:   "Add Bookmark",
			Action: nil,
		},
		&Menu{
			Name:   "Time-sorted List",
			Action: nil,
		},
		&Menu{
			Name:   "Name-sorted List",
			Action: nil,
		},
	},
}

var Go_Menu = &Menu{
	Name: "Go",
	Items: []*Menu{
		&Menu{
			Name:   "AAAAAAAAAAA",
			Action: nil,
		},
		&Menu{
			Name:   "BBBBBBBBBBBBB",
			Action: nil,
		},
		&Menu{
			Name:   "CCCCCCCCCCCCCC",
			Action: nil,
		},
	},
}

var Mount_Menu = &Menu{
	Name: "Mount",
	Items: []*Menu{
		&Menu{
			Name:   "0 : Mount Drive 0",
			Action: MakeMounter(0),
		},
		&Menu{
			Name:   "1 : Mount Drive 1",
			Action: MakeMounter(1),
		},
		&Menu{
			Name:   "2 : Mount Drive 2",
			Action: MakeMounter(2),
		},
		&Menu{
			Name:   "3 : Mount Drive 3",
			Action: MakeMounter(3),
		},
		&Menu{
			Name:   "4 : Mount Drive 4",
			Action: MakeMounter(4),
		},
		&Menu{
			Name:   "5 : Mount Drive 5",
			Action: MakeMounter(5),
		},
		&Menu{
			Name:   "6 : Mount Drive 6",
			Action: MakeMounter(6),
		},
		&Menu{
			Name:   "7 : Mount Drive 7",
			Action: MakeMounter(7),
		},
		&Menu{
			Name:   "8 : Mount Drive 8",
			Action: MakeMounter(8),
		},
		&Menu{
			Name:   "9 : Mount Drive 9",
			Action: MakeMounter(9),
		},
		&Menu{
			Name:   "Unmount All",
			Action: nil,
		},
	},
}

var Help_Menu = &Menu{
	Name: "?Help",
	Items: []*Menu{
		&Menu{
			Name:   "AAAAAAAAAAA",
			Action: nil,
		},
	},
}

/*
	type MenuAction interface {
		Set(nav *Navigator, mod Model, path string)
		Do()
		Undo()
		String() string
	}

	type MenuActionBase struct {
		nav  *Navigator
		path string
	}
*/

type ViewTextAction struct {
}

func (o *ViewTextAction) Do(nav *Navigator, mod Model, path string) {
	filename := *FlagNavRoot + "/" + path
	contents, err := os.ReadFile(filename)
	if err != nil {
		log.Panicf(Format("ERROR in viewing %q: %v", path, err))
		return
	}
	nav.t.SetColor(T.SimpleWhite)
	// ViewBoxedText(nav.t, contents, T.SimpleWhite)
	ViewFullScreenText(nav.t, contents, T.SimpleWhite)
}
func (o *ViewTextAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *ViewTextAction) String(nav *Navigator, mod Model, path string) string {
	return Format("View file %q as text", path)
}

type ViewHexAction struct {
}

func (o *ViewHexAction) Do(nav *Navigator, mod Model, path string) {
	filename := *FlagNavRoot + "/" + path
	contents, err := os.ReadFile(filename)
	if err != nil {
		log.Panicf(Format("ERROR in viewing %q: %v", path, err))
		return
	}

	var lines []string
	var bb, raw bytes.Buffer
	n := len(contents)
	up := (n + 7) & 0xFFFFFFF8 // up to next multiple of 8
	for i := 0; i < up; i++ {
		var c byte
		if i < n {
			c = contents[i]
		} else {
			c = 0
		}
		if (i & 7) == 0 {
			fmt.Fprintf(&bb, "%04x: ", i)
		}

		if i < n {
			fmt.Fprintf(&bb, "%02x", c)
		} else {
			fmt.Fprintf(&bb, "  ", c)
		}

		if i >= n {
			// beyond the end
			raw.WriteByte(' ')
		} else if 32 <= c && c <= 126 {
			// SPACE and Printable Ascii
			raw.WriteByte(c)
		} else {
			// Control and Non-Ascii
			raw.WriteByte('#')
		}

		if (i & 3) == 3 {
			bb.WriteByte(' ')
		}

		if (i & 7) == 7 {
			bb.WriteString(raw.String())
			lines = append(lines, bb.String())
			raw.Reset()
			bb.Reset()
		}
	}
	lines = append(lines, bb.String())

	nav.t.SetColor(T.SimpleWhite)
	ViewFullScreenLines(nav.t, lines, T.SimpleWhite)
}
func (o *ViewHexAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *ViewHexAction) String(nav *Navigator, mod Model, path string) string {
	return Format("View file %q as text", path)
}

type MountAction struct {
	N byte
}

	// Do(nav *Navigator, mod Model, path string)
	// Undo(nav *Navigator, mod Model, path string)
	// String(nav *Navigator, mod Model, path string) string
func MakeMounter(n byte) MenuAction {
	return &MountAction{n}
}
func (o *MountAction) Do(nav *Navigator, kid Model, path string) {
			kid.Kids()
			Log("NavStep: gonna SetDrive: %v ( %v , %v )", nav, o.N, kid)
			nav.ds.SetDrive(o.N, kid) // mounter.go
}
func (o *MountAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *MountAction) String(nav *Navigator, mod Model, path string) string {
	return Format("Mount file %q as drive %d", path, o.N)
}
