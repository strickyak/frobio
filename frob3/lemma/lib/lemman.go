package lib

import (
	"bufio"
	"bytes"
	"flag"
	"io"
	"log"
	"net"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"syscall"
	"time"

	"github.com/strickyak/frobio/frob3/lemma/sym"
)

var LEMMAN_FS = flag.String(
	"lemman_fs", ".", "directory to use as LemMan filesystem")

var Inodes [1024]*LMPath // Index excess 1024

var NextInodeVar = 1024

func NextInode() int {
	NextInodeVar++
	if NextInodeVar >= 2048 {
		NextInodeVar = 1024
	}
	return NextInodeVar
}

type LMPath struct {
	PathName   string
	Mode       byte
	IsDir      bool
	Offset     int64
	Size       int64
	DirEntries []os.DirEntry
	DirData    []byte
	FD         *os.File
	R          io.Reader
}

var LMPaths = make(map[uint]*LMPath)

type LMRegs struct {
	rcc byte
	ra  byte
	rb  byte
	rdp byte
	rx  uint
	ry  uint
	ru  uint
	rpc uint
}

func NewLMRegs(in []byte) *LMRegs {
	return &LMRegs{
		rcc: in[0],
		ra:  in[1],
		rb:  in[2],
		rdp: in[3],
		rx:  HiLo(in[4], in[5]),
		ry:  HiLo(in[6], in[7]),
		ru:  HiLo(in[8], in[9]),
		rpc: HiLo(in[10], in[11]),
	}
}
func (r LMRegs) WriteBytes(to *bytes.Buffer) {
	to.WriteByte(r.rcc)
	to.WriteByte(r.ra)
	to.WriteByte(r.rb)
	to.WriteByte(r.rdp)

	to.WriteByte(byte(r.rx >> 8))
	to.WriteByte(byte(r.rx))
	to.WriteByte(byte(r.ry >> 8))
	to.WriteByte(byte(r.ry))
	to.WriteByte(byte(r.ru >> 8))
	to.WriteByte(byte(r.ru))
	to.WriteByte(byte(r.rpc >> 8))
	to.WriteByte(byte(r.rpc))
}

var NiceFilenamePattern = regexp.MustCompile(`^[/]?[A-Za-z0-9][-A-Za-z0-9_.%@]*$`)

func TimeVector(t time.Time) []byte {
	y, m, d := t.Date()
	hr, min := t.Hour(), t.Minute()
	var vec [5]byte
	vec[0] = byte(y - 1900)
	vec[1], vec[2] = byte(m), byte(d)
	vec[3], vec[4] = byte(hr), byte(min)
	return vec[:]
}

func LMGetStt(pd uint, regs *LMRegs) (payload []byte, status byte) {
	// Input: B is GetStt opcode

	_, ok := LMPaths[pd]
	if !ok {
		return nil, sym.E_BPNum //  0x00C9 // E$BPNum
	}

	switch regs.rb {
	case 0x20: // SS.FDInf -- File Descriptor Info
		ino := ((regs.ry & 0xFF00) << 8) | regs.ru
		inode := Inodes[ino-1024]
		n := regs.ry & 0xFF
		bb := make([]byte, n)

		jname := filepath.Join(*LEMMAN_FS, inode.PathName)
		fileInfo, err := os.Stat(jname)
		if err != nil {
			log.Panicf("I$GetStt/SS.FDInf: cannot Stat inode=%d. %q: %v", ino, jname, err)
		}
		mtime := fileInfo.ModTime()

		sysInfo := fileInfo.Sys().(*syscall.Stat_t)
		ctime := time.Unix(int64(sysInfo.Ctim.Sec), 0)

		// FD.ATT, 1 byte at offset 0
		/*
		   Bit 7 Directory
		   Bit 6 Single user
		   Bit 5 Public execute
		   Bit 4 Public write
		   Bit 3 Public read
		   Bit 2 Execute
		   Bit 1 Write
		   Bit 0 Read
		*/
		if fileInfo.IsDir() {
			bb[0] = 0277
		} else {
			bb[0] = 0077
		}

		// FD.OWN, 2 bytes at offset 1

		// FD.DAT, mod time, 5 bytes at offset 3
		copy(bb[3:3+5], TimeVector(mtime)[:5])

		// FD.LNK, 1 byte at offset 8
		nlink := sysInfo.Nlink
		bb[8] = byte(Min(nlink, 255))

		// FD.SIZ, 4 bytes at offset 9
		sz := fileInfo.Size()
		bb[9] = byte(sz >> 24)
		bb[10] = byte(sz >> 16)
		bb[11] = byte(sz >> 8)
		bb[12] = byte(sz >> 0)

		// FD.Creat, creation time, 3 bytes at offset 13
		copy(bb[13:], TimeVector(ctime)[:3])

		// FD.Seg, segment list, starts at offset 16.
		payload = bb
	}

	return
}
func LMSeek(pd uint, regs *LMRegs) (status byte) {
	// Input: X++U are 32 bit offset desired.

	p, ok := LMPaths[pd]
	if !ok {
		return sym.E_BPNum //  0x00C9 // E$BPNum
	}

	want := int64((regs.rx << 16) | regs.ru)
	p.Offset = want // but what about p.Size?
	return 0
}

