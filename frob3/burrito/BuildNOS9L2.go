package main

import (
	"flag"
	"io/ioutil"
	"log"
	"os"
	"strings"
)

type Os9ConfigForLemma struct {
	Boot1Mods []string
	Boot2Mods []string
}

func (cf *Os9ConfigForLemma) Slurp() (b1 []byte, b2 []byte) {
	for _, filename := range cf.Boot1Mods {
		b1 = append(b1, Slurp(filename)...)
	}
	for _, filename := range cf.Boot2Mods {
		b2 = append(b2, Slurp(filename)...)
	}
	return b1, b2
}

var Nitros9Level2Conf = &Os9ConfigForLemma{
	Boot1Mods: []string{
		"rel_80", "boot_lemma", "krn",
	},
	Boot2Mods: []string{
		"krnp2",
		"ioman",
		"init",
		"rbf.mn",
		"scf.mn",
		"vtio.dr",
		"keydrv_cc3.sb",
		"snddrv_cc3.sb",
		"joydrv_joy.sb",
		"cowin.io",
		"term_win80.dt",

		"w.dw",
		"w1.dw",
		"w2.dw",
		"w3.dw",
		"w4.dw",
		"w5.dw",
		"w6.dw",
		"w7.dw",

		"clock_60hz",
		"clock2_soft",
		"sysgo_dd",

		"F3/rblemma",
		"F3/dd.b0",
	},
}

var nitros9dir = flag.String("nitros9", "", "root of nitros9 sources")

func Quint(cmd byte, n int, p int) []byte {
	return []byte{
		cmd,
		byte(n >> 8),
		byte(n),
		byte(p >> 8),
		byte(p)}
}

func ChunksOf(bb []byte) [][]byte {
	var z [][]byte
	for len(bb) > 0 {
		var n int
		if len(bb) < 1024 {
			n = 1024
		} else {
			n = len(bb)
		}
		z = append(z, bb[:n])
		bb = bb[n:]
	}
	return z
}

func Slurp(filename string) []byte {
	if strings.HasPrefix(filename, "F3/") {
		filename = "results/MODULES/" + filename[3:]
	} else {
		filename = *nitros9dir + "/level2/coco3/modules/" + filename
	}
	bb, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Panicf("Cannot read filename %q: %v", filename, err)
	}
	return bb
}

func Emit(bb []byte) {
	cc, err := os.Stdout.Write(bb)
	if err != nil {
		log.Panicf("Cannot write %d bytes to Stdout: %v", len(bb), err)
	}
	if cc != len(bb) {
		log.Panicf("Short write of %d bytes to Stdout", len(bb))
	}
}

func main() {
	flag.Parse()
	b1, b2 := Nitros9Level2Conf.Slurp()

	// Set interrupt vectors.
	Emit(Quint(0, 16, 0xFFF0))
	Emit(Slurp("vectors"))

	// Load binary b1 at $2600.
	Emit(Quint(0, len(b1), 0x2600))
	Emit(b1)

	// Launch binary b1.
	Emit(Quint(255, 0, 0x2602))

	// Send b2: 211 begin, 212 chunks, 213 end.
	Emit(Quint(211, len(b2), 0))
	for _, chunk := range ChunksOf(b2) {
		Emit(Quint(212, len(chunk), 0))
		Emit(chunk)
	}
	Emit(Quint(213, len(b2), 0))
}
