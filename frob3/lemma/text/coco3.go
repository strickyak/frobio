package text

import (
	"github.com/strickyak/frobio/frob3/lemma/comm"
)

const (
	SimpleBlack = iota
	SimpleRed
	SimpleGreen
	SimpleBlue
	SimpleYellow
	SimpleMagenta
	SimpleCyan
	SimpleWhite
)

func SetSimplePalette(com *comm.Comm) {
	com.PokeRam(0xFFB0, []byte{
		// Background Text:
		RGB(0, 0, 0), // 0 = black
		RGB(3, 0, 0), // 1 = red
		RGB(0, 3, 0), // 2 = green
		RGB(0, 0, 3), // 3 = blue
		RGB(3, 3, 0), // 4 = yellow
		RGB(2, 0, 2), // 5 = magenta
		RGB(0, 2, 2), // 6 = cyan
		RGB(3, 3, 3), // 7 = white

		// Foreground Text:
		RGB(0, 0, 0), // 0 = black
		RGB(3, 0, 0), // 1 = red
		RGB(0, 3, 0), // 2 = green
		RGB(0, 0, 3), // 3 = blue
		RGB(3, 3, 0), // 4 = yellow
		RGB(2, 0, 2), // 5 = magenta
		RGB(0, 2, 2), // 6 = cyan
		RGB(3, 3, 3), // 7 = white
	})
}

func UndoSimplePalette(com *comm.Comm, savedPalette []byte) {
	com.PokeRam(0xFFB0, savedPalette)
}

func FgBg(fg, bg byte) byte {
	return ((fg & 7) << 3) | (bg & 7)
}

func GimeDisplayCode(b byte) byte {
	switch {
	case b == 0x5E: // ascii hat
		return 0x60 // coco3 hat
	case b == 0x5F: // ascii under
		return 0x7F // coco3 under
	case b == 0x60: // ascii hat
		return 0x1E // coco3 degrees
	case b > 127:
		return 0x1F // forte 'f'
	default:
		return b
	}
}

func RGB(r, g, b byte) byte {
	var z byte
	if (r & 2) != 0 {
		z |= 0x20
	}
	if (g & 2) != 0 {
		z |= 0x10
	}
	if (b & 2) != 0 {
		z |= 0x08
	}
	if (r & 1) != 0 {
		z |= 0x04
	}
	if (g & 1) != 0 {
		z |= 0x02
	}
	if (b & 1) != 0 {
		z |= 0x01
	}
	return z
}
