package os9

import (
	"github.com/strickyak/tcl67/extra"
	. "github.com/strickyak/tcl67/tcl"
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

func cmdOs9ScanModules(fr *Frame, argv []T) T {
	a := Arg1(argv)
	b := []byte(a.String())

	var z []T
	for i := 0; i < len(b)-12; i++ {
		if b[i] == 0x87 && b[i+1] == 0xCD {
			name, n := parseModule(b[i:])
			if n > 0 {
				z = append(z, extra.MkBox(name, MkString(string(b[i:i+n]))))
				i += n - 1
			}
		}
	}

	return MkList(z)
}

var os9Ensemble = []EnsembleItem{
	{Name: "scanmodules", Cmd: cmdOs9ScanModules},
}

func init() {
	if Safes == nil {
		Safes = make(map[string]Command, 333)
	}

	Safes["os9"] = MkEnsemble(os9Ensemble)
}
