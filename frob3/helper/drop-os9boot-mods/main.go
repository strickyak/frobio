// drop-os9boot-mods

package main

import (
    "bytes"
    "log"
    "flag"
    "strings"
    "os"
    "io/ioutil"
)

var ModulesToOmit = make(map[string]bool)

func Word(hi byte, lo byte) int {
    return int((uint(hi)<<8) | uint(lo))
}

func NameAt(bb []byte, i int) string {
    var buf bytes.Buffer
    for {
        c := bb[i]
        buf.WriteByte(c&127)
        if (c&128) == 128 {
            break
        }
        i++
    }
    return strings.ToUpper(buf.String())
}

func RememberModulesToOmit() {
    for _, m := range flag.Args() {
        m = strings.ToUpper(m)
        ModulesToOmit[m] = true
    }

}

func main() {
    flag.Parse()

    RememberModulesToOmit()
    log.Printf("omit => %v", ModulesToOmit)

    bb, err := ioutil.ReadAll(os.Stdin)
    if err != nil {
        panic(err)
    }
    log.Printf("input len => %d", len(bb))

    i := 0
    for i < len(bb) {
        if bb[i] != 0x87 || bb[i+1] != 0xCD {
            log.Panicf("Bad magic number at position %d", i)
        }
       modLen := Word(bb[i+2], bb[i+3])
       namePtr := Word(bb[i+4], bb[i+5])
       name := NameAt(bb, i + namePtr)

       if _, ok := ModulesToOmit[name]; ok {
         // omit
         log.Printf("OMIT %q", name)
       } else {
         // keep
         _, err := os.Stdout.Write(bb[i : i+modLen])
         if err != nil {
            log.Panicf("Short Write")
         }
         log.Printf("keep %q", name)
       }

       i += modLen
    }
}
