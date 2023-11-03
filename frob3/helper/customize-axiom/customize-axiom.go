package main

import (
	"crypto/sha256"
	"flag"
	"io/ioutil"
	"log"
)

var HOST = flag.String("hostname", "COCOHOST", "Common name of the Coco machine")
var SECRET = flag.String("secret", "", "Seed string for secret password in ROM")

const SALT = "/cocosalt/"

func main() {
	flag.Parse()

	args := flag.Args()
	if len(args) != 1 {
		log.Fatalf("Usage:  customize-axiom [flags] romfile")
	}
	romfile := args[0]

	rom, err := ioutil.ReadFile(romfile)
	if err != nil {
		log.Fatalf("Cannot read romfile %q: %v", romfile, err)
	}
	log.Printf("INPUT ROM FILE SIZE: %d", len(rom))

	for i := len(rom); i < 8*1024-32; i++ {
		rom = append(rom, 0xFF)
	}
	log.Printf("MID ROM FILE SIZE: %d", len(rom))

	// struct axiom4_rom_tail { // $DFE0..$DFFF
	//   byte rom_hostname[8];
	//   byte rom_reserved[3];
	//   byte rom_mac_tail[5];  // After initial $02 byte, 5 random bytes!
	//   byte rom_secrets[16];  // For Challenge/Response Authentication Protocols.
	// };

	// Append hostname [8]
	for i := 0; i < 8; i++ {
		if i < len(*HOST) {
			rom = append(rom, (*HOST)[i])
		} else {
			rom = append(rom, '_')
		}
	}

	// Append reserved [3]
	rom = append(rom, 0x00, 0x00, 0x00)

	// Append mac_tail [5]
	macBytes := sha256.Sum256([]byte(*HOST))
	rom = append(rom, macBytes[:5]...)

	// If no --secret is specified, use deterministic bytes based on HOST.
	secretBytes := sha256.Sum256([]byte(*HOST + SALT))
	if *SECRET != "" {
		// Or use --secret if specified.
		secretBytes = sha256.Sum256([]byte(*SECRET + SALT))
	}

	// Append secret [16]
	rom = append(rom, secretBytes[:16]...)
	log.Printf("FINAL ROM FILE SIZE: %d", len(rom))

	if len(rom) != 8*1024 {
		log.Fatalf("Wrong size of new ROM, got %d want %d", len(rom), 8*1024)
	}

	err = ioutil.WriteFile(romfile, rom, 0666)
	if err != nil {
		log.Fatalf("Cannot write romfile %q: %v", romfile, err)
	}

	log.Printf("OKAY: wrote %s", romfile)
}
