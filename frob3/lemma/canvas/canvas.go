package canvas

import (
	"io"
	"log"
	"net"
)

type DemosAPI struct {
	WriteFive func(conn net.Conn, cmd byte, n uint, p uint)
	CMD_POKE  byte
	CMD_PEEK  byte
	CMD_DATA  byte
	// CMD_REV   byte
	// CMD_SP_PC byte
}

var API *DemosAPI

const warp = true

const (
	// These are for G3CMode:
	W  = 128 // screen width, height
	H  = 192 / 2
	BW = 128 / 4 // ram byte width, height
	BH = 192 / 2
)

const ScreenLoc = 0x800

type Color byte

const (
	Buff    = Color(0)
	Cyan    = iota
	Magenta = iota
	Orange  = iota
)

type Canvas struct {
	bm [BH * BW]byte
}

func (can *Canvas) Get(x, y int, width int) Color {
	if warp {
		x = (width + x) % width
		y = (H + y) % H
	} else if x < 0 || x >= width || y < 0 || y >= H {
		return 0
	}

	bx := x / 4
	shift := 2 * (3 - (byte(x) & 3))

	return Color(3 & (can.bm[y*BW+bx] >> shift))
}

func (can *Canvas) Set(x, y int, c Color, width int) {
	if warp {
		x = (width + x) % width
		y = (H + y) % H
	} else if x < 0 || x >= width || y < 0 || y >= H {
		return
	}

	bx := x / 4
	shift := 2 * (3 - (byte(x) & 3))
	mask := byte(0xFF) ^ (byte(3) << shift)

	can.bm[y*BW+bx] = (can.bm[y*BW+bx] & mask) | (byte(c) << shift)
}

func MustWrite(conn net.Conn, buf []byte) {
	_, err := conn.Write(buf)
	if err != nil {
		log.Panicf("Canvas::Render: stopping due to error: %v", err)
	}
}

func (can *Canvas) Render(conn net.Conn) {
	API.WriteFive(conn, API.CMD_POKE, BW*BH, ScreenLoc)
	MustWrite(conn, can.bm[:])
}
func (can *Canvas) Verify(conn net.Conn) {
	const PeekChunkSize = 1024
	for i := uint(0); i < 3072/PeekChunkSize; i++ {
		API.WriteFive(conn, API.CMD_PEEK, PeekChunkSize, PeekChunkSize*i+ScreenLoc)

	HDR:
		for {
			hdr := make([]byte, 5)
			cc, err := conn.Read(hdr)
			if err != nil {
				panic("read hdr")
			}
			if cc != 5 {
				log.Panicf("short read: got %d, want 5", cc)
			}
			switch hdr[0] {
			case API.CMD_DATA:
				break HDR
			// case API.CMD_REV:
				// log.Printf("CMD: %d", hdr[0])
				// {
					// paylen := (uint(hdr[1]) << 8) | uint(hdr[2])
					// pay := make([]byte, paylen)
					// cc, err = conn.Read(pay)
					// if err != nil {
						// panic("read pay")
					// }
					// if uint(cc) != paylen {
						// log.Panicf("short read payload: got %d, want %d", cc, paylen)
					// }
					// log.Printf("DATA: %d %q", hdr[0], pay)
				// }
			// case API.CMD_SP_PC:
				// log.Printf("SP_PC: %v", hdr)

			default:
				log.Fatalf("expecting CMD_DATA, got %d", hdr[0])
			}
		}

		v := make([]byte, PeekChunkSize)
		cc, err := io.ReadFull(conn, v)
		if err != nil {
			panic("read peek")
		}
		if cc != PeekChunkSize {
			panic("short read")
		}

		for j := uint(0); j < PeekChunkSize; j++ {
			want := can.bm[j+PeekChunkSize*i]
			if v[j] != want {
				log.Panicf("Diff [%d]: %x vs %x", j+PeekChunkSize*i, v[j], want)
			}
		}
	}
}

// G3CMode is 128 columns, 96 rows, 4 colors,
// using a buffer size of 3072 bytes.
// Rows are 32 bytes with 4 pixels per byte.
// Color set C=0: enum { Green, Yellow, Blue, Red } border Green.
// Color set C=1: enum { Buff, Cyan, Magenta, Orange } border Buff.
// Color set is bit 3 of $FF22.
func G3CMode(conn net.Conn) {

	API.WriteFive(conn, API.CMD_POKE, 1, 0xFFC5)
	MustWrite(conn, []byte{anything})

	API.WriteFive(conn, API.CMD_POKE, 1, 0xFFC2)
	MustWrite(conn, []byte{anything})

	API.WriteFive(conn, API.CMD_POKE, 1, 0xFFC0)
	MustWrite(conn, []byte{anything})

	API.WriteFive(conn, API.CMD_POKE, 1, 0xFF22)
	MustWrite(conn, []byte{0xC8}) // C0 for color0
}

// G6CMode is 128 columns, 192 rows, 4 colors,
// using a buffer size of 6144 bytes.
// Rows are 32 bytes with 4 pixels per byte.
// Color set C=0: enum { Green, Yellow, Blue, Red } border Green.
// Color set C=1: enum { Buff, Cyan, Magenta, Orange } border Buff.
// Color set is bit 3 of $FF22.
func G6CMode(conn net.Conn) {

	API.WriteFive(conn, API.CMD_POKE, 1, 0xFFC5)
	MustWrite(conn, []byte{anything})

	API.WriteFive(conn, API.CMD_POKE, 1, 0xFFC3)
	MustWrite(conn, []byte{anything})

	API.WriteFive(conn, API.CMD_POKE, 1, 0xFFC0)
	MustWrite(conn, []byte{anything})

	API.WriteFive(conn, API.CMD_POKE, 1, 0xFF22)
	MustWrite(conn, []byte{0xE8}) // E0 for color0
}

func SetVdgScreenAddr(conn net.Conn, addr uint) {
	addr >>= 10
	reg := uint(0xFFC8)
	for i := 0; i < 7; i++ {
		lobit := addr & 1
		API.WriteFive(conn, API.CMD_POKE, 1, reg+lobit)
		MustWrite(conn, []byte{anything})
		reg += 2
		addr >>= 1
	}
}

const anything = 42
