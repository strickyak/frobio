package main

import (
	"flag"
	"github.com/strickyak/frobio/frob2/waiter"
)

func main() {
	flag.Parse()
	go waiter.Listen()
	waiter.Console()
}
