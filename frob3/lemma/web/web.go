package web

import ()

type LemmaWeb struct {
	DataDir    string // to be served
	SecretsDir string // for basic auth
}

func NewLemmaWeb(bind string) {
	lw := &LemmaWeb{}
}
