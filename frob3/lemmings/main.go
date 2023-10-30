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

const TODO_shelf = "../"

var shelfFlag = flag.String("shelf", "..", "coco-shelf directory")
var nitros9dirFlag = flag.String("nitros9dir", "../nitros9", "root of nitros9 sources")

type Os9ConfigForLemma struct {
	Name        string
	Level       string
	Port        string
	DefaultDisk string
	Boot1Base   string
	Boot1Mods   []string
	Boot2Base   string
	Boot2Mods   []string
}

func (cf *Os9ConfigForLemma) SlurpBoots() (b1 []byte, b2 []byte) {
	var base1 map[string][]byte
	if cf.Boot1Base != "" {
		path1 := filepath.Join(*shelfFlag, cf.Boot1Base)
		track := GetBootTrackOf(path1)
		base1 = splitModules(track)
		log.Printf("Boot1 Base %q len=$%x=%d.", path1, len(track), len(track))
	}
	for _, filename := range cf.Boot1Mods {
		b1 = append(b1, SlurpFilename(cf, base1, filename)...)
	}

	var base2 map[string][]byte
	if cf.Boot2Base != "" {
		path2 := filepath.Join(*shelfFlag, cf.Boot2Base)
		track := getOs9BootOf(path2)
		base2 = splitModules(track)
		log.Printf("Boot2 Base %q len=$%x=%d.", path2, len(track), len(track))
	}
	for _, filename := range cf.Boot2Mods {
		b2 = append(b2, SlurpFilename(cf, base2, filename)...)
	}

	return b1, b2
}

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

func SlurpFilename(cf *Os9ConfigForLemma, base map[string][]byte, filename string) []byte {
  log.Printf("SLURP %q", filename)
	if strings.HasPrefix(filename, "<") {
		key := strings.ToUpper(filename[1:])
		mod, ok := base[key]
		if !ok {
			log.Panicf("Module %q not found in %q", key, cf.Boot2Base)
		}
		return mod
	} else {
		// Determine the true filename
		if strings.HasPrefix(filename, "/") {
			filename = filename
		} else if strings.HasPrefix(filename, "SHELF/") {
			filename = filepath.Join(*shelfFlag, filename[6:])
		} else if strings.HasPrefix(filename, "./") {
			filename = filepath.Join("results/MODULES/", filename)
		} else {
			filename = filepath.Join(*nitros9dirFlag, cf.Level, cf.Port, "modules", filename)
		}

		// Slurp the file.
		bb, err := ioutil.ReadFile(filename)
		if err != nil {
			log.Panicf("Cannot read filename %q: %v", filename, err)
		}
		log.Printf("Read $%x=%d. bytes from %q", len(bb), len(bb), filename)

		// Return the contents.
		return bb
	}
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

	for _, cf := range Os9Configs {
		log.Printf("Building Os9Config: %q", cf.Name)
		BuildConfig(cf)
	}
}

func BuildConfig(cf *Os9ConfigForLemma) {
	name := cf.Name + ".lem"
	w, err := os.Create(name)
	if err != nil {
		log.Panicf("Cannot create %q: %v", name, err)
	}
	defer w.Close()
	log.Printf("==== Writing to %q ====", name)

	b1, b2 := cf.SlurpBoots()

	// Set interrupt vectors.
	Emit(w, Quint(0, 16, 0xFFF0))
	Emit(w, SlurpFilename(cf, nil, "vectors"))

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

	if cf.DefaultDisk != "" {
		// in := filepath.Join(*shelfFlag, cf.DefaultDisk)
		in := cf.DefaultDisk
		out := cf.Name + ".dsk"
		Run("dd", "conv=sparse", "bs=4096", "if="+in, "of="+out)
	}
}
