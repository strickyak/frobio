package menu

type MenuGroup struct {
	Name     string
	Shortcut string
	Items    []*MenuItem
}

type MenuItem struct {
	Name     string
	Shortcut string
	Action   MenuAction
}

type MenuAction interface {
	Do()
	Undo()
	String() string
}

type MenuActionHistory struct {
	History []*MenuAction
}

var MenuGroups = []*MenuGroup{
	FileMenuGroup,
	MountMenuGroup,
}

var FileMenuGroup = &MenuGroup{
	Name:     "File",
	Shortcut: "F",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "Cut",
			Shortcut: "X",
			Action:   nil,
		},
		&MenuItem{
			Name:     "Copy",
			Shortcut: "C",
			Action:   nil,
		},
		&MenuItem{
			Name:     "Paste",
			Shortcut: "V",
			Action:   nil,
		},
	},
}

var MountMenuGroup = &MenuGroup{
	Name:     "Mount",
	Shortcut: "M",
	Items: []*MenuItem{
		&MenuItem{
			Name:     "0",
			Shortcut: "0",
			Action:   nil,
		},
		&MenuItem{
			Name:     "1",
			Shortcut: "1",
			Action:   nil,
		},
		&MenuItem{
			Name:     "2",
			Shortcut: "2",
			Action:   nil,
		},
		&MenuItem{
			Name:     "3",
			Shortcut: "3",
			Action:   nil,
		},
		&MenuItem{
			Name:     "4",
			Shortcut: "4",
			Action:   nil,
		},
		&MenuItem{
			Name:     "Unmount All",
			Shortcut: "u",
			Action:   nil,
		},
	},
}
