package canvas

import (
	"log"
	"net"

	lem "github.com/strickyak/frobio/frob2/waiter"
)

const warp = true

const (
	W  = 128 // screen width, height
	H  = 192
	BW = 128 / 4 // ram byte width, height
	BH = 192
)

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
		x %= W
		y %= H
	} else if x < 0 || x >= W || y < 0 || y >= H {
		return 0
	}

	bx := x / 4
	shift := 2 * (3 - (byte(x) & 3))

	return 3 & (can.bm[y*BW+bx] >> shift)
}

func (can *Canvas) Set(x, y int, c Color) {
	if warp {
		x %= W
		y %= H
	} else if x < 0 || x >= W || y < 0 || y >= H {
		return
	}

	bx := x / 4
	shift := 2 * (3 - (byte(x) & 3))
	mask := byte(0xFF) ^ (byte(3) << shift)

	can.bm[y*BW+bx] = (can.bm[y*BW+bx] & mask) | (c << shift)
}

func (can *Canvas) Render(conn net.Conn) {
	lem.WriteFive(conn, lem.CMD_POKE, BW*BH, 0x400)
	_, err := conn.Write(can.bm[:])
	if err != nil {
		log.Panicf("Canvas::Render: stopping due to error: %v", err)
	}
}
