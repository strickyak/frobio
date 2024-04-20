package main

import (
	"flag"
	"log"

	"github.com/strickyak/frobio/frob3/lemma"
	//"github.com/strickyak/frobio/frob3/lemma/canvas"
)

func main() {
	flag.Parse()
	log.SetFlags(0)
	log.SetPrefix("# ")
	lemma.L0Init()
	lemma.Listen()
}
