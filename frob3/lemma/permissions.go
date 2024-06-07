package lemma

import (
	P "path"
	"strings"
)

func CanAccess(principals []string, path string, toWrite bool) bool {
	for _, prin := range principals {
		if CanAccess1(prin, path, toWrite) {
			return true
		}
	}
	return false
}

func CanAccess1(prin string, path string, toWrite bool) bool {
	prin = strings.ToUpper(prin)
	path = strings.ToUpper(P.Clean(path))

	words := strings.Split(path, "/")
	if len(words) > 0 && words[0] == "" {
		words = words[1:]
	}

	// some quick ones first:

	if words[0] == "HOMES" && words[1] == prin {
		return true // Can read or write under their home.
	}

	if words[0] == "PUBLIC" && !toWrite {
		return true // Anyone can read under /public
	}

	if words[0] == "RESTRICTED" && prin == "" {
		return false // Anonymous cannot read or write /restricted
	}

	var public, private bool
	for _, w := range words {
		switch w {
		case "PUBLIC":
			public = true
		case "PRIVATE":
			private = true
		}
	}

	if public && !private && !toWrite {
		return true // public readable
	}

	if prin != "" && !private && !toWrite {
		return true // not private, so readable by all except anon.
	}

	return false
}
