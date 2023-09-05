package main

import (
	"flag"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

type Os9ConfigForLemma struct {
	Name      string
	Level     string
	Port      string
	Boot1Mods []string
	Boot2Mods []string
}

func (cf *Os9ConfigForLemma) SlurpBoots() (b1 []byte, b2 []byte) {
	for _, filename := range cf.Boot1Mods {
		b1 = append(b1, SlurpFilename(cf, filename)...)
	}
	for _, filename := range cf.Boot2Mods {
		b2 = append(b2, SlurpFilename(cf, filename)...)
	}
	return b1, b2
}

var nitros9dirFlag = flag.String("nitros9dir", "", "root of nitros9 sources")

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
		if len(bb) > 444 {
			n = 444
		} else {
			n = len(bb)
		}
		z = append(z, bb[:n])
		bb = bb[n:]
	}
	return z
}

func SlurpFilename(cf *Os9ConfigForLemma, filename string) []byte {
	if strings.HasPrefix(filename, "./") {
		filename = filepath.Join("results/MODULES/", filename)
	} else {
		filename = filepath.Join(*nitros9dirFlag, cf.Level, cf.Port, "modules", filename)
	}
	bb, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Panicf("Cannot read filename %q: %v", filename, err)
	}
	log.Printf("Read %d bytes from %q", len(bb), filename)
	return bb
}

func Emit(w io.Writer, bb []byte) {
	cc, err := w.Write(bb)
	if err != nil {
		log.Panicf("Cannot write %d bytes to file: %v", len(bb), err)
	}
	if cc != len(bb) {
		log.Panicf("Short write of %d bytes to file", len(bb))
	}
}

var Os9Configs []*Os9ConfigForLemma

func Os9(cf *Os9ConfigForLemma) {
	Os9Configs = append(Os9Configs, cf)
}

func main() {
	flag.Parse()
	if *nitros9dirFlag == "" {
		dir := os.Getenv("NITROS9DIR")
		if dir == "" {
			log.Panicf("You must define the --nitros9dir flag or the NITROS9DIR environment.")
		}
		nitros9dirFlag = &dir
	}
	//BuildConfig(Nitros9_Coco3_M6809_Level2_Cf)
	//BuildConfig(Nitros9_Coco3_M6809_Level2_N_Cf)
	//BuildConfig(Nitros9_Coco3_M6809_Level2_Nminus_Cf)
	//BuildConfig(Nitros9_Coco3_H6309_Level2_Cf)
	for _, cf := range Os9Configs {
		log.Printf("Building Os9Config: %q", cf.Name)
		BuildConfig(cf)
	}
}

func BuildConfig(cf *Os9ConfigForLemma) {
	w, err := os.Create(cf.Name + ".lem")
	if err != nil {
		log.Panicf("Cannot create %q: %v", cf.Name, err)
	}
	defer w.Close()
	log.Printf("==== Writing to %q ====", cf.Name)

	b1, b2 := cf.SlurpBoots()

	// Set interrupt vectors.
	Emit(w, Quint(0, 16, 0xFFF0))
	Emit(w, SlurpFilename(cf, "vectors"))

	// Load binary b1 at $2600.
	Emit(w, Quint(0, len(b1), 0x2600))
	Emit(w, b1)

	// Launch binary b1.
	Emit(w, Quint(255, 0, 0x2602))

	// Send b2: 211 begin, 212 chunks, 213 end.
	Emit(w, Quint(211, len(b2), 0))
	for _, chunk := range ChunksOf(b2) {
		Emit(w, Quint(212, len(chunk), 0))
		Emit(w, chunk)
	}
	Emit(w, Quint(213, len(b2), 0))
}
