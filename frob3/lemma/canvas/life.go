package canvas

import (
	"fmt"
	"log"
	"math/rand"
	"net"
	"time"

	lem "github.com/strickyak/frobio/frob3/lemma/lib"
)

const StirFreq = 100

// Install demos in Lemma.
func init() {
	if lem.Demos == nil {
		lem.Demos = make(map[string]func(net.Conn))
	}
	lem.Demos["life"] = func(conn net.Conn) { RunLife(conn, false, "") }
	lem.Demos["life2"] = func(conn net.Conn) { RunLife(conn, false, "2") }
	lem.Demos["test"] = func(conn net.Conn) { RunLife(conn, true, "") }
	lem.Demos["test2"] = func(conn net.Conn) { RunLife(conn, true, "2") }
}

func RunLife(conn net.Conn, verify bool, mode string) {
	G3CMode(conn)
	SetVdgScreenAddr(conn, ScreenLoc)
	life := &Life{
		curr:  &Canvas{},
		Start: time.Now(),
	}

	// TODO: gc goroutines
	generator := make(chan *Canvas, 3)
	go func() {
		game := &Canvas{}
		g := 0
		for {
			if g%StirFreq == 0 {
				if mode == "2" {
					LifeStir(game)
				} else {
					LifeSetR(game)
				}
			}

			game = LifeNext(game)
			generator <- game
			g++
		}
	}()

	for {
		life.curr = <-generator
		life.Generation++

		life.curr.Render(conn)
		if verify {
			VerifyAndCountErrors(life, conn)
		}

		duration := time.Since(life.Start)
		mtbf := "Inf"
		if life.Errors > 0 {
			mtbf = fmt.Sprintf("%0.2f", duration/time.Duration(life.Errors))
		}
		fps := float64(life.Generation) / duration.Seconds()

		s := fmt.Sprintf("Generation: %d Duration: %v  FPS: %.2f",
			life.Generation, duration, fps)
		if verify {
			s += fmt.Sprintf("  Errors: %d  MTBF: %v", life.Errors, mtbf)
		}
		log.Print(s)
	}
}

func VerifyAndCountErrors(life *Life, conn net.Conn) {
	func() {
		r := recover()
		if r != nil {
			log.Printf("Recovered: %v", r)
			life.Errors++
			log.Printf("Total Errors: %d", life.Errors)
		}
	}()

	life.curr.Verify(conn)
}

type Life struct {
	curr, next *Canvas
	Errors     uint
	Start      time.Time
	Generation uint
}

func LifeNext(curr *Canvas) *Canvas {
	next := &Canvas{}

	for x := 0; x < W; x++ {
		for y := 0; y < H; y++ {
			old := curr.Get(x, y)

			cy, or := 0, 0
			for i := -1; i < 2; i++ {
				for j := -1; j < 2; j++ {
					switch curr.Get(x+i, y+j) {
					case Magenta:
						cy++
					case Orange:
						or++
					}
				}
			}
			n := cy + or

			if old > 0 && (n == 2+1 || n == 3+1) {
				next.Set(x, y, old)
			} else if n == 3 {
				if cy > or {
					next.Set(x, y, Magenta)
				} else {
					next.Set(x, y, Orange)
				}
			}

		}
	}

	return next
}

var nextOrange = false

func LifeSetR(curr *Canvas) {
	x := rand.Intn(10) + 40
	y := rand.Intn(10) + 40
	var color Color = Magenta
	if nextOrange {
		color = Orange
	}
	curr.Set(x+5, y+5, color)
	curr.Set(x+6, y+5, color)
	curr.Set(x+4, y+6, color)
	curr.Set(x+5, y+6, color)
	curr.Set(x+5, y+7, color)
	nextOrange = !nextOrange
}

func LifeStir(curr *Canvas) {
	const Thick = 12 / 2 // thickness of stir
	const Mid = W / 2    // midpoint of Width
	for x := Mid - Thick; x < Mid+Thick; x++ {
		for y := 0; y < H; y++ {
			r := rand.Intn(6)
			switch r {
			case 1:
				curr.Set(x, y, Magenta)
			case 3:
				curr.Set(x, y, Orange)
			}
		}
	}
}
