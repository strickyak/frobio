package lib

import (
	"log"
)

// MidMux marks.
const MuxGoAhead = ':'
const MuxOK = ';'
const MuxNotYet = '~'

// EndMux marks.
const MuxGood = '+'
const MuxBad = '-'

type Mux struct {
}

func BeginMux(ses *Session, p uint, pay []byte) {
	log.Printf("BeginMux %04x (%v) BEGIN %q", p, ses, pay)
	ses.SendQuintAndPayload(CMD_MID_MUX, p, []byte{MuxOK})
}

func MidMux(ses *Session, p uint, pay []byte) {
	log.Printf("MidMux %04x (%v) %q", p, ses, pay)
	ses.SendQuintAndPayload(CMD_MID_MUX, p, []byte{MuxNotYet})
}

func EndMux(ses *Session, p uint, pay []byte) {
	log.Printf("EndMux %04x (%v) END %q", p, ses, pay)
	ses.SendQuintAndPayload(CMD_MID_MUX, p, []byte{MuxGood})
}
