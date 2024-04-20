package finder

import (
	"flag"
	"log"
	"os"
	PFP "path/filepath"
	"time"

	"github.com/strickyak/frobio/frob3/lemma/comm"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

const RETAIN_10_DAYS = "RETAIN-10-DAYS"

const NumDrives = 10
const DirPerm = 0775
const FilePerm = 0664

var FlagNavRoot = flag.String("nav_root", "", "top of navigable tree on native filesystem")

type DriveImage struct {
	Path              string
	Image             []byte
	Dirty             bool
	CreationTimestamp string
}

func (di *DriveImage) String() string {
	return Format("{DriveImage p=%q len=%d dirt=%v ts=%q}",
		di.Path, len(di.Image), di.Dirty, di.CreationTimestamp)
}
func (di *DriveImage) GoString() string {
	return Format("{gs:DriveImage p=%q len=%d dirt=%v ts=%q}",
		di.Path, len(di.Image), di.Dirty, di.CreationTimestamp)
}

type DriveSession struct {
	com    *comm.Comm
	drives [NumDrives]*DriveImage
	home   string
	retain string
}

func (di *DriveSession) String() string {
	return Format("{DriveSession: %v home=%q}", di.drives, di.home)
}

func NewDriveSession(com *comm.Comm, home string, retain string) *DriveSession {
	return &DriveSession{
		com:    com,
		home:   home,
		retain: retain,
	}
}

func (ds *DriveSession) CreateDriveIfNeeded(nth byte) *DriveImage {
	Log("CDIN: nth=%d", nth)
	if ds.drives[nth] == nil {
		ds.drives[nth] = &DriveImage{}
		log.Printf("@@@@@@@@@@ CDIN: created DriveImage{} number %d.", nth)
	}

	z := ds.drives[nth]
	Log("CDIN: return: path %q size %d", z.Path, len(z.Image))
	return z
}

func (ds *DriveSession) SetDrive(nth byte, c Chooser) {
	log.Printf("SetDrive: %d %v", nth, c)

	// Create the drive record, if needed.  Call it d.
	if ds.drives[nth] == nil {
		ds.drives[nth] = &DriveImage{}
		log.Printf("@@@@@@@@@@ SetDrive: created DriveImage{} number %d.", nth)
	}
	d := ds.drives[nth]

	ds.HdbDosCleanupOneDrive(nth)

	uc, ok := c.(*UnixChooser)
	Log("@@@@@@@@@@@@@@ uc, ok c: %v %v %v", uc, ok, c)
	if ok && !uc.isDir {
		Log("@@@@@@@@@@@@@@ uc.size, FloppySizes: %v %v", uc.size, FloppySizes)
		if In(uc.size, FloppySizes) {
			Log("@@@@@@@@@@@@@@ IN")
			d.Path = c.Path()
			d.Image = nil
			d.Dirty = false
		}
	}
	Log("@@@@@@@@@@@@@@ SetDrive final: %#v", d)
}

func (ds *DriveSession) HdbDosCleanupOneDrive(driveNum byte) {
	drive := ds.drives[driveNum]
	if drive == nil || !drive.Dirty {
		return
	}
	// if dirty:
	dest := PFP.Join(*FlagNavRoot, ds.retain, drive.CreationTimestamp+"-"+PFP.Base(drive.Path))
	err := os.WriteFile(dest, drive.Image, FilePerm)
	if err != nil {
		log.Printf("BAD: HdbDosCleanup: Error saving dirty file %d bytes %q as %q: %v", len(drive.Image), drive.Path, dest, err)
	} else {
		log.Printf("HdbDosCleanup: Saved dirty file %d bytes %q as %q", len(drive.Image), drive.Path, dest)
	}
	ds.drives[driveNum] = nil // Delete the drive record.
}
func Timestamp() string {
	return time.Now().UTC().Format("2006-01-02-150405Z")
}
func (ds *DriveSession) HdbDosCleanup() {
	for driveNum, _ := range ds.drives {
		ds.HdbDosCleanupOneDrive(byte(driveNum))
	}
}
