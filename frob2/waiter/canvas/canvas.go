package canvas

import (
	"io"
	"log"
	"net"

	lem "github.com/strickyak/frobio/frob2/waiter"
)

const warp = true

const (
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

func (can *Canvas) Get(x, y int) Color {
	if warp {
		x = (W + x) % W
		y = (H + y) % H
	} else if x < 0 || x >= W || y < 0 || y >= H {
		return 0
	}

	bx := x / 4
	shift := 2 * (3 - (byte(x) & 3))

	return Color(3 & (can.bm[y*BW+bx] >> shift))
}

func (can *Canvas) Set(x, y int, c Color) {
	if warp {
		x = (W + x) % W
		y = (H + y) % H
	} else if x < 0 || x >= W || y < 0 || y >= H {
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
	lem.WriteFive(conn, lem.CMD_POKE, BW*BH, ScreenLoc)
	MustWrite(conn, can.bm[:])
}
func (can *Canvas) Verify(conn net.Conn) {
	const PeekChunkSize = 1024
	for i := uint(0); i < 3072/PeekChunkSize; i++ {
		lem.WriteFive(conn, lem.CMD_PEEK, PeekChunkSize, PeekChunkSize*i+ScreenLoc)

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
			case lem.CMD_DATA:
				break HDR
			case lem.CMD_REV:
				log.Printf("CMD: %d", hdr[0])
				{
					paylen := (uint(hdr[1]) << 8) | uint(hdr[2])
					pay := make([]byte, paylen)
					cc, err = conn.Read(pay)
					if err != nil {
						panic("read pay")
					}
					if uint(cc) != paylen {
						log.Panicf("short read payload: got %d, want %d", cc, paylen)
					}
					log.Printf("DATA: %d %q", hdr[0], pay)
				}
			case lem.CMD_SP_PC:
				log.Printf("SP_PC: %v", hdr)

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

func G3CMode(conn net.Conn) {

	lem.WriteFive(conn, lem.CMD_POKE, 1, 0xFFC5)
	MustWrite(conn, []byte{anything})

	lem.WriteFive(conn, lem.CMD_POKE, 1, 0xFFC2)
	MustWrite(conn, []byte{anything})

	lem.WriteFive(conn, lem.CMD_POKE, 1, 0xFFC0)
	MustWrite(conn, []byte{anything})

	lem.WriteFive(conn, lem.CMD_POKE, 1, 0xFF22)
	MustWrite(conn, []byte{0xC8})
}

func G6CMode(conn net.Conn) {

	lem.WriteFive(conn, lem.CMD_POKE, 1, 0xFFC5)
	MustWrite(conn, []byte{anything})

	lem.WriteFive(conn, lem.CMD_POKE, 1, 0xFFC3)
	MustWrite(conn, []byte{anything})

	lem.WriteFive(conn, lem.CMD_POKE, 1, 0xFFC0)
	MustWrite(conn, []byte{anything})

	lem.WriteFive(conn, lem.CMD_POKE, 1, 0xFF22)
	MustWrite(conn, []byte{0xE8})
}

func SetVdgScreenAddr(conn net.Conn, addr uint) {
	addr >>= 10
	reg := uint(0xFFC8)
	for i := 0; i < 7; i++ {
		lobit := addr & 1
		lem.WriteFive(conn, lem.CMD_POKE, 1, reg+lobit)
		MustWrite(conn, []byte{anything})
		reg += 2
		addr >>= 1
	}
}

const anything = 42
