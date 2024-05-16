package lemma

import (
	T "github.com/strickyak/frobio/frob3/lemma/text"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type Menu struct {
	Name  string // first char is shortcut
	Items []*MenuItem
}

type MenuItem struct {
	Name     string
	Shortcut byte
	Action   MenuAction
}

type MenuAction interface {
	Set(nav *Navigator, path string)
	Do()
	Undo()
	String() string
}

type MenuActionBase struct {
	nav  *Navigator
	path string
}

func (o *MenuActionBase) Set(nav *Navigator, path string) {
	o.nav = nav
	o.path = path
}

type MenuActionHistory struct {
	History []*MenuAction
}

var Menus = []*Menu{
	View_Menu,
	Edit_Menu,
	Quik_Menu,
	Pref_Menu,
	Mount_Menu,
	Book_Menu,
	Go_Menu,
}

func (m *Menu) Lines() (lines []string) {
	for _, it := range m.Items {
		lines = append(lines, Format("%c %s", it.Shortcut, it.Name))
	}
	return
}

const BorderChar = '#'

func RecolorMenu(t T.TextScreen, menus []*Menu, menu *Menu) {
	x := uint(0)
	for _, m := range menus {
		if m == menu {
			t.SetColor(T.SimpleCyan)
		} else {
			t.SetColor(T.SimpleWhite)
		}
		n := LenStr(m.Name)
		for i := uint(0); i < n; i++ {
			t.Put(x+i, 0, m.Name[i], 0)
			invert := (menu == nil) && (i == 0)
			if invert {
				t.InvertChar(x+i, 0)
			}
		}
		x += LenStr(m.Name) + 1
	}
	w := t.W()
	for ; x < w; x++ {
		t.Put(x, 0, ' ', 0)
	}
}

var View_Menu = &Menu{
	Name: "View",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "Info",
			Shortcut: 'I',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Text View",
			Shortcut: 'T',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Hex View",
			Shortcut: 'H',
			Action:   nil,
		},
	},
}

var Edit_Menu = &Menu{
	Name: "Edit",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "Cut",
			Shortcut: 'X',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Copy",
			Shortcut: 'C',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Paste",
			Shortcut: 'V',
			Action:   nil,
		},
	},
}

var Quik_Menu = &Menu{
	Name: "Quik",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "Fast 1.8 MHz Poke",
			Shortcut: 'F',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Native 6309 Mode",
			Shortcut: 'N',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Reboot!",
			Shortcut: 'R',
			Action:   nil,
		},
	},
}

var Pref_Menu = &Menu{
	Name: "Pref",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "User Settings",
			Shortcut: 'U',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Boot Preferences",
			Shortcut: 'B',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Environment Variables",
			Shortcut: 'E',
			Action:   nil,
		},
	},
}

var Book_Menu = &Menu{
	Name: "Book",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "Add Bookmark",
			Shortcut: 'A',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Time-sorted List",
			Shortcut: 'T',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Name-sorted List",
			Shortcut: 'N',
			Action:   nil,
		},
	},
}

var Go_Menu = &Menu{
	Name: "Go",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "AAAAAAAAAAA",
			Shortcut: 'a',
			Action:   nil,
		},
		&MenuItem{
			Name:     "BBBBBBBBBBBBB",
			Shortcut: 'B',
			Action:   nil,
		},
		&MenuItem{
			Name:     "CCCCCCCCCCCCCC",
			Shortcut: 'C',
			Action:   nil,
		},
	},
}

var Mount_Menu = &Menu{
	Name: "Mount",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "0",
			Shortcut: '0',
			Action:   nil,
		},
		&MenuItem{
			Name:     "1",
			Shortcut: '1',
			Action:   nil,
		},
		&MenuItem{
			Name:     "2",
			Shortcut: '2',
			Action:   nil,
		},
		&MenuItem{
			Name:     "3",
			Shortcut: '3',
			Action:   nil,
		},
		&MenuItem{
			Name:     "4",
			Shortcut: '4',
			Action:   nil,
		},
		&MenuItem{
			Name:     "Unmount All",
			Shortcut: 'U',
			Action:   nil,
		},
	},
}
