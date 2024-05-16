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
var RESULTS_DIR = flag.String("results_dir", "lemma", "root of lemma results")

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
	var track []byte
	var base1 map[string][]byte
	if cf.Boot1Base != "" {
		path1 := cf.Boot1Base
		if path1[0] != '/' {
			path1 = filepath.Join(*shelfFlag, cf.Boot1Base)
		}
		track = GetBootTrackOf(path1)
		base1 = splitModules(track)
		log.Printf("Boot1 Base %q len=$%x=%d.", path1, len(track), len(track))
	}
	if len(cf.Boot1Mods) == 3 && cf.Boot1Mods[0] == "_HEAD_" && cf.Boot1Mods[2] == "_TAIL_" {
		head, mid, tail := splitBootTrackAroundBootModule(track)
		log.Printf("HARK3 *head* %3x (len $%x)", head[:32], len(head))
		log.Printf("HARK3 *mid* %3x (len $%x)", mid[:32], len(mid))
		log.Printf("HARK3 *tail* %3x (len $%x)", tail[:32], len(tail))

		boot := SlurpFilename(cf, nil, cf.Boot1Mods[1])
		log.Printf("HARK3 Boot %3x (len $%x)", boot[:32], len(boot))

		b1 = append(b1, head...)
		b1 = append(b1, boot...)
		b1 = append(b1, tail...)
		log.Printf("HARK3 b1 %3x (len $%x)", b1[:32], len(b1))

	} else {
		for i, filename := range cf.Boot1Mods {

			var mod = SlurpFilename(cf, base1, filename)
			log.Printf("HARK %q i=%d level=%q len=%d head=%02x", filename, i, cf.Level, len(mod), mod[:16])

			b1 = append(b1, mod...)
			log.Printf("HARK b1 -> $%x = %d.", len(b1), len(b1))
		}
	}

	var base2 map[string][]byte
	if cf.Boot2Base != "" {
		path2 := cf.Boot2Base
		if path2[0] != '/' {
			path2 = filepath.Join(*shelfFlag, cf.Boot2Base)
		}
		track := getOs9BootOf(path2)
		base2 = splitModules(track)
		log.Printf("Boot2 Base %q len=$%x=%d.", path2, len(track), len(track))
	}
	for _, filename := range cf.Boot2Mods {
		b2 = append(b2, SlurpFilename(cf, base2, filename)...)
		log.Printf("HARK b2 -> $%x = %d.", len(b2), len(b2))
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
			filename = filepath.Join(*RESULTS_DIR, "MODULES", filename)
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

/* From level1/coco1/modules/rel.asm about Level 2/3:
*************************************************************************
** Start of Level2/Level3 code
**
** The boot-loader loads the "kernelfile" from track34 of the disk into
** memory at $26xx and then jumps into it at address $2602. The
** kernelfile is formed by concatenating the modules REL, BOOT, KRN.
** The entry point jumps via the REL module header to label "start".
**
** One function of REL is to copy the kernelfile from $26xx to high
** memory (specifically, to Bt.Start). $1200 bytes are copied.
**
** The size of each of these modules is controlled with filler bytes (eg,
** see label "Filler" below) so that (after relocation):
** REL  starts at $ED00 and is $130 bytes in size (i.e. 304. bytes)
** BOOT starts at $EE30 and is $1D0 bytes in size (i.e. 464 bytes)
** KRN  starts at $F000 and ends at $FEFF (i.e. 3840 bytes)
** (but the 'emod' comes before the end of krn -- refer to the source file for details)
**
** When REL starts, it has NO STACK
*************************************************************************/
