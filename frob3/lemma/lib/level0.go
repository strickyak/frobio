package lib

import (
	"bytes"
	"flag"
	"io/ioutil"
	"log"
	"path/filepath"
)

var FlagL0ModuleDirPath = flag.String("l0_module_dir", "", "where to find commands for Level0")

const (
	Sys_Data = 0x7E00 // and upward
	Sys_Load = 0x7000 // and upward

	Usr_Load  = 0x3000 // and upward
	Usr_Stack = 0x2FF0 // and downward
	Usr_Data  = 0x0800 // and upward
)

const ( // state:
	Ready   = 1
	Waiting = 2
	Blocked = 3
)

type word uint16

type L0Proc struct {
	pid   word
	ppid  word
	stack word
	data  word // data address
	uid   word
	state word

	module *L0Module
	params []byte

	files [16]*L0File
}

type L0File struct {
	num  word
	name string
}

type L0Module struct {
	name     string
	filename string
	contents []byte
}

var moduleDir map[string]*L0Module
var nextPid = word(0)

func FindModule(name string) (*L0Module, error) {
	m, ok := moduleDir[name]
	if !ok {
		return m, Errorf("E_MNF cannot find module %q", name)
	}
	return m, nil
}

func Launch(module string, params string, parent *L0Proc) (*L0Proc, error) {
	mod, err := FindModule(module)
	if mod == nil {
		return nil, err
	}
	nextPid++
	p := &L0Proc{
		pid:   nextPid,
		ppid:  Cond(parent == nil, 0, parent.pid),
		stack: Usr_Stack,
		data:  Usr_Data,
		state: Ready,

		module: mod,
	}
	return p, nil
}

func ExtractName(bb []byte, i int, filename string) string {
	var buf bytes.Buffer
	for {
		ch := 127 & bb[i]
		AssertLE('-', ch, filename, i, ch)
		AssertLE(ch, 'z', filename, i, ch)
		buf.WriteByte(127 & bb[i])
		if 128 <= bb[i] {
			break
		}
		i++
	}
	return buf.String()
}

func AttemptToLoadModule(filename string) {
	bb, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Fatalf("cannot Read module file %q: %v", filename, err)
	}
	if len(bb) > 12 && bb[0] == 0x87 && bb[1] == 0xCD {
		hlen := int(bb[2])*256 + int(bb[3])
		if hlen != len(bb) {
			log.Fatal("Bad module length: header wants %d, got %d: %q", hlen, len(bb), filename)
		}
		hname := int(bb[4])*256 + int(bb[5])
		name := ExtractName(bb, hname, filename)

		moduleDir[name] = &L0Module{
			name:     name,
			filename: filename,
			contents: bb,
		}
		log.Printf("Loaded module %q len %d.", filename, hlen)
	} else {
		log.Printf("Not a module %q (wrong header)", filename)
	}
}

func L0Init() {
	moduleDir = make(map[string]*L0Module)
	if *FlagL0ModuleDirPath != "" {
		matches, err := filepath.Glob(filepath.Join(*FlagL0ModuleDirPath, "*"))
		log.Printf("matches: %v", matches)
		if err != nil {
			log.Fatalf("Error globbing l0_module_dir %q: %v", *FlagL0ModuleDirPath, err)
		}
		for _, m := range matches {
			log.Printf("attempt: %v", m)
			AttemptToLoadModule(m)
		}
	}
}
