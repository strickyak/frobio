package lib

import (
	"log"
	"time"
)

// MidMux marks.
const MuxGoAhead = ':'
const MuxOK = ';'
const MuxNotYet = '~'
const MuxEnough = '.'

// EndMux marks.
const MuxGood = '+'
const MuxBad = '-'

var nextMux uint

func NextMux() uint {
	nextMux++
	return nextMux
}

type Mux struct {
	demo int
}

func BeginMux(ses *Session, p uint, pay []byte) {
	log.Printf("BeginMux %04x (%v) BEGIN %q", p, ses, pay)

	ses.Muxen[p] = &Mux{
		demo: 5,
	}

	ses.SendQuintAndPayload(CMD_MID_MUX, p, []byte{MuxOK})
}

func MidMux(ses *Session, p uint, pay []byte) {
	log.Printf("MidMux %04x (%v) %q", p, ses, pay)

	mux, ok := ses.Muxen[p]
	if !ok {
		bad := Format("mux not found: %u", p)
		ses.SendQuintAndPayload(CMD_MID_MUX, p, []byte(bad))
	}

	var reply string
	if mux.demo == 5 {
		reply = Format("azzz%v\000", time.Now().UnixMilli())
	} else if mux.demo > 1 {
		reply = Format("1Number %d\r\000", mux.demo)
	} else if mux.demo == 1 {
		reply = "c"
	} else {
		reply = Format(".Enough")
	}
	if mux.demo > 0 {
		mux.demo--
	}

	ses.SendQuintAndPayload(CMD_MID_MUX, p, []byte(reply))
}

func EndMux(ses *Session, p uint, pay []byte) {
	log.Printf("EndMux %04x (%v) END %q", p, ses, pay)

	_, ok := ses.Muxen[p]
	if !ok {
		bad := Format("mux not found: %u", p)
		ses.SendQuintAndPayload(CMD_MID_MUX, p, []byte(bad))
	}
	delete(ses.Muxen, p)

	ses.SendQuintAndPayload(CMD_MID_MUX, p, []byte("+ok"))
}
