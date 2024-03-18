package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
)

var oflag = flag.String("o", "", "output prefix")

func main() {
	flag.Parse()
	split(os.Stdin)
}

func split(r io.Reader) {
	offset := uint(0)
	for {
		q := make([]byte, 5) // input quint
		cc, err := io.ReadFull(r, q)
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Fatalf("cannot ReadFull stdin: %v", err)
		}
		if cc != 5 {
			log.Fatalf("short ReadFull: got %d, wanted 5 bytes", cc)
		}

		// Decode the quint
		cmd := q[0]
		n := (uint(q[1]) << 8) | uint(q[2])
		p := (uint(q[3]) << 8) | uint(q[4])
		_ = p

		switch cmd {
		case CMD_POKE:
			take(r, q, n, &offset, "poke")
		case CMD_CALL:
			take(r, q, n, &offset, "call")
		case CMD_BLOCK_READ:
			take(r, q, n, &offset, "block_read")
		case CMD_BLOCK_WRITE:
			take(r, q, n, &offset, "block_write")
		case CMD_BLOCK_ERROR:
			take(r, q, n, &offset, "block_error")
		case CMD_BLOCK_OKAY:
			take(r, q, n, &offset, "block_okay")
		case CMD_BOOT_BEGIN:
			take(r, q, 0, &offset, "boot_begin")
		case CMD_BOOT_CHUNK:
			take(r, q, n, &offset, "boot_chunk")
		case CMD_BOOT_END:
			take(r, q, 0, &offset, "boot_end")
		default:
			take(r, q, n, &offset, fmt.Sprintf("cmd%d", cmd))
		}
	}
}

func take(r io.Reader, q []byte, sz uint, offset *uint, comment string) {
	bb := make([]byte, sz)
	cc, err := io.ReadFull(r, bb)
	if err != nil {
		log.Fatalf("cannot ReadFull stdin: %v", err)
	}
	if uint(cc) != sz {
		log.Fatalf("short ReadFull: got %d, wanted %d bytes", cc, sz)
	}

	// Decode the quint
	cmd := q[0]
	n := (uint(q[1]) << 8) | uint(q[2])
	p := (uint(q[3]) << 8) | uint(q[4])

	filename := fmt.Sprintf("%s_%06x.%02x.%04x.%04x.%s.qnt", *oflag, *offset, cmd, n, p, comment)
	err = ioutil.WriteFile(filename, bb, 0644)
	if err != nil {
		log.Fatalf("cannot WriteFile %q: %v", filename, err)
	}

	*offset += (5 + sz)
}

const (
	CMD_POKE = 0
	CMD_CALL = 255

	CMD_BEGIN_MUX = 196
	CMD_MID_MUX   = 197
	CMD_END_MUX   = 198

	CMD_LEVEL0 = 199 // when Level0 needs attention, it sends 199.

	CMD_LOG     = 200
	CMD_INKEY   = 201
	CMD_PUTCHAR = 202
	CMD_PEEK    = 203
	CMD_DATA    = 204
	CMD_SP_PC   = 205
	CMD_REV     = 206

	CMD_BLOCK_READ     = 207 // block device
	CMD_BLOCK_WRITE    = 208 // block device
	CMD_BLOCK_ERROR    = 209 // nack
	CMD_BLOCK_OKAY     = 210 // ack
	CMD_BOOT_BEGIN     = 211 // boot_lemma
	CMD_BOOT_CHUNK     = 212 // boot_lemma
	CMD_BOOT_END       = 213 // boot_lemma
	CMD_LEMMAN_REQUEST = 214 // LemMan
	CMD_LEMMAN_REPLY   = 215 // LemMan

	CMD_RTI    = 216
	CMD_ECHO   = 217 // reply with CMD_DATA, with high bits toggled.
	CMD_DW     = 218
	CMD_HDBDOS = 219
)
