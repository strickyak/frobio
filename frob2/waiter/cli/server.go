package main

import (
	"flag"
	"github.com/strickyak/frobio/frob2/waiter"
	_ "github.com/strickyak/frobio/frob2/waiter/canvas"
)

func main() {
	flag.Parse()
	waiter.Listen()
}
