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
	lem.Demos["life"] = func(conn net.Conn) { RunLife(conn, false, 128) }
	//lem.Demos["life2"] = func(conn net.Conn) { RunLife(conn, false, 128) }
	lem.Demos["test"] = func(conn net.Conn) { RunLife(conn, true, 96) }
	//lem.Demos["test2"] = func(conn net.Conn) { RunLife(conn, true, 96) }
}

func RunLife(conn net.Conn, verify bool, width int) {
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
				life.LifeSetR(game, width)
			}

			game = LifeNext(life, game, width)
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
	nextOrange bool
}

func Impress(c *Canvas, life *Life, nMagenta, nOrange int) {
	hms := time.Now().UTC().Format("15:04:05")
	Print(Cyan, c, 0, 0, hms[0]-'0')
	Print(Cyan, c, 1, 0, hms[1]-'0')
	Print(Cyan, c, 2, 0, hms[3]-'0')
	Print(Cyan, c, 3, 0, hms[4]-'0')
	Print(Cyan, c, 4, 0, hms[6]-'0')
	Print(Cyan, c, 5, 0, hms[7]-'0')

	duration := time.Since(life.Start)
	PrintInt(Cyan, c, 1, uint(duration.Seconds()))

	PrintInt(Cyan, c, 3, life.Generation/1000000)
	PrintInt(Cyan, c, 4, life.Generation%1000000)

	fps := float64(life.Generation) / duration.Seconds()
	// u, m := uint(fps), uint(fps * 1000) % 1000

	PrintInt(Cyan, c, 6, uint(fps*1000))

	PrintInt(Magenta, c, 8, uint(nMagenta))
	PrintInt(Orange, c, 9, uint(nOrange))
}

func LifeNext(life *Life, curr *Canvas, width int) *Canvas {
	next := &Canvas{}
	nMagenta, nOrange := 0, 0

	for x := 0; x < width; x++ {
		for y := 0; y < H; y++ {
			old := curr.Get(x, y, width)

			cy, or := 0, 0
			for i := -1; i < 2; i++ {
				for j := -1; j < 2; j++ {
					switch curr.Get(x+i, y+j, width) {
					case Magenta:
						cy++
					case Orange:
						or++
					}
				}
			}
			n := cy + or

			if old > 0 && (n == 2+1 || n == 3+1) {
				next.Set(x, y, old, width)
				if old == Magenta {
					nMagenta++
				} else {
					nOrange++
				}
			} else if n == 3 {
				if cy > or {
					next.Set(x, y, Magenta, width)
					nMagenta++
				} else {
					next.Set(x, y, Orange, width)
					nOrange++
				}
			}

		}
	}

	Impress(next, life, nMagenta, nOrange)

	return next
}

func (life *Life) LifeSetR(curr *Canvas, width int) {
	x := rand.Intn(10) + 40
	y := rand.Intn(10) + 40
	var color Color = Magenta
	if life.nextOrange {
		color = Orange
	}
	curr.Set(x+5, y+5, color, width)
	curr.Set(x+6, y+5, color, width)
	curr.Set(x+4, y+6, color, width)
	curr.Set(x+5, y+6, color, width)
	curr.Set(x+5, y+7, color, width)
	life.nextOrange = !life.nextOrange
}

func LifeStir(curr *Canvas, width int) {
	const Thick = 12 / 2 // thickness of stir
	mid := width / 2     // midpoint of Width
	for x := mid - Thick; x < mid+Thick; x++ {
		for y := 0; y < H; y++ {
			r := rand.Intn(6)
			switch r {
			case 1:
				curr.Set(x, y, Magenta, width)
			case 3:
				curr.Set(x, y, Orange, width)
			}
		}
	}
}
