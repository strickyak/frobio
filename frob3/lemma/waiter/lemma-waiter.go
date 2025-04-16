package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"runtime"
	"syscall"

	"github.com/strickyak/frobio/frob3/lemma"
)

func main() {
	// Trapping SIGQUIT and getting a Stack Trace:
	// Thanks James Henstridge
	// https://stackoverflow.com/questions/23354810/how-can-i-dump-all-a-go-processs-stacks-without-killing-it
	sigChan := make(chan os.Signal)
	go func() {
		stacktrace := make([]byte, 8192)
		for _ = range sigChan {
			length := runtime.Stack(stacktrace, true)
			fmt.Println(string(stacktrace[:length]))
		}
	}()
	signal.Notify(sigChan, syscall.SIGQUIT)

	flag.Parse()

	if *lemma.PRINT_VERSION {
		log.Printf("Protcol Version 41. Default TCP port %d.\n", *lemma.PORT)
		os.Exit(0)
	}

	lemma.L0Init()
	lemma.Listen()
}
