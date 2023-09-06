package main

import (
	"io"
	"log"
	"os"
	"os/exec"
	"strings"
)

func hilo(h, l byte) int {
	return (int(h) << 8) | int(l)
}

func parseModule(b []byte) (string, int) {
	if b[0] != 0x87 || b[1] != 0xCD {
		return "", 0
	}
	modSize := hilo(b[2], b[3])
	modNamePtr := hilo(b[4], b[5])

	want := b[8]
	got := byte(255)
	for i := 0; i < 8; i++ {
		got ^= b[i]
	}
	if got != want {
		return "", 0
	}

	var name []byte
	var i int
	for i = modNamePtr; i < modSize && b[i] < 128; i++ {
		name = append(name, b[i])
	}
	name = append(name, b[i]&127)
	return string(name), modSize
}

func splitModules(b []byte) map[string][]byte {
	mods := make(map[string][]byte)
	for i := 0; i < len(b)-12; i++ {
		if b[i] == 0x87 && b[i+1] == 0xCD {
			name, n := parseModule(b[i:])
			if n > 0 {
				mods[strings.ToUpper(name)] = b[i : i+n]
				log.Printf("... mod(%q) len=$%x=%d.", name, n, n)
			}
		}
	}
	return mods
}

func GetBootTrackOf(filename string) []byte {
	// sectorsPerTrack = 18, sectorSize = 256
	const offset = 34 * 18 * 256 // 35th track
	const length = 18 * 256

	fd, err := os.Open(filename)
	if err != nil {
		log.Panicf("cannot open file: %q: %v", filename, err)
	}
	defer fd.Close()

	at, err := fd.Seek(offset, 0)
	if err != nil {
		log.Panicf("cannot seek file: %q: %v", filename, err)
	}
	if at != offset {
		log.Panicf("bad seek: %q: %v", filename, err)
	}

	track := make([]byte, length)
	n, err := io.ReadFull(fd, track)
	if err != nil {
		log.Panicf("cannot read track: %q: %v", filename, err)
	}
	if n != length {
		log.Panicf("short track read: %q: %v", filename, err)
	}

	return track
}

func getOs9BootOf(filename string) []byte {
	cmd := exec.Command("os9", "copy", filename+",os9boot", "/dev/stdout")
	output, err := cmd.Output()
	if err != nil {
		log.Panicf("Error in OS9 COPY: %q: %v", filename, err)
	}
	return output
}

func Run(command string, words ...string) {
	cmd := exec.Command(command, words...)
	err := cmd.Run()
	if err != nil {
		log.Panicf("Error in unix command: `%v`: %v", words, err)
	}
}
