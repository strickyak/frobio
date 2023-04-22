package main

import (
	"flag"
	"github.com/strickyak/frobio/frob2/waiter"
	_ "github.com/strickyak/frobio/frob2/waiter/canvas"
	"log"
)

func main() {
	flag.Parse()
	log.SetFlags(0)
	log.SetPrefix("@ ")
	waiter.Listen()
}
