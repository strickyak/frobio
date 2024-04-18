package rsdos

import (
	"bytes"
	"fmt"
	. "github.com/strickyak/frobio/frob3/lemma/util"
	"log"
	"strings"
)

const FloppySize35 = 161280
const FloppySize40 = 184320
const FloppySize80 = 368640

const BytesPerSector = 256
const SectorsPerTrack = 18
const SectorsPerGranule = 9
const DirectoryTrack = 17
const FatLSN = SectorsPerTrack*DirectoryTrack + 1
const FirstDirectoryLSN = SectorsPerTrack*DirectoryTrack + 2
const NumDirectorySectors = 8
const DirEntrySize = 32

const ( // FileType
	BasicType       = 0
	DataType        = 1
	MachineLangType = 2
	TextType        = 3
)

var TypeNames = []string{"BASIC", "DATA", "ML", "TEXT"}

type FileRec struct {
	FileName             string
	FileType             int
	IsAscii              bool
	Granules             []byte
	SectorsInLastGranule int
	BytesInLastSector    int
	Size                 int
}

type DiskRec interface {
	VolumeName() string
	OS() string
	Free() string
	Codes() string
}

type Os9DiskRec struct {
	totalSectors      int
	sectorsPerCluster int
	volumeName        string
	allocationMapSize int // bytes
	rootSector        int
	format            int
	os                string
}

type RSDiskRec struct {
	files         []*FileRec
	freeGranules  int
	totalGranules int
	numTracks     int
	volumeName    string
	os            string
}

func (r *Os9DiskRec) VolumeName() string { return r.volumeName }
func (r *RSDiskRec) VolumeName() string  { return r.volumeName }
func (r *Os9DiskRec) OS() string         { return r.os }
func (r *RSDiskRec) OS() string          { return r.os }
func (r *Os9DiskRec) Free() string       { return "OS-9 disk" }
func (r *RSDiskRec) Free() string {
	return fmt.Sprintf("RSDOS %d/%d gran free", r.freeGranules, r.totalGranules)
}
func (r *Os9DiskRec) Codes() string { return fmt.Sprintf("N%s", Cond(len(r.os) > 0, "$", "")) }
func (r *RSDiskRec) Codes() string  { return fmt.Sprintf("R%s", Cond(len(r.os) > 0, "$", "")) }

func AllAre(bb []byte, val byte) bool {
	for _, b := range bb {
		if b != val {
			return false
		}
	}
	return true
}

func ParseOs9Disk(a []byte) *Os9DiskRec {
	var name bytes.Buffer
	for i := 0x1F; i < 0x3F; i++ {
		ch := a[i]
		var hibit bool
		if 0x80&ch != 0 {
			hibit = true
			ch &^= 0x80
		}
		if ' ' <= ch && ch <= '~' {
			name.WriteByte(ch)
		} else {
			name.WriteByte('?')
		}
		if hibit {
			break
		}
	}
	return &Os9DiskRec{
		totalSectors:      (int(a[0]) << 16) | (int(a[1]) << 8) | int(a[2]),
		sectorsPerCluster: int(a[6]),
		volumeName:        name.String(),
		allocationMapSize: (int(a[1]) << 4) | int(a[5]),
		rootSector:        (int(a[8]) << 16) | (int(a[9]) << 8) | int(a[10]),
		format:            int(a[16]),
	}
}

