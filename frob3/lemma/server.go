package main

import (
	"flag"
	_ "github.com/strickyak/frobio/frob3/lemma/canvas"
	"github.com/strickyak/frobio/frob3/lemma/lib"
	"log"
)

func main() {
	flag.Parse()
	log.SetFlags(0)
	log.SetPrefix("# ")
	lib.L0Init()
	lib.Listen()
}
