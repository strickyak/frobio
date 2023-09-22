package lib

import (
	"bytes"
	"fmt"
	"log"
)

func DumpHexLines(label string, offset uint, bb []byte) {
	for i := uint(0); i < uint(len(bb)); i += 32 {
		n := Min(uint(len(bb))-i, 32)
		DumpHexLine(fmt.Sprintf("%s $%04x: ", label, offset+i), bb[i:i+n])
	}
}

func DumpHexLine(label string, bb []byte) {
	var buf bytes.Buffer
	buf.WriteString(label)
	for i, b := range bb {
		if i&1 == 0 {
			buf.WriteByte(' ')
		}
		fmt.Fprintf(&buf, "%02x", b)
	}
	buf.WriteRune(' ')
	for _, b := range bb {
		c := b & 127
		if ' ' <= c && c <= '~' {
			buf.WriteByte(c)
		} else {
			buf.WriteByte('.')
		}
	}
	log.Print(buf.String())
}
