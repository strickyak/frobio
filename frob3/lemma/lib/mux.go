package lib

import (
	"bufio"
	"bytes"
	"flag"
	"io"
	"log"
	"os"
	P "path"
	"runtime/debug"
	"sort"
	"strings"
	"time"
)

var FS = flag.String("fs", "", "Danger: Filesystem root that can be read and written remotely")

// MidMux marks.
const MuxGoAhead = ':'
const MuxOK = ';'
const MuxNotYet = '~'
const MuxEnough = '.'

// EndMux marks.
const MuxGood = '+'
const MuxBad = '-'

type TaggedBytes struct {
	Tag   byte
	Bytes []byte
}

type Proc struct {
	Count   int
	Command string
	Argv    []string
	Ses     *Session
	Channel uint

	InPacket chan TaggedBytes
}

func BeginMux(ses *Session, p uint, pay []byte) {
	// Reply to BEGIN with END{-...} if the session cannot start.
	// Reply to BEGIN with BEGIN{+} if session starts.
	log.Printf("BeginMux $%04x (%v) BEGIN %q", p, ses, pay)

	{ // Error if channel p already exists.
		_, exists := ses.Procs[p]
		if exists {
			ses.ReplyOnChannel(CMD_END_MUX, p, BytesFormat("- Channel %d already exists in session %v", p, ses))
			return
		}
	}

	argv := strings.Fields(string(pay))
	for len(argv) > 0 && len(argv[0]) > 0 && argv[0][0] == '(' {
		log.Printf("Remove front comment: %q", argv[0])
		argv = argv[1:]
	}
	log.Printf("argv=%#v", argv)
	if len(argv) == 0 {
		ses.ReplyOnChannel(CMD_END_MUX, p, BytesFormat("- Empty Command"))
		return
	}

	cname := strings.ToLower(argv[0])
	fn, ok := CommandFuncs[cname]
	if !ok {
		ses.ReplyOnChannel(CMD_END_MUX, p, BytesFormat("- No such command: %v", cname))
		return
	}

	proc := &Proc{
		Command:  string(pay),
		Argv:     argv,
		Ses:      ses,
		InPacket: make(chan TaggedBytes, 0), // currently protocol is synchronous.
		Channel:  p,
	}
	ses.Procs[p] = proc

	// Start the command in the background.
	// It will block reading InPacket until our following ReplyOnChannel
	// is called, and the client gets that, and then sends a MID request.
	go proc.RunCommandFunc(fn)

	ses.ReplyOnChannel(CMD_BEGIN_MUX, p, []byte{MuxGood})
}

func MidMux(ses *Session, p uint, pay []byte) {
	log.Printf("MidMux $%04x (%v) %q", p, ses, pay)

	proc, ok := ses.Procs[p]
	if !ok {
		ses.ReplyOnChannel(CMD_MID_MUX, p,
			BytesFormat("- Proc %d not found in session %v", p, ses))
		return
	}

	proc.InPacket <- TaggedBytes{CMD_MID_MUX, pay}
}

func EndMux(ses *Session, p uint, pay []byte) {
	log.Printf("EndMux %04x (%v) END %q", p, ses, pay)

	proc, ok := ses.Procs[p]
	if !ok {
		ses.ReplyOnChannel(CMD_END_MUX, p,
			BytesFormat("- Proc %d not found in session %v", p, ses))
		return
	}

	// Send the final packet with type CMD_END_MUX
	// and shut down the channel.  The goroutine should either be
	// expecting the CMD_END_MUX packet, or it will be panicked
	// as a KILL signal.
	proc.InPacket <- TaggedBytes{CMD_END_MUX, pay}
	close(proc.InPacket)
	delete(ses.Procs, p)
}

func (o *Proc) LowGetOne(expected byte) []byte {
	// TODO: some timeout to kill orphans (like for dropped connections).

	var packet TaggedBytes
	select {
	case _p, ok := <-o.InPacket:
		if !ok {
			debug.PrintStack()
			log.Panicf("CLOSED_CHAN Proc %d", o.Channel)
		}
		packet = _p
	case <-time.After(500 * time.Second):
		o.InPacket = nil // Garbagize the chan.
		debug.PrintStack()
		log.Panicf("TIMEOUT Proc %d", o.Channel)
	}

	log.Printf("LowGetOne: %d:%q <-", packet.Tag, packet.Bytes)

	// How kill signal is received.
	if expected == CMD_MID_MUX && packet.Tag == CMD_END_MUX {
		debug.PrintStack()
		log.Panicf("KILL Proc %d: %q", o.Channel, packet.Bytes)
	}

	// Otherwise the expected should match the command byte.
	if packet.Tag != expected {
		debug.PrintStack()
		log.Panicf("GetOneLow: got type %d, expected %d", packet.Tag, expected)
	}

	// Return just the payload (with 1 subcommand character).
	return packet.Bytes
}

