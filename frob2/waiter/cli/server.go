package main

import (
	"flag"
	"github.com/strickyak/frobio/frob2/waiter"
	_ "github.com/strickyak/frobio/frob2/waiter/canvas"
	"github.com/strickyak/frobio/frob2/waiter/future"
)

func main() {
	flag.Parse()
	go waiter.Listen()
	future.Run()
	waiter.Console()
}
