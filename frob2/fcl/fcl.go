package main

import (
	"github.com/strickyak/tcl67/cli"

	_ "github.com/strickyak/frobio/frob2/fcl/os9"
	_ "github.com/strickyak/tcl67/extra"
	_ "github.com/strickyak/tcl67/posix"
	_ "github.com/strickyak/tcl67/tcl"
)

func main() {
	cli.Main()
}