func (o *Proc) LowPutOne(q Quint, args ...any) {
	var bb bytes.Buffer
	bb.Write(q[:])
	for _, a := range args {
		switch t := a.(type) {
		case byte:
			bb.WriteByte(t)
			// bb.Write([]byte{t})
		case []byte:
			bb.Write(t)
		case string:
			bb.WriteString(t)
		default:
			debug.PrintStack()
			log.Panicf("LowPutOnePANIC (%T) %#v", a, a)
			bb.WriteString(Format("%v", t))
		}
	}
	z := bb.Bytes()
	log.Printf("LowPutOne: %02x %02x %02x %02x %02x    [$%x]%q", q[0], q[1], q[2], q[3], q[4], len(z)-5, z[5:])
	debug.PrintStack()
	cc, err := o.Ses.Conn.Write(z)
	if err != nil {
		debug.PrintStack()
		log.Panicf("Cannot Write: Proc %d: %v", o.Channel, err)
	}
	if cc != len(z) {
		debug.PrintStack()
		log.Panicf("Short Write: got %d want %d: Proc %d", cc, len(z), o.Channel)
	}
}

func (o *Proc) ReadLine() string {
	return "TODO"
}
func (o *Proc) Read(buf []byte) {
	copy(buf, "TODO")
	return /*"TODO"*/
}

func (o *Proc) CocoError(message string) {
	o.LowGetOne(CMD_MID_MUX)
	q := NewQuint(CMD_MID_MUX, uint(1+len(message)), o.Channel)
	o.LowPutOne(q, "3", message)
}
func (o *Proc) CocoCreate(filename string) {
	o.LowGetOne(CMD_MID_MUX)
	q := NewQuint(CMD_MID_MUX, uint(1+len(filename)), o.Channel)
	o.LowPutOne(q, "a", filename)
}
func (o *Proc) CocoOpen(filename string) {
	o.LowGetOne(CMD_MID_MUX)
	q := NewQuint(CMD_MID_MUX, uint(1+len(filename)), o.Channel)
	o.LowPutOne(q, "b", filename)
}
func (o *Proc) CocoClose() {
	o.LowGetOne(CMD_MID_MUX)
	q := NewQuint(CMD_MID_MUX, uint(1), o.Channel)
	o.LowPutOne(q, "c")
}

func (o *Proc) PrintfStderr(format string, args ...any) {
	line := Format(format, args...)
	o.LowGetOne(CMD_MID_MUX)
	q := NewQuint(CMD_MID_MUX, uint(1+len(line)), o.Channel)
	o.LowPutOne(q, "2", line)
}
func (o *Proc) WriteLine(line string) {
	o.LowGetOne(CMD_MID_MUX)
	q := NewQuint(CMD_MID_MUX, uint(1+len(line)), o.Channel)
	o.LowPutOne(q, "1", line)
}
func (o *Proc) WriteBytes(buf []byte) {
	o.LowGetOne(CMD_MID_MUX)
	q := NewQuint(CMD_MID_MUX, uint(1+len(buf)), o.Channel)
	o.LowPutOne(q, "4", buf)
}

func (o *Proc) Exit3(verdict string) {
	channel := o.Channel
	AssertGT(channel, 0)
	o.LowGetOne(CMD_MID_MUX)
	q1 := NewQuint(CMD_MID_MUX, 1, channel)
	o.LowPutOne(q1, ".")

	o.LowGetOne(CMD_END_MUX)
	q2 := NewQuint(CMD_END_MUX, uint(len(verdict)), channel)
	o.LowPutOne(q2, verdict)
}

type CommandFunc func(*Proc) string

func (o *Proc) RunCommandFunc(fn CommandFunc) {
	AssertGT(o.Channel, 0)
	message := ""

	defer func() {
		r := recover()
		if r != nil {
			o.Exit3(Format("-255 caught exception: %v", r))
		} else {
			o.Exit3(message)
		}
	}()

	message = fn(o)
}

var CommandFuncs = make(map[string]CommandFunc)

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

func HelpCommand(o *Proc) string {
	var names []string
	for k := range CommandFuncs {
		names = append(names, k)
	}
	sort.Strings(names)
	for _, v := range names {
		o.WriteLine(v)
	}
	return "+ok"
}

func EchoCommand(o *Proc) string {
	var bb bytes.Buffer
	for i, a := range o.Argv[1:] {
		if i > 0 {
			bb.WriteByte(' ')
		}
		bb.WriteString(a)
	}
	bb.WriteByte('\n')
	o.WriteLine(bb.String())
	return "+ok"
}

func VerticalEchoCommand(o *Proc) string {
	for _, a := range o.Argv[1:] {
		o.WriteLine(Format("%s\n", a))
	}
	return "+ok"
}

