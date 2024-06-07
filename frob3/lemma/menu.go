package lemma

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	P "path"

	"github.com/strickyak/frobio/frob3/lemma/coms"
	T "github.com/strickyak/frobio/frob3/lemma/text"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var TopMenu = &Menu{
	Name: "", // Top level has empty name.
	Items: []*Menu{
		View_Menu,
		Edit_Menu,
		Quik_Menu,
		/*
			Pref_Menu,
		*/
		Mount_Menu,
		Book_Menu,
		/*
			Go_Menu,
		*/
		Help_Menu,
	},
}

type Menu struct {
	Name   string // first char is shortcut
	Items  []*Menu
	Action MenuAction
}

type MenuAction interface {
	Do(nav *Navigator, mod Model, path string)
	Undo(nav *Navigator, mod Model, path string)
	String(nav *Navigator, mod Model, path string) string
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
			Action: &ViewInfoAction{},
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
			Name:   "X : Cut",
			Action: &EditAction{'X'},
		},
		&Menu{
			Name:   "C : Copy",
			Action: &EditAction{'C'},
		},
		&Menu{
			Name:   "V : Paste",
			Action: &EditAction{'V'},
		},
	},
}

var Quik_Menu = &Menu{
	Name: "Quik",
	Items: []*Menu{
		&Menu{
			Name: "Speed Poke",
			Items: []*Menu{
				&Menu{Name: "Y : Fast 1.78 MHz", Action: &ModeAction{'S', 'Y'}},
				&Menu{Name: "N : Slow 0.9 MHz", Action: &ModeAction{'S', 'N'}},
				&Menu{Name: "R : ROM fast, rest slow", Action: &ModeAction{'S', 'R'}},
				&Menu{Name: "X : GimeX 2.86 MHz", Action: &ModeAction{'S', 'X'}},
			},
		},
		&Menu{
			Name: "Hitachi 6309 Mode",
			Items: []*Menu{
				&Menu{Name: "Y : 6309 Native Mode", Action: &ModeAction{'H', 'Y'}},
				&Menu{Name: "N : 6809 Compatability Mode", Action: &ModeAction{'H', 'N'}},
			},
		},
		&Menu{
			Name:   "Un-hijack the CLEAR Key",
			Action: &ModeAction{'U', 0},
		},
		&Menu{
			Name:   "Reboot",
			Action: &ModeAction{'R', 0},
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
			Name:  "Set Bookmark",
			Items: SetBookmark_Menu.Items,
		},
		&Menu{
			Name:   "View Bookmarks",
			Action: &ViewBookmarksAction{},
		},
		&Menu{
			Name:   "0 : Goto Bookmark 0",
			Action: MakeGotoBookmark(0),
		},
		&Menu{
			Name:   "1 : Goto Bookmark 1",
			Action: MakeGotoBookmark(1),
		},
		&Menu{
			Name:   "2 : Goto Bookmark 2",
			Action: MakeGotoBookmark(2),
		},
		&Menu{
			Name:   "3 : Goto Bookmark 3",
			Action: MakeGotoBookmark(3),
		},
		&Menu{
			Name:   "4 : Goto Bookmark 4",
			Action: MakeGotoBookmark(4),
		},
		&Menu{
			Name:   "5 : Goto Bookmark 5",
			Action: MakeGotoBookmark(5),
		},
		&Menu{
			Name:   "6 : Goto Bookmark 6",
			Action: MakeGotoBookmark(6),
		},
		&Menu{
			Name:   "7 : Goto Bookmark 7",
			Action: MakeGotoBookmark(7),
		},
		&Menu{
			Name:   "8 : Goto Bookmark 8",
			Action: MakeGotoBookmark(8),
		},
		&Menu{
			Name:   "9 : Goto Bookmark 9",
			Action: MakeGotoBookmark(9),
		},
	},
}

