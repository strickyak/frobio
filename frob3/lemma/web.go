package lemma

import (
	"bytes"
	"crypto/sha256"
	"crypto/subtle"
	"encoding/base64"
	"flag"
	"fmt"
	"log"
	"net/http"
	P "path"
	"regexp"
	"runtime/debug"
	"strings"
	"time"

	. "github.com/strickyak/frobio/frob3/lemma/util"
)

var FlagWebStatic = flag.String("web_static", "", "web-static serving directory")

const (
	BadRequest = 400
	ImATeapot  = 418
)

type SlashHandler struct {
}

func (sh *SlashHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	headers := w.Header()
	headers.Add("Content-Type", "text/html")

	fmt.Fprintf(w, HomePageHtml, r.Host)
}

type LemmaHandler struct {
	NavRoot  string
	Handlers map[string]http.Handler
}

var MatchPathFront = regexp.MustCompile("^(/[-a-z]*)")

func (lh *LemmaHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	defer func() {
		caught := recover()
		if caught != nil {
			log.Printf("\n")
			log.Printf("ServeHTTP: (%q) CAUGHT: %v", r.URL.Path, caught)
			debug.PrintStack()
			log.Printf("\n")
		}
	}()

	p := P.Clean(r.URL.Path)
	log.Printf("ServeHTTP %q (%q)", r.URL.Path, p)

	headers := w.Header()
	headers.Add("Server", "Lemma")

	if p == "/favicon.ico" {
		p = "/web-static/favicon.ico"
		r.URL.Path = p
	}

	m := MatchPathFront.FindStringSubmatch(p)
	if m == nil || len(m) != 2 {
		w.WriteHeader(ImATeapot)
		fmt.Fprintf(w, "Bad Path: %q\n", p)
		return
	}

	front := m[1] // The front word; may be empty.
	handler, ok := lh.Handlers[front]
	if !ok {
		w.WriteHeader(ImATeapot)
		fmt.Fprintf(w, "No Handler: %q :: %q\n", front, p)
		return
	}

	handler.ServeHTTP(w, r)
	// fmt.Fprintf(w, "\n<P><BR>(* %q :: %q *)\n", r.URL.Path, p)
}

func RunWeb() {
	println("FlagNavRoot", *FlagNavRoot)
	Assert(*FlagNavRoot != "")   // A root must be specified, so we don't serve "." unintentionally.
	Assert(*FlagWebStatic != "") // A root must be specified, so we don't serve "." unintentionally.

	lh := &LemmaHandler{
		NavRoot: *FlagNavRoot,
		Handlers: map[string]http.Handler{
			"/":           &SlashHandler{},
			"/pizga":      basicAuth(http.StripPrefix("/pizga", http.FileServer(http.Dir(*FlagNavRoot))).(http.HandlerFunc)),
			"/web-static": http.StripPrefix("/web-static", http.FileServer(http.Dir(*FlagWebStatic))).(http.HandlerFunc),
		},
	}

	s := &http.Server{
		Addr:           ":8080",
		Handler:        lh,
		ReadTimeout:    10 * time.Second,
		WriteTimeout:   10 * time.Second,
		MaxHeaderBytes: 1 << 20,
	}
	log.Fatal(s.ListenAndServe())
}

////////////////////////////////////////////////////////////////////
//
//  Thanks to https://github.com/go-kit/kit/blob/v0.13.0/auth/basic/middleware.go
//  MIT License.

// parseBasicAuth parses an HTTP Basic Authentication string.
// "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==" returns ([]byte("Aladdin"), []byte("open sesame"), true).
func parseBasicAuth(auth string) (username, password []byte, ok bool) {
	const prefix = "Basic "
	if !strings.HasPrefix(auth, prefix) {
		return
	}
	c, err := base64.StdEncoding.DecodeString(auth[len(prefix):])
	if err != nil {
		return
	}

	s := bytes.IndexByte(c, ':')
	if s < 0 {
		return
	}
	return c[:s], c[s+1:], true
}

// Returns a hash of a given slice.
func toHashSlice(s []byte) []byte {
	hash := sha256.Sum256(s)
	return hash[:]
}

