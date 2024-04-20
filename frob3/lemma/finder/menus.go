package finder

/*
import (
	"github.com/strickyak/frobio/frob3/lemma/menu"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type MenuChooser struct {
	NamedChooser

	Group *menu.MenuGroup
}

type MenuItemChooser struct {
	NamedChooser

	Group *menu.MenuGroup
	Nth int  // which MenuItem
}

func (mc *MenuChooser) String() string {
	return Format("{MC(%s)%d}", mc.name, len(mc.kids))
}

func (mc *MenuChooser) Hot() bool { return false }

func (mc *MenuChooser) ShowMenuGroups() []*menu.MenuGroup {
	return nil
}

func (mc *MenuItemChooser) String() string {
	return Format("{MIC(%s)%d}", mc.name, len(mc.kids))
}

func (mc *MenuItemChooser) Hot() bool { return true }

func (mc *MenuItemChooser) ShowMenuGroups() []*menu.MenuGroup {
	return nil
}

func (mc *MenuChooser) Kids() []Chooser {
	var kids []Chooser

	for nth, item := range mc.Group.Items {
		kids = append ( kids, &MenuItemChooser {
			NamedChooser: NamedChooser {
				order: nth,
				name: item.Name,
			},
		})
	}
	return kids
}

func (mc *MenuChooser) Decoration(ds *DriveSession) string { return "" }
func (mic *MenuItemChooser) Decoration(ds *DriveSession) string {
	return mic.Group.Item[mic.Nth].Shortcut
}

func (mc *MenuChooser) KidAtFocus(focus uint) Chooser {
	return mc.Group.Item[mc.Nth].Shortcut
	return &MenuItemChooser{
		NamedChooser: NamedChooser{
			name:
		},
		Group: mc.Group,
		Nth: 0,
	}
}
func (mic *MenuItemChooser) KidAtFocus(focus uint) Chooser {
	panic(0)
}
*/