var SetBookmark_Menu = &Menu{
	Items: []*Menu{
		&Menu{
			Name:   "0 : Set Bookmark 0",
			Action: MakeSetBookmark(0),
		},
		&Menu{
			Name:   "1 : Set Bookmark 1",
			Action: MakeSetBookmark(1),
		},
		&Menu{
			Name:   "2 : Set Bookmark 2",
			Action: MakeSetBookmark(2),
		},
		&Menu{
			Name:   "3 : Set Bookmark 3",
			Action: MakeSetBookmark(3),
		},
		&Menu{
			Name:   "4 : Set Bookmark 4",
			Action: MakeSetBookmark(4),
		},
		&Menu{
			Name:   "5 : Set Bookmark 5",
			Action: MakeSetBookmark(5),
		},
		&Menu{
			Name:   "6 : Set Bookmark 6",
			Action: MakeSetBookmark(6),
		},
		&Menu{
			Name:   "7 : Set Bookmark 7",
			Action: MakeSetBookmark(7),
		},
		&Menu{
			Name:   "8 : Set Bookmark 8",
			Action: MakeSetBookmark(8),
		},
		&Menu{
			Name:   "9 : Set Bookmark 9",
			Action: MakeSetBookmark(9),
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
			Name:   "1 : Mount on Drive 1",
			Action: MakeMounter(1),
		},
		&Menu{
			Name:   "2 : Mount on Drive 2",
			Action: MakeMounter(2),
		},
		&Menu{
			Name:   "3 : Mount on Drive 3",
			Action: MakeMounter(3),
		},
		&Menu{
			Name:   "4 : Mount on Drive 4",
			Action: MakeMounter(4),
		},
		&Menu{
			Name:   "5 : Mount on Drive 5",
			Action: MakeMounter(5),
		},
		&Menu{
			Name:   "6 : Mount on Drive 6",
			Action: MakeMounter(6),
		},
		&Menu{
			Name:   "7 : Mount on Drive 7",
			Action: MakeMounter(7),
		},
		&Menu{
			Name:   "8 : Mount on Drive 8",
			Action: MakeMounter(8),
		},
		&Menu{
			Name:   "9 : Mount on Drive 9",
			Action: MakeMounter(9),
		},
		&Menu{
			Name:   "Unmount All",
			Action: &UnmountAllAction{},
		},
		&Menu{
			Name:   "View Mounts",
			Action: &ViewMountsAction{},
		},
	},
}

var Help_Menu = &Menu{
	Name:   "?Help",
	Action: &HelpAction{},
}

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

//////////////////////////////////

type MountAction struct {
	N byte
}

func MakeMounter(n byte) MenuAction {
	return &MountAction{n}
}
func (o *MountAction) Do(nav *Navigator, mod Model, path string) {
	mod.Kids()
	Log("NavStep: gonna SetDrive: %v ( %v , %v )", nav, o.N, mod)
	nav.ds.SetDrive(o.N, mod) // mounter.go
}
func (o *MountAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *MountAction) String(nav *Navigator, mod Model, path string) string {
	return Format("Mount file %q as drive %d", path, o.N)
}

//////////////////////////////////

type GotoBookmarkAction struct {
	N byte
}

func MakeGotoBookmark(n byte) MenuAction {
	return &GotoBookmarkAction{n}
}
func (o *GotoBookmarkAction) Do(nav *Navigator, mod Model, path string) {
	if nav.Bookmarks[o.N] == "" {
		ErrorAlertChop(nav.t, "That bookmark is not set.")
		return
	}
	nav.GotoPath = nav.Bookmarks[o.N]
}
func (o *GotoBookmarkAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *GotoBookmarkAction) String(nav *Navigator, mod Model, path string) string {
	return Format("GotoBookmark %d", o.N)
}

//////////////////////////////////

type SetBookmarkAction struct {
	N byte
}

func MakeSetBookmark(n byte) MenuAction {
	return &SetBookmarkAction{n}
}
func (o *SetBookmarkAction) Do(nav *Navigator, mod Model, path string) {
	nav.Bookmarks[o.N] = path
}
func (o *SetBookmarkAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *SetBookmarkAction) String(nav *Navigator, mod Model, path string) string {
	return Format("SetBookmark %d", o.N)
}

//////////////////////////////////

type UnmountAllAction struct {
}

func (o *UnmountAllAction) Do(nav *Navigator, mod Model, path string) {
	// Currently this is the same body as ds.HdbDosCleanup.
	// But I copied it, in case we add stuff to ds.HdbDosCleanup.
	for driveNum, _ := range nav.ds.drives {
		nav.ds.HdbDosCleanupOneDrive(byte(driveNum))
		nav.ds.drives[driveNum] = nil
	}
}
func (o *UnmountAllAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *UnmountAllAction) String(nav *Navigator, mod Model, path string) string {
	return Format("Unmount All Drives")
}

//////////////////////////////////

type ViewMountsAction struct {
}