func PutTCommand(o *Proc) string {
	if *FS == "" {
		log.Panicf("No filesystem on remote server (use --fs=<topdir>)")
	}

	var src, dest string
	switch len(o.Argv) {
	default:
		log.Panicf("Usage:  putt local_src remote_dest")

	case 2:
		dest = P.Clean(o.Argv[1])
		src = P.Base(dest)

	case 3:
		src = P.Clean(o.Argv[1])
		dest = P.Clean(o.Argv[2])
	}

	longDest := P.Join(*FS, dest)
	w, err := os.Create(longDest)
  if err != nil {
    log.Panicf("remote cannot create %q: %v", dest, err)
  }
	defer w.Close()

	o.CocoOpen(src)
	error_packet := o.LowGetOne(CMD_MID_MUX)
	if len(error_packet) > 0 {
		log.Panicf("Did not local open: %q", src)
	}

	for {
		q := NewQuint(CMD_MID_MUX, 1, o.Channel)
		o.LowPutOne(q, byte('6'))

		packet := o.LowGetOne(CMD_MID_MUX)
		if len(packet) == 0 {
			break
		} else if packet[0] == '6' && len(packet) == 1 {
			break
		} else if packet[0] == '6' {
			for i, e := range packet {
				if e == '\r' {
					packet[i] = '\n'
				}
			}
			nb, err := w.Write(packet[1:])
			if err != nil {
				log.Panicf("Error writing %q: %v", dest, err)
			}
			if nb != len(packet)-1 {
				log.Panicf("Short Write to %q: %v", dest, err)
			}
			continue
		} else {
			log.Panicf("Error: %q", packet)
		}
	}
	q9 := NewQuint(CMD_MID_MUX, 1, o.Channel)
	o.LowPutOne(q9, byte('c')) // Close.
	// yak //

	return Format("+ PutT %q to %q", src, dest)
}

func GetTCommand(o *Proc) string {
	if *FS == "" {
		log.Panicf("No filesystem on remote server (use --fs=<topdir>)")
	}

	var src, dest string
	switch len(o.Argv) {
	default:
		log.Panicf("Usage:  gett remote_src local_dest")

	case 2:
		src = P.Clean(o.Argv[1])
		dest = P.Base(src)

	case 3:
		src = P.Clean(o.Argv[1])
		dest = P.Clean(o.Argv[2])
	}

	longSrc := P.Join(*FS, src)
	rf, err := os.Open(longSrc)
  if err != nil {
    log.Panicf("remote cannot Open %q: %v", src, err)
  }
	defer rf.Close()
	r := bufio.NewReader(rf)

	o.CocoCreate(dest)

	for {
		// TODO -- handle either \n or \r
		line, isPrefix, err := r.ReadLine()
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Panicf("Error reading %q: %v", src, err)
		}
		if isPrefix {
			log.Panicf("Error reading long line in %q", src)
		}
		line = append(line, '\r')
		o.WriteLine(string(line))
	}
	o.CocoClose()
	return "+ok"
}

func GetBCommand(o *Proc) string {
	if *FS == "" {
		log.Panicf("No filesystem on remote server (use --fs=<topdir>)")
	}

	var src, dest string
	switch len(o.Argv) {
	default:
		log.Panicf("Usage:  getb remote_src local_dest")

	case 2:
		src = P.Clean(o.Argv[1])
		dest = P.Base(src)

	case 3:
		src = P.Clean(o.Argv[1])
		dest = P.Clean(o.Argv[2])
	}

	longSrc := P.Join(*FS, src)
	rf, err := os.Open(longSrc)
  if err != nil {
    log.Panicf("remote cannot Open %q: %v", src, err)
  }
	defer rf.Close()
	r := bufio.NewReader(rf)

	o.CocoCreate(dest)
	log.Printf("getb: Created: %q", dest)

	buf := make([]byte, 128)
	for {
		nc, err := r.Read(buf)
		log.Printf("getb: Read: %d. bytes", len(buf))

		if err == io.EOF {
			break
		}
		if err != nil {
			log.Printf("- getb: Error reading %q: %v", src, err)

			return Format("- getb: Error reading %q: %v", src, err)
		}

		log.Printf("getb: Writing: %d. bytes", nc)
		o.WriteBytes(buf[:nc])
	}
	o.CocoClose()
	log.Printf("getb: Closed: %q", dest)
	return "+ok"
}

func init() {
	CommandFuncs["help"] = HelpCommand
	CommandFuncs["echo"] = EchoCommand
	CommandFuncs["vecho"] = VerticalEchoCommand
	CommandFuncs["gett"] = GetTCommand
	CommandFuncs["getb"] = GetBCommand
	CommandFuncs["putt"] = PutTCommand
}
