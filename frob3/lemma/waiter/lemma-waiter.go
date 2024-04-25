package main

import (
	"flag"
	"log"
	"os"

	"github.com/strickyak/frobio/frob3/lemma"
)

func main() {
	flag.Parse()
	log.SetFlags(0)
	log.SetPrefix("# ")

	if *lemma.PRINT_VERSION {
		log.Printf("Protcol Version 41. Default TCP port %d.\n", *lemma.PORT)
		os.Exit(0)
	}

	lemma.L0Init()
	lemma.Listen()
}
