package canvas

import (
	"fmt"
	"log"
	"math/rand"
	"net"
	"time"

	lem "github.com/strickyak/frobio/frob2/waiter"
)

const StirFreq = 256

func init() {
	if lem.Demos == nil {
		lem.Demos = make(map[string]func(net.Conn))
	}
	lem.Demos["life"] = RunLife
	lem.Demos["test"] = RunTest
}

func RunLife(conn net.Conn) {
	RunLifeCommon(conn, false)
}
func RunTest(conn net.Conn) {
	RunLifeCommon(conn, true)
}

func RunLifeCommon(conn net.Conn, verify bool) {
	G3CMode(conn)
	SetVdgScreenAddr(conn, ScreenLoc)
	life := &Life{
		curr:  &Canvas{},
		Start: time.Now(),
	}

	for {
		if life.Generation%StirFreq == 0 {
			life.Stir()
		}
		life.Generation++

		life.Next()
		life.curr.Render(conn)
		if verify {
			VerifyAndCountErrors(life, conn)
		}

		duration := time.Since(life.Start)
		mtbf := duration / time.Duration(life.Errors+1)
		fps := float64(life.Generation) / duration.Seconds()

		s := fmt.Sprintf("Generation: %d Duration: %v  FPS: %.2f",
			life.Generation, duration, fps)
		if verify {
			s += fmt.Sprintf("  Errors: %d MTBF: %v", life.Errors, mtbf)
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

func (life *Life) Next() {
	life.next = &Canvas{}

	for x := 0; x < W; x++ {
		for y := 0; y < H; y++ {
			old := life.curr.Get(x, y)

			cy, or := 0, 0
			for i := -1; i < 2; i++ {
				for j := -1; j < 2; j++ {
					switch life.curr.Get(x+i, y+j) {
					case Magenta:
						cy++
					case Orange:
						or++
					}
				}
			}
			n := cy + or

			if old > 0 && (n == 2+1 || n == 3+1) {
				life.next.Set(x, y, old)
			} else if n == 3 {
				if cy > or {
					life.next.Set(x, y, Magenta)
				} else {
					life.next.Set(x, y, Orange)
				}
			}

		}
	}

	life.curr, life.next = life.next, nil
}

func (life *Life) Stir() {
	const Thick = 10 / 2 // thickness of stir
	const Mid = W / 2    // midpoint of Width
	for x := Mid - Thick; x < Mid+Thick; x++ {
		for y := 0; y < H; y++ {
			r := rand.Intn(6)
			switch r {
			case 1:
				life.curr.Set(x, y, Magenta)
			case 3:
				life.curr.Set(x, y, Orange)
			}
		}
	}
}
