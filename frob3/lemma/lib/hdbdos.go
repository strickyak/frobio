package lib

import (
	"bytes"
	"flag"
	"fmt"
	. "github.com/strickyak/frobio/frob3/lemma/util"
	"io/ioutil"
	"log"
	"os"
	PFP "path/filepath"
	"strings"
	"time"
)

var FlagHomesRoot = flag.String("homes_root", "", "User home directories, for HdbDos")
var FlagPublicRoot = flag.String("public_root", "", "public files, for HdbDos")
var FlagSideloadRaw = flag.String("sideload_raw", "sideload.lwraw", "raw machine code fragment for HdbDos")
var FlagInkeyRaw = flag.String("inkey_raw", "inkey_trap.lwraw", "raw machine code fragment for HdbDos")

const NumDrives = 10
const DirPerm = 0775
const FilePerm = 0664

const FloppySize = 161280

type HdbDosDrive struct {
	Path  string
	Image []byte
	Dirty bool
	CreationTimestamp string
}

type HdbDosSession struct {
	Ses  *Session
	NumReads  int64   // When 1, send injections.

	Drives [NumDrives]*HdbDosDrive
}

func (h *HdbDosSession) SetDrive(drive byte, c *Chooser) {
	HdbDosCleanupOneDrive(h.Ses, drive)

	log.Printf("SetDrive: %d %v", drive, *c)

	// Create the drive record, if needed.  Call it d.
	if h.Drives[drive] == nil {
		h.Drives[drive] = &HdbDosDrive{}
		log.Printf("@@@@@@@@@@ created HdbDosDrive{} number %d.", drive)
	}
	d := h.Drives[drive]

	if !c.IsDir && c.Size == FloppySize {
		d.Path = c.Path()
		d.Image = nil
		d.Dirty = false
	}
}

func TextChooserShell(ses *Session) {
	t := &Text40{
		Ses: ses,
	}
	t.Loop() // Loop until we return from Hijack.
}

func RGB(r, g, b byte) byte {
	var z byte
	if (r & 2) != 0 {
		z |= 0x20
	}
	if (g & 2) != 0 {
		z |= 0x10
	}
	if (b & 2) != 0 {
		z |= 0x08
	}
	if (r & 1) != 0 {
		z |= 0x04
	}
	if (g & 1) != 0 {
		z |= 0x02
	}
	if (b & 1) != 0 {
		z |= 0x01
	}
	return z
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

func UndoSimplePalette(ses *Session, savedPalette []byte) {
	PokeRam(ses.Conn, 0xFFB0, savedPalette)
}

const (
	White = iota
	Red
	Green
	Blue
	Yellow
	Magenta
	Cyan
	Black
)

func SetSimplePaletteReturnCurrent(ses *Session, payload []byte) []byte {
	// savedPalette := Peek2Ram(ses.Conn, 0xFFB0, 16)
	savedPalette := payload[256+16 : 256+32]
	palette := []byte{
		RGB(3, 3, 3), // 0 = white
		RGB(3, 0, 0), // 1 = red
		RGB(0, 3, 0), // 2 = green
		RGB(0, 0, 3), // 3 = blue
		RGB(3, 3, 0), // 4 = yellow
		RGB(2, 0, 2), // 5 = magenta
		RGB(0, 2, 2), // 6 = cyan
		RGB(0, 0, 0), // 7 = black

		RGB(3, 3, 3), // 0 = white
		RGB(3, 0, 0), // 1 = red
		RGB(0, 3, 0), // 2 = green
		RGB(0, 0, 3), // 3 = blue
		RGB(3, 3, 0), // 4 = yellow
		RGB(2, 0, 2), // 5 = magenta
		RGB(0, 2, 2), // 6 = cyan
		RGB(0, 0, 0), // 7 = black
	}
	PokeRam(ses.Conn, 0xFFB0, palette)
	return savedPalette
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
		bb[2*i+1] = ((i % 7) << 3) | Black
		if bb[2*i+1] == 077 {
			bb[2*i+1] = 7
		}
	}
	bb[0] = 'A' + byte(nthRow)
	return bb
}