func DiskParse(contents []byte) DiskRec {
	if string(contents[0:4]) == "SDF1" {
		log.Panicf("Cannot handle SDF1 format")
	}

	numSectors := len(contents) / BytesPerSector
	residue := len(contents) % BytesPerSector
	if residue != 0 {
		log.Panicf("Bad rsdos contents size=%d, numSectors=%d, residue=%d", len(contents), numSectors, residue)
	}

	if AllAre(contents[0x80:0x100], 0) && AllAre(contents[0x100:0x108], 255) {
		return ParseOs9Disk(contents)
	}

	density := 2
	if numSectors < 324 {
		log.Panicf("Bad rsdos Too Few Sectors size=%d, numSectors=%d", len(contents), numSectors)
	} else if numSectors <= 720 /* 40 tracks * 18 sectos per track */ {
		density = 1
	} else if numSectors > 2880 {
		log.Panicf("Bad rsdos Too Many Sectors size=%d, numSectors=%d", len(contents), numSectors)
	} // else density = 2

	numTracks := len(contents) / (density * SectorsPerTrack * BytesPerSector)
	residue = len(contents) % (density * SectorsPerTrack * BytesPerSector)
	if false && residue != 0 {
		log.Panicf("Bad rsdos contents size=%d, numTracks=%d, residue=%d", len(contents), numTracks, residue)
	}
	d := &RSDiskRec{
		numTracks:     numTracks,
		totalGranules: numTracks * (density * SectorsPerTrack / SectorsPerGranule),
	}
	fat := contents[density*FatLSN*BytesPerSector : (1+density*FatLSN)*BytesPerSector]
	dirs := contents[density*FirstDirectoryLSN*BytesPerSector : (NumDirectorySectors+density*FirstDirectoryLSN)*BytesPerSector]

	granulesUsed := 2 // Call the directory track "used".
	for i := 0; i < NumDirectorySectors*BytesPerSector; i += DirEntrySize {
		initial := dirs[i]
		log.Printf("dir[%d] initial %d %02x", i, initial, dirs[i:i+16])
		if initial == 0 || initial == 255 {
			continue
		}
		/*
			Track 17 sectors 3-11 are used for the directory:
			0-7:  filename, blank padded.  first byte 0: deleted.  ff: never used.
			8-10: extension, blank padded.
			11:   File Type: 0=basic 1=basicData 2=machineLang 3=text
			12:   0=binary ff=ascii
			13:   first granule (0-67)
			14-15 num bytes in use in the last sector
			16-31 reserved
		*/
		filename := strings.TrimRight(string(dirs[i:i+8]), " \000")
		extension := strings.TrimRight(string(dirs[i+8:i+11]), " \000")
		if extension != "" {
			filename += "." + extension
		}
		granule := dirs[i+13]
		granules := []byte{granule}
		numBytesInLastSector := int(dirs[i+14])*256 + int(dirs[i+15])
		granulesUsed++
		log.Printf("file %q first granule %d (used...%d)", filename, granule, granulesUsed)
		for j := 0; j < 256; j++ {
			if j > d.totalGranules {
				log.Panicf("rsdos: Granule Loop: %v", granules)
			}
			granule = fat[granule]
			if granule < 0xC0 {
				granules = append(granules, granule)
				granulesUsed++
				log.Printf("next granule %d (used...%d)", granule, granulesUsed)
			} else {
				log.Printf("    break because %d", granule)
				break
			}
		}
		numSectorsInLastGranule := int(granule & 0x3F)
		numSectors := (len(granules)-1)*density*SectorsPerGranule + numSectorsInLastGranule
		numBytes := (numSectors-1)*BytesPerSector + numBytesInLastSector

		d.files = append(d.files, &FileRec{
			FileName:             filename,
			FileType:             int(dirs[i+11]),
			IsAscii:              (dirs[i+12] == 255),
			Granules:             granules,
			SectorsInLastGranule: numSectorsInLastGranule,
			BytesInLastSector:    numBytesInLastSector,
			Size:                 numBytes,
		})
	}

	freeGranules := 0
	for i, e := range fat[:d.totalGranules] {
		if i/2 == DirectoryTrack {
			continue
		}
		if e == 255 {
			freeGranules++
		}
		log.Printf("[%d = %d.%d] %d   (free...%d)", i, i/2, i%2, e, freeGranules)
	}
	for i := 0; i < 32; i++ { // Look for disk name
		// TODO : density
		start := BytesPerSector * (16 + DirectoryTrack*SectorsPerTrack)
		ch := contents[i+start]
		if 32 <= ch && ch <= 126 {
			d.volumeName += string([]byte{ch})
		} else {
			break
		}
	}
	if d.numTracks >= 35 {
		start := BytesPerSector * (34 * SectorsPerTrack)
		if contents[start] == 'O' && contents[start+1] == 'S' {
			for i := 0; i < 1024; i++ {
				ch := contents[i+start]
				if 'A' <= ch && ch <= 'Z' || 'a' <= ch && ch <= 'z' {
					d.os += string([]byte{ch})
					if len(d.os) >= 64 {
						break
					}
				}
			}
		}
	}
	if false && freeGranules != d.totalGranules-granulesUsed {
		log.Panicf("rsdos: freeGranules=%d but total=%d - used=%d is %d",
			freeGranules, d.totalGranules, granulesUsed, d.totalGranules-granulesUsed)
	}
	if d.volumeName != "" {
		log.Printf("volumeName: %q", d.volumeName)
	}
	if d.os != "" {
		log.Printf("os: %q", d.os)
	}
	d.freeGranules = freeGranules

	return d
}

/* Thanks
https://subethasoftware.com/2023/04/25/coco-disk-basic-disk-structure-part-1/

The disk drive was a single-sided 35 track device, and each of those
tracks contain 18 sectors. Each sector was 256 bytes. Each track of 18
256-byte sectors could hold 4608 bytes. This meant that a disk could
hold 161,280 bytes of data! (35 * 18 * 256)

Track 17 was used to store the directory and file allocation table (FAT).

Tracks are numbers from 0 to 34, so track 17 is actually the 18th
track. Sectors, however, are numbers 1 to 18. Drives were numbers 0 to 3.

Each of the available 34 tracks was split in half (each half being 9 256-byte sectors) and called granules. Each granule was, therefore, 9 * 256 bytes in size — 2304 bytes.

Of the 18 sectors contained in track 17, sectors 1 and 12-18 were “for future use.”

Sector 1 – Unused (“for future use”)
Sector 2 – File Allocation Table (FAT)
Sectors 3-11 – Directory Entries
Sectors 13-18 – Unused (“for future use”)

The first 68 bytes of Sector 2 contained the file allocation table. Each
byte represented the status of one of the available granules on the
disk. If the granule was not used by any file, the byte representing it
would be set to 255 (&HFF). I expect that the FREE() command simply read
Track 17, Sector 2, and quickly scanned the first 68 bytes, counting
how many were 255.

If the granule is used, but the file is larger and continues on to another
granule, the value will be the granule number for the next granule of the
file. It’s a linked list!  If the granule is used, but the file does
not continue to another granule, the top two bits will be set (11xxxxxx,
hex value &HC0 or decimal 192) and the remaining five bits will indicate
how many sectors of that granule are part of the file.

Track 17 sectors 3-11 are used for the directory:
0-7:  filename, blank padded.  first byte 0: deleted.  ff: never used.
8-10: extension, blank padded.
11:   File Type: 0=basic 1=basicData 2=machineLang 3=text
12:   0=binary ff=ascii
13:   first granule (0-67)
14-15 num bytes in use in the last sector
16-31 reserved

*/

/*  Does this empty image suggest heuristics to recognize RSDOS disks?

$ decb dskini /tmp/z1
$ hd /tmp/z1

00000000  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00013200  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00013300  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00013340  ff ff ff ff 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00013350  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00013400  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00027600

*/
