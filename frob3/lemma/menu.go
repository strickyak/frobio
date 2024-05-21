package lemma

import (
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
	},
}

type Menu struct {
	Name   string // first char is shortcut
	Items  []*Menu
	Action MenuAction
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

func (m *Menu) Shortcut() byte {
	return Up(m.Name[0])
}

func (m *Menu) Lines() (lines []string) {
	for _, it := range m.Items {
		lines = append(lines, it.Name)
	}
	return
}

const BorderChar = '#'

func RecolorMenu(t T.TextScreen, group *Menu, chosen *Menu) {
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
		x += LenStr(m.Name) + 1
	}
	w := t.W()
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
			Action: nil,
		},
		&Menu{
			Name:   "Hex View",
			Action: nil,
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
			Name:   "0",
			Action: nil,
		},
		&Menu{
			Name:   "1",
			Action: nil,
		},
		&Menu{
			Name:   "2",
			Action: nil,
		},
		&Menu{
			Name:   "3",
			Action: nil,
		},
		&Menu{
			Name:   "4",
			Action: nil,
		},
		&Menu{
			Name:   "Unmount All",
			Action: nil,
		},
	},
}
