/*
  helper/insert-gap-in-asm/main.go

	This rearranges the routines in the input assembly language so
	that it can fill $C880-$C8FF with $FFs.  We also need the ".map"
	file from one linkage of it, so we know where all the symbols
	land when functions from libraries get included.

	It first figures out points at which the assembly file
	can be broken on function (or sometimes data) boundaries.

	A portion at the beginning ("-FIRST") and at the end
	("-LAST") are not disturbed.

	Then it "fills" after "-FIRST" with the largest functions
	that will fit, until no more will fit before $C880.  Then
	it fills through $C8FF.

	The "-FIRST", in our case, contains the preboot.asm that
	has its own fill from $C080-$C0FF.  It has SECTION and
	ENDSECTION directives instead of ".area" directives.
	The "-FIRST" will last at least through the first ".area".
*/
package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"regexp"
	"sort"
	"strconv"
	"strings"
)

var O = flag.String("o", "", "output file")
var ASM = flag.String("asm", "", "assembly language file")
var MAP = flag.String("map", "", "linkage map file")

var GAP = flag.Int("gap", 0xC880, "start gap at or before here")
var GAP_SIZE = flag.Int("gap_size", 0x0080, "size of the required gap")

func main() {
	flag.Parse()

	if *ASM == "" {
		log.Fatalf("You must define a filename for --asm flag")
	}
	if *MAP == "" {
		log.Fatalf("You must define a filename for --map flags")
	}

	SlurpSymbolMap()
	SlurpAsm()
	Rearrange()
}

var MatchArea = regexp.MustCompile(`^[ \t]+[.]area[ \t]+([A-Za-z0-9.]+)$`)
var MatchJump = regexp.MustCompile(`^[ \t]+([.]area|[.]globl|[.]ascii|jmp|rts|puls.*pc).*`)
var MatchLabel = regexp.MustCompile(`^(_[A-Za-z0-9_.]+)([: ].*|$)`)

var SymbolMap = make(map[string]int)
var Pieces = make(map[string]*Piece)

type Piece struct {
	name  string
	lines []string
	next  string
	begin int
	end   int
	size  int
	used  bool
}

func SlurpAsm() {
	contents, err := ioutil.ReadFile(*ASM)
	if err != nil {
		log.Fatalf("Cannot ReadFile %q: %v", *ASM, err)
	}
	lines := strings.Split(string(contents), "\n")
	wasJump := false
	gotText := false
	p := &Piece{
		name: "-FIRST",
	}
	Pieces["-FIRST"] = p
	for _, line := range lines {
		mArea := MatchArea.FindStringSubmatch(line)
		if mArea != nil {
			line = "  .area .text" // The only area we want.
			gotText = true
		}
		mLabel := MatchLabel.FindStringSubmatch(line)
		if gotText && mLabel != nil && wasJump {
			label := mLabel[1]
			p.next = label
			p = &Piece{
				name: label,
			}
			Pieces[label] = p
		}
		p.lines = append(p.lines, line)
		wasJump = MatchJump.MatchString(line)
	}
	// Change the name of the final piece.
	p.name = "-LAST"
	Pieces["-LAST"] = p

	log.Printf("===============================")
	for k, v := range Pieces {
		a, _ := SymbolMap[k]
		b, _ := SymbolMap[v.next]
		v.begin, v.end, v.size = a, b, b-a
		log.Printf("==== PIECE %q %04x   (next %q %04x)  size %04x", k, a, v.next, b, v.size)
		for _, line := range v.lines {
			log.Printf(".......... %s", line)
		}
	}
	log.Printf("===============================")
}

func Emit(w io.Writer, p *Piece) {
	fmt.Fprintf(w, "\n\n;;;;;;;;;;  PIECE  %q  (%x %x %x)\n\n", p.name, p.begin, p.end, p.size)
	for _, line := range p.lines {
		fmt.Fprintf(w, "%s\n", line)
	}
	fmt.Fprintf(w, "\n\n")
}

func Rearrange() {
	w0, err := os.Create(*O)
	if err != nil {
		log.Fatalf("Cannot os.Create %q: %v", *O, err)
	}
	w := bufio.NewWriter(w0)

	first, _ := Pieces["-FIRST"]
	Emit(w, first)
	at := first.end

	var wanted int
	for {
		wanted = *GAP - at
		p := ChooseBiggestPieceNotExceeding(wanted)
		if p == nil {
			break
		}
		Emit(w, p)
		p.used = true
		at += p.size
	}

	fmt.Fprintf(w, "\n\nFILLING:  fill $FF,%d ;;;; nando\nFILLED:\n\n", wanted+*GAP_SIZE)

	var temp []string // Alphabetize in temp for reproducability.
	for _, p := range Pieces {
		if !strings.HasPrefix(p.name, "-") && !p.used {
			temp = append(temp, p.name)
			p.used = true
		}
	}
	sort.Strings(temp) // alphabetize!
	for _, name := range temp {
		Emit(w, Pieces[name])
	}

	last, _ := Pieces["-LAST"]
	Emit(w, last)
	fmt.Fprintf(w, "\n\nFINAL:")

	w.Flush()
	w0.Close()
}

func ChooseBiggestPieceNotExceeding(wanted int) *Piece {
	var biggest *Piece

	var temp []string // Alphabetize in temp for reproducability.
	for _, p := range Pieces {
		if p.name != "_main" && !strings.HasPrefix(p.name, "-") && !p.used && p.begin > 0 && p.end > 0 && p.size <= wanted {
			temp = append(temp, p.name)
		}
	}
	sort.Strings(temp) // alphabetize!
	for _, name := range temp {
		p := Pieces[name]
		if biggest == nil {
			biggest = p
		}
		if p.size > biggest.size {
			biggest = p
		}
	}
	return biggest
}

// EXAMPLE: "Symbol: _SkipWhite (_rework.o) = C151"
var MatchMapSymbol = regexp.MustCompile(`^Symbol: ([A-Za-z0-9_.]+) [(].*[)] = ([0-9A-F]{4})$`)

func SlurpSymbolMap() {
	contents, err := ioutil.ReadFile(*MAP)
	if err != nil {
		log.Fatalf("Cannot ReadFile %q: %v", *MAP, err)
	}
	lines := strings.Split(string(contents), "\n")
	for _, line := range lines {
		if m := MatchMapSymbol.FindStringSubmatch(line); m != nil {
			label, hexstr := m[1], m[2]
			if strings.HasPrefix(label, "_") {
				hex, err := strconv.ParseInt(hexstr, 16, 64)
				if err != nil {
					log.Fatalf("Cannot ParseInt[hex]: %q", hexstr)
				}
				SymbolMap[label] = int(hex)
				// println(label, hexstr)
			}
		}
	}
}
