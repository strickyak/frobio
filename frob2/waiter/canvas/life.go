package canvas

import (
	"math/rand"
	"net"

	lem "github.com/strickyak/frobio/frob2/waiter"
)

func init() {
	if lem.Demos == nil {
		lem.Demos = make(map[string]func(net.Conn))
	}
	lem.Demos["life"] = RunLife
}

func RunLife(conn net.Conn) {
	life := &Life{
		curr: &Canvas{},
	}
	gen := 0
	for {
		if gen%100 == 0 {
			life.Stir()
		}
		life.Next()
		life.curr.Render(conn)
	}
}

type Life struct {
	curr, next Canvas
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
					case Cyan:
						cy++
					case Orange:
						or++
					}
				}
			}
			n := cy + or

			if old > 0 && (n == 2 || n == 3) {
				b.Set(x, y, old)
			} else if n == 3 {
				if cy > or {
					b.Set(x, y, Cyan)
				} else {
					b.Set(x, y, Orange)
				}
			}

		}
	}

	life, curr, life.next = life.next, nil
}

func (life *Life) Stir() {
	const Mid = W / 2
	for x := Mid - 8; x < Mid+8; x++ {
		for y := 0; y < H; y++ {
			r := rand.Intn(6)
			switch r {
			case 1:
				life.curr.Set(x, y, Cyan)
			case 3:
				life.curr.Set(x, y, Orange)
			}
		}
	}
}
