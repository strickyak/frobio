package finder

import (
	"github.com/strickyak/frobio/frob3/lemma/menu"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type MenuChooser struct {
	NamedChooser

	Group *menu.MenuGroup
}

func (uc *MenuChooser) String() string {
	return Format("{MC(%s)%d}", uc.name, len(uc.kids))
}

func (uc *MenuChooser) Order() int      { return uc.order }
func (uc *MenuChooser) Name() string    { return uc.name }
func (uc *MenuChooser) Parent() Chooser { return uc.parent }
