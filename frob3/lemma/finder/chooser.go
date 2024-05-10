package finder

//X
//Ximport (
//X	"log"
//X	"os"
//X	PFP "path/filepath"
//X
//X	"github.com/strickyak/frobio/frob3/lemma/menu"
//X	. "github.com/strickyak/frobio/frob3/lemma/util"
//X)
//X
//X/*
//Xtype Region int
//Xconst (
//X	KidsRegion = iota
//X	MenusRegion
//X	MenuRegion
//X	HelpRegion
//X	InfoRegion
//X)
//X
//Xtype Focus struct {
//X	Region Region
//X	Major uint
//X	Minor uint
//X}
//X*/
//X
//Xtype Chooser interface {
//X	Order() int
//X	Path() string
//X	Name() string
//X	Kids() []Chooser
//X	Parent() Chooser
//X	Decoration(ds *DriveSession) string
//X	KidAtFocus(focus uint) Chooser
//X	ShowMenuGroups() []*menu.MenuGroup
//X	Hot() bool // turn ENTER or -> into Action.
//X}
//X
//Xtype NamedChooser struct {
//X	order  int
//X	name   string
//X	kids   []Chooser
//X	parent Chooser
//X}
//X
//Xfunc (uc *NamedChooser) Order() int      { return uc.order }
//Xfunc (uc *NamedChooser) Name() string    { return uc.name }
//Xfunc (uc *NamedChooser) Parent() Chooser { return uc.parent }
//X
//Xtype UnixChooser struct {
//X	NamedChooser
//X	isDir      bool
//X	size       int64
//X	kidsFilled bool
//X}
//X
//Xfunc (uc *UnixChooser) String() string {
//X	return Format("{UC(%s)%d}", uc.name, len(uc.kids))
//X}
//X
//Xfunc (uc *UnixChooser) ShowMenuGroups() []*menu.MenuGroup {
//X	return menu.MenuGroups
//X}
//X
//Xfunc (uc *UnixChooser) Hot() bool { return false }
//X
//Xfunc (uc *UnixChooser) KidAtFocus(focus uint) Chooser {
//X	if focus == 2 { // Parent
//X		if uc.parent != nil {
//X			return uc.parent
//X		} else {
//X			return uc
//X		}
//X	}
//X	return uc.kids[focus-3]
//X}
//X
//Xfunc TopChooser() *UnixChooser {
//X	return &UnixChooser{
//X		NamedChooser: NamedChooser{
//X			order: 100,
//X			name:  ".",
//X		},
//X	}
//X}
//X
//Xfunc (uc *UnixChooser) UnixPath() string {
//X	if *FlagNavRoot == "" {
//X		log.Panicf("Missing --dos_root flag on server")
//X	}
//X	return PFP.Join(*FlagNavRoot, uc.Path())
//X}
//Xfunc (uc *NamedChooser) Path() string {
//X	if uc.parent == nil {
//X		return "."
//X	} else {
//X		return PFP.Join(uc.parent.Path(), uc.name)
//X	}
//X}
//X
//Xfunc (uc *UnixChooser) Kids() []Chooser {
//X	if uc.kidsFilled {
//X		return uc.kids
//X	}
//X	uPath := uc.UnixPath()
//X	stat := Value(os.Stat(uPath))
//X	if stat.IsDir() {
//X		uc.isDir = true
//X		entries := Value(os.ReadDir(uPath))
//X		for _, e := range entries {
//X			name := e.Name()
//X			if name[0] == '.' {
//X				continue
//X			}
//X			uc.kids = append(uc.kids, &UnixChooser{
//X				NamedChooser: NamedChooser{
//X					order:  100,
//X					name:   name,
//X					parent: uc,
//X				},
//X			})
//X		}
//X	} else {
//X		// Not a directory.
//X		uc.size = stat.Size()
//X	}
//X	uc.kidsFilled = true
//X	return uc.kids
//X}
//X
//Xfunc (uc *UnixChooser) MakeChoice(kid string) Chooser {
//X	kids := uc.Kids() // Forces filling kids.
//X
//X	for _, e := range kids {
//X		if e.Name() == kid {
//X			return e
//X		}
//X	}
//X
//X	log.Panicf("Kid %q not found in UnixChooser %q", kid, uc.Path())
//X	panic(0)
//X}
//X
//Xfunc (uc *UnixChooser) Decoration(ds *DriveSession) string {
//X	if uc.isDir {
//X		return "/"
//X	}
//X	floppySized := (uc.size == FloppySize35 || uc.size == FloppySize40 || uc.size == FloppySize80)
//X	if !floppySized {
//X		return "-"
//X	}
//X	path := uc.Path()
//X	for d := byte(0); d < NumDrives; d++ {
//X		if ds.drives[d] != nil && ds.drives[d].Path == path {
//X			return string('0' + d)
//X		}
//X	}
//X	switch uc.size {
//X	case FloppySize35:
//X		return "=" // not mounted
//X	case FloppySize40:
//X		return "+" // not mounted
//X	case FloppySize80:
//X		return "#" // not mounted
//X	}
//X	panic(uc.size)
//X}
//X
//Xconst FloppySize35 = 161280
//Xconst FloppySize40 = 184320
//Xconst FloppySize80 = 368640
//X
//Xvar FloppySizes = []int64{FloppySize35, FloppySize40, FloppySize80}
