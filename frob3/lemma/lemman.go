package lemma

import (
	"bytes"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"path/filepath"
	// "regexp"
	"strings"
	"syscall"
	"time"

	"github.com/strickyak/frobio/frob3/lemma/hex"
	"github.com/strickyak/frobio/frob3/lemma/sym"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var LEMMAN_FS = flag.String(
	"lemman_fs", "lemman_fs", "directory to use as LemMan filesystem")

var Inodes [1024]*LMPath // Index excess 1024

var NextInodeVar = 1024

func NextInode() int {
	NextInodeVar++
	if NextInodeVar >= 2048 {
		NextInodeVar = 1024
	}
	return NextInodeVar
}

// LemmaError() takes Printf arguments,
// logs the error in the lemma server,
// and returns an error number in 80..99
// for the OS9 error.
const LemmaErrorMin = 80
const LemmaErrorMax = 99

var PrevLemmaError byte = LemmaErrorMin - 1

func LemmaError(format string, args ...any) byte {
	PrevLemmaError++
	if PrevLemmaError > LemmaErrorMax {
		PrevLemmaError = LemmaErrorMin
	}
	errnum := PrevLemmaError

	pre := fmt.Sprintf("@LEMMA_ERROR@ $%x=%d. :: ", errnum, errnum)
	log.Printf(pre+format, args...)

	return errnum
}

var OpNames = []string{
	"???", "Cre", "Ope", "Mkd", "Cgd", "Del", "Sek", "Rea", "Wri", "RLn", "WLn", "Get", "Set", "Clo",
}

type LMPath struct {
	PathName   string
	Mode       byte
	IsDir      bool
	Offset     int64
	Size       int64
	DirEntries []os.DirEntry
	DirData    []byte
	osFile     *os.File
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

		ctime := mtime // re-use mtime for ctime if not UNIX.
		sysInfo, okUnix := fileInfo.Sys().(*syscall.Stat_t)
		if okUnix {
			ctime = time.Unix(int64(sysInfo.Ctim.Sec), 0)
		}

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
func LMDelete(pd uint, regs *LMRegs, name string) (status byte) {
	err := os.Remove(name)
	if err != nil {
		return LemmaError("Cannot delete %q: %v", name, err)
	}
	return 0
}

func LMChgDir(pd uint, regs *LMRegs) (status byte) {
	if false { // TODO why is this called on different pds?
		p, ok := LMPaths[pd]
		if !ok {
			return sym.E_BPNum //  0x00C9 // E$BPNum
		}
		_ = p
	}
	return 0
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

func LMWrite(pd uint, regs *LMRegs, pay_in []byte, linely bool) (status byte) {
	p, ok := LMPaths[pd]
	if !ok {
		return sym.E_BPNum //  0x00C9 // E$BPNum
	}
	log.Printf("LMWrite: @@@@@@@: len=%d. pay_in=%v", len(pay_in), pay_in)

	if (p.Mode & 0x02) != 0x02 {
		return LemmaError("Cannot write because mode is $%x: %q", p.Mode, p.PathName)
	}
	if (p.Mode & 0x80) != 0 {
		return LemmaError("Cannot write to directory mode $%x: %q", p.Mode, p.PathName)
	}
	if p.IsDir {
		return LemmaError("Cannot write to IsDir: %q", p.Mode, p.PathName)
	}

	{
		off, err := p.osFile.Seek(p.Offset, 0)
		if err != nil {
			log.Panicf("Cannot Seek %q: %v", p.PathName, err)
		}
		if off != p.Offset {
			return LemmaError("Wrong Seek: %q (%d vs %d)", p.PathName, off, p.Offset)
		}
		log.Printf("LMWrite: @@@@@@@: offset=%d.", p.Offset)
	}

	{
		cc, err := p.osFile.Write(pay_in)
		log.Printf("LMWrite: @@@@@@@: wrote %d. -> %d. (%v)", len(pay_in), cc, err)
		if err != nil {
			return LemmaError("Cannot LMWrite %q: %v", p.PathName, err)
		}
		if cc != len(pay_in) {
			log.Panicf("Short Read: %q (%d vs %d)", p.PathName, cc, len(pay_in))
		}
		p.Offset += int64(cc)
	}

	return 0
}

func LMRead(pd uint, regs *LMRegs, linely bool) (payload []byte, status byte) {
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
		off, err := p.osFile.Seek(p.Offset, 0)
		if err != nil {
			return nil, LemmaError("Cannot Seek %q: %v)", p.PathName, err)
		}
		if off != p.Offset {
			return nil, LemmaError("Wrong Seek: %q (%d vs %d)", p.PathName, off, p.Offset)
		}

		cc, err := p.osFile.Read(buf)
		if err != nil {
			return nil, LemmaError("Cannot LMRead %q: %v", p.PathName, err)
		}
		if int64(cc) != n {
			return nil, LemmaError("Short Read: %q (%d vs %d)", p.PathName, cc, n)
		}
	}

	if linely {
		// Find the first \n or \r.  Its index is `i`.
		i := int64(bytes.IndexAny(buf, "\n\r"))
		// Chop after the \n or \r, and change \n to \r.
		if i >= 0 && i+1 < n {
			// Change n and buf, to stop after the CR.
			n = i + 1
			buf = buf[:n]
			buf[i] = '\r' // Not \n but \r
		}
	}

	p.Offset += n
	log.Printf("LMRead: returning n=%d : %v", n, buf)
	return buf, 0
}

func LMClose(pd uint, regs *LMRegs) (status byte) {
	fd, ok := LMPaths[pd]
	if !ok {
		return sym.E_BPNum //  0x00C9 // E$BPNum
	}
	if fd.osFile != nil {
		fd.osFile.Close()
		fd.osFile = nil
	}

	delete(LMPaths, pd)
	return 0
}

func LMOpen(pd uint, regs *LMRegs, mode byte, name string, create bool) (status byte) {
	clean := filepath.Clean(name)
	clean = strings.TrimPrefix(clean, "/") // not an absolute path
	splitWords := strings.Split(clean, "/")
	log.Printf("LMOpen: name: %q clean: %q splitWords: %#v", name, clean, splitWords)

	var words []string // we clean splitWords even more, to get words.
	for _, w := range splitWords {
		if w == ".." {
			return LemmaError("Bad word %q in filepath %q", w, clean)
			// log.Panicf("why did a `..` survive cleaning? %q %#v", clean, splitWords)
		}
		if w == "" || w == "." || w == ".." {
			continue
		}
		if strings.HasPrefix(w, ".") {
			return LemmaError("Bad dotted word %q in filepath %q", w, clean)
		}
		words = append(words, w)
	}

	jname := *LEMMAN_FS + "/" + filepath.Join(words...)
	log.Printf("LMOpen: name: %q jname: %q", name, jname)

	if create {
		if mode != 2 {
			return LemmaError("LMCreate requires mode 2 (for now): %q %q", jname, name)
		}

		osFile, err := os.Create(jname) // TODO: write mode
		if err != nil {
			return LemmaError("LMCreate cannot Create %q %q: %v", jname, name, err)
		}

		LMPaths[pd] = &LMPath{
			PathName: clean,
			Mode:     mode,
			IsDir:    false,
			osFile:   osFile,
			Size:     0,
			Offset:   0,
		}
	} else {

		info, statErr := os.Stat(jname)
		if statErr != nil {
			return LemmaError("LMOpen cannot stat filepath %q: %v", clean, statErr)
			return sym.E_PNNF
		}

		if info.IsDir() {
			entries, err := os.ReadDir(jname)
			if err != nil {
				return LemmaError("LMOpen cannot ReadDir %q: %v", clean, err)
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
			osFile, err := os.Open(jname) // TODO: write mode
			if err != nil {
				return LemmaError("LMOpen cannot Open %q: %v", clean, err)
			}

			LMPaths[pd] = &LMPath{
				PathName: clean,
				Mode:     mode,
				IsDir:    false,
				osFile:   osFile,
				Size:     info.Size(),
				Offset:   0,
			}
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

	var payload_out []byte
	var status byte
	var name string

	log.Printf("DoLemMan:::: op=%q=%d paylen=%d len(in)=%d", OpNames[op], op, paylen, len(in))

	if 1 <= op && op <= 5 {
		name = ExtractName(in[headerLen:headerLen+paylen], 0, "LemMan")
		log.Printf("ExtractName ==> %q", name)
	}

	switch op {
	case 1: // Create
		status = LMOpen(pd, regs, regs.ra, name, true)
	case 2: // Open
		status = LMOpen(pd, regs, regs.ra, name, false)
	case 4: // ChgDir
		status = LMChgDir(pd, regs)
	case 5: // Delete
		status = LMDelete(pd, regs, name)
	case 6: // Seek
		status = LMSeek(pd, regs)

	case 7: // Read
		payload_out, status = LMRead(pd, regs, false)
	case 9: // ReadLn
		payload_out, status = LMRead(pd, regs, true)

	case 8: // Write
		status = LMWrite(pd, regs, in[headerLen:], false)
	case 10: // WritLn
		status = LMWrite(pd, regs, in[headerLen:], true)

	case 11: // GetStt
		payload_out, status = LMGetStt(pd, regs)

	case 13: // Close
		status = LMClose(pd, regs)

	default:
		status = LemmaError("Lemma: unknown file op=%d.", op)
	}

	var z bytes.Buffer
	z.WriteByte(status)
	regs.WriteBytes(&z)
	z.WriteByte(byte(len(payload_out) >> 8))
	z.WriteByte(byte(len(payload_out)))

	if payload_out != nil {
		hex.DumpHexLines("REPLY PAYLOAD", 0, payload_out)
		z.Write(payload_out)
	}

	result := z.Bytes()
	log.Printf("@@@@@@@@@@@@@@ Reply Status=$%x PayLen=$%x Regs=>%q", status, len(payload_out), RegsString(result[1:]))

	return result
}
