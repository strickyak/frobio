package main

import (
	"flag"

	"github.com/strickyak/frobio/frob3/lemma/strdy"
)

var Filename = flag.String("f", "_temp_db", "filename of database")

func main() {
	flag.Parse()
	argv := flag.Args()

	//for i, a := range argv {
	//println(i, a)
	//}

	db := strdy.Load(*Filename)

	switch len(argv) {
	case 1:
		println(db.Get(argv[0]))
	case 2:
		db.Set(argv[0], argv[1], "comment")
	}
}
