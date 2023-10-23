package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
)

func main() {
	flag.Parse()

	for _, filename := range flag.Args() {
		contents, err := ioutil.ReadFile(filename)
		if err != nil {
			log.Fatalf("Error: cannot open %q: %v", filename, err)
		} else {
			fmt.Printf("%q:\n", filename)
			LsLem(contents)
		}
	}
}

var serial int

func SaveBytes(bb []byte) string {
	filename := fmt.Sprintf("__%d", serial)
	serial++
	ioutil.WriteFile(filename, bb, 0777)
	return filename
}

func LsLem(a []byte) {
	for len(a) > 0 {
		// Want header.
		if len(a) < 5 {
			log.Fatalf("Error: too short for next header: %v", a)
		}

		c, n, p := a[0], JoinHiLo(a[1], a[2]), JoinHiLo(a[3], a[4])

		switch c {
		case CMD_POKE:
			//f := fmt.Sprintf("__%d", serial)
			//serial++
			//ioutil.WriteFile(f, a[5:5+n], 0777)
			fmt.Printf("  Poke $%x=%d. bytes at $%x == %q\n", n, n, p, SaveBytes(a[5:5+n]))
			a = a[5+n:]
		case CMD_CALL:
			fmt.Printf("  Call $%x\n", p)
			a = a[5:]
		case CMD_LEVEL0:
			panic("level0")
		case CMD_LOG:
			panic("log")
		case CMD_INKEY:
			panic("inkey")
		case CMD_PUTCHAR:
			panic("putchar")
		case CMD_PEEK:
			panic("peek")
		case CMD_DATA:
			panic("data")
		case CMD_SP_PC:
			panic("sp_pc")
		case CMD_REV:
			panic("rev")
		case CMD_BLOCK_READ:
			panic("block")
		case CMD_BLOCK_WRITE:
			panic("block")
		case CMD_BLOCK_ERROR:
			panic("block")
		case CMD_BLOCK_OKAY:
			panic("block")
		case CMD_BOOT_BEGIN:
			fmt.Printf("  Boot Begin ($%x, $%x)\n", n, p)
			a = a[5:]
		case CMD_BOOT_CHUNK:
			fmt.Printf("  Boot Chunk ($%x, $%x)\n", n, p)
			a = a[5+n:]
		case CMD_BOOT_END:
			fmt.Printf("  Boot End ($%x, $%x)\n", n, p)
			a = a[5:]
		case CMD_LEMMAN_REQUEST:
			panic("lemman")
		case CMD_LEMMAN_REPLY:
			panic("lemman")
		}
	}
}

const (
	CMD_POKE           = 0
	CMD_CALL           = 255
	CMD_LEVEL0         = 199 // when Level0 needs attention, it sends 199.
	CMD_LOG            = 200
	CMD_INKEY          = 201
	CMD_PUTCHAR        = 202
	CMD_PEEK           = 203
	CMD_DATA           = 204
	CMD_SP_PC          = 205
	CMD_REV            = 206
	CMD_BLOCK_READ     = 207 // block device
	CMD_BLOCK_WRITE    = 208 // block device
	CMD_BLOCK_ERROR    = 209 // nack
	CMD_BLOCK_OKAY     = 210 // ack
	CMD_BOOT_BEGIN     = 211 // boot_lemma
	CMD_BOOT_CHUNK     = 212 // boot_lemma
	CMD_BOOT_END       = 213 // boot_lemma
	CMD_LEMMAN_REQUEST = 214 // LemMan
	CMD_LEMMAN_REPLY   = 215 // LemMan
)

func JoinHiLo(hi byte, lo byte) uint {
	return (uint(hi) << 8) | uint(lo)
}
