package coms

import (
	"io"
	"log"
	"net"

	. "github.com/strickyak/frobio/frob3/lemma/util"
)

const (
	CMD_POKE  = 0
	CMD_HELLO = 1 // Similar to Peek Data.
	CMD_CALL  = 255

	CMD_GETCHAR   = 192 // Blocking. request with n=0.  reply with n=0, p is char.
	CMD_KEYBOARD  = 193 // request with n=0.  reply with n=8.
	CMD_SUM       = 194 // request with n & p.  reply with sum in p.
	CMD_PEEK2     = 195 // request: n=4, p=FFFF (wanted n, wanted p)  reply: n, p, data.
	CMD_BEGIN_MUX = 196
	CMD_MID_MUX   = 197
	CMD_END_MUX   = 198

	CMD_LEVEL0 = 199 // when Level0 needs attention, it sends 199.

	CMD_LOG     = 200
	CMD_INKEY   = 201 // Nonblocking. request with n==0.  reply with n=0, p is char.
	CMD_PUTCHAR = 202
	CMD_PEEK    = 203
	CMD_DATA    = 204
	// CMD_SP_PC   = 205
	// CMD_REV     = 206

	CMD_BLOCK_READ     = 207 // block device
	CMD_BLOCK_WRITE    = 208 // block device
	CMD_BLOCK_ERROR    = 209 // nack
	CMD_BLOCK_OKAY     = 210 // ack
	CMD_BOOT_BEGIN     = 211 // boot_lemma
	CMD_BOOT_CHUNK     = 212 // boot_lemma
	CMD_BOOT_END       = 213 // boot_lemma
	CMD_LEMMAN_REQUEST = 214 // LemMan
	CMD_LEMMAN_REPLY   = 215 // LemMan

	CMD_RTI           = 216
	CMD_ECHO          = 217 // reply with CMD_DATA, with high bits toggled.
	CMD_DW            = 218
	CMD_HDBDOS_SECTOR = 219 // Normal HDBDOS block I/O.
	CMD_HDBDOS_EXEC   = 220 // Slip in sideloaded pages via EXEC records.
	CMD_HDBDOS_HIJACK = 221 // CLEAR key hijacks the machine from BASIC.
)

type Comm struct {
	Conn net.Conn
}

func Wrap(conn net.Conn) *Comm {
	return &Comm{conn}
}

type Quint [5]byte // Quintabyte commands.

func NewQuint(cmd byte, n uint, p uint) Quint {
	var q Quint
	q[0] = cmd
	q[1], q[2] = Hi(n), Lo(n)
	q[3], q[4] = Hi(p), Lo(p)
	return q
}

func (q Quint) Command() byte {
	return q[0]
}
func (q Quint) N() uint {
	return HiLo(q[1], q[2])
}
func (q Quint) P() uint {
	return HiLo(q[3], q[4])
}

/* TODO

func ReadQuint(conn net.Conn) []bytes {
  var q Quint
  _, err := io.ReadFull(conn, q[:])
  Check(err)
  n := q.N()
  bb := make([]byte, n)
  _, err = io.ReadFull(conn, bb)
  Check(err)
}
TODO */

func (o *Comm) ReadQuintAndPayload() (cmd byte, p uint, bb []byte) {
	var q Quint
	Value(io.ReadFull(o.Conn, q[:]))
	cmd, p = q[0], q.P()

	bb = make([]byte, q.N())
	Value(io.ReadFull(o.Conn, bb))
	return
}

func (o *Comm) WriteQuintAndPayload(cmd byte, p uint, bb []byte) {
	n := len(bb)
	q := NewQuint(cmd, uint(n), p)
	Value(o.Conn.Write(q[:]))
	Value(o.Conn.Write(bb))
}

func (o *Comm) ReadFull(bb []byte) {
	n := Value(io.ReadFull(o.Conn, bb))
	if n != len(bb) {
		log.Panicf("ReadFull: short read: got %d want %d bytes", n, len(bb))
	}
}

func (o *Comm) CallAddr(addr uint) {
	o.WriteQuintAndPayload(CMD_CALL, addr, nil)
}

func (o *Comm) PokeRam(addr uint, bb []byte) {
	o.WriteQuintAndPayload(CMD_POKE, addr, bb)
}

func (o *Comm) PeekRam(addr uint, n uint) []byte {
	// Anomolous quint: payload is empty, not n.
	_, err := o.Conn.Write([]byte{CMD_PEEK, Hi(n), Lo(n), Hi(addr), Lo(addr)}) // == WriteFull
	if err != nil {
		panic(err)
	}

	cmd, p, bb := o.ReadQuintAndPayload()
	AssertEQ(cmd, CMD_DATA)
	AssertEQ(p, addr)
	AssertEQ(LenSlice(bb), n)
	return bb
}

func (o *Comm) Unwrap() net.Conn {
	return o.Conn
}