func LMRead(pd uint, regs *LMRegs) (payload []byte, status byte) {
	p, ok := LMPaths[pd]
	if !ok {
		return nil, sym.E_BPNum //  0x00C9 // E$BPNum
	}
	if p.Offset >= p.Size {
		return nil, sym.E_EOF // 0x00D3 // E$EOF
	}
	n := int64(regs.ry)
	if n > p.Size-p.Offset { // not enuf data
		n = p.Size - p.Offset // use all the rest
	}

	buf := make([]byte, n)
	if p.IsDir {
		copy(buf, p.DirData[p.Offset:p.Offset+n])
	} else {
		cc, err := p.R.Read(buf)
		if err != nil {
			log.Panicf("Cannot LMRead %q: %v", p.PathName, err)
		}
		if int64(cc) != n {
			log.Panicf("Short Read: %q (%d vs %d)", p.PathName, cc, n)
		}
	}
	p.Offset += n
	log.Printf("LMRead: returning n=%d : %v", n, buf)
	return buf, 0
}

func LMClose(pd uint, regs *LMRegs) (status byte) {
	_, ok := LMPaths[pd]
	if !ok {
		return sym.E_BPNum //  0x00C9 // E$BPNum
	}
	delete(LMPaths, pd)
	return 0
}

func LMOpen(pd uint, regs *LMRegs, mode byte, name string) (status byte) {
	clean := filepath.Clean(name)
	if len(clean) > 0 && clean[0] == '/' {
		clean = clean[1:]
	}
	words := strings.Split(clean, "/")
	for _, w := range words {
		if w == "" {
			continue
		}
		if !NiceFilenamePattern.MatchString(w) {
			log.Panicf("Bad word %q in filepath %q", w, clean)
		}
	}

	jname := filepath.Join(*LEMMAN_FS, clean)
	info, err := os.Stat(jname)
	if err != nil {
		log.Panicf("LMOpen cannot stat filepath %q: %v", clean, err)
	}
	isDir := info.IsDir()

	var fd *os.File
	if isDir {

		entries, err := os.ReadDir(jname)
		if err != nil {
			log.Panicf("LMOpen cannot ReadDir %q: %v", clean, err)
		}

		log.Printf("Dir %q has %d entries", clean, len(entries))

		var buf bytes.Buffer
		buf.Write([]byte{
			'.', '.' | 0x80, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 42,
		})
		buf.Write([]byte{
			'.' | 0x80, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 43,
		})
		for i, e := range entries {
			log.Printf("  Entry %d is %q (%v) %v", i, e.Name(), e.IsDir(), e.Type())

			var chunk [32]byte
			s := e.Name()
			if len(s) > 29 {
				continue // cannot do long filenames.
			}
			n := len(s)
			for i = 0; i < n; i++ {
				if i == n-1 {
					chunk[i] = s[i] | 0x80 // Mark as final.
				} else {
					chunk[i] = s[i]
				}
			}
			ino := NextInode()
			chunk[31] = byte(ino)
			chunk[30] = byte(ino >> 8)
			chunk[29] = byte(ino >> 16)
			buf.Write(chunk[:])
			Inodes[ino-1024] = &LMPath{
				PathName: filepath.Join(clean, s),
			}
		}

		dirData := buf.Bytes()
		LMPaths[pd] = &LMPath{
			PathName:   clean,
			Mode:       mode,
			IsDir:      true,
			DirEntries: entries,
			DirData:    dirData,
			Size:       int64(len(dirData)),
		}
		log.Printf("DirData len %d: %v", len(dirData), dirData)

	} else {
		fd, err = os.Open(jname) // TODO: write mode
		if err != nil {
			log.Panicf("LMOpen cannot Open %q: %v", clean, err)
		}
		r := bufio.NewReader(fd)

		LMPaths[pd] = &LMPath{
			PathName: clean,
			Mode:     mode,
			IsDir:    false,
			FD:       fd,
			R:        r,
			Size:     info.Size(),
			Offset:   0,
		}
	}

	regs.rx += uint(len(name))
	return 0
}

func DoLemMan(conn net.Conn, in []byte, pd uint) []byte {
	const headerLen = 15
	const regsLen = 12

	op := in[0]
	regs := NewLMRegs(in[1 : 1+regsLen])
	paylen := HiLo(in[headerLen-2], in[headerLen-1])

	var payload []byte
	var status byte
	var name string

	if 1 <= op && op <= 5 {
		name = ExtractName(in[headerLen:headerLen+paylen], 0, "LemMan_Open")
		log.Printf("ExtractName ==> %q", name)
	}

	switch op {
	case 1: // Create
	case 2: // Open
		status = LMOpen(pd, regs, regs.ra, name)
	case 4: // ChgDir
	case 6: // Seek
		status = LMSeek(pd, regs)
	case 7: // Read
		payload, status = LMRead(pd, regs)
	case 11: // GetStt
		payload, status = LMGetStt(pd, regs)
	case 13: // Close
		status = LMClose(pd, regs)
	}

	var z bytes.Buffer
	z.WriteByte(status)
	regs.WriteBytes(&z)
	z.WriteByte(byte(len(payload) >> 8))
	z.WriteByte(byte(len(payload)))

	if payload != nil {
		DumpHexLines("REPLY PAYLOAD", 0, payload)
		z.Write(payload)
	}

	result := z.Bytes()
	log.Printf("@@@@@@@@@@@@@@ Reply Status=$%x PayLen=$%x Regs=>%q", status, len(payload), RegsString(result[1:]))

	return result
}
