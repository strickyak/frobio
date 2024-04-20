package lib

import (
	// "fmt"
	// "io"
	"log"
	"os"
	PFP "path/filepath"

	"github.com/strickyak/frobio/frob3/lemma/rsdos"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type Chooser struct {
	Order  int
	Name   string
	Kids   []*Chooser
	Parent *Chooser

	IsDir bool
	Size  int64
	Disk  rsdos.DiskRec

	Line int // in Parent's chooser
}

func AtFocus(c *Chooser, focus uint) *Chooser {
	if focus == 2 { // Parent
		if c.Parent != nil {
			return c.Parent
		} else {
			return c
		}
	}
	return c.Kids[focus-3]
}

func TopChooser() *Chooser {
	return &Chooser{
		Order: 100,
		Name:  ".",
	}
}

func (c *Chooser) UnixPath() string {
	if *FlagPublicRoot == "" {
		log.Panicf("Missing --dos_root flag on server")
	}
	return PFP.Join(*FlagPublicRoot, c.Path())
}
func (c *Chooser) Path() string {
	if c.Parent == nil {
		return "."
	} else {
		return PFP.Join(c.Parent.Path(), c.Name)
	}
}

func (c *Chooser) FillChooser() {
	if c.Kids != nil {
		return
	}
	uPath := c.UnixPath()
	stat := Value(os.Stat(uPath))
	if stat.IsDir() {
		c.IsDir = true
		entries := Value(os.ReadDir(uPath))
		line := 0
		for _, e := range entries {
			name := e.Name()
			if name[0] == '.' {
				continue
			}
			c.Kids = append(c.Kids, &Chooser{
				Order:  100,
				Name:   name,
				Parent: c,
				Line:   line,
			})
			line++
		}
	} else {
		// Not a directory.
		c.Size = stat.Size()
		// TODO // contents := Value(os.ReadFile(uPath))
		// TODO // c.Disk = rsdos.DiskParse(contents)
	}
}

func (c *Chooser) MakeChoice(kid string) *Chooser {
	c.FillChooser()

	for _, e := range c.Kids {
		if e.Name == kid {
			return e
		}
	}

	log.Panicf("Kid %q not found in Chooser %q", kid, c.Path())
	panic(0)
}
