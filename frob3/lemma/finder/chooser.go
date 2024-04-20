package finder

import (
	"log"
	"os"
	PFP "path/filepath"

	"github.com/strickyak/frobio/frob3/lemma/menu"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

/*
type Region int
const (
	KidsRegion = iota
	MenusRegion
	MenuRegion
	HelpRegion
	InfoRegion
)

type Focus struct {
	Region Region
	Major uint
	Minor uint
}
*/

type Chooser interface {
	Order() int
	Path() string
	Name() string
	Kids() []Chooser
	Parent() Chooser
	Decoration(ds *DriveSession) string
	KidAtFocus(focus uint) Chooser
	ShowMenuGroups() []*menu.MenuGroup
	Hot() bool // turn ENTER or -> into Action.
}

type NamedChooser struct {
	order  int
	name   string
	kids   []Chooser
	parent Chooser
}

func (uc *NamedChooser) Order() int      { return uc.order }
func (uc *NamedChooser) Name() string    { return uc.name }
func (uc *NamedChooser) Parent() Chooser { return uc.parent }

type UnixChooser struct {
	NamedChooser
	isDir      bool
	size       int64
	kidsFilled bool
}

func (uc *UnixChooser) String() string {
	return Format("{UC(%s)%d}", uc.name, len(uc.kids))
}

func (uc *UnixChooser) ShowMenuGroups() []*menu.MenuGroup {
	return menu.MenuGroups
}

func (uc *UnixChooser) Hot() bool { return false }

func (uc *UnixChooser) KidAtFocus(focus uint) Chooser {
	if focus == 2 { // Parent
		if uc.parent != nil {
			return uc.parent
		} else {
			return uc
		}
	}
	return uc.kids[focus-3]
}

func TopChooser() *UnixChooser {
	return &UnixChooser{
		NamedChooser: NamedChooser{
			order: 100,
			name:  ".",
		},
	}
}

func (uc *UnixChooser) UnixPath() string {
	if *FlagNavRoot == "" {
		log.Panicf("Missing --dos_root flag on server")
	}
	return PFP.Join(*FlagNavRoot, uc.Path())
}
func (uc *NamedChooser) Path() string {
	if uc.parent == nil {
		return "."
	} else {
		return PFP.Join(uc.parent.Path(), uc.name)
	}
}

func (uc *UnixChooser) Kids() []Chooser {
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
			uc.kids = append(uc.kids, &UnixChooser{
				NamedChooser: NamedChooser{
					order:  100,
					name:   name,
					parent: uc,
				},
			})
		}
	} else {
		// Not a directory.
		uc.size = stat.Size()
	}
	uc.kidsFilled = true
	return uc.kids
}

func (uc *UnixChooser) MakeChoice(kid string) Chooser {
	kids := uc.Kids() // Forces filling kids.

	for _, e := range kids {
		if e.Name() == kid {
			return e
		}
	}

	log.Panicf("Kid %q not found in UnixChooser %q", kid, uc.Path())
	panic(0)
}

func (uc *UnixChooser) Decoration(ds *DriveSession) string {
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