func (o *ViewMountsAction) Do(nav *Navigator, mod Model, path string) {
	var bb bytes.Buffer
	for num, drive := range nav.ds.drives {
		// TODO -- if drive == nil || (drive.Path=="" && !drive.Dirty) -- TODO
		if drive == nil {
			fmt.Fprintf(&bb, "Drive %d: %q\n", num, "")
			continue
		}
		dirty := ""
		if drive.Dirty {
			dirty = " [Dirty]"
		}
		fmt.Fprintf(&bb, "Drive %d: %q%s\n", num, drive.Path, dirty)
	}
	ViewFullScreenText(nav.t, bb.Bytes(), T.SimpleWhite)
}
func (o *ViewMountsAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *ViewMountsAction) String(nav *Navigator, mod Model, path string) string {
	return Format("View Mounts")
}

//////////////////////////////////

type ViewBookmarksAction struct {
}

func (o *ViewBookmarksAction) Do(nav *Navigator, mod Model, path string) {
	var bb bytes.Buffer
	for num, bm := range nav.Bookmarks {
		fmt.Fprintf(&bb, "Bookmark %d: %q\n", num, bm)
	}
	ViewFullScreenText(nav.t, bb.Bytes(), T.SimpleWhite)
}
func (o *ViewBookmarksAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *ViewBookmarksAction) String(nav *Navigator, mod Model, path string) string {
	return Format("View Bookmarks")
}

type ViewInfoAction struct {
}

func (o *ViewInfoAction) Do(nav *Navigator, mod Model, path string) {
	stat, err := os.Stat(TruePath(path))
	if err != nil {
		ViewFullScreenText(nav.t, []byte(Format("ERROR: %q: %v", path, err)), T.SimpleWhite)
		return
	}

	const FMT = `
Path: %q
Name: %q
IsDirectory: %v
Size: %d bytes
ModTime: %s
`
	s := Format(FMT, path, stat.Name(), stat.IsDir(), stat.Size(), stat.ModTime().UTC().Format("2006-01-02 15:04:05Z"))

	ViewFullScreenText(nav.t, []byte(s), T.SimpleWhite)
}
func (o *ViewInfoAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *ViewInfoAction) String(nav *Navigator, mod Model, path string) string {
	return Format("View Info about %q", path)
}

//////////////////////////////////

type HelpAction struct {
}

func (o *HelpAction) Do(nav *Navigator, mod Model, path string) {
	const HELP = `
You are in the Pizga Net
navigator.

From the navigator, hit CLEAR
to return to Disk Basic.

In general, use BREAK to
escape from submenus.  Use
arrows to navigate the cloud
filesystem.  Use highlighted
letters for top menus bar.

Mount disk images for basic
with the 'M' menu.  If you
change a public disk, the
changed disk will be copied
to the Temp directory in your
home directory (under /Homes/
and your hostname).

`
	ViewFullScreenText(nav.t, []byte(HELP), T.SimpleWhite)
}
func (o *HelpAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *HelpAction) String(nav *Navigator, mod Model, path string) string {
	return Format("View Bookmarks")
}

/*
	Notes:
		113D01   ldmd #1
		39       rts

	$00F3..$00FF:  13 spare bytes.
*/

func HitachiMode(com *coms.Comm, native bool) {
	var mdValue byte
	if native {
		mdValue = 1
	}

	gist := []byte{0x11, 0x3D, mdValue, 0x39}
	com.PokeRam(0x00F3, gist)
	com.CallAddr(0x00F3)
}

// file:///home/strick/doc/CoCo%20Hardware%20Reference.pdf
const SET_R0_ZERO = 0xFFD6
const SET_R0_ONE = 0xFFD7
const SET_R1_ZERO = 0xFFD8
const SET_R1_ONE = 0xFFD9

func SpeedMode(com *coms.Comm, value byte) {
	switch value {
	case 'X':
		com.PokeRam(SET_R1_ONE, []byte{0xA5}) // 2.86 MHz
		com.PokeRam(SET_R0_ZERO, []byte{0})
	case 'Y':
		com.PokeRam(SET_R1_ONE, []byte{0}) // 1.78 MHZ
		com.PokeRam(SET_R0_ZERO, []byte{0})
	case 'N':
		com.PokeRam(SET_R1_ZERO, []byte{0}) // 0.89 MHz
		com.PokeRam(SET_R0_ZERO, []byte{0})
	case 'R':
		com.PokeRam(SET_R1_ZERO, []byte{0}) // 0.89 MHz but
		com.PokeRam(SET_R0_ZERO, []byte{1}) // 1.78 MHz for ROM
	}
}