//
////////////////////////////////////////////////////////////////////
//
//  Thanks https://www.alexedwards.net/blog/basic-authentication-in-go
//
//    © Copyright 2013-2024 Alex Edwards
//    Code snippets are MIT licensed

var PublicExceptions = map[string]bool{
	"/":      true,
	"/pizga": true,
	"/pizga/Homes": true,
}

func basicAuth(next http.HandlerFunc) http.HandlerFunc {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		p := P.Clean(r.URL.Path)
		words := strings.Split(p, "/")

		public, private := false, false
		for _, word := range words {
			if strings.ToUpper(word) == "PUBLIC" {
				public = true
			}
			if strings.ToUpper(word) == "PRIVATE" {
				private = true
			}
		}
		if _, ok := PublicExceptions[p]; ok {
			public = true
		}

		if public && !private {
			next.ServeHTTP(w, r)
			return
		}

		// Extract the username and password from the request
		// Authorization header. If no Authentication header is present
		// or the header value is invalid, then the 'ok' return value
		// will be false.
		username, password, ok := r.BasicAuth()
		username = strings.ToUpper(strings.TrimSpace(username))
		password = strings.ToUpper(strings.TrimSpace(password))
		if ok {
			// Calculate SHA-256 hashes for the provided and expected
			// usernames and passwords.
			usernameHash := sha256.Sum256([]byte(username))
			passwordHash := sha256.Sum256([]byte(password))
			expectedUsernameHash := sha256.Sum256([]byte("PIZGA"))
			expectedPasswordHash := sha256.Sum256([]byte("COCO"))

			// Use the subtle.ConstantTimeCompare() function to check if
			// the provided username and password hashes equal the
			// expected username and password hashes. ConstantTimeCompare
			// will return 1 if the values are equal, or 0 otherwise.
			// Importantly, we should to do the work to evaluate both the
			// username and password before checking the return values to
			// avoid leaking information.
			usernameMatch := (subtle.ConstantTimeCompare(usernameHash[:], expectedUsernameHash[:]) == 1)
			passwordMatch := (subtle.ConstantTimeCompare(passwordHash[:], expectedPasswordHash[:]) == 1)

			// If the username and password are correct, then call
			// the next handler in the chain. Make sure to return
			// afterwards, so that none of the code below is run.
			if usernameMatch && passwordMatch {
				next.ServeHTTP(w, r)
				return
			}
		}

		// If the Authentication header is not present, is invalid, or the
		// username or password is wrong, then set a WWW-Authenticate
		// header to inform the client that we expect them to use basic
		// authentication and send a 401 Unauthorized response.
		w.Header().Set("WWW-Authenticate", `Basic realm="Pizga", charset="UTF-8"`)
		http.Error(w, "Unauthorized", http.StatusUnauthorized)
		log.Printf("Unauthorized: %q", p)
	})
}

//
////////////////////////////////////////////////////////////////////

const HomePageHtml = `<html>

<head>
<style>
body {
  font-family: courier, serif;
  font-size: 42px;
  color: #00BB00;
  background-color: #002200;
}
.credit {
  font-size: 12px;
}
</style>
</head>

<body>

Welcome to Pizga, your Home in the Clouds
<br>for <a href="https://www.google.com/search?q=Radio+Shack+(Tandy)+Color+Computers">Radio Shack (Tandy) Color Computers</a>
<br>with the <a href="https://computerconect.com/products/cocoio-prom?variant=42556416196788">CocoIOr cartridge</a>.
<p>

My name is %q and I'll be your waiter today.
<p>

Visit the <a href="/pizga/">Pizga Repository</a>.
<p>
<a href="/web-static/mt_pisgah_pisgah_view_ranch_by_brotherwayword_d1cpxsy.jpg"><img src="/web-static/mt_pisgah_pisgah_view_ranch_by_brotherwayword_d1cpxsy-w1200.jpg" xxxxWIDTH="100%"></a>
<br>

<div class=credit>Thanks to
<a href="https://www.deviantart.com/brotherwayword/art/Mt-Pisgah-Pisgah-View-Ranch-81831778">brotherwayword</a>
for this painting of "Mt. Pisgah -- Pisgah View Ranch"
(<a href="https://creativecommons.org/licenses/by-nc-nd/3.0/">License</a>).
</div>
<p>

</body>

</html>
`
