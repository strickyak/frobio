// unused //package main
// unused //
// unused //import (
// unused //	"crypto/sha256"
// unused //	"flag"
// unused //	"io/ioutil"
// unused //	"log"
// unused //)
// unused //
// unused //var HOST = flag.String("hostname", "COCOHOST", "Common name of the Coco machine")
// unused //var SECRET = flag.String("secret", "", "Seed string for secret password in ROM")
// unused //
// unused //const SALT = "/cocosalt/"
// unused //
// unused //func main() {
// unused //	flag.Parse()
// unused //
// unused //	args := flag.Args()
// unused //	if len(args) != 1 {
// unused //		log.Fatalf("Usage:  customize-axiom [flags] romfile")
// unused //	}
// unused //	romfile := args[0]
// unused //
// unused //	rom, err := ioutil.ReadFile(romfile)
// unused //	if err != nil {
// unused //		log.Fatalf("Cannot read romfile %q: %v", romfile, err)
// unused //	}
// unused //	log.Printf("INPUT ROM FILE SIZE: %d", len(rom))
// unused //
// unused //	for i := len(rom); i < 8*1024-32; i++ {
// unused //		rom = append(rom, 0xFF)
// unused //	}
// unused //	log.Printf("MID ROM FILE SIZE: %d", len(rom))
// unused //
// unused //	// struct axiom4_rom_tail { // $DFE0..$DFFF
// unused //	//   byte rom_hostname[8];
// unused //	//   byte rom_reserved[3];
// unused //	//   byte rom_mac_tail[5];  // After initial $02 byte, 5 random bytes!
// unused //	//   byte rom_secrets[16];  // For Challenge/Response Authentication Protocols.
// unused //	// };
// unused //
// unused //	// Append hostname [8]
// unused //	for i := 0; i < 8; i++ {
// unused //		if i < len(*HOST) {
// unused //			rom = append(rom, (*HOST)[i])
// unused //		} else {
// unused //			rom = append(rom, ' ')
// unused //		}
// unused //	}
// unused //
// unused //	// Append reserved [3]
// unused //	rom = append(rom, 0x00, 0x00, 0x00)
// unused //
// unused //	// Append mac_tail [5]
// unused //	macBytes := sha256.Sum256([]byte(*HOST))
// unused //	rom = append(rom, macBytes[:5]...)
// unused //
// unused //	// If no --secret is specified, use deterministic bytes based on HOST.
// unused //	secretBytes := sha256.Sum256([]byte(*HOST + SALT))
// unused //	if *SECRET != "" {
// unused //		// Or use --secret if specified.
// unused //		secretBytes = sha256.Sum256([]byte(*SECRET + SALT))
// unused //	}
// unused //
// unused //	// Append secret [16]
// unused //	rom = append(rom, secretBytes[:16]...)
// unused //	log.Printf("FINAL ROM FILE SIZE: %d", len(rom))
// unused //
// unused //	if len(rom) != 8*1024 {
// unused //		log.Fatalf("Wrong size of new ROM, got %d want %d", len(rom), 8*1024)
// unused //	}
// unused //
// unused //	err = ioutil.WriteFile(romfile, rom, 0666)
// unused //	if err != nil {
// unused //		log.Fatalf("Cannot write romfile %q: %v", romfile, err)
// unused //	}
// unused //
// unused //	log.Printf("OKAY: wrote %s", romfile)
// unused //}
