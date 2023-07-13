package lib

import (
//"log"
)

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