func OnText40At2000Run(ses *Session, payload []byte, runMe func()) {
	// savedPage0 := PeekPage0(ses)
	savedPage0 := payload[:256]
	hrmode, hrwidth, pmode := VideoModes(savedPage0)
	strV, bytV := DescribeVideoModes(savedPage0)

	log.Printf("Incoming Video Modes: %s", strV)

	numRows := GimeText40x24_OnPage37_ReturnNumRows(ses, payload, 1)
	log.Printf("OnText40At2000Run: %d. complete rows", numRows)
	savedMMUByte := AlterTask0Map37To2000ReturnSaved(ses, payload)

	savedPalette := SetSimplePaletteReturnCurrent(ses, payload)

	for row := byte(0); row < 24; row++ {
		bb := TestCharRow40(row)
		for i := byte(0); i < 40; i++ { // invert some bg/fg
			if uint(row+i)%13 == 9 { // choose which ones
				x := bb[i+i+1]
				x = (x >> 3) | 070 // the inversion, assuming bg is Black.
				bb[i+i+1] = x
			}
		}
		PokeRam(ses.Conn, 0x2000+40*2*uint(row), Cond(row == 0, bytV, bb))
	}

	defer func() {
		SetVideoMode(ses, hrmode, hrwidth, pmode)
		UndoSimplePalette(ses, savedPalette)
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

func CreateHomeIfNeeded(ses *Session) {
	os.Mkdir(*FlagHomesRoot, 0770)
	os.Mkdir(PFP.Join(*FlagHomesRoot, ses.Hostname), DirPerm)
	os.Mkdir(PFP.Join(*FlagHomesRoot, ses.Hostname, TTL_10_DAY_DIR_NAME), DirPerm)
}

// Entry from waiter.go for a Hijack.
func HdbDosHijack(ses *Session, payload []byte) {
	CreateHomeIfNeeded(ses)

	OnText40At2000Run(ses, payload, func() {
		time.Sleep(5 * time.Second)
		TextChooserShell(ses)
	})

	// We tell coco that this is the END of the HIJACK.
	WriteQuint(ses.Conn, CMD_HDBDOS_HIJACK, 0, []byte{})
}

const TTL_10_DAY_DIR_NAME = "TTL-10-day"

func HdbDosCleanupOneDrive(ses *Session, driveNum byte) {
	drive := ses.HdbDos.Drives[driveNum]
	if drive == nil || !drive.Dirty {
		return
	}
	// if dirty:
	dest := PFP.Join(*FlagHomesRoot, ses.Hostname, TTL_10_DAY_DIR_NAME, drive.CreationTimestamp+"-"+PFP.Base(drive.Path))
	err := os.WriteFile(dest, drive.Image, FilePerm)
	if err != nil {
		log.Printf("BAD: HdbDosCleanup: Error saving dirty file %d bytes %q as %q: %v", len(drive.Image), drive.Path, dest, err)
	} else {
		log.Printf("HdbDosCleanup: Saved dirty file %d bytes %q as %q", len(drive.Image), drive.Path, dest)
	}
	ses.HdbDos.Drives[driveNum] = nil // Delete the drive record.
}
func Timestamp() string {
	return time.Now().UTC().Format("2006-01-02-150405Z")
}
func HdbDosCleanup(ses *Session) {
	for driveNum, _ := range ses.HdbDos.Drives {
		HdbDosCleanupOneDrive(ses, byte(driveNum))
	}
}

// Entry from waiter.go for a Sector.
func HdbDosSector(ses *Session, payload []byte) {
	if ses.HdbDos == nil {
		ses.HdbDos = &HdbDosSession{
			Ses: ses,
		}
	}
	ses.Cleanups = append(ses.Cleanups, func() {
		HdbDosCleanup(ses)
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
	if h.Drives[drive] == nil {
		h.Drives[drive] = &HdbDosDrive{}
		log.Printf("@@@@@@@@@@ created HdbDosDrive{} number %d.", drive)
	}
	d := h.Drives[drive]

	// Open the drive, if needed.
	// Has it been mounted?
	if d.Path == "" {
		// No mount, so make it an empty disk.
		d.Path = Format("new-rs%02d.dsk", drive) // temporary name
		d.Image = EmptyDecbInitializedDiskImage()
		d.Dirty = false
		d.CreationTimestamp =  Timestamp()
		log.Printf("@@@@@@@@@@ made empty HdbDosDrive{} name %q number %d. TS %q", d.Path, drive, d.CreationTimestamp)
	}
	if d.Image == nil {
		bb, err := os.ReadFile(PFP.Join(*FlagPublicRoot, d.Path))
		if err != nil {
			log.Panicf("HdbDosSector ReadFile failed %q: %v", d.Path, err)
		}
		if len(bb) != 161280 {
			log.Panicf("HdbDosSector ReadFile got %d bytes, wanted 161280", len(bb))
		}
		d.Image = bb
		d.Dirty = false
		d.CreationTimestamp =  Timestamp()
		log.Printf("@@@@@@@@@@ opend file %q HdbDosDrive{} number %d. TS %q", d.Path, drive, d.CreationTimestamp)
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
		WriteQuint(ses.Conn, CMD_HDBDOS_SECTOR, 0, buf)

	case 3: // Write
		copy(d.Image[front:back], payload[5:])
		d.Dirty = true
		log.Printf("HDB WRITE lsn=%d.: %02x", lsn, payload[5:])

	default:
		panic(cmd)
	}
}

func Inject(ses *Session, sideload []byte, dest uint, exec bool, payload []byte) {
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

	WriteQuint(ses.Conn, CMD_HDBDOS_EXEC, 0, buf)
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
	Inject(ses, sideload, 0x04A0 /* on text screen*/, false, AsciiToInjectBytes("(CLEAR KEY TOGGLES DISK CHOOSER)"))
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
