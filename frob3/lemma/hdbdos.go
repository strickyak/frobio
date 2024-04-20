package lemma

import (
	"bytes"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	PFP "path/filepath"
	"strings"
	"time"

	"github.com/strickyak/frobio/frob3/lemma/comm"
	"github.com/strickyak/frobio/frob3/lemma/finder"
	T "github.com/strickyak/frobio/frob3/lemma/text"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var FlagSideloadRaw = flag.String("sideload_raw", "sideload.lwraw", "raw machine code fragment for HdbDos")
var FlagInkeyRaw = flag.String("inkey_raw", "inkey_trap.lwraw", "raw machine code fragment for HdbDos")

type HdbDosSession struct {
	Ses      *Session
	NumReads int64 // When 1, send injections.

	DriveSession *finder.DriveSession
}

func UndoAlterTask0(ses *Session, saved byte) {
	PokeRam(ses.Conn, 0xFFA1, []byte{saved}) // Restore $2000 with to page $39
}
func AlterTask0Map37To2000ReturnSaved(ses *Session, payload []byte) byte {
	saved := payload[256+1]
	PokeRam(ses.Conn, 0xFFA1, []byte{0x37}) // Replace $2000 with our page $37
	return saved
}

// extraVerticalGap may be 0, 1, 2, or 3.
// Returns number of full rows visibile, up to 24.
func GimeText40x24_OnPage37_ReturnNumRows(ses *Session, payload []byte, extraVerticalGap byte) int {
	AssertLE(extraVerticalGap, 3)
	// Set screen to 40-char width mode.
	// $DC00 for vertical offset ($FF9[DE]) means unused MMU page $37.
	PokeRam(ses.Conn, 0xFF90, []byte{0x4c})
	PokeRam(ses.Conn, 0xFF98, []byte{3 + extraVerticalGap, 0x05, 0x00, 0x00, 0x00, 0xDC, 0, 0})
	// Use mapping $37 (unused by BASIC) for our screen buffer,
	// in Task0 to $2000.
	switch extraVerticalGap {
	case 0:
		return 24
	case 1:
		return 21
	case 2:
		return 18
	case 3:
		return 17
	}
	panic(0)
}

func PeekPage0(ses *Session) []byte {
	return Peek2Ram(ses.Conn, 0x0000, 256)
}
func VideoModes(page0 []byte) (hrmode, hrwidth, pmode byte) {
	hrmode, hrwidth, pmode = page0[0xE6], page0[0xE7], page0[0xB6]
	return
}
func DescribeVideoModes(page0 []byte) (str string, byt []byte) {
	hrmode, hrwidth, pmode := VideoModes(page0)
	str = fmt.Sprintf("( hrmode=%02x, hrwidth=%02x, pmode=%02x )", hrmode, hrwidth, pmode)
	byt = make([]byte, 80)
	for i, _ := range byt {
		byt[i] = 7
	}
	for i, ch := range str {
		byt[2*i] = byte(ch)
	}
	return
}

func TestCharRow40(nthRow byte) []byte {
	bb := make([]byte, 2*40)
	for i := byte(0); i < 40; i++ {
		bb[2*i] = (nthRow * 29) + i
		bb[2*i+1] = ((i % 7) << 3) | T.SimpleBlack
		if bb[2*i+1] == 077 {
			bb[2*i+1] = 7
		}
	}
	bb[0] = 'A' + byte(nthRow)
	return bb
}

func OnText40At2000Run(ses *Session, payload []byte, runMe func()) {
	com := comm.Wrap(ses.Conn)

	savedPage0 := payload[:256]
	// _ = payload[256 : 256+16] // savedMMU
	savedPalette := payload[256+16 : 256+32]
	hrmode, hrwidth, pmode := VideoModes(savedPage0)
	strV, bytV := DescribeVideoModes(savedPage0)

	log.Printf("Incoming Video Modes: %s", strV)

	numRows := GimeText40x24_OnPage37_ReturnNumRows(ses, payload, 0)
	log.Printf("OnText40At2000Run: %d. complete rows", numRows)
	savedMMUByte := AlterTask0Map37To2000ReturnSaved(ses, payload)

	T.SetSimplePalette(com)

	for row := byte(0); row < 24; row++ {
		bb := TestCharRow40(row)
		for i := byte(0); i < 40; i++ { // invert some bg/fg
			if uint(row+i)%13 == 9 { // choose which ones
				x := bb[i+i+1]
				x = (x >> 3) | (T.SimpleBlack << 3) // the inversion, assuming bg is T.SimpleBlack.
				bb[i+i+1] = x
			}
		}
		PokeRam(ses.Conn, 0x2000+40*2*uint(row), Cond(row == 0, bytV, bb))
	}

	defer func() {
		SetVideoMode(ses, hrmode, hrwidth, pmode)
		T.UndoSimplePalette(com, savedPalette)
		UndoAlterTask0(ses, savedMMUByte)
	}()

	runMe()
}

type VideoSettings struct {
	Major byte
	Minor [8]byte
}

func (vs *VideoSettings) Push(ses *Session) {
	PokeRam(ses.Conn, 0xFF90, []byte{vs.Major})
	PokeRam(ses.Conn, 0xFF98, vs.Minor[:])
}

var VdgText32Video = &VideoSettings{0xCC, [8]byte{0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0, 0}}
var GimeText40Video = &VideoSettings{0x4C, [8]byte{0x03, 0x05, 0x12, 0x00, 0x0F, 0xD8, 0, 0}}
var GimeText80Video = &VideoSettings{0x4C, [8]byte{0x03, 0x15, 0x12, 0x00, 0x0F, 0xD8, 0, 0}}

func SetVideoMode(ses *Session, hrmode, hrwidth, pmode byte) {
	// Restore COCO VDG text mode. TODO use modes.
	switch hrwidth {
	case 0: // VDG Text 32
		VdgText32Video.Push(ses)
	case 1: // Gime Text 40
		GimeText40Video.Push(ses)
	case 2: // Gime Text 80
		GimeText80Video.Push(ses)
	}
}

func CreateHomeIfNeeded(ses *Session) (string, string) {
	root := *finder.FlagNavRoot
	homes := "HOMES"
	home := PFP.Join(homes, strings.ToUpper(ses.Hostname))
	retain := PFP.Join(home, finder.RETAIN_10_DAYS)

	os.Mkdir(root, finder.DirPerm)
	os.Mkdir(PFP.Join(root, homes), finder.DirPerm)
	os.Mkdir(PFP.Join(root, home), finder.DirPerm)
	os.Mkdir(PFP.Join(root, retain), finder.DirPerm)
	Log("CreateHomeIfNeeded: home = %q", home)
	return home, retain
}

// Entry from waiter.go for a Hijack.
func HdbDosHijack(ses *Session, payload []byte) {
	com := comm.Wrap(ses.Conn)

	OnText40At2000Run(ses, payload, func() {
		time.Sleep(1 * time.Second)
		TextChooserShell(com, ses.HdbDos.DriveSession)
	})

	// We tell coco that this is the END of the HIJACK.
	com.WriteQuint(comm.CMD_HDBDOS_HIJACK, 0, []byte{})
}

// Entry from waiter.go for a Sector.
func HdbDosSector(ses *Session, payload []byte) {
	home, retain := CreateHomeIfNeeded(ses)
	com := comm.Wrap(ses.Conn)
	if ses.HdbDos == nil {
		ses.HdbDos = &HdbDosSession{
			Ses: ses,
			DriveSession: finder.NewDriveSession(
				comm.Wrap(ses.Conn),
				home, retain),
		}
	}
	ses.Cleanups = append(ses.Cleanups, func() {
		ses.HdbDos.DriveSession.HdbDosCleanup()
	})

	h := ses.HdbDos
	cmd := payload[0]
	lsn3 := uint(payload[1])
	lsn2 := uint(payload[2])
	lsn1 := uint(payload[3])
	lsn0 := uint(payload[4])
	lsnBig := (lsn3 << 24) | (lsn2 << 16) | (lsn1 << 8) | lsn0

	drive := lsnBig / 630
	lsn := lsnBig % 630
	front := 256 * lsn
	back := front + 256

	log.Printf("HdbDos cmd=%x drive=%x lsn=%x paylen=%d.", cmd, drive, lsn, len(payload))
	DumpHexLines("payload", 0, payload)

	// Create the drive record, if needed.  Call it d.
	ds := h.DriveSession
	d := ds.CreateDriveIfNeeded(byte(drive))

	// Open the drive, if needed.
	// Has it been mounted?
	if d.Path == "" {
		// No mount, so make it an empty disk.
		d.Path = Format("new-rs%02d.dsk", drive) // temporary name
		Log("HdbDosSector: d.Path empty, creating path %q, empty dskini", d.Path)
		d.Image = EmptyDecbInitializedDiskImage()
		d.Dirty = false
		d.CreationTimestamp = finder.Timestamp()
		log.Printf("@@@@@@@@@@ made empty DriveImage name %q number %d. TS %q", d.Path, drive, d.CreationTimestamp)
	}
	if d.Image == nil {
		Log("HdbDosSector: d.Image empty, reading path %q, empty dskini", d.Path)
		unixfile := PFP.Join(*finder.FlagNavRoot, d.Path)
		bb, err := os.ReadFile(unixfile)
		if err != nil {
			log.Panicf("HdbDosSector ReadFile failed %q: %q: %v", d.Path, unixfile, err)
		}
		if !In(int64(len(bb)), finder.FloppySizes) {
			log.Panicf("HdbDosSector ReadFile got %d bytes, wanted 161280", len(bb))
		}
		d.Image = bb
		d.Dirty = false
		d.CreationTimestamp = finder.Timestamp()
		log.Printf("@@@@@@@@@@ HdbDosSector: opend file %q DriveImage number %d. TS %q", d.Path, drive, d.CreationTimestamp)
	}

	switch cmd {
	case 2: // Read
		h.NumReads++
		if h.NumReads == 1 {
			SendInitialInjections(ses)
		}

		buf := make([]byte, 256+5)
		for i := 0; i < 5; i++ {
			buf[i] = payload[i]
		}
		copy(buf[5:], d.Image[front:back])
		log.Printf("HDB READ lsn=%d.: %02x", lsn, buf[5:])

		log.Printf("HdbDos Reply Packet: quint + %d.", len(buf))
		DumpHexLines("REPLY", 0, buf)
		com.WriteQuint(comm.CMD_HDBDOS_SECTOR, 0, buf)

	case 3: // Write
		copy(d.Image[front:back], payload[5:])
		d.Dirty = true
		log.Printf("HDB WRITE lsn=%d.: %02x", lsn, payload[5:])

	default:
		panic(cmd)
	}
}

func Inject(ses *Session, sideload []byte, dest uint, exec bool, payload []byte) {
	com := comm.Wrap(ses.Conn)
	log.Printf("Inject: len=%4x dest=%4x exec=%v", len(payload), dest, exec)
	buf := make([]byte, 5+256)
	copy(buf[5:], sideload)
	copy(buf[5+64:], payload)

	buf[5+0x3C] = byte(len(payload))
	buf[5+0x38] = byte(dest >> 8)
	buf[5+0x39] = byte(dest)

	if exec {
		buf[5+0x3A] = byte(dest >> 8)
		buf[5+0x3B] = byte(dest)
	}

	com.WriteQuint(comm.CMD_HDBDOS_EXEC, 0, buf)
	DumpHexLines("Inject: did ", dest, buf)
}

func AsciiToInjectBytes(s string) []byte {
	var bb bytes.Buffer
	for _, ch := range strings.ToUpper(s) {
		bb.WriteByte(63 & byte(ch))
	}
	return bb.Bytes()
}

func SendInitialInjections(ses *Session) {
	sideload := Value(ioutil.ReadFile(*FlagSideloadRaw))
	Inject(ses, sideload, 0x04A0 /* on text screen*/, false, AsciiToInjectBytes("(CLEAR KEY TOGGLES DISK FINDER)"))
	// Inject(ses, sideload, 0x05A0 /* on text screen*/, false, AsciiToInjectBytes("  USE ARROW KEYS TO NAVIGATE."))
	// Inject(ses, sideload, 0x05C0 /* on text screen*/, false, AsciiToInjectBytes("  HIT 0 TO MOUNT DRIVE 0,"))
	// Inject(ses, sideload, 0x05E0 /* on text screen*/, false, AsciiToInjectBytes("  1 FOR 1, ... THROUGH 9."))

	bb := Value(ioutil.ReadFile(*FlagInkeyRaw))

	const InkeyTrapInit = 0xFA12
	const ChunkSize = 0xC0
	endAddr := uint(InkeyTrapInit + len(bb))
	for len(bb) > 0 {
		n := len(bb)
		if n > ChunkSize {
			// All-but-last injection:
			endAddr -= ChunkSize
			Inject(ses, sideload, endAddr, false, bb[n-ChunkSize:])
			bb = bb[:n-ChunkSize]
		} else {
			// Last injection:
			Inject(ses, sideload, InkeyTrapInit, true, bb)
			bb = bb[:0]
		}
	}
}

func EmptyDecbInitializedDiskImage() []byte {
	/*
		$ decb dskini /tmp/empty
		$ hd /tmp/empty
		00000000  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
		*
		00013200  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
		*
		00013300  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
		*
		00013340  ff ff ff ff 00 00 00 00  00 00 00 00 00 00 00 00
		00013350  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
		*
		00013400  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
		*
		00027600
		$
	*/

	bb := make([]byte, 161280)
	for i := 0; i < 0x00013200; i++ {
		bb[i] = 0xFF
	}
	for i := 0x00013300; i < 0x00013344; i++ {
		bb[i] = 0xFF
	}
	for i := 0x00013400; i < 0x00027600; i++ {
		bb[i] = 0xFF
	}
	return bb
}
