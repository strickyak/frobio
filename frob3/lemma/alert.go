package lemma

import (
	"time"
)

type Alert struct {
	Summary string
	Text    string
	ID      int
	Created time.Time
	Expires time.Time
}
