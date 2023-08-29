package main

import (
	"flag"
	"github.com/strickyak/frobio/frob3/lemma/lib"
	"log"

	_ "github.com/strickyak/frobio/frob3/lemma/canvas"
	_ "github.com/strickyak/frobio/frob3/lemma/level0"
)

func main() {
	flag.Parse()
	log.SetFlags(0)
	log.SetPrefix("# ")

	lib.Listen()
}
