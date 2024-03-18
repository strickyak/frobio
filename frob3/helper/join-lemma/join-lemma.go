package main

import (
	"flag"
	"io"
	"io/ioutil"
	"log"
	"os"
	"regexp"
	"strconv"
)

var oflag = flag.String("o", "", "output prefix")

// e.g. /tmp/EM/EM_000000.00.0010.fff0.poke.qnt
const pattern = "(.*)_([0-9A-Fa-f]{6})[.]([0-9A-Fa-f]{2})[.]([0-9A-Fa-f]{4})[.]([0-9A-Fa-f]{4})[.]([0-9A-Za-z_)]*)[.]qnt"

var match = regexp.MustCompile(pattern)

func main() {
	flag.Parse()
	for _, arg := range flag.Args() {
		emit(os.Stdout, arg)
	}
}

func emit(w io.Writer, filename string) {
	m := match.FindStringSubmatch(filename)
	if m == nil {
		log.Fatalf("Argument does not match filename pattern: %q", filename)
	}
	c_str, n_str, p_str := m[3], m[4], m[5]
	c, err := strconv.ParseUint(c_str, 16, 8)
	if err != nil {
		log.Fatalf("Cannot Parse Hex %q in filename %q: %v", c_str, filename, err)
	}
	n, err := strconv.ParseUint(n_str, 16, 16)
	if err != nil {
		log.Fatalf("Cannot Parse Hex %q in filename %q: %v", n_str, filename, err)
	}
	p, err := strconv.ParseUint(p_str, 16, 16)
	if err != nil {
		log.Fatalf("Cannot Parse Hex %q in filename %q: %v", p_str, filename, err)
	}
	q := []byte{byte(c), byte(n >> 8), byte(n), byte(p >> 8), byte(p)}
	cc, err := w.Write(q)
	if err != nil {
		log.Fatalf("Cannot Write to stdout: %v", err)
	}
	if cc != 5 {
		log.Fatalf("Short Write to stdout, did %d, wanted 5: %v", cc, err)
	}
	bb, err := ioutil.ReadFile(filename)
	cc, err = w.Write(bb)
	if err != nil {
		log.Fatalf("Cannot Write to stdout: %v", err)
	}
	if cc != len(bb) {
		log.Fatalf("Short Write to stdout, did %d, wanted %d: %v", cc, len(bb), err)
	}
}
