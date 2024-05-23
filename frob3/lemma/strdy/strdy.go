/*
	strdy (pronounced "sturdy") is a database
	for holding a string-to-string map.
	Keys and Values should not contain ASCII
	control characters, and particularly not
	tabs nor newlines.

	It is not designed for large amounts of data
	that would not fit nicely in memory.

	The in-memory data structure is a skiplist
	in lexical order of the keys.

	The database is backed by a flat textfile,
	which is appended as records are inserted or updated.
*/
package strdy

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"os"
	"strings"
	"time"

	"github.com/strickyak/frobio/frob3/lemma/skiplist"
	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type DB struct {
	List     *skiplist.SkipList
	Filename string
	W        io.Writer
}

type Record struct {
	Key       string
	Timestamp string
	Value     string
}

func Load(filename string) *DB {
	db := &DB{
		List:     skiplist.New(skiplist.String),
		Filename: filename,
		W:        Value(os.OpenFile(filename, os.O_CREATE|os.O_RDWR|os.O_APPEND|os.O_SYNC, 0640)),
	}
	db.slurpFile(filename)
	return db
}

func (db *DB) slurpFile(filename string) {
	file := Value(os.Open(filename))
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		db.addLine(scanner.Text())
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}
}

// addLines expects syntax, but tabs instead of spaces:
// ( key timestamp value ) ;comment...
// 0 1   2         3     4 5
func (db *DB) addLine(line string) {
	if !strings.HasPrefix(line, "(") {
		return // Not a valid line.
	}
	words := strings.SplitN(line, "\t", 6)
	if len(words) != 6 {
		return // Not a valid line.
	}
	if words[0] != "(" || words[4] != ")" {
		return // Not a valid line.
	}
	key, timestamp, value := words[1], words[2], words[3]

	val, ok := db.List.GetValue(key)
	if ok {
		old := val.(*Record)
		if old.Timestamp > timestamp {
			return // already obsoleted.
		}
	}

	db.List.Set(key, &Record{
		Key:       key,
		Timestamp: timestamp,
		Value:     value,
	})
}

// Get returns "" if key not found.
func (db *DB) Get(key string) string {
	val, ok := db.List.GetValue(key)
	if !ok {
		return ""
	}
	return val.(*Record).Value
}

func (db *DB) Set(key, value, comment string) {
	db.List.Set(key, value)
	ts := Timestamp()
	fmt.Fprintf(db.W, "(\t%s\t%s\t%s\t)\t;%s\n", key, ts, value, comment)
}


const TimeFormat = "2006-0102-150405"
func Timestamp() string {
	return time.Now().UTC().Format(TimeFormat)
}

/*
	import PFP "path/filepath"
	func LoadGlob(globPattern string) *DB {
		db := &DB{
			List: skiplist.New(skiplist.Int),
		}
		for _, filename := range Value(PFP.Glob(globPattern)) {
			db.slurpFile(filename)
		}
		return db
	}
*/
