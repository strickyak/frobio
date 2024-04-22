package main

import (
	"bytes"
	"fmt"
)

func main() {
	GenerateShips()
}

var East = []int{
	-2, 1,
	-1, 1,
	0, 1,
	1, 0,
	-2, -1,
	-1, -1,
	0, -1,
}

var EastNorthEast = []int{
	-2, 1,
	-1, 1,
	0, 1,
	1, 1,
	0, 0,
	-1, -1,
	-2, -2,
}

var NorthEast = []int{
	-2, -1,
	-1, 0,
	0, 1,
	1, 1,
	1, 0,
	0, -1,
	-1, -2,
}

var NorthNorthEast = []int{
	1, -2,
	1, -1,
	1, 0,
	1, 1,
	0, 0,
	-1, -1,
	-2, -2,
}

var Ships = [4][]int{East, EastNorthEast, NorthEast, NorthNorthEast}

// 2D Rotation Matrix
//
// [ cos A  -sin A ]  x
// [ sin A   cos A ]  y

var Degree0 = [4]int{+1, 0, 0, +1}
var Degree90 = [4]int{0, -1, +1, 0}
var Degree180 = [4]int{-1, 0, 0, -1}
var Degree270 = [4]int{0, +1, -1, 0}
var Rots = [4][4]int{Degree0, Degree90, Degree180, Degree270}

func RenderMono(ship []int, rot [4]int) (z [5][5]bool) {
	for i := 0; i < len(ship); i += 2 {
		x, y := ship[i], ship[i+1]
		x, y = Mult(x, y, rot)
		z[x+2][y+2] = true
	}
	return
}

func Mult(x, y int, rot [4]int) (x2, y2 int) {
	x2 = rot[0]*x + rot[1]*y
	y2 = rot[2]*x + rot[3]*y
	return
}

var Directions [16][5][5]bool

func SetBitsInPair(pair [2]byte, color, pos int) [2]byte {
	i := uint(pos) >> 2
	c := byte(color) << (2 * (3 - (pos & 3)))
	pair[i] |= c
	return pair
}

func Binary(x byte) string {
	var bb bytes.Buffer
	for bit := byte(0x80); bit != 0; bit >>= 1 {
		if (x & bit) == 0 {
			bb.WriteByte('-')
		} else {
			bb.WriteByte('@')
		}
	}
	return bb.String()
}

func GenerateRow(dir, offset int) {
	const color = 3 // Use only color 3 in table, and mask for color at runtime.
	fmt.Printf("// ------ color=%d dir=%d offset=%d ---------------------\n", color, dir, offset)
	d := Directions[dir]
	p := 0 + offset
	for y := 4; y >= 0; y-- {
		var pair [2]byte
		for x := 0; x < 5; x++ {
			if d[x][y] {
				pair = SetBitsInPair(pair, color, p+x)
			}
		}
		fmt.Printf("    0x%02x, 0x%02x,  //  %s %s\n",
			pair[0],
			pair[1],
			Binary(pair[0]),
			Binary(pair[1]),
		)
	}
}

func GenerateShips() {
	for rot := 0; rot < 4; rot++ {
		for ship := 0; ship < 4; ship++ {
			fmt.Printf("// // rot %d ship %d\n", rot, ship)
			bits := RenderMono(Ships[ship], Rots[rot])
			Directions[ship+4*rot] = bits
			ShowBits(bits)
		}
	}
	fmt.Printf("// %v", Directions)

	fmt.Printf("// -----------------------------------------\n")

	for dir := 0; dir < 16; dir++ {
		for offset := 0; offset < 4; offset++ {
			GenerateRow(dir, offset)
		}
	}

}

func ShowBits(bits [5][5]bool) {
	for y := 2; y >= -2; y-- {
		fmt.Printf("//                 ")
		for x := -2; x <= 2; x++ {
			c := '.'
			if bits[x+2][y+2] {
				c = '#'
			}
			fmt.Printf("%c", c)
		}
		fmt.Printf("\n")
	}
}