func UnHijack(com *coms.Comm) {
	// See coco-shelf/toolshed/cocoroms/coco3.rom.list
	// for what bytes were there before
	// coco-shelf/frobio/frob3/hdbdos/inkey_trap.asm
	// inserted the hijack.
	com.PokeRam(0xA1CB, []byte{0x34, 0x54, 0xCE})
}
func Reboot(com *coms.Comm) {
	bb := Peek2Ram(com.Unwrap(), 0xFFFE, 2)
	addr := (uint(bb[0]) << 8) + uint(bb[1])
	log.Printf("Rebooting by calling [0xFFFE] = $%04x", addr)
	com.CallAddr(addr)
}

type ModeAction struct {
	What  byte
	Value byte
}

func (o *ModeAction) Do(nav *Navigator, mod Model, path string) {
	switch o.What {
	case 'H':
		HitachiMode(nav.com, o.Value == 'Y')
	case 'S':
		SpeedMode(nav.com, o.Value)
	case 'U':
		UnHijack(nav.com)
	case 'R':
		Reboot(nav.com)
	}
}
func (o *ModeAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *ModeAction) String(nav *Navigator, mod Model, path string) string {
	return Format("Mode Switch ('%c', '%v')", o.What, o.Value)
}

type EditAction struct {
	What byte
}

func (o *EditAction) Do(nav *Navigator, mod Model, path string) {
	principals := []string{nav.ds.ses.Hostname}

	if !CanAccess(principals, path, false) {
		ErrorAlertChop(nav.t, Format("User %q does not have permission to read %q", principals[0], path))
		return
	}

	switch o.What {
	case 'X': // X : Cut
		nav.CutPath = path
		ts := Timestamp()
		nav.HoldingPath = P.Join(nav.ds.trash, ts+"-"+P.Base(path))

		err := os.Rename(TruePath(path), TruePath(nav.HoldingPath))
		if err != nil {
			ErrorAlertChop(nav.ds.ses.T, Format("Cannot Cut %q: %v", path, err))
			return
		}

		if CanAccess(principals, path, true) {
			os.Remove(TruePath(path))
		} else {
			ErrorAlertChop(nav.t, Format("User %q does not have permission to delete %q but your copy was made.", principals[0], path))
		}

	case 'C': // C : Copy
		nav.CutPath = path
		ts := Timestamp()
		nav.HoldingPath = P.Join(nav.ds.trash, ts+"-"+P.Base(path))

		err := CopyFile(TruePath(path), TruePath(nav.HoldingPath))
		if err != nil {
			ErrorAlertChop(nav.ds.ses.T, Format("Cannot Copy %q: %v", path, err))
			return
		}

	case 'V': // V : Paste
		var dest string // tentative

		if len(nav.HoldingPath) < 2 {
			ErrorAlertChop(nav.t, "There is nothing to paste.")
			return
		}

		stat, _ := os.Stat(TruePath(path))
		switch {
		case stat == nil: // rare, but ok.
			dest = path
		case stat.IsDir():
			dest = P.Join(path, P.Base(nav.CutPath))
		default:
			dest = path
		}

		if !CanAccess(principals, dest, true) {
			ErrorAlertChop(nav.ds.ses.T, Format("User %q does not have permission to write file %q", principals[0], dest))
			return
		}

		var yes bool
		dstat, _ := os.Stat(TruePath(dest))
		switch {
		case dstat == nil: // nothing there, feel free to write.
			yes = true
		case dstat.IsDir():
			log.Panic("how did it become a directory?")
		default:
			yes = nav.Confirm(Format(`
Do you want to overwrite file %q ?
`, dest))
		}

		if yes {
			err := CopyFile(TruePath(nav.HoldingPath), TruePath(dest))
			if err != nil {
				ErrorAlertChop(nav.ds.ses.T, Format("Cannot Paste %q: %v", dest, err))
				return
			}
		}

	case 'R': // R : Rename
		panic("TODO")
	}
}
func (o *EditAction) Undo(nav *Navigator, mod Model, path string) {
}
func (o *EditAction) String(nav *Navigator, mod Model, path string) string {
	return Format("Edit ('%c')", o.What)
}

func CopyFile(src, dest string) error {
	r, err := os.Open(src)
	if err != nil {
		return err
	}
	defer r.Close()

	w, err := os.Create(dest)
	if err != nil {
		return err
	}
	defer w.Close()

	_, err = io.Copy(w, r)
	return err
}
