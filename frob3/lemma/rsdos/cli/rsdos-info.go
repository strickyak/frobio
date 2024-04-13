package main

import "github.com/strickyak/frobio/frob3/lemma/rsdos"

import (
	"fmt"
	"io"
	"os"
)

func main() {
	contents, err := io.ReadAll(os.Stdin)
	if err != nil {
		panic(err)
	}
	d := rsdos.DiskParse(contents)

/*
	switch d.(type) {
	case *RSDiskRec:
	fmt.Printf("NumTracks %d\n", d.NumTracks)
	fmt.Printf("TotalGranules %d\n", d.TotalGranules)
	fmt.Printf("FreeGranules %d\n", d.FreeGranules)
	for _, e := range d.Files {
		ct := "binary"
		if e.IsAscii {
			ct = "ascii"
		}
		tn := rsdos.TypeNames[e.FileType]
		fmt.Printf("File=%q Len=%d Type=%s : %s\n", e.FileName, e.Size, tn, ct)
	}

	case *Os9DiskRec:
*/

	fmt.Printf("VolumeName %q\n", d.VolumeName())
	fmt.Printf("OS %q\n", d.OS())
	fmt.Printf("Free: %s\n", d.Free())
	fmt.Printf("Codes: %s\n", d.Codes())
}
