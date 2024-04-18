package quickdb

import (
	"bufio"
	"log"
	"os"
	PFP "path/filepath"
	"strings"

	. "github.com/strickyak/frobio/frob3/lemma/util"
)

type QuickDB struct {
	Recs map[string]*QuickRec
}

type QuickRec struct {
	Timestamp string
	Key       string
	Value     string
}

func New(globPattern string) {
	q := &QuickDB{
		Recs: make(map[string]*QuickRec),
	}
	// PFP.Glob(globPattern string) (matches []string, err error)
	for _, filename := range Value(PFP.Glob(globPattern)) {
		q.SlurpFile(filename)
	}
	return q
}

func (q *QuickDB) SlurpFile(filename string) {
	file := Value(os.Open(filename))
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		q.AddLine(scanner.Text())
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}
}
